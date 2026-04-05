#include "execution/SandboxManager.h"
#include "execution/FileSystemGuard.h"
#include "execution/CommandExecutor.h"
#include "execution/PermissionManager.h"
#include "infrastructure/DatabaseManager.h"
#include "service/Logger.h"

#include <QDir>

SandboxManager::SandboxManager(const QString &runtimePath, QObject *parent)
    : QObject(parent), m_runtimePath(runtimePath)
{
    ensureDirectories();

    m_guard = new FileSystemGuard(runtimePath, this);
    m_permissions = new PermissionManager(this);
    m_executor = new CommandExecutor(*m_guard, *m_permissions, workspacePath(), this);

    connect(m_executor, &CommandExecutor::executionFinished,
            this, [this](const CommandResult &result) {
        emit commandExecuted(result.output.left(100), result.exitCode);
        // Log to agent_logs if database is available
        if (m_db) {
            m_db->addAgentLog("command_execution", result.output.left(200),
                              result.output, result.exitCode);
        }
    });
}

void SandboxManager::ensureDirectories()
{
    QStringList dirs = {"workspace", "projects", "temp", "logs"};
    for (const auto &dir : dirs) {
        QDir().mkpath(m_runtimePath + "/" + dir);
    }
}

void SandboxManager::setDatabaseManager(DatabaseManager *db)
{
    m_db = db;
}

FileSystemGuard &SandboxManager::guard() { return *m_guard; }
CommandExecutor &SandboxManager::executor() { return *m_executor; }
PermissionManager &SandboxManager::permissions() { return *m_permissions; }

QString SandboxManager::runtimePath() const { return m_runtimePath; }
QString SandboxManager::workspacePath() const { return m_runtimePath + "/workspace"; }
QString SandboxManager::projectsPath() const { return m_runtimePath + "/projects"; }
QString SandboxManager::tempPath() const { return m_runtimePath + "/temp"; }
QString SandboxManager::logsPath() const { return m_runtimePath + "/logs"; }
