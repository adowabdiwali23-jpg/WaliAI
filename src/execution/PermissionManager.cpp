#include "execution/PermissionManager.h"
#include "service/Logger.h"

PermissionManager::PermissionManager(QObject *parent)
    : QObject(parent)
{
    m_blockedCommands = {
        "sudo", "su", "passwd", "shutdown", "reboot", "poweroff",
        "halt", "init", "systemctl", "service", "mount", "umount",
        "fdisk", "mkfs", "dd", "chown", "chroot",
        "kill", "killall", "pkill", "iptables", "ufw"
    };

    m_confirmRequired = {
        "rm -rf", "rm -r", "rmdir", "chmod", "mv"
    };
}

bool PermissionManager::isOperationAllowed(const QString &operation) const
{
    return !isDangerousCommand(operation);
}

bool PermissionManager::isDangerousCommand(const QString &command) const
{
    QString cmd = command.trimmed().toLower();
    for (const auto &blocked : m_blockedCommands) {
        if (cmd.startsWith(blocked + " ") || cmd == blocked) {
            Logger::instance().warning("Dangerous command blocked: " + command);
            return true;
        }
    }
    return false;
}

bool PermissionManager::requiresConfirmation(const QString &command) const
{
    QString cmd = command.trimmed().toLower();
    for (const auto &risky : m_confirmRequired) {
        if (cmd.startsWith(risky)) {
            return true;
        }
    }
    return false;
}
