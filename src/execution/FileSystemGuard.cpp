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

    // Block shell metacharacters that enable command injection.
    // Commands are executed via /bin/sh -c, so characters like ; && || |
    // backticks and $() allow chaining arbitrary commands after a
    // whitelisted prefix, completely bypassing the safety checks.
    QStringList shellMetachars = {
        ";", "&&", "||", "|", "`", "$(", ">", ">>", "<"
    };
    for (const auto &meta : shellMetachars) {
        if (cmd.contains(meta)) {
            Logger::instance().warning(
                "Shell metacharacter blocked (command injection risk): " + command);
            return false;
        }
    }

    // Block dangerous commands anywhere in the string, not just at the
    // start.  A command like "echo hi; sudo rm -rf /" would previously
    // slip through because the startsWith check only saw "echo".
    QStringList dangerous = {
        "sudo", "su", "passwd", "shutdown", "reboot", "poweroff",
        "halt", "init", "systemctl", "service", "mount", "umount",
        "fdisk", "mkfs", "dd", "chown", "chmod 777", "chroot",
        "kill", "killall", "pkill"
    };

    for (const auto &d : dangerous) {
        if (cmd.startsWith(d + " ") || cmd == d
            || cmd.contains(" " + d + " ") || cmd.endsWith(" " + d)) {
            Logger::instance().warning("Dangerous command blocked: " + command);
            return false;
        }
    }

    // Block dangerous rm patterns (rm -rf /, rm -r /, etc.)
    // rm is whitelisted for normal use, but recursive operations on
    // root or absolute paths outside the sandbox must be prevented.
    QRegularExpression dangerousRm(
        "\\brm\\s+(-[a-z]*r[a-z]*\\s+)?/",
        QRegularExpression::CaseInsensitiveOption);
    if (dangerousRm.match(cmd).hasMatch()) {
        Logger::instance().warning(
            "Dangerous rm with absolute path blocked: " + command);
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
