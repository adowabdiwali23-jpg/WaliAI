#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>

class StateManager;

class CognitiveSuiteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CognitiveSuiteWidget(StateManager &state, QWidget *parent = nullptr);

    void refreshState();

signals:
    void stateToggled(const QString &feature, bool enabled);

private:
    void setupUi();
    void connectToggles();

    StateManager &m_state;
    QCheckBox *m_search = nullptr;
    QCheckBox *m_research = nullptr;
    QCheckBox *m_deepThink = nullptr;
};
