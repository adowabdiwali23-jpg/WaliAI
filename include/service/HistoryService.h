#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QVariantMap>

class DatabaseManager;

class HistoryService : public QObject
{
    Q_OBJECT

public:
    explicit HistoryService(DatabaseManager &db, QObject *parent = nullptr);

    int createSession(const QString &title = "New Chat");
    bool deleteSession(int sessionId);
    QVector<QVariantMap> listSessions();
    bool renameSession(int sessionId, const QString &newTitle);

    bool addMessage(int sessionId, const QString &role, const QString &content,
                    bool searchEnabled = false, bool researchEnabled = false,
                    bool deepThinkEnabled = false);
    QVector<QVariantMap> getMessages(int sessionId);
    bool truncateSession(int sessionId, int afterMessageId);

    int currentSessionId() const;
    void setCurrentSession(int sessionId);

    QString buildConversationContext(int sessionId, int maxMessages = 20);

signals:
    void sessionCreated(int sessionId);
    void sessionDeleted(int sessionId);
    void sessionRenamed(int sessionId, const QString &newTitle);
    void messageAdded(int sessionId, const QString &role, const QString &content);
    void currentSessionChanged(int sessionId);

private:
    DatabaseManager &m_db;
    int m_currentSessionId = -1;
};
