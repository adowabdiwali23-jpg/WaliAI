#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>

#include "infrastructure/DatabaseManager.h"
#include "infrastructure/ModelLocator.h"
#include "state/CognitiveState.h"
#include "state/PersonalizationState.h"
#include "state/AgentState.h"
#include "service/Logger.h"
#include "service/MemoryService.h"
#include "service/HistoryService.h"
#include "service/HiddenBrowser.h"
#include "execution/SandboxManager.h"
#include "execution/ProjectManager.h"
#include "intelligence/CognitionEngine.h"
#include "intelligence/ModelInference.h"
#include "voice/VoiceService.h"
#include "control/StateManager.h"
#include "control/Controller.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("WaliAI");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("WaliAI");

    // Ensure runtime directories exist
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString runtimePath = dataPath + "/runtime";
    QStringList dirs = {"workspace", "projects", "temp", "logs"};
    for (const auto &dir : dirs) {
        QDir().mkpath(runtimePath + "/" + dir);
    }

    // Initialize infrastructure
    Logger::instance().init(runtimePath + "/logs");
    Logger::instance().log("WaliAI starting up...");

    DatabaseManager dbManager(runtimePath + "/wali.db");
    if (!dbManager.initialize()) {
        Logger::instance().error("Failed to initialize database");
        QMessageBox::critical(nullptr, "Wali AI",
            "Failed to initialize database. The application cannot start.");
        return 1;
    }

    ModelLocator modelLocator(runtimePath);

    // Initialize state
    CognitiveState cognitiveState;
    PersonalizationState personalizationState;
    AgentState agentState;
    dbManager.loadSettings(personalizationState);

    // Initialize services
    MemoryService memoryService(dbManager);
    HistoryService historyService(dbManager);
    HiddenBrowser hiddenBrowser;

    // Initialize execution layer
    SandboxManager sandboxManager(runtimePath);
    sandboxManager.setDatabaseManager(&dbManager);
    ProjectManager projectManager(dbManager, sandboxManager);

    // Initialize intelligence
    ModelInference modelInference(modelLocator);
    CognitionEngine cognitionEngine(modelInference, memoryService, hiddenBrowser);

    // Initialize voice
    VoiceService voiceService(modelLocator);

    // Initialize control
    StateManager stateManager(cognitiveState, personalizationState, agentState);
    Controller controller(
        cognitionEngine,
        memoryService,
        historyService,
        hiddenBrowser,
        sandboxManager,
        voiceService,
        stateManager,
        dbManager
    );

    // Create and show main window
    MainWindow mainWindow(controller, historyService, stateManager,
                          sandboxManager, projectManager, dbManager, voiceService);
    mainWindow.setWindowTitle("Wali AI");
    mainWindow.resize(1200, 800);
    mainWindow.show();

    Logger::instance().log("WaliAI ready.");

    int result = app.exec();

    Logger::instance().log("WaliAI shutting down.");
    return result;
}
