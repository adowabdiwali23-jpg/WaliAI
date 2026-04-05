#pragma once

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QVBoxLayout>

class ProjectManager;

class SovereignWorkshopDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SovereignWorkshopDialog(ProjectManager &projects, QWidget *parent = nullptr);

private:
    void setupUi();
    void refreshProjects();
    void onCreateProject();
    void onDeleteProject();

    ProjectManager &m_projects;
    QListWidget *m_projectList = nullptr;
    QLineEdit *m_titleEdit = nullptr;
    QTextEdit *m_descEdit = nullptr;
    QComboBox *m_templateCombo = nullptr;
    QPushButton *m_createBtn = nullptr;
    QPushButton *m_deleteBtn = nullptr;
};
