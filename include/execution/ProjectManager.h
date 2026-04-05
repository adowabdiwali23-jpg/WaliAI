#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QVariantMap>

class DatabaseManager;
class SandboxManager;

class ProjectManager : public QObject
{
    Q_OBJECT

public:
    explicit ProjectManager(DatabaseManager &db, SandboxManager &sandbox,
                            QObject *parent = nullptr);

    int createProject(const QString &title, const QString &description,
                      const QString &templateType = "cpp");
    bool deleteProject(int projectId);
    QVector<QVariantMap> listProjects();
    QString projectPath(const QString &title) const;

signals:
    void projectCreated(int id, const QString &title);
    void projectDeleted(int id);

private:
    void createProjectTemplate(const QString &path, const QString &templateType);

    DatabaseManager &m_db;
    SandboxManager &m_sandbox;
};
