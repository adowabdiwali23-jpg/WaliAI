#include "state/PersonalizationState.h"

PersonalizationState::PersonalizationState(QObject *parent)
    : QObject(parent)
{
}

QString PersonalizationState::userName() const { return m_userName; }
void PersonalizationState::setUserName(const QString &name)
{
    if (m_userName != name) {
        m_userName = name;
        emit userNameChanged(name);
    }
}

QString PersonalizationState::systemPrompt() const { return m_systemPrompt; }
void PersonalizationState::setSystemPrompt(const QString &prompt)
{
    if (m_systemPrompt != prompt) {
        m_systemPrompt = prompt;
        emit systemPromptChanged(prompt);
    }
}

QString PersonalizationState::theme() const { return m_theme; }
void PersonalizationState::setTheme(const QString &theme)
{
    if (m_theme != theme) {
        m_theme = theme;
        emit themeChanged(theme);
    }
}

int PersonalizationState::maxTokens() const { return m_maxTokens; }
void PersonalizationState::setMaxTokens(int tokens)
{
    if (m_maxTokens != tokens) {
        m_maxTokens = tokens;
        emit maxTokensChanged(tokens);
    }
}

double PersonalizationState::temperature() const { return m_temperature; }
void PersonalizationState::setTemperature(double temp)
{
    if (m_temperature != temp) {
        m_temperature = temp;
        emit temperatureChanged(temp);
    }
}

int PersonalizationState::contextWindowSize() const { return m_contextWindowSize; }
void PersonalizationState::setContextWindowSize(int size)
{
    if (m_contextWindowSize != size) {
        m_contextWindowSize = size;
        emit contextWindowSizeChanged(size);
    }
}

QString PersonalizationState::voiceProfile() const { return m_voiceProfile; }
void PersonalizationState::setVoiceProfile(const QString &profile)
{
    if (m_voiceProfile != profile) {
        m_voiceProfile = profile;
        emit voiceProfileChanged(profile);
    }
}

bool PersonalizationState::memoryEnabled() const { return m_memoryEnabled; }
void PersonalizationState::setMemoryEnabled(bool enabled)
{
    if (m_memoryEnabled != enabled) {
        m_memoryEnabled = enabled;
        emit memoryEnabledChanged(enabled);
    }
}

bool PersonalizationState::ttsEnabled() const { return m_ttsEnabled; }
void PersonalizationState::setTtsEnabled(bool enabled)
{
    if (m_ttsEnabled != enabled) {
        m_ttsEnabled = enabled;
        emit ttsEnabledChanged(enabled);
    }
}

int PersonalizationState::uiMaxMessages() const { return m_uiMaxMessages; }
void PersonalizationState::setUiMaxMessages(int max)
{
    if (m_uiMaxMessages != max) {
        m_uiMaxMessages = max;
        emit uiMaxMessagesChanged(max);
    }
}
