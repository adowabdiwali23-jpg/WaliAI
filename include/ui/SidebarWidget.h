#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCheckBox>

class HistoryService;

class SidebarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarWidget(HistoryService &history, QWidget *parent = nullptr);

    void refreshSessions();

signals:
    void sessionSelected(int sessionId);
    void newSessionRequested();
    void deleteSessionRequested(int sessionId);
    void settingsRequested();
    void historyDialogRequested();
    void agentModeToggled(bool enabled);

private:
    void setupUi();
    void onSessionClicked(QListWidgetItem *item);
    void onNewSession();
    void onDeleteSession();

    HistoryService &m_history;
    QListWidget *m_sessionList = nullptr;
    QPushButton *m_newSessionBtn = nullptr;
    QPushButton *m_deleteBtn = nullptr;
    QPushButton *m_settingsBtn = nullptr;
    QPushButton *m_historyBtn = nullptr;
    QCheckBox *m_agentModeCheck = nullptr;
};
