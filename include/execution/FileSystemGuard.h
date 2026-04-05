#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class FileSystemGuard : public QObject
{
    Q_OBJECT

public:
    explicit FileSystemGuard(const QString &sandboxRoot, QObject *parent = nullptr);

    bool isPathAllowed(const QString &path) const;
    bool isCommandSafe(const QString &command) const;
    QString resolvePath(const QString &path) const;
    bool isSymlinkSafe(const QString &path) const;

    QString sandboxRoot() const;

private:
    QString m_sandboxRoot;
    QStringList m_blockedPaths;
};
