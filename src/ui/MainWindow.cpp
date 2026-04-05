#include "ui/MainWindow.h"
#include "ui/ChatWidget.h"
#include "ui/InputBarWidget.h"
#include "ui/SidebarWidget.h"
#include "ui/CognitiveSuiteWidget.h"
#include "ui/TerminalPanel.h"
#include "ui/FileExplorer.h"
#include "ui/HistoryDialog.h"
#include "ui/SettingsDialog.h"
#include "ui/SovereignWorkshopDialog.h"
#include "control/Controller.h"
#include "control/StateManager.h"
#include "service/HistoryService.h"
#include "execution/SandboxManager.h"
#include "execution/ProjectManager.h"
#include "infrastructure/DatabaseManager.h"
#include "voice/VoiceService.h"
#include "state/PersonalizationState.h"
#include "state/AgentState.h"

#include <QMenuBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QMessageBox>

MainWindow::MainWindow(Controller &controller, HistoryService &history,
                       StateManager &state, SandboxManager &sandbox,
                       ProjectManager &projects, DatabaseManager &db,
                       VoiceService &voice, QWidget *parent)
    : QMainWindow(parent), m_controller(controller), m_history(history),
      m_state(state), m_sandbox(sandbox), m_projects(projects), m_db(db), m_voice(voice)
{
    setupUi();
    setupMenuBar();
    setupConnections();

    // Load or create initial session
    auto sessions = m_history.listSessions();
    if (sessions.isEmpty()) {
        m_controller.createNewSession();
    } else {
        int sessionId = sessions.first()["id"].toInt();
        m_controller.switchSession(sessionId);
        loadSession(sessionId);
    }
}

void MainWindow::setupUi()
{
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Sidebar
    m_sidebar = new SidebarWidget(m_history, this);

    // Cognitive Suite panel
    m_cogSuite = new CognitiveSuiteWidget(m_state, this);

    // Main splitter (left: sidebar+cogSuite | center: chat | right: terminal+explorer)
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Left panel (sidebar)
    m_mainSplitter->addWidget(m_sidebar);

    // Center panel (chat + input)
    auto *centerWidget = new QWidget(this);
    auto *centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);

    m_chatWidget = new ChatWidget(this);
    m_inputBar = new InputBarWidget(this);

    centerLayout->addWidget(m_chatWidget, 1);
    centerLayout->addWidget(m_inputBar);

    m_mainSplitter->addWidget(centerWidget);

    // Right panel (cognitive suite + terminal + file explorer)
    auto *rightWidget = new QWidget(this);
    auto *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    rightLayout->addWidget(m_cogSuite);

    m_rightSplitter = new QSplitter(Qt::Vertical, this);
    m_terminal = new TerminalPanel(m_sandbox, this);
    m_fileExplorer = new FileExplorer(m_sandbox.workspacePath(), this);
    m_rightSplitter->addWidget(m_terminal);
    m_rightSplitter->addWidget(m_fileExplorer);
    rightLayout->addWidget(m_rightSplitter, 1);

    m_mainSplitter->addWidget(rightWidget);

    // Set splitter sizes
    m_mainSplitter->setSizes({250, 600, 350});

    mainLayout->addWidget(m_mainSplitter);

    // Style
    setStyleSheet(
        "QMainWindow { background-color: #0f1117; }"
        "QSplitter::handle { background-color: #2d3748; width: 2px; }"
    );
}

void MainWindow::setupMenuBar()
{
    auto *menuBar = this->menuBar();
    menuBar->setStyleSheet(
        "QMenuBar { background-color: #171923; color: #e2e8f0; }"
        "QMenuBar::item:selected { background-color: #2d3748; }"
        "QMenu { background-color: #1a202c; color: #e2e8f0; border: 1px solid #4a5568; }"
        "QMenu::item:selected { background-color: #4299e1; }"
    );

    auto *fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction("New Chat", this, [this]() { m_controller.createNewSession(); });
    fileMenu->addSeparator();
    fileMenu->addAction("Settings...", this, &MainWindow::onSettingsRequested);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", qApp, &QApplication::quit);

    auto *viewMenu = menuBar->addMenu("&View");
    viewMenu->addAction("History...", this, &MainWindow::onHistoryDialogRequested);
    viewMenu->addAction("Sovereign Workshop...", this, &MainWindow::onWorkshopRequested);

    auto *helpMenu = menuBar->addMenu("&Help");
    helpMenu->addAction("About Wali AI", this, [this]() {
        QMessageBox::about(this, "About Wali AI",
            "Wali AI v1.0.0\n\n"
            "A sovereign, fully local, sandboxed AI agent.\n"
            "Built with Qt6 and llama.cpp.\n\n"
            "Model: Qwen2.5-Coder-7B-Instruct");
    });
}

