#include "service/Logger.h"

#include <QDateTime>
#include <QDir>
#include <QTextStream>
#include <QMutexLocker>

Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

Logger::~Logger()
{
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

void Logger::init(const QString &logDir)
{
    QMutexLocker locker(&m_mutex);
    m_logDir = logDir;
    QDir().mkpath(logDir);

    QString fileName = logDir + "/wali_" +
                        QDateTime::currentDateTime().toString("yyyy-MM-dd") + ".log";
    m_logFile.setFileName(fileName);
    m_initialized = m_logFile.open(QIODevice::Append | QIODevice::Text);
}

void Logger::log(const QString &message, Level level)
{
    QMutexLocker locker(&m_mutex);
    if (!m_initialized) return;

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString line = QString("[%1] [%2] %3\n").arg(timestamp, levelToString(level), message);

    QTextStream stream(&m_logFile);
    stream << line;
    stream.flush();
}

void Logger::debug(const QString &message) { log(message, Level::Debug); }
void Logger::warning(const QString &message) { log(message, Level::Warning); }
void Logger::error(const QString &message) { log(message, Level::Error); }

QString Logger::logFilePath() const { return m_logFile.fileName(); }

QString Logger::levelToString(Level level) const
{
    switch (level) {
        case Level::Debug:   return "DEBUG";
        case Level::Info:    return "INFO";
        case Level::Warning: return "WARN";
        case Level::Error:   return "ERROR";
        case Level::Fatal:   return "FATAL";
    }
    return "UNKNOWN";
}
