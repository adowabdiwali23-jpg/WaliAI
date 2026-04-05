#include "state/AgentState.h"

AgentState::AgentState(QObject *parent)
    : QObject(parent)
{
}

bool AgentState::agentModeEnabled() const { return m_agentModeEnabled; }
void AgentState::setAgentModeEnabled(bool enabled)
{
    if (m_agentModeEnabled != enabled) {
        m_agentModeEnabled = enabled;
        emit agentModeEnabledChanged(enabled);
    }
}
