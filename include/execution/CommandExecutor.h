#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class FileSystemGuard;

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
    explicit CommandExecutor(FileSystemGuard &guard, const QString &workDir,
                             QObject *parent = nullptr);

    CommandResult execute(const QString &command, int timeoutMs = 30000);
    void executeAsync(const QString &command, int timeoutMs = 30000);

    bool isCommandWhitelisted(const QString &command) const;

signals:
    void executionFinished(const CommandResult &result);
    void outputReady(const QString &output);

private:
    QString extractBaseCommand(const QString &command) const;

    FileSystemGuard &m_guard;
    QString m_workDir;
    QStringList m_whitelist;
};
