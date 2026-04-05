cmake_minimum_required(VERSION 3.16)
project(WaliAI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    # Main
    src/main.cpp

    # State Layer
    src/state/CognitiveState.cpp
    src/state/PersonalizationState.cpp
    src/state/AgentState.cpp

    # Control Layer
    src/control/Controller.cpp
    src/control/StateManager.cpp

    # Intelligence Layer
    src/intelligence/CognitionEngine.cpp
    src/intelligence/IntentClassifier.cpp
    src/intelligence/ModelInference.cpp

    # Service Layer
    src/service/MemoryService.cpp
    src/service/HistoryService.cpp
    src/service/HiddenBrowser.cpp
    src/service/Logger.cpp

    # Execution Layer
    src/execution/SandboxManager.cpp
    src/execution/CommandExecutor.cpp
    src/execution/FileSystemGuard.cpp
    src/execution/PermissionManager.cpp
    src/execution/ProjectManager.cpp
    src/execution/SystemInspector.cpp

    # Voice Layer
    src/voice/VoiceService.cpp

    # Infrastructure Layer
    src/infrastructure/DatabaseManager.cpp
    src/infrastructure/ModelLocator.cpp

    # UI Layer
    src/ui/MainWindow.cpp
    src/ui/ChatWidget.cpp
    src/ui/InputBarWidget.cpp
    src/ui/SidebarWidget.cpp
    src/ui/CognitiveSuiteWidget.cpp
    src/ui/TerminalPanel.cpp
    src/ui/FileExplorer.cpp
    src/ui/HistoryDialog.cpp
    src/ui/SettingsDialog.cpp
    src/ui/SovereignWorkshopDialog.cpp
)

set(HEADERS
    # State Layer
    include/state/CognitiveState.h
    include/state/PersonalizationState.h
    include/state/AgentState.h

    # Control Layer
    include/control/Controller.h
    include/control/StateManager.h

    # Intelligence Layer
    include/intelligence/CognitionEngine.h
    include/intelligence/IntentClassifier.h
    include/intelligence/ModelInference.h

    # Service Layer
    include/service/MemoryService.h
    include/service/HistoryService.h
    include/service/HiddenBrowser.h
    include/service/Logger.h

    # Execution Layer
    include/execution/SandboxManager.h
    include/execution/CommandExecutor.h
    include/execution/FileSystemGuard.h
    include/execution/PermissionManager.h
    include/execution/ProjectManager.h
    include/execution/SystemInspector.h

    # Voice Layer
    include/voice/VoiceService.h

    # Infrastructure Layer
    include/infrastructure/DatabaseManager.h
    include/infrastructure/ModelLocator.h

    # UI Layer
    include/ui/MainWindow.h
    include/ui/ChatWidget.h
    include/ui/InputBarWidget.h
    include/ui/SidebarWidget.h
    include/ui/CognitiveSuiteWidget.h
    include/ui/TerminalPanel.h
    include/ui/FileExplorer.h
    include/ui/HistoryDialog.h
    include/ui/SettingsDialog.h
    include/ui/SovereignWorkshopDialog.h
)

add_executable(WaliAI ${SOURCES} ${HEADERS})

target_link_libraries(WaliAI PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
)

target_compile_definitions(WaliAI PRIVATE
    QT_DEPRECATED_WARNINGS
)
