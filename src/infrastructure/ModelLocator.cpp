#include "infrastructure/ModelLocator.h"
#include "service/Logger.h"

#include <QDir>
#include <QFileInfo>

ModelLocator::ModelLocator(const QString &runtimePath, QObject *parent)
    : QObject(parent), m_runtimePath(runtimePath)
{
    autoDetectModels();
}

void ModelLocator::autoDetectModels()
{
    QDir modelsDir(m_runtimePath + "/models");
    if (!modelsDir.exists()) {
        modelsDir.mkpath(".");
    }

    // Look for GGUF files (llama.cpp models)
    // Preferred model: Qwen2.5-Coder-7B-Instruct-abliterated-Q4_K_L.gguf
    static const QString kPreferredModel = "Qwen2.5-Coder-7B-Instruct-abliterated-Q4_K_L.gguf";

    QStringList ggufFiles = modelsDir.entryList({"*.gguf"}, QDir::Files);
    if (!ggufFiles.isEmpty()) {
        // Prioritize the preferred Qwen model if present
        if (ggufFiles.contains(kPreferredModel)) {
            m_llmModelPath = modelsDir.absoluteFilePath(kPreferredModel);
            Logger::instance().log("Found preferred LLM model: " + m_llmModelPath);
        } else {
            m_llmModelPath = modelsDir.absoluteFilePath(ggufFiles.first());
            Logger::instance().log("Found LLM model: " + m_llmModelPath);
            Logger::instance().warning(
                "Preferred model '" + kPreferredModel + "' not found. "
                "Using '" + ggufFiles.first() + "' instead.");
        }
    } else {
        Logger::instance().warning(
            "No GGUF model found. Place '" + kPreferredModel +
            "' in " + modelsDir.absolutePath() + "/");
    }

    // Look for Whisper models
    QStringList whisperFiles = modelsDir.entryList({"ggml-*.bin", "whisper-*"}, QDir::Files);
    if (!whisperFiles.isEmpty()) {
        m_whisperModelPath = modelsDir.absoluteFilePath(whisperFiles.first());
        Logger::instance().log("Found Whisper model: " + m_whisperModelPath);
    }

    // Look for Piper models
    QStringList piperFiles = modelsDir.entryList({"*.onnx"}, QDir::Files);
    if (!piperFiles.isEmpty()) {
        m_piperModelPath = modelsDir.absoluteFilePath(piperFiles.first());
        // Look for corresponding config
        QString configName = QFileInfo(m_piperModelPath).baseName() + ".onnx.json";
        if (QFileInfo::exists(modelsDir.absoluteFilePath(configName))) {
            m_piperConfigPath = modelsDir.absoluteFilePath(configName);
        }
        Logger::instance().log("Found Piper model: " + m_piperModelPath);
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
