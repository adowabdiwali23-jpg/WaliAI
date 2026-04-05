#include "infrastructure/ModelLocator.h"
#include "service/Logger.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

ModelLocator::ModelLocator(const QString &runtimePath, QObject *parent)
    : QObject(parent), m_runtimePath(runtimePath)
{
    autoDetectModels();
}

void ModelLocator::autoDetectModels()
{
    // Preferred model filename
    static const QString kPreferredModel = "Qwen2.5-Coder-7B-Instruct-abliterated-Q4_K_L.gguf";

    // Build a list of directories to search for models, in priority order:
    //   1. Runtime models dir (~/.local/share/WaliAI/runtime/models/)
    //   2. Repo-local models/ dir (next to binary or in source tree)
    //   3. App binary directory
    QStringList searchDirs;
    QString runtimeModels = m_runtimePath + "/models";
    QDir().mkpath(runtimeModels);
    searchDirs << runtimeModels;

    // Check for a models/ dir next to the application binary
    QString appDir = QCoreApplication::applicationDirPath();
    QString appModels = appDir + "/models";
    if (QDir(appModels).exists() && !searchDirs.contains(appModels)) {
        searchDirs << appModels;
    }

    // Check for a models/ dir relative to the source tree (e.g. when running
    // from the build/ subdirectory of the repo)
    QString parentModels = QDir(appDir + "/..").canonicalPath() + "/models";
    if (QDir(parentModels).exists() && !searchDirs.contains(parentModels)) {
        searchDirs << parentModels;
    }

    // Also check the binary directory itself (model placed alongside binary)
    if (!searchDirs.contains(appDir)) {
        searchDirs << appDir;
    }

    // Scan all search directories for GGUF files
    for (const auto &dirPath : searchDirs) {
        QDir dir(dirPath);
        if (!dir.exists()) continue;

        QStringList ggufFiles = dir.entryList({"*.gguf"}, QDir::Files);
        if (ggufFiles.isEmpty()) continue;

        if (ggufFiles.contains(kPreferredModel)) {
            m_llmModelPath = dir.absoluteFilePath(kPreferredModel);
            Logger::instance().log("Found preferred LLM model: " + m_llmModelPath);
            break;
        } else if (m_llmModelPath.isEmpty()) {
            // Use the first GGUF found as fallback, but keep searching for preferred
            m_llmModelPath = dir.absoluteFilePath(ggufFiles.first());
            Logger::instance().log("Found LLM model: " + m_llmModelPath);
            Logger::instance().warning(
                "Preferred model '" + kPreferredModel + "' not found in " +
                dirPath + ". Using '" + ggufFiles.first() + "' instead.");
        }
    }

    if (m_llmModelPath.isEmpty()) {
        Logger::instance().warning(
            "No GGUF model found in any search path. Place '" + kPreferredModel +
            "' in one of: " + searchDirs.join(", "));
    }

    // Look for Whisper and Piper models across all search directories
    for (const auto &dirPath : searchDirs) {
        QDir dir(dirPath);
        if (!dir.exists()) continue;

        // Whisper models (ggml-*.bin or whisper-*)
        if (m_whisperModelPath.isEmpty()) {
            QStringList whisperFiles = dir.entryList({"ggml-*.bin", "whisper-*"}, QDir::Files);
            if (!whisperFiles.isEmpty()) {
                m_whisperModelPath = dir.absoluteFilePath(whisperFiles.first());
                Logger::instance().log("Found Whisper model: " + m_whisperModelPath);
            }
        }

        // Piper models (*.onnx)
        if (m_piperModelPath.isEmpty()) {
            QStringList piperFiles = dir.entryList({"*.onnx"}, QDir::Files);
            if (!piperFiles.isEmpty()) {
                m_piperModelPath = dir.absoluteFilePath(piperFiles.first());
                QString configName = QFileInfo(m_piperModelPath).baseName() + ".onnx.json";
                if (QFileInfo::exists(dir.absoluteFilePath(configName))) {
                    m_piperConfigPath = dir.absoluteFilePath(configName);
                }
                Logger::instance().log("Found Piper model: " + m_piperModelPath);
            }
        }
    }
}

QString ModelLocator::llmModelPath() const { return m_llmModelPath; }
QString ModelLocator::whisperModelPath() const { return m_whisperModelPath; }
QString ModelLocator::piperModelPath() const { return m_piperModelPath; }
QString ModelLocator::piperConfigPath() const { return m_piperConfigPath; }

bool ModelLocator::llmModelAvailable() const { return QFileInfo::exists(m_llmModelPath); }
bool ModelLocator::whisperModelAvailable() const { return QFileInfo::exists(m_whisperModelPath); }
bool ModelLocator::piperModelAvailable() const { return QFileInfo::exists(m_piperModelPath); }

void ModelLocator::setLlmModelPath(const QString &path) { m_llmModelPath = path; }
void ModelLocator::setWhisperModelPath(const QString &path) { m_whisperModelPath = path; }
void ModelLocator::setPiperModelPath(const QString &path) { m_piperModelPath = path; }
