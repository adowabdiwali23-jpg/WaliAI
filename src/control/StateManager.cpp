#include "control/StateManager.h"
#include "state/CognitiveState.h"
#include "state/PersonalizationState.h"
#include "state/AgentState.h"

StateManager::StateManager(CognitiveState &cognitive, PersonalizationState &personalization,
                           AgentState &agent, QObject *parent)
    : QObject(parent), m_cognitive(cognitive), m_personalization(personalization), m_agent(agent)
{
}

CognitiveState &StateManager::cognitiveState() { return m_cognitive; }
PersonalizationState &StateManager::personalizationState() { return m_personalization; }
AgentState &StateManager::agentState() { return m_agent; }

const CognitiveState &StateManager::cognitiveState() const { return m_cognitive; }
const PersonalizationState &StateManager::personalizationState() const { return m_personalization; }
const AgentState &StateManager::agentState() const { return m_agent; }

void StateManager::toggleSearch()
{
    m_cognitive.setSearchEnabled(!m_cognitive.searchEnabled());
    emit stateChanged();
}

void StateManager::toggleResearch()
{
    m_cognitive.setResearchEnabled(!m_cognitive.researchEnabled());
    emit stateChanged();
}

void StateManager::toggleDeepThink()
{
    m_cognitive.setDeepThinkEnabled(!m_cognitive.deepThinkEnabled());
    emit stateChanged();
}

void StateManager::toggleAgentMode()
{
    m_agent.setAgentModeEnabled(!m_agent.agentModeEnabled());
    emit stateChanged();
}
