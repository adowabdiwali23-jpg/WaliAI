#include "execution/FileSystemGuard.h"
#include "service/Logger.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

FileSystemGuard::FileSystemGuard(const QString &sandboxRoot, QObject *parent)
    : QObject(parent), m_sandboxRoot(QDir(sandboxRoot).canonicalPath())
{
    m_blockedPaths = {
        "/etc", "/usr", "/bin", "/sbin", "/boot", "/dev", "/proc",
        "/sys", "/root", "/var", "/lib", "/lib64", "/opt",
        "/home/other_users"
    };
}

bool FileSystemGuard::isPathAllowed(const QString &path) const
{
    QString resolved = resolvePath(path);
    if (resolved.isEmpty()) return false;

    // Must be inside sandbox
    if (!resolved.startsWith(m_sandboxRoot)) {
        Logger::instance().warning("Path outside sandbox: " + path);
        return false;
    }

    // Check blocked paths
    for (const auto &blocked : m_blockedPaths) {
        if (resolved.startsWith(blocked)) {
            Logger::instance().warning("Blocked path access: " + path);
            return false;
        }
    }

    return true;
}

bool FileSystemGuard::isCommandSafe(const QString &command) const
{
    QString cmd = command.trimmed().toLower();

    // Block shell metacharacters to prevent command chaining / injection.
    // Commands are executed via /bin/sh -c, so these are dangerous.
    static const QStringList metacharacters = {
        ";", "&&", "||", "|", "`", "$(", ">", ">>", "<"
    };
    for (const auto &mc : metacharacters) {
        if (cmd.contains(mc)) {
            Logger::instance().warning("Shell metacharacter blocked in command: " + command);
            return false;
        }
    }

    // Block dangerous commands — check anywhere in the string, not just the start
    static const QStringList dangerous = {
        "sudo", "su", "passwd", "shutdown", "reboot", "poweroff",
        "halt", "init", "systemctl", "service", "mount", "umount",
        "fdisk", "mkfs", "dd", "chown", "chmod 777", "chroot",
        "kill -9", "killall", "pkill"
    };

    for (const auto &d : dangerous) {
        // Check if the dangerous command appears as a word boundary in the input
        if (cmd == d || cmd.startsWith(d + " ") || cmd.contains(" " + d + " ") ||
            cmd.contains(" " + d)) {
            Logger::instance().warning("Dangerous command blocked: " + command);
            return false;
        }
    }

    // Block dangerous rm patterns targeting absolute paths
    QRegularExpression dangerousRm("\\brm\\s+(-[a-z]*r[a-z]*\\s+)?/");
    if (dangerousRm.match(cmd).hasMatch()) {
        Logger::instance().warning("Dangerous rm pattern blocked: " + command);
        return false;
    }

    // Block path traversal in commands
    if (cmd.contains("../") || cmd.contains("..\\")) {
        Logger::instance().warning("Path traversal in command blocked: " + command);
        return false;
    }

    return true;
}

QString FileSystemGuard::resolvePath(const QString &path) const
{
    QString cleanPath = QDir::cleanPath(path);

    // If relative, resolve against sandbox root
    if (!QDir::isAbsolutePath(cleanPath)) {
        cleanPath = m_sandboxRoot + "/" + cleanPath;
    }

    // Resolve symlinks
    QFileInfo info(cleanPath);
    if (info.isSymLink()) {
        QString target = info.symLinkTarget();
        QFileInfo targetInfo(target);
        QString canonical = targetInfo.canonicalFilePath();
        if (canonical.isEmpty()) {
            canonical = QDir::cleanPath(target);
        }
        // Validate symlink target is inside sandbox
        if (!canonical.startsWith(m_sandboxRoot)) {
            Logger::instance().warning("Symlink escape detected: " + path + " -> " + canonical);
            return QString();
        }
        return canonical;
    }

    // Use canonical path if file exists, otherwise clean path
    QString canonical = info.canonicalFilePath();
    if (!canonical.isEmpty()) {
        return canonical;
    }
    return QDir::cleanPath(cleanPath);
}

bool FileSystemGuard::isSymlinkSafe(const QString &path) const
{
    QFileInfo info(path);
    if (!info.isSymLink()) return true;

    QString target = info.symLinkTarget();
    QFileInfo targetInfo(target);
    QString canonical = targetInfo.canonicalFilePath();
    if (canonical.isEmpty()) {
        canonical = QDir::cleanPath(target);
    }

    bool safe = canonical.startsWith(m_sandboxRoot);
    if (!safe) {
        Logger::instance().warning("Unsafe symlink: " + path + " -> " + canonical);
    }
    return safe;
}

QString FileSystemGuard::sandboxRoot() const { return m_sandboxRoot; }
