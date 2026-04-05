#pragma once

#include <QObject>
#include <QString>

class PersonalizationState : public QObject
{
    Q_OBJECT

public:
    explicit PersonalizationState(QObject *parent = nullptr);

    QString userName() const;
    void setUserName(const QString &name);

    QString systemPrompt() const;
    void setSystemPrompt(const QString &prompt);

    QString theme() const;
    void setTheme(const QString &theme);

    int maxTokens() const;
    void setMaxTokens(int tokens);

    double temperature() const;
    void setTemperature(double temp);

    int contextWindowSize() const;
    void setContextWindowSize(int size);

    QString voiceProfile() const;
    void setVoiceProfile(const QString &profile);

    bool memoryEnabled() const;
    void setMemoryEnabled(bool enabled);

    bool ttsEnabled() const;
    void setTtsEnabled(bool enabled);

    int uiMaxMessages() const;
    void setUiMaxMessages(int max);

signals:
    void userNameChanged(const QString &name);
    void systemPromptChanged(const QString &prompt);
    void themeChanged(const QString &theme);
    void maxTokensChanged(int tokens);
    void temperatureChanged(double temp);
    void contextWindowSizeChanged(int size);
    void voiceProfileChanged(const QString &profile);
    void memoryEnabledChanged(bool enabled);
    void ttsEnabledChanged(bool enabled);
    void uiMaxMessagesChanged(int max);

private:
    QString m_userName = "User";
    QString m_systemPrompt = "You are Wali, a helpful AI assistant.";
    QString m_theme = "dark";
    int m_maxTokens = 2048;
    double m_temperature = 0.7;
    int m_contextWindowSize = 4096;
    QString m_voiceProfile = "breeze";
    bool m_memoryEnabled = true;
    bool m_ttsEnabled = false;
    int m_uiMaxMessages = 1000;
};
