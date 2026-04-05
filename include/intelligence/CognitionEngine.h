#pragma once

#include <QObject>
#include <QString>

class ModelInference;
class MemoryService;
class HiddenBrowser;
class HistoryService;
class SandboxManager;
class CognitiveState;
class PersonalizationState;
class AgentState;
class IntentClassifier;

class CognitionEngine : public QObject
{
    Q_OBJECT

public:
    explicit CognitionEngine(ModelInference &model, MemoryService &memory,
                             HiddenBrowser &browser, QObject *parent = nullptr);

    void processRequest(const QString &userMessage,
                        const CognitiveState &cognitive,
                        const PersonalizationState &personalization,
                        const AgentState &agent,
                        HistoryService &history,
                        MemoryService &memory,
                        HiddenBrowser &browser,
                        SandboxManager &sandbox);

    void stopGeneration();

signals:
    void responseChunk(const QString &token);
    void responseFinished(const QString &fullResponse);

private:
    QString buildPrompt(const QString &userMessage,
                        const CognitiveState &cognitive,
                        const PersonalizationState &personalization,
                        const AgentState &agent,
                        const QString &conversationContext,
                        const QString &memoryContext,
                        const QString &webContext);

    ModelInference &m_model;
    MemoryService &m_memory;
    HiddenBrowser &m_browser;
    IntentClassifier *m_intentClassifier = nullptr;

    // Pending state for async web-context fetches
    QString m_pendingUserMessage;
    const CognitiveState *m_pendingCognitive = nullptr;
    const PersonalizationState *m_pendingPersonalization = nullptr;
    const AgentState *m_pendingAgent = nullptr;
    QString m_pendingConversationContext;
    QString m_pendingMemoryContext;
};
