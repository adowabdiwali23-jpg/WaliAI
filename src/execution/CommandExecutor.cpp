#include "execution/CommandExecutor.h"
#include "execution/FileSystemGuard.h"
#include "service/Logger.h"

#include <QProcess>
#include <QDir>

CommandExecutor::CommandExecutor(FileSystemGuard &guard, const QString &workDir,
                                 QObject *parent)
    : QObject(parent), m_guard(guard), m_workDir(workDir)
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

CommandResult CommandExecutor::execute(const QString &command, int timeoutMs)
{
    CommandResult result;

    // Safety check
    if (!m_guard.isCommandSafe(command)) {
        result.error = "Command blocked by safety guard: " + command;
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
