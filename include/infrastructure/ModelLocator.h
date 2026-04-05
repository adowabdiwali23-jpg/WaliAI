#pragma once

#include <QObject>
#include <QString>

class ModelLocator : public QObject
{
    Q_OBJECT

public:
    explicit ModelLocator(const QString &runtimePath, QObject *parent = nullptr);

    QString llmModelPath() const;
    QString whisperModelPath() const;
    QString piperModelPath() const;
    QString piperConfigPath() const;

    bool llmModelAvailable() const;
    bool whisperModelAvailable() const;
    bool piperModelAvailable() const;

    void setLlmModelPath(const QString &path);
    void setWhisperModelPath(const QString &path);
    void setPiperModelPath(const QString &path);

private:
    QString m_runtimePath;
    QString m_llmModelPath;
    QString m_whisperModelPath;
    QString m_piperModelPath;
    QString m_piperConfigPath;

    void autoDetectModels();
};
