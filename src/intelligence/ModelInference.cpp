#include "intelligence/ModelInference.h"
#include "infrastructure/ModelLocator.h"
#include "service/Logger.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

ModelInference::ModelInference(ModelLocator &locator, QObject *parent)
    : QObject(parent), m_locator(locator)
{
}

void ModelInference::generate(const QString &prompt, int maxTokens, double temperature)
{
    if (m_generating) {
        Logger::instance().warning("Already generating, ignoring request");
        return;
    }

    QString llamaBinary = findLlamaBinary();
    QString modelPath = m_locator.llmModelPath();

    if (modelPath.isEmpty()) {
        QString msg = "[Wali AI] No model file found. Please place a GGUF model in: " +
                      m_locator.llmModelPath().section('/', 0, -2);
        emit tokenGenerated(msg);
        emit generationFinished(msg);
        return;
    }

    if (llamaBinary.isEmpty()) {
        // Stub mode: echo response
        QString msg = "[Wali AI - Stub Mode] Model inference requires llama.cpp binary. "
                      "Model configured: " + QFileInfo(modelPath).fileName() + "\n"
                      "Build llama.cpp and place the binary alongside WaliAI.";
        emit tokenGenerated(msg);
        emit generationFinished(msg);
        return;
    }

    m_response.clear();
    m_generating = true;

    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &ModelInference::onReadyRead);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ModelInference::onProcessFinished);

    QStringList args;
    args << "-m" << modelPath
         << "-p" << prompt
         << "-n" << QString::number(maxTokens)
         << "--temp" << QString::number(temperature)
         << "-no-cnv"
         << "--no-display-prompt";

    Logger::instance().log("Starting inference: " + llamaBinary);
    m_process->start(llamaBinary, args);
}

void ModelInference::stop()
{
    if (m_process && m_generating) {
        m_process->kill();
        m_generating = false;
    }
}

bool ModelInference::isGenerating() const { return m_generating; }

void ModelInference::onReadyRead()
{
    if (!m_process) return;
    QString output = QString::fromUtf8(m_process->readAllStandardOutput());
    m_response += output;
    emit tokenGenerated(output);
}

void ModelInference::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)
    m_generating = false;
    emit generationFinished(m_response);
    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
}

QString ModelInference::findLlamaBinary() const
{
    // Check environment variable
    QString envPath = qEnvironmentVariable("WALIAI_LLAMA_CLI");
    if (!envPath.isEmpty() && QFileInfo::exists(envPath)) return envPath;

    // Check next to application binary
    QString appDir = QCoreApplication::applicationDirPath();
    QStringList names = {"llama-cli", "llama-completion", "main"};
    for (const auto &name : names) {
        QString path = appDir + "/" + name;
        if (QFileInfo::exists(path)) return path;
    }

    // Check PATH
    for (const auto &name : names) {
        QProcess which;
        which.start("which", QStringList() << name);
        if (which.waitForFinished(3000) && which.exitCode() == 0) {
            return QString::fromUtf8(which.readAllStandardOutput()).trimmed();
        }
    }

    return QString();
}
