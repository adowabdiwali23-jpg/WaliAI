#include "ui/SidebarWidget.h"
#include "service/HistoryService.h"

#include <QLabel>

SidebarWidget::SidebarWidget(HistoryService &history, QWidget *parent)
    : QWidget(parent), m_history(history)
{
    setupUi();
    refreshSessions();
}

void SidebarWidget::setupUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    auto *titleLabel = new QLabel("Chat History");
    titleLabel->setStyleSheet("QLabel { color: #e2e8f0; font-size: 16px; font-weight: bold; padding: 8px; }");
    layout->addWidget(titleLabel);

    m_newSessionBtn = new QPushButton("+ New Chat", this);
    m_newSessionBtn->setStyleSheet(
        "QPushButton { background-color: #4299e1; color: white; "
        "border-radius: 6px; padding: 8px; font-weight: bold; }"
        "QPushButton:hover { background-color: #3182ce; }"
    );
    layout->addWidget(m_newSessionBtn);

    m_sessionList = new QListWidget(this);
    m_sessionList->setStyleSheet(
        "QListWidget { background-color: #1a202c; border: none; color: #e2e8f0; }"
        "QListWidget::item { padding: 10px; border-radius: 6px; margin: 2px; }"
        "QListWidget::item:selected { background-color: #2d3748; }"
        "QListWidget::item:hover { background-color: #2d3748; }"
    );
    layout->addWidget(m_sessionList, 1);

    // Agent mode toggle
    QString checkboxStyle =
        "QCheckBox { color: #e2e8f0; font-size: 12px; spacing: 6px; padding: 4px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border-radius: 3px; "
        "border: 2px solid #4a5568; background-color: #2d3748; }"
        "QCheckBox::indicator:checked { background-color: #48bb78; border-color: #48bb78; }";
    m_agentModeCheck = new QCheckBox("Agent Mode", this);
    m_agentModeCheck->setToolTip("Enable autonomous command execution");
    m_agentModeCheck->setStyleSheet(checkboxStyle);
    layout->addWidget(m_agentModeCheck);

    // Bottom buttons
    auto *bottomLayout = new QHBoxLayout();

    m_deleteBtn = new QPushButton("Delete", this);
    m_deleteBtn->setStyleSheet(
        "QPushButton { background-color: #e53e3e; color: white; "
        "border-radius: 6px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #c53030; }"
    );

    m_historyBtn = new QPushButton("History", this);
    m_historyBtn->setStyleSheet(
        "QPushButton { background-color: #4a5568; color: white; "
        "border-radius: 6px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #718096; }"
    );

    m_settingsBtn = new QPushButton("Settings", this);
    m_settingsBtn->setStyleSheet(
        "QPushButton { background-color: #4a5568; color: white; "
        "border-radius: 6px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #718096; }"
    );

    bottomLayout->addWidget(m_deleteBtn);
    bottomLayout->addWidget(m_historyBtn);
    bottomLayout->addWidget(m_settingsBtn);
    layout->addLayout(bottomLayout);

    connect(m_newSessionBtn, &QPushButton::clicked, this, &SidebarWidget::onNewSession);
    connect(m_deleteBtn, &QPushButton::clicked, this, &SidebarWidget::onDeleteSession);
    connect(m_settingsBtn, &QPushButton::clicked, this, &SidebarWidget::settingsRequested);
    connect(m_historyBtn, &QPushButton::clicked, this, &SidebarWidget::historyDialogRequested);
    connect(m_sessionList, &QListWidget::itemClicked, this, &SidebarWidget::onSessionClicked);
    connect(m_agentModeCheck, &QCheckBox::toggled, this, &SidebarWidget::agentModeToggled);

    setMinimumWidth(220);
    setMaximumWidth(300);
    setStyleSheet("QWidget { background-color: #171923; }");
}

void SidebarWidget::refreshSessions()
{
    m_sessionList->clear();
    auto sessions = m_history.listSessions();

    for (const auto &session : sessions) {
        auto *item = new QListWidgetItem(session["title"].toString());
        item->setData(Qt::UserRole, session["id"]);
        m_sessionList->addItem(item);
    }

    int currentId = m_history.currentSessionId();
    for (int i = 0; i < m_sessionList->count(); ++i) {
        auto *item = m_sessionList->item(i);
        if (item->data(Qt::UserRole).toInt() == currentId) {
            m_sessionList->setCurrentItem(item);
            break;
        }
    }
}

void SidebarWidget::onSessionClicked(QListWidgetItem *item)
{
    int sessionId = item->data(Qt::UserRole).toInt();
    emit sessionSelected(sessionId);
}

void SidebarWidget::onNewSession()
{
    emit newSessionRequested();
}

void SidebarWidget::onDeleteSession()
{
    auto *currentItem = m_sessionList->currentItem();
    if (currentItem) {
        int sessionId = currentItem->data(Qt::UserRole).toInt();
        emit deleteSessionRequested(sessionId);
    }
}
