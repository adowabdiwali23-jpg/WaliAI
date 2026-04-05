#include "infrastructure/DatabaseManager.h"
#include "state/PersonalizationState.h"
#include "service/Logger.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QVariant>

DatabaseManager::DatabaseManager(const QString &dbPath, QObject *parent)
    : QObject(parent), m_dbPath(dbPath)
{
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::initialize()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", "wali-ai");
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        Logger::instance().error("Failed to open database: " + m_db.lastError().text());
        return false;
    }

    QSqlQuery query(m_db);
    query.exec("PRAGMA foreign_keys = ON");

    if (!createTables()) {
        Logger::instance().error("Failed to create database tables");
        return false;
    }

    Logger::instance().log("Database initialized at: " + m_dbPath);
    return true;
}

void DatabaseManager::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_db);

    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS schema_version ("
        "  version INTEGER PRIMARY KEY"
        ")"
    );
    if (!ok) return false;

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS chat_sessions ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  title TEXT NOT NULL DEFAULT 'New Chat',"
        "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "  updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "  active INTEGER DEFAULT 0"
        ")"
    );
    if (!ok) return false;

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS chat_messages ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  session_id INTEGER NOT NULL REFERENCES chat_sessions(id) ON DELETE CASCADE,"
        "  role TEXT NOT NULL CHECK(role IN ('user', 'assistant', 'system')),"
        "  content TEXT NOT NULL,"
        "  timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "  search_enabled INTEGER DEFAULT 0,"
        "  research_enabled INTEGER DEFAULT 0,"
        "  deep_think_enabled INTEGER DEFAULT 0"
        ")"
    );
    if (!ok) return false;

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS memories ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  content TEXT NOT NULL,"
        "  timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ")"
    );
    if (!ok) return false;

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS projects ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  title TEXT NOT NULL,"
        "  description TEXT,"
        "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "  updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ")"
    );
    if (!ok) return false;

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS settings ("
        "  key TEXT PRIMARY KEY,"
        "  value TEXT"
        ")"
    );
    if (!ok) return false;

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS agent_logs ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  action TEXT NOT NULL,"
        "  command TEXT,"
        "  result TEXT,"
        "  exit_code INTEGER,"
        "  timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ")"
    );
    if (!ok) return false;

    query.exec("INSERT OR IGNORE INTO schema_version (version) VALUES (1)");
    return true;
}

bool DatabaseManager::migrateSchema()
{
    return true;
}

int DatabaseManager::createSession(const QString &title)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO chat_sessions (title) VALUES (:title)");
    query.bindValue(":title", title);
    if (query.exec()) {
        return query.lastInsertId().toInt();
    }
    return -1;
}

bool DatabaseManager::deleteSession(int sessionId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM chat_sessions WHERE id = :id");
    query.bindValue(":id", sessionId);
    return query.exec() && query.numRowsAffected() > 0;
}

QVector<QVariantMap> DatabaseManager::listSessions()
{
    QVector<QVariantMap> sessions;
    QSqlQuery query(m_db);
    query.exec("SELECT id, title, created_at, updated_at FROM chat_sessions ORDER BY updated_at DESC");
    while (query.next()) {
        QVariantMap session;
        session["id"] = query.value(0);
        session["title"] = query.value(1);
        session["created_at"] = query.value(2);
        session["updated_at"] = query.value(3);
        sessions.append(session);
    }
    return sessions;
}

bool DatabaseManager::renameSession(int sessionId, const QString &newTitle)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE chat_sessions SET title = :title, updated_at = CURRENT_TIMESTAMP WHERE id = :id");
    query.bindValue(":title", newTitle);
    query.bindValue(":id", sessionId);
    return query.exec() && query.numRowsAffected() > 0;
}

bool DatabaseManager::addMessage(int sessionId, const QString &role, const QString &content,
                                  bool searchEnabled, bool researchEnabled, bool deepThinkEnabled)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO chat_messages (session_id, role, content, search_enabled, research_enabled, deep_think_enabled) "
        "VALUES (:sid, :role, :content, :search, :research, :deepthink)"
    );
    query.bindValue(":sid", sessionId);
    query.bindValue(":role", role);
    query.bindValue(":content", content);
    query.bindValue(":search", searchEnabled ? 1 : 0);
    query.bindValue(":research", researchEnabled ? 1 : 0);
    query.bindValue(":deepthink", deepThinkEnabled ? 1 : 0);
    bool ok = query.exec();
    if (ok) {
        QSqlQuery update(m_db);
        update.prepare("UPDATE chat_sessions SET updated_at = CURRENT_TIMESTAMP WHERE id = :id");
        update.bindValue(":id", sessionId);
        update.exec();
    }
    return ok;
}

QVector<QVariantMap> DatabaseManager::getMessages(int sessionId)
{
    QVector<QVariantMap> messages;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT id, role, content, timestamp, search_enabled, research_enabled, deep_think_enabled "
        "FROM chat_messages WHERE session_id = :sid ORDER BY id ASC"
    );
    query.bindValue(":sid", sessionId);
    query.exec();
    while (query.next()) {
        QVariantMap msg;
        msg["id"] = query.value(0);
        msg["role"] = query.value(1);
        msg["content"] = query.value(2);
        msg["timestamp"] = query.value(3);
        msg["search_enabled"] = query.value(4);
        msg["research_enabled"] = query.value(5);
        msg["deep_think_enabled"] = query.value(6);
        messages.append(msg);
    }
    return messages;
}

