#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QVariantMap>

class DatabaseManager;

class MemoryService : public QObject
{
    Q_OBJECT

public:
    explicit MemoryService(DatabaseManager &db, QObject *parent = nullptr);

    bool store(const QString &key, const QString &value, const QString &category = "general");
    bool remove(int memoryId);
    QVector<QVariantMap> search(const QString &query);
    QVector<QVariantMap> all();
    QString recall(const QString &query);
    QString buildContextBlock(const QString &query, int maxMemories = 5);

signals:
    void memoryStored(const QString &key);
    void memoryRemoved(int memoryId);

private:
    DatabaseManager &m_db;
};
