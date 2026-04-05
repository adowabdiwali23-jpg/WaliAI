#include "state/CognitiveState.h"

CognitiveState::CognitiveState(QObject *parent)
    : QObject(parent)
{
}

bool CognitiveState::searchEnabled() const { return m_searchEnabled; }
void CognitiveState::setSearchEnabled(bool enabled)
{
    if (m_searchEnabled != enabled) {
        m_searchEnabled = enabled;
        emit searchEnabledChanged(enabled);
    }
}

bool CognitiveState::researchEnabled() const { return m_researchEnabled; }
void CognitiveState::setResearchEnabled(bool enabled)
{
    if (m_researchEnabled != enabled) {
        m_researchEnabled = enabled;
        emit researchEnabledChanged(enabled);
    }
}

bool CognitiveState::deepThinkEnabled() const { return m_deepThinkEnabled; }
void CognitiveState::setDeepThinkEnabled(bool enabled)
{
    if (m_deepThinkEnabled != enabled) {
        m_deepThinkEnabled = enabled;
        emit deepThinkEnabledChanged(enabled);
    }
}
