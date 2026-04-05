#include "service/MemoryService.h"
#include "infrastructure/DatabaseManager.h"
#include "service/Logger.h"

MemoryService::MemoryService(DatabaseManager &db, QObject *parent)
    : QObject(parent), m_db(db)
{
}

bool MemoryService::store(const QString &key, const QString &value, const QString &category)
{
    Q_UNUSED(category)
    QString content = key + ": " + value;
    bool ok = m_db.addMemory(content);
    if (ok) emit memoryStored(key);
    return ok;
}

bool MemoryService::remove(int memoryId)
{
    bool ok = m_db.deleteMemory(memoryId);
    if (ok) emit memoryRemoved(memoryId);
    return ok;
}

QVector<QVariantMap> MemoryService::search(const QString &query)
{
    return m_db.searchMemories(query);
}

QVector<QVariantMap> MemoryService::all()
{
    return m_db.getAllMemories();
}

QString MemoryService::recall(const QString &query)
{
    auto results = search(query);
    if (results.isEmpty()) return QString();
    return results.first()["content"].toString();
}

QString MemoryService::buildContextBlock(const QString &query, int maxMemories)
{
    auto results = search(query);
    if (results.isEmpty()) return QString();

    QString block;
    int count = qMin(results.size(), maxMemories);
    for (int i = 0; i < count; ++i) {
        block += "- " + results[i]["content"].toString() + "\n";
    }
    return block;
}
