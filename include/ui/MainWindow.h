#pragma once

#include <QMainWindow>
#include <QSplitter>

class Controller;
class HistoryService;
class StateManager;
class SandboxManager;
class ProjectManager;
class DatabaseManager;
class VoiceService;
class ChatWidget;
class InputBarWidget;
class SidebarWidget;
class CognitiveSuiteWidget;
class TerminalPanel;
class FileExplorer;
class HistoryDialog;
class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Controller &controller,
                        HistoryService &history,
                        StateManager &state,
                        SandboxManager &sandbox,
                        ProjectManager &projects,
                        DatabaseManager &db,
                        VoiceService &voice,
                        QWidget *parent = nullptr);

private:
    void setupUi();
    void setupMenuBar();
    void setupConnections();
    void loadSession(int sessionId);
    void onSettingsRequested();
    void onHistoryDialogRequested();
    void onWorkshopRequested();

    Controller &m_controller;
    HistoryService &m_history;
    StateManager &m_state;
    SandboxManager &m_sandbox;
    ProjectManager &m_projects;
    DatabaseManager &m_db;
    VoiceService &m_voice;

    ChatWidget *m_chatWidget = nullptr;
    InputBarWidget *m_inputBar = nullptr;
    SidebarWidget *m_sidebar = nullptr;
    CognitiveSuiteWidget *m_cogSuite = nullptr;
    TerminalPanel *m_terminal = nullptr;
    FileExplorer *m_fileExplorer = nullptr;
    QSplitter *m_mainSplitter = nullptr;
    QSplitter *m_rightSplitter = nullptr;
};
