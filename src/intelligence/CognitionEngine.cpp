#include "intelligence/CognitionEngine.h"
#include "intelligence/IntentClassifier.h"
#include "intelligence/ModelInference.h"
#include "service/MemoryService.h"
#include "service/HistoryService.h"
#include "service/HiddenBrowser.h"
#include "service/Logger.h"
#include "execution/SandboxManager.h"
#include "execution/CommandExecutor.h"
#include "execution/SystemInspector.h"
#include "state/CognitiveState.h"
#include "state/PersonalizationState.h"
#include "state/AgentState.h"

CognitionEngine::CognitionEngine(ModelInference &model, MemoryService &memory,
                                 HiddenBrowser &browser, QObject *parent)
    : QObject(parent), m_model(model), m_memory(memory), m_browser(browser)
{
    m_intentClassifier = new IntentClassifier(this);

    connect(&m_model, &ModelInference::tokenGenerated, this, &CognitionEngine::responseChunk);
    connect(&m_model, &ModelInference::generationFinished, this, &CognitionEngine::responseFinished);
}

void CognitionEngine::processRequest(const QString &userMessage,
                                      const CognitiveState &cognitive,
                                      const PersonalizationState &personalization,
                                      const AgentState &agent,
                                      HistoryService &history,
                                      MemoryService &memory,
                                      HiddenBrowser &browser,
                                      SandboxManager &sandbox)
{
    IntentClassifier::Intent intent = m_intentClassifier->classify(userMessage);
    Logger::instance().log("Intent classified: " + IntentClassifier::intentToString(intent));

    // Handle execution intent
    if (intent == IntentClassifier::Intent::Execute && agent.agentModeEnabled()) {
        QString command = userMessage;
        if (command.startsWith("run ") || command.startsWith("execute ") ||
            command.startsWith("$ ") || command.startsWith("shell ") ||
            command.startsWith("cmd ")) {
            int spaceIdx = command.indexOf(' ');
            command = command.mid(spaceIdx + 1).trimmed();
        }
        CommandResult result = sandbox.executor().execute(command);
        QString response = "Command: " + command + "\nExit code: " +
                          QString::number(result.exitCode) + "\nOutput:\n" + result.output;
        if (!result.error.isEmpty()) {
            response += "\nError: " + result.error;
        }
        emit responseChunk(response);
        emit responseFinished(response);
        return;
    }

    // Handle diagnostic intent
    if (intent == IntentClassifier::Intent::Diagnostic) {
        SystemInspector inspector;
        QString report = inspector.fullReport();
        emit responseChunk(report);
        emit responseFinished(report);
        return;
    }

    // Build context
    QString conversationContext = history.buildConversationContext(
        history.currentSessionId(), 20);
    QString memoryContext;
    if (personalization.memoryEnabled()) {
        memoryContext = memory.buildContextBlock(userMessage, 5);
    }

    // Web context: fetch via HiddenBrowser when Search or Research is enabled.
    // This is a local HTTP fetch — no external cloud API is used.
    QString webContext;
    if (cognitive.searchEnabled() || cognitive.researchEnabled()) {
        Logger::instance().log("Web context requested (search=" +
            QString::number(cognitive.searchEnabled()) + ", research=" +
            QString::number(cognitive.researchEnabled()) + ")");

        // Use a one-shot blocking fetch so the context is available for the prompt.
        // HiddenBrowser::searchWeb uses DuckDuckGo’s HTML endpoint locally.
        m_pendingUserMessage = userMessage;
        m_pendingCognitive = &cognitive;
        m_pendingPersonalization = &personalization;
        m_pendingAgent = &agent;
        m_pendingConversationContext = conversationContext;
        m_pendingMemoryContext = memoryContext;

        // Connect once for this request — use member pointers (already stored
        // above) so the lambda does not capture dangling stack references.
        QMetaObject::Connection conn;
        conn = connect(&browser, &HiddenBrowser::pageFetched, this,
            [this, conn](const WebResult &result) mutable {
                disconnect(conn);
                QString webCtx;
                if (result.success && !result.content.isEmpty()) {
                    webCtx = result.content.left(4000);
                    Logger::instance().log("Web context fetched: " +
                        QString::number(webCtx.length()) + " chars from " + result.url);
                } else {
                    Logger::instance().warning("Web fetch failed: " + result.error);
                }
                QString prompt = buildPrompt(m_pendingUserMessage, *m_pendingCognitive,
                                             *m_pendingPersonalization, *m_pendingAgent,
                                             m_pendingConversationContext, m_pendingMemoryContext, webCtx);
                m_model.generate(prompt, m_pendingPersonalization->maxTokens(),
                                 m_pendingPersonalization->temperature());
            });

        browser.searchWeb(userMessage);
        return;
    }

    QString prompt = buildPrompt(userMessage, cognitive, personalization, agent,
                                 conversationContext, memoryContext, webContext);

    m_model.generate(prompt, personalization.maxTokens(), personalization.temperature());
}

void CognitionEngine::stopGeneration()
{
    m_model.stop();
}

QString CognitionEngine::buildPrompt(const QString &userMessage,
                                      const CognitiveState &cognitive,
                                      const PersonalizationState &personalization,
                                      const AgentState &agent,
                                      const QString &conversationContext,
                                      const QString &memoryContext,
                                      const QString &webContext)
{
    QString systemPrompt = personalization.systemPrompt();

    // Add mode indicators
    QStringList modes;
    if (cognitive.searchEnabled()) modes << "Search";
    if (cognitive.researchEnabled()) modes << "Research";
    if (cognitive.deepThinkEnabled()) modes << "DeepThink";
    if (agent.agentModeEnabled()) modes << "Agent";

    if (!modes.isEmpty()) {
        systemPrompt += "\n[Active modes: " + modes.join(", ") + "]";
    }

    if (cognitive.deepThinkEnabled()) {
        systemPrompt += "\nYou are in DeepThink mode. Analyze thoroughly, consider multiple angles, "
                        "and provide detailed reasoning before giving your answer.";
    }

    // Build ChatML format for Qwen model
    QString prompt = "<|im_start|>system\n" + systemPrompt + "<|im_end|>\n";

    if (!memoryContext.isEmpty()) {
        prompt += "<|im_start|>system\n[Memory context]\n" + memoryContext + "<|im_end|>\n";
    }

    if (!webContext.isEmpty()) {
        prompt += "<|im_start|>system\n[Web research results]\n" + webContext + "<|im_end|>\n";
    }

    if (!conversationContext.isEmpty()) {
        prompt += conversationContext;
    }

    prompt += "<|im_start|>user\n" + userMessage + "<|im_end|>\n";
    prompt += "<|im_start|>assistant\n";

    return prompt;
}
