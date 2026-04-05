#pragma once

#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QVector>
#include <QVariantMap>

class PersonalizationState;

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(const QString &dbPath, QObject *parent = nullptr);
    ~DatabaseManager();

    bool initialize();
    void close();

    // Chat sessions
    int createSession(const QString &title);
    bool deleteSession(int sessionId);
    QVector<QVariantMap> listSessions();
    bool renameSession(int sessionId, const QString &newTitle);

    // Chat messages (with toggle columns)
    bool addMessage(int sessionId, const QString &role, const QString &content,
                    bool searchEnabled = false, bool researchEnabled = false,
                    bool deepThinkEnabled = false);
    QVector<QVariantMap> getMessages(int sessionId);
    bool truncateSession(int sessionId, int afterMessageId);

    // Memories
    bool addMemory(const QString &content);
    bool deleteMemory(int memoryId);
    QVector<QVariantMap> searchMemories(const QString &query);
    QVector<QVariantMap> getAllMemories();

    // Projects
    int createProject(const QString &title, const QString &description);
    bool deleteProject(int projectId);
    QVector<QVariantMap> listProjects();

    // Agent logs
    bool addAgentLog(const QString &action, const QString &command,
                     const QString &result, int exitCode);
    QVector<QVariantMap> getAgentLogs(int limit = 100);

    // Settings
    bool saveSetting(const QString &key, const QString &value);
    QString loadSetting(const QString &key, const QString &defaultValue = "");
    void loadSettings(PersonalizationState &state);
    void saveSettings(const PersonalizationState &state);

private:
    bool createTables();
    bool migrateSchema();

    QString m_dbPath;
    QSqlDatabase m_db;
};
