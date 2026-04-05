#pragma once

#include <QObject>
#include <QString>
#include <QFile>
#include <QMutex>

class Logger : public QObject
{
    Q_OBJECT

public:
    enum class Level {
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    };

    static Logger &instance();

    void init(const QString &logDir);
    void log(const QString &message, Level level = Level::Info);
    void debug(const QString &message);
    void warning(const QString &message);
    void error(const QString &message);

    QString logFilePath() const;

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    QString levelToString(Level level) const;

    QFile m_logFile;
    QMutex m_mutex;
    QString m_logDir;
    bool m_initialized = false;
};
