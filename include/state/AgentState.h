#pragma once

#include <QObject>
#include <QString>

class AgentState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool agentModeEnabled READ agentModeEnabled WRITE setAgentModeEnabled NOTIFY agentModeEnabledChanged)

public:
    explicit AgentState(QObject *parent = nullptr);

    bool agentModeEnabled() const;
    void setAgentModeEnabled(bool enabled);

signals:
    void agentModeEnabledChanged(bool enabled);

private:
    bool m_agentModeEnabled = false;
};
