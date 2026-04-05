#include "execution/ProjectManager.h"
#include "infrastructure/DatabaseManager.h"
#include "execution/SandboxManager.h"
#include "service/Logger.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

ProjectManager::ProjectManager(DatabaseManager &db, SandboxManager &sandbox,
                               QObject *parent)
    : QObject(parent), m_db(db), m_sandbox(sandbox)
{
}

int ProjectManager::createProject(const QString &title, const QString &description,
                                   const QString &templateType)
{
    int id = m_db.createProject(title, description);
    if (id < 0) return -1;

    QString path = projectPath(title);
    QDir().mkpath(path);
    createProjectTemplate(path, templateType);

    emit projectCreated(id, title);
    Logger::instance().log("Project created: " + title + " (" + templateType + ")");
    return id;
}

bool ProjectManager::deleteProject(int projectId)
{
    bool ok = m_db.deleteProject(projectId);
    if (ok) emit projectDeleted(projectId);
    return ok;
}

QVector<QVariantMap> ProjectManager::listProjects()
{
    return m_db.listProjects();
}

QString ProjectManager::projectPath(const QString &title) const
{
    QString safeName = title;
    safeName.replace(' ', '_');
    safeName.replace('/', '_');
    return m_sandbox.projectsPath() + "/" + safeName;
}

void ProjectManager::createProjectTemplate(const QString &path, const QString &templateType)
{
    if (templateType == "cpp") {
        QFile f(path + "/main.cpp");
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&f);
            out << "#include <iostream>\n\n"
                << "int main() {\n"
                << "    std::cout << \"Hello from Wali AI project!\" << std::endl;\n"
                << "    return 0;\n"
                << "}\n";
        }
        QFile cmake(path + "/CMakeLists.txt");
        if (cmake.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&cmake);
            out << "cmake_minimum_required(VERSION 3.16)\n"
                << "project(WaliProject)\n"
                << "set(CMAKE_CXX_STANDARD 20)\n"
                << "add_executable(main main.cpp)\n";
        }
    } else if (templateType == "python") {
        QFile f(path + "/main.py");
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&f);
            out << "#!/usr/bin/env python3\n"
                << "\"\"\"Wali AI Project\"\"\"\n\n"
                << "def main():\n"
                << "    print(\"Hello from Wali AI project!\")\n\n"
                << "if __name__ == \"__main__\":\n"
                << "    main()\n";
        }
    } else if (templateType == "js" || templateType == "javascript") {
        QFile f(path + "/index.js");
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&f);
            out << "// Wali AI Project\n"
                << "console.log(\"Hello from Wali AI project!\");\n";
        }
        QFile pkg(path + "/package.json");
        if (pkg.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&pkg);
            out << "{\n"
                << "  \"name\": \"wali-project\",\n"
                << "  \"version\": \"1.0.0\",\n"
                << "  \"main\": \"index.js\"\n"
                << "}\n";
        }
    }
}
