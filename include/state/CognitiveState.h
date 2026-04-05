#pragma once

#include <QObject>
#include <QString>

class CognitiveState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool searchEnabled READ searchEnabled WRITE setSearchEnabled NOTIFY searchEnabledChanged)
    Q_PROPERTY(bool researchEnabled READ researchEnabled WRITE setResearchEnabled NOTIFY researchEnabledChanged)
    Q_PROPERTY(bool deepThinkEnabled READ deepThinkEnabled WRITE setDeepThinkEnabled NOTIFY deepThinkEnabledChanged)

public:
    explicit CognitiveState(QObject *parent = nullptr);

    bool searchEnabled() const;
    void setSearchEnabled(bool enabled);

    bool researchEnabled() const;
    void setResearchEnabled(bool enabled);

    bool deepThinkEnabled() const;
    void setDeepThinkEnabled(bool enabled);

signals:
    void searchEnabledChanged(bool enabled);
    void researchEnabledChanged(bool enabled);
    void deepThinkEnabledChanged(bool enabled);

private:
    bool m_searchEnabled = false;
    bool m_researchEnabled = false;
    bool m_deepThinkEnabled = false;
};