void MainWindow::setupConnections()
{
    // Input -> Controller
    connect(m_inputBar, &InputBarWidget::messageSent, this, [this](const QString &msg) {
        m_chatWidget->addMessage("user", msg);
        m_controller.handleUserInput(msg);
    });

    // Voice input
    connect(m_inputBar, &InputBarWidget::voiceButtonClicked, this, [this]() {
        m_voice.startListening();
    });

    // Stop generation
    connect(m_inputBar, &InputBarWidget::stopRequested, this, [this]() {
        m_controller.handleUserInput(""); // Will be ignored (empty)
    });

    // Controller -> UI
    connect(&m_controller, &Controller::responseChunk, m_chatWidget, &ChatWidget::appendToLastMessage);
    connect(&m_controller, &Controller::processingChanged, m_inputBar, &InputBarWidget::setProcessing);

    // Session management
    connect(m_sidebar, &SidebarWidget::sessionSelected, this, [this](int id) {
        m_controller.switchSession(id);
        loadSession(id);
    });
    connect(m_sidebar, &SidebarWidget::newSessionRequested, this, [this]() {
        m_controller.createNewSession();
        m_chatWidget->clear();
        m_sidebar->refreshSessions();
    });
    connect(m_sidebar, &SidebarWidget::deleteSessionRequested, this, [this](int id) {
        m_controller.deleteSession(id);
        m_sidebar->refreshSessions();
    });
    connect(m_sidebar, &SidebarWidget::settingsRequested, this, &MainWindow::onSettingsRequested);
    connect(m_sidebar, &SidebarWidget::historyDialogRequested, this, &MainWindow::onHistoryDialogRequested);

    // Agent mode toggle
    connect(m_sidebar, &SidebarWidget::agentModeToggled, this, [this](bool enabled) {
        m_state.agentState().setAgentModeEnabled(enabled);
    });

    // Session switched
    connect(&m_controller, &Controller::sessionSwitched, this, [this](int id) {
        loadSession(id);
        m_sidebar->refreshSessions();
    });

    // Chat speak button -> VoiceService
    connect(m_chatWidget, &ChatWidget::speakRequested, this, [this](const QString &text) {
        m_voice.speak(text);
    });

    // Voice transcription -> input
    connect(&m_voice, &VoiceService::transcriptionReady, this, [this](const QString &text) {
        m_controller.handleVoiceInput(text);
        m_chatWidget->addMessage("user", text);
    });

    // Error handling
    connect(&m_controller, &Controller::errorOccurred, this, [this](const QString &error) {
        m_chatWidget->addMessage("system", "[Error] " + error);
    });
}

void MainWindow::loadSession(int sessionId)
{
    m_chatWidget->clear();
    auto messages = m_history.getMessages(sessionId);
    for (const auto &msg : messages) {
        m_chatWidget->addMessage(msg["role"].toString(), msg["content"].toString());
    }
}

void MainWindow::onSettingsRequested()
{
    SettingsDialog dialog(m_state.personalizationState(), m_db, this);
    dialog.exec();
}

void MainWindow::onHistoryDialogRequested()
{
    HistoryDialog dialog(m_history, this);
    connect(&dialog, &HistoryDialog::sessionSelected, this, [this](int id) {
        m_controller.switchSession(id);
        loadSession(id);
    });
    connect(&dialog, &HistoryDialog::sessionDeleted, this, [this](int id) {
        m_history.deleteSession(id);
        m_sidebar->refreshSessions();
    });
    connect(&dialog, &HistoryDialog::sessionRenamed, this, [this](int id, const QString &title) {
        m_history.renameSession(id, title);
        m_sidebar->refreshSessions();
    });
    dialog.exec();
}

void MainWindow::onWorkshopRequested()
{
    SovereignWorkshopDialog dialog(m_projects, this);
    dialog.exec();
}
