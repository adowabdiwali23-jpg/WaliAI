#include "execution/CommandExecutor.h"
#include "execution/FileSystemGuard.h"
#include "execution/PermissionManager.h"
#include "service/Logger.h"

#include <QProcess>
#include <QDir>

CommandExecutor::CommandExecutor(FileSystemGuard &guard, PermissionManager &permissions,
                                 const QString &workDir, QObject *parent)
    : QObject(parent), m_guard(guard), m_permissions(permissions), m_workDir(workDir)
{
    m_whitelist = {
        "ls", "cat", "echo", "mkdir", "rm", "cp", "mv",
        "grep", "find", "python3", "g++", "make", "cmake",
        "npm", "node", "head", "tail", "wc", "sort", "touch",
        "diff", "pwd", "env", "date", "whoami"
    };
}

bool CommandExecutor::isCommandWhitelisted(const QString &command) const
{
    QString base = extractBaseCommand(command);
    return m_whitelist.contains(base);
}

QString CommandExecutor::extractBaseCommand(const QString &command) const
{
    QString trimmed = command.trimmed();
    int spaceIdx = trimmed.indexOf(' ');
    QString base = (spaceIdx > 0) ? trimmed.left(spaceIdx) : trimmed;
    // Strip path prefix
    int slashIdx = base.lastIndexOf('/');
    if (slashIdx >= 0) base = base.mid(slashIdx + 1);
    return base;
}

bool CommandExecutor::requiresPermission(const QString &command) const
{
    return m_permissions.requiresConfirmation(command);
}

QStringList CommandExecutor::whitelist() const
{
    return m_whitelist;
}

void CommandExecutor::setWhitelist(const QStringList &list)
{
    m_whitelist = list;
}

CommandResult CommandExecutor::execute(const QString &command, int timeoutMs)
{
    CommandResult result;

    // Safety check: block dangerous commands and shell metacharacters
    if (!m_guard.isCommandSafe(command)) {
        result.error = "Command blocked by safety guard: " + command;
        result.exitCode = -1;
        Logger::instance().warning(result.error);
        return result;
    }

    // Blocked-command check via PermissionManager
    if (m_permissions.isDangerousCommand(command)) {
        result.error = "Command blocked (unsafe): " + command;
        result.exitCode = -1;
        Logger::instance().warning(result.error);
        return result;
    }

    // Whitelist check
    if (!isCommandWhitelisted(command)) {
        result.error = "Command not in whitelist: " + extractBaseCommand(command);
        result.exitCode = -1;
        Logger::instance().warning(result.error);
        return result;
    }

    Logger::instance().log("Executing command: " + command);

    QProcess process;
    process.setWorkingDirectory(m_workDir);
    process.setProcessChannelMode(QProcess::MergedChannels);

    process.start("/bin/sh", QStringList() << "-c" << command);

    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        result.error = "Command timed out after " + QString::number(timeoutMs / 1000) + "s";
        result.exitCode = -1;
        Logger::instance().warning(result.error);
        return result;
    }

    result.output = QString::fromUtf8(process.readAllStandardOutput());
    result.exitCode = process.exitCode();
    result.success = (result.exitCode == 0);

    Logger::instance().log(QString("Executed: %1 (exit=%2)").arg(command).arg(result.exitCode));
    return result;
}

void CommandExecutor::executeAsync(const QString &command, int timeoutMs)
{
    CommandResult result = execute(command, timeoutMs);
    emit executionFinished(result);
}

void CommandExecutor::executeApproved(const QString &command, int timeoutMs)
{
    // This is called after user has granted permission, so execute directly.
    Logger::instance().log("User approved command: " + command);
    CommandResult result = execute(command, timeoutMs);
    emit executionFinished(result);
}
