#pragma once

#include <QObject>
#include <QString>
#include <QProcess>

class ModelLocator;

class ModelInference : public QObject
{
    Q_OBJECT

public:
    explicit ModelInference(ModelLocator &locator, QObject *parent = nullptr);

    void generate(const QString &prompt, int maxTokens = 2048, double temperature = 0.7);
    void stop();
    bool isGenerating() const;

signals:
    void tokenGenerated(const QString &token);
    void generationFinished(const QString &fullResponse);
    void errorOccurred(const QString &error);

private slots:
    void onReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString findLlamaBinary() const;

    ModelLocator &m_locator;
    QProcess *m_process = nullptr;
    QString m_response;
    bool m_generating = false;
};
