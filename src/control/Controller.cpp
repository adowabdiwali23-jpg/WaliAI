#include "control/Controller.h"
#include "intelligence/CognitionEngine.h"
#include "service/MemoryService.h"
#include "service/HistoryService.h"
#include "service/HiddenBrowser.h"
#include "service/Logger.h"
#include "execution/SandboxManager.h"
#include "voice/VoiceService.h"
#include "control/StateManager.h"
#include "state/CognitiveState.h"
#include "state/PersonalizationState.h"
#include "state/AgentState.h"
#include "infrastructure/DatabaseManager.h"

#include <QMutexLocker>

Controller::Controller(CognitionEngine &engine, MemoryService &memory,
                       HistoryService &history, HiddenBrowser &browser,
                       SandboxManager &sandbox, VoiceService &voice,
                       StateManager &state, DatabaseManager &db,
                       QObject *parent)
    : QObject(parent), m_engine(engine), m_memory(memory), m_history(history),
      m_browser(browser), m_sandbox(sandbox), m_voice(voice), m_state(state), m_db(db)
{
    connect(&m_engine, &CognitionEngine::responseChunk, this, &Controller::responseChunk);
    connect(&m_engine, &CognitionEngine::responseFinished, this, [this](const QString &response) {
        // Store assistant response in history
        int sessionId = m_history.currentSessionId();
        if (sessionId > 0) {
            m_history.addMessage(sessionId, "assistant", response);
        }

        // Store memory summary if enabled
        if (m_state.personalizationState().memoryEnabled()) {
            if (response.length() > 50) {
                m_memory.store("chat_summary", response.left(200));
            }
        }

        // Speak response if TTS enabled
        if (m_state.personalizationState().ttsEnabled()) {
            m_voice.speak(response);
        }

        m_processing = false;
        emit processingChanged(false);
        emit responseFinished(response);

        processNextRequest();
    });
}

void Controller::handleUserInput(const QString &message)
{
    if (message.trimmed().isEmpty()) return;

    // Enforce 4000-char limit
    QString trimmed = message.left(4000);

    // Store user message in history
    int sessionId = m_history.currentSessionId();
    if (sessionId <= 0) {
        sessionId = m_history.createSession("New Chat");
        m_history.setCurrentSession(sessionId);
    }

    auto &cs = m_state.cognitiveState();
    m_history.addMessage(sessionId, "user", trimmed);

    enqueueRequest(trimmed);
}

void Controller::handleVoiceInput(const QString &transcription)
{
    handleUserInput(transcription);
}

void Controller::enqueueRequest(const QString &message)
{
    QMutexLocker locker(&m_queueMutex);
    m_requestQueue.enqueue(message);
    locker.unlock();

    if (!m_processing) {
        processNextRequest();
    }
}

void Controller::processNextRequest()
{
    QMutexLocker locker(&m_queueMutex);
    if (m_requestQueue.isEmpty()) return;
    QString message = m_requestQueue.dequeue();
    locker.unlock();

    m_processing = true;
    emit processingChanged(true);

    m_engine.processRequest(message,
                            m_state.cognitiveState(),
                            m_state.personalizationState(),
                            m_state.agentState(),
                            m_history,
                            m_memory,
                            m_browser,
                            m_sandbox);
}

void Controller::switchSession(int sessionId)
{
    m_history.setCurrentSession(sessionId);
    emit sessionSwitched(sessionId);
}

void Controller::createNewSession()
{
    int id = m_history.createSession("New Chat");
    m_history.setCurrentSession(id);
    emit sessionSwitched(id);
}

void Controller::deleteSession(int sessionId)
{
    m_history.deleteSession(sessionId);
}

bool Controller::isProcessing() const { return m_processing; }