bool DatabaseManager::truncateSession(int sessionId, int afterMessageId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM chat_messages WHERE session_id = :sid AND id > :mid");
    query.bindValue(":sid", sessionId);
    query.bindValue(":mid", afterMessageId);
    return query.exec();
}

bool DatabaseManager::addMemory(const QString &content)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO memories (content) VALUES (:content)");
    query.bindValue(":content", content);
    return query.exec();
}

bool DatabaseManager::deleteMemory(int memoryId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM memories WHERE id = :id");
    query.bindValue(":id", memoryId);
    return query.exec();
}

QVector<QVariantMap> DatabaseManager::searchMemories(const QString &queryStr)
{
    QVector<QVariantMap> results;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, content, timestamp FROM memories "
                  "WHERE content LIKE :q ORDER BY timestamp DESC");
    query.bindValue(":q", "%" + queryStr + "%");
    query.exec();
    while (query.next()) {
        QVariantMap mem;
        mem["id"] = query.value(0);
        mem["content"] = query.value(1);
        mem["timestamp"] = query.value(2);
        results.append(mem);
    }
    return results;
}

QVector<QVariantMap> DatabaseManager::getAllMemories()
{
    QVector<QVariantMap> results;
    QSqlQuery query(m_db);
    query.exec("SELECT id, content, timestamp FROM memories ORDER BY timestamp DESC");
    while (query.next()) {
        QVariantMap mem;
        mem["id"] = query.value(0);
        mem["content"] = query.value(1);
        mem["timestamp"] = query.value(2);
        results.append(mem);
    }
    return results;
}

int DatabaseManager::createProject(const QString &title, const QString &description)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO projects (title, description) VALUES (:title, :desc)");
    query.bindValue(":title", title);
    query.bindValue(":desc", description);
    if (query.exec()) {
        return query.lastInsertId().toInt();
    }
    return -1;
}

bool DatabaseManager::deleteProject(int projectId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM projects WHERE id = :id");
    query.bindValue(":id", projectId);
    return query.exec();
}

QVector<QVariantMap> DatabaseManager::listProjects()
{
    QVector<QVariantMap> projects;
    QSqlQuery query(m_db);
    query.exec("SELECT id, title, description, created_at, updated_at FROM projects ORDER BY updated_at DESC");
    while (query.next()) {
        QVariantMap proj;
        proj["id"] = query.value(0);
        proj["title"] = query.value(1);
        proj["description"] = query.value(2);
        proj["created_at"] = query.value(3);
        proj["updated_at"] = query.value(4);
        projects.append(proj);
    }
    return projects;
}

bool DatabaseManager::addAgentLog(const QString &action, const QString &command,
                                   const QString &result, int exitCode)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO agent_logs (action, command, result, exit_code) "
        "VALUES (:action, :command, :result, :exit_code)"
    );
    query.bindValue(":action", action);
    query.bindValue(":command", command);
    query.bindValue(":result", result);
    query.bindValue(":exit_code", exitCode);
    return query.exec();
}

QVector<QVariantMap> DatabaseManager::getAgentLogs(int limit)
{
    QVector<QVariantMap> logs;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT id, action, command, result, exit_code, timestamp "
        "FROM agent_logs ORDER BY timestamp DESC LIMIT :limit"
    );
    query.bindValue(":limit", limit);
    query.exec();
    while (query.next()) {
        QVariantMap log;
        log["id"] = query.value(0);
        log["action"] = query.value(1);
        log["command"] = query.value(2);
        log["result"] = query.value(3);
        log["exit_code"] = query.value(4);
        log["timestamp"] = query.value(5);
        logs.append(log);
    }
    return logs;
}

bool DatabaseManager::saveSetting(const QString &key, const QString &value)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (:key, :value)");
    query.bindValue(":key", key);
    query.bindValue(":value", value);
    return query.exec();
}

QString DatabaseManager::loadSetting(const QString &key, const QString &defaultValue)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT value FROM settings WHERE key = :key");
    query.bindValue(":key", key);
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return defaultValue;
}

void DatabaseManager::loadSettings(PersonalizationState &state)
{
    state.setUserName(loadSetting("user_name", "User"));
    state.setSystemPrompt(loadSetting("system_prompt", "You are Wali, a helpful AI assistant."));
    state.setTheme(loadSetting("theme", "dark"));
    state.setMaxTokens(loadSetting("max_tokens", "2048").toInt());
    state.setTemperature(loadSetting("temperature", "0.7").toDouble());
    state.setContextWindowSize(loadSetting("context_window", "4096").toInt());
    state.setVoiceProfile(loadSetting("voice_profile", "breeze"));
    state.setMemoryEnabled(loadSetting("memory_enabled", "1").toInt() != 0);
    state.setTtsEnabled(loadSetting("tts_enabled", "0").toInt() != 0);
    state.setUiMaxMessages(loadSetting("ui_max_messages", "500").toInt());
}

void DatabaseManager::saveSettings(const PersonalizationState &state)
{
    saveSetting("user_name", state.userName());
    saveSetting("system_prompt", state.systemPrompt());
    saveSetting("theme", state.theme());
    saveSetting("max_tokens", QString::number(state.maxTokens()));
    saveSetting("temperature", QString::number(state.temperature()));
    saveSetting("context_window", QString::number(state.contextWindowSize()));
    saveSetting("voice_profile", state.voiceProfile());
    saveSetting("memory_enabled", state.memoryEnabled() ? "1" : "0");
    saveSetting("tts_enabled", state.ttsEnabled() ? "1" : "0");
    saveSetting("ui_max_messages", QString::number(state.uiMaxMessages()));
}
