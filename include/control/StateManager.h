#pragma once

#include <QObject>

class CognitiveState;
class PersonalizationState;
class AgentState;

class StateManager : public QObject
{
    Q_OBJECT

public:
    explicit StateManager(CognitiveState &cognitive, PersonalizationState &personalization,
                          AgentState &agent, QObject *parent = nullptr);

    CognitiveState &cognitiveState();
    PersonalizationState &personalizationState();
    AgentState &agentState();

    const CognitiveState &cognitiveState() const;
    const PersonalizationState &personalizationState() const;
    const AgentState &agentState() const;

    void toggleSearch();
    void toggleResearch();
    void toggleDeepThink();
    void toggleAgentMode();

signals:
    void stateChanged();

private:
    CognitiveState &m_cognitive;
    PersonalizationState &m_personalization;
    AgentState &m_agent;
};
