#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class FileSystemGuard;
class PermissionManager;

struct CommandResult {
    QString output;
    int exitCode = -1;
    bool success = false;
    QString error;
};

class CommandExecutor : public QObject
{
    Q_OBJECT

public:
    explicit CommandExecutor(FileSystemGuard &guard, PermissionManager &permissions,
                             const QString &workDir, QObject *parent = nullptr);

    CommandResult execute(const QString &command, int timeoutMs = 30000);
    void executeAsync(const QString &command, int timeoutMs = 30000);

    bool isCommandWhitelisted(const QString &command) const;
    bool requiresPermission(const QString &command) const;

    QStringList whitelist() const;
    void setWhitelist(const QStringList &list);

signals:
    void executionFinished(const CommandResult &result);
    void outputReady(const QString &output);
    /// Emitted when a command needs user permission before execution.
    /// The UI should connect to this and show a confirmation dialog.
    void permissionRequired(const QString &command);

public slots:
    /// Called by the UI after the user grants permission to run a command.
    void executeApproved(const QString &command, int timeoutMs = 30000);

private:
    QString extractBaseCommand(const QString &command) const;

    FileSystemGuard &m_guard;
    PermissionManager &m_permissions;
    QString m_workDir;
    QStringList m_whitelist;
};
