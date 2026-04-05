#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class PermissionManager : public QObject
{
    Q_OBJECT

public:
    explicit PermissionManager(QObject *parent = nullptr);

    bool isOperationAllowed(const QString &operation) const;
    bool isDangerousCommand(const QString &command) const;
    bool requiresConfirmation(const QString &command) const;

private:
    QStringList m_blockedCommands;
    QStringList m_confirmRequired;
};
