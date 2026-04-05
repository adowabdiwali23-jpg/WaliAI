#pragma once

#include <QObject>
#include <QString>
#include <QQueue>
#include <QMutex>

class CognitionEngine;
class MemoryService;
class HistoryService;
class HiddenBrowser;
class SandboxManager;
class VoiceService;
class StateManager;
class DatabaseManager;

class Controller : public QObject
{
    Q_OBJECT

public:
    explicit Controller(CognitionEngine &engine,
                        MemoryService &memory,
                        HistoryService &history,
                        HiddenBrowser &browser,
                        SandboxManager &sandbox,
                        VoiceService &voice,
                        StateManager &state,
                        DatabaseManager &db,
                        QObject *parent = nullptr);

    void handleUserInput(const QString &message);
    void handleVoiceInput(const QString &transcription);
    void switchSession(int sessionId);
    void createNewSession();
    void deleteSession(int sessionId);

    bool isProcessing() const;

signals:
    void responseChunk(const QString &token);
    void responseFinished(const QString &fullResponse);
    void processingChanged(bool processing);
    void errorOccurred(const QString &error);
    void sessionSwitched(int sessionId);

private:
    void processNextRequest();
    void enqueueRequest(const QString &message);

    CognitionEngine &m_engine;
    MemoryService &m_memory;
    HistoryService &m_history;
    HiddenBrowser &m_browser;
    SandboxManager &m_sandbox;
    VoiceService &m_voice;
    StateManager &m_state;
    DatabaseManager &m_db;

    QQueue<QString> m_requestQueue;
    QMutex m_queueMutex;
    bool m_processing = false;
};
