#pragma once

#include <QObject>
#include <QString>

class FileSystemGuard;
class CommandExecutor;
class PermissionManager;
class DatabaseManager;

class SandboxManager : public QObject
{
    Q_OBJECT

public:
    explicit SandboxManager(const QString &runtimePath, QObject *parent = nullptr);

    FileSystemGuard &guard();
    CommandExecutor &executor();
    PermissionManager &permissions();

    QString runtimePath() const;
    QString workspacePath() const;
    QString projectsPath() const;
    QString tempPath() const;
    QString logsPath() const;

    void setDatabaseManager(DatabaseManager *db);

signals:
    void commandExecuted(const QString &command, int exitCode);

private:
    void ensureDirectories();

    QString m_runtimePath;
    FileSystemGuard *m_guard = nullptr;
    CommandExecutor *m_executor = nullptr;
    PermissionManager *m_permissions = nullptr;
    DatabaseManager *m_db = nullptr;
};
