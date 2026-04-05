#include "service/HistoryService.h"
#include "infrastructure/DatabaseManager.h"
#include "service/Logger.h"

HistoryService::HistoryService(DatabaseManager &db, QObject *parent)
    : QObject(parent), m_db(db)
{
}

int HistoryService::createSession(const QString &title)
{
    int id = m_db.createSession(title);
    if (id > 0) emit sessionCreated(id);
    return id;
}

bool HistoryService::deleteSession(int sessionId)
{
    bool ok = m_db.deleteSession(sessionId);
    if (ok) emit sessionDeleted(sessionId);
    return ok;
}

QVector<QVariantMap> HistoryService::listSessions()
{
    return m_db.listSessions();
}

bool HistoryService::renameSession(int sessionId, const QString &newTitle)
{
    bool ok = m_db.renameSession(sessionId, newTitle);
    if (ok) emit sessionRenamed(sessionId, newTitle);
    return ok;
}

bool HistoryService::addMessage(int sessionId, const QString &role, const QString &content,
                                 bool searchEnabled, bool researchEnabled, bool deepThinkEnabled)
{
    bool ok = m_db.addMessage(sessionId, role, content, searchEnabled, researchEnabled, deepThinkEnabled);
    if (ok) emit messageAdded(sessionId, role, content);
    return ok;
}

QVector<QVariantMap> HistoryService::getMessages(int sessionId)
{
    return m_db.getMessages(sessionId);
}

bool HistoryService::truncateSession(int sessionId, int afterMessageId)
{
    return m_db.truncateSession(sessionId, afterMessageId);
}

int HistoryService::currentSessionId() const { return m_currentSessionId; }

void HistoryService::setCurrentSession(int sessionId)
{
    if (m_currentSessionId != sessionId) {
        m_currentSessionId = sessionId;
        emit currentSessionChanged(sessionId);
    }
}

QString HistoryService::buildConversationContext(int sessionId, int maxMessages)
{
    auto messages = getMessages(sessionId);
    QString context;
    int start = qMax(0, messages.size() - maxMessages);
    for (int i = start; i < messages.size(); ++i) {
        const auto &msg = messages[i];
        QString role = msg["role"].toString();
        QString content = msg["content"].toString();
        context += "<|im_start|>" + role + "\n" + content + "<|im_end|>\n";
    }
    return context;
}
