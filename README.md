# Wali AI

A fully self-contained desktop AI assistant built with Qt6 and C++20. Features local inference, voice interaction, persistent memory, chat history, web research, and sandboxed project management.

## Architecture

| Layer | Responsibility | Components |
|-------|---------------|------------|
| **UI Layer** | Chat & voice interface | MainWindow, ChatWidget, InputBarWidget, SidebarWidget, CognitiveSuiteWidget, SettingsDialog, SovereignWorkshopDialog |
| **Control Layer** | Orchestration | Controller, StateManager |
| **State Layer** | Runtime toggles | CognitiveState, PersonalizationState |
| **Intelligence Layer** | Prompt construction, inference | CognitionEngine, ModelInference (Qwen2.5-Coder-7B via llama.cpp) |
| **Service Layer** | Persistence & web | MemoryService, HistoryService, HiddenBrowser, Logger |
| **Execution Layer** | Sandboxed commands & projects | SandboxManager, CommandExecutor, FileSystemGuard, PermissionManager, ProjectManager, SystemInspector |
| **Voice Layer** | STT/TTS | VoiceService (Whisper + Piper stubs) |
| **Infrastructure Layer** | Build & runtime | DatabaseManager, ModelLocator, CMake build system |

## Prerequisites

- CMake 3.16+
- C++20 compatible compiler (GCC 11+, Clang 14+)
- Qt6 (Core, Gui, Widgets, Sql, Network, Multimedia)
- SQLite3
- OpenGL development headers

### Ubuntu/Debian

```bash
sudo apt-get install cmake qt6-base-dev qt6-multimedia-dev qt6-tools-dev \
    libgl-dev libglx-dev libegl-dev libsqlite3-dev pkg-config
```

## Build

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

The compiled binary will be at `build/WaliAI`.

## Run

```bash
./build/WaliAI
```

On first launch, the application creates its runtime directory at:
```
~/.local/share/WaliAI/runtime/
├── workspace/
├── projects/
├── temp/
├── logs/
└── wali.db
```

## Features

- **Local Chat**: Conversational AI powered by Qwen2.5-Coder-7B-Instruct via llama.cpp (ChatML prompt format)
- **Web Research**: Built-in headless browser for fetching and summarizing web content
- **Voice Interaction**: Speech-to-text (Whisper) and text-to-speech (Piper) integration points
- **Persistent Memory**: SQLite-backed memory system for storing and recalling user context
- **Chat History**: Full conversation history with session management
- **Sandboxed Execution**: Safe command execution with filesystem guards and dangerous command blocking
- **Project Management**: Create and manage code projects within a sandboxed environment
- **Cognitive Suite**: Toggle features on/off (web access, voice, sandbox, memory, streaming)
- **Dark Theme**: Modern dark UI with a clean chat interface

## Adding Models

Place model files in `~/.local/share/WaliAI/runtime/models/`:

- **LLM (required)**: `Qwen2.5-Coder-7B-Instruct-abliterated-Q4_K_L.gguf` (preferred, auto-detected)
- **Whisper**: Whisper model file (e.g., `ggml-base.bin`)
- **Piper**: ONNX voice model + JSON config (e.g., `en_US-lessac-medium.onnx`)

## Database Schema

- `chat_sessions` — conversation sessions
- `chat_messages` — messages within sessions (user/assistant/system)
- `memories` — key-value persistent memory entries
- `projects` — managed sandbox projects
- `settings` — user preferences
- `schema_version` — migration tracking

## Security

- All file operations restricted to the sandbox (`~/.local/share/WaliAI/runtime/`)
- System directories (`/etc`, `/usr`, `/bin`, `/root`, etc.) are blocked
- Dangerous commands (`sudo`, `rm -rf /`, `mkfs`, etc.) are intercepted
- Optional user confirmation for risky operations

## License

MIT
