#include "ui/SovereignWorkshopDialog.h"
#include "execution/ProjectManager.h"

#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>

SovereignWorkshopDialog::SovereignWorkshopDialog(ProjectManager &projects,
                                                   QWidget *parent)
    : QDialog(parent), m_projects(projects)
{
    setWindowTitle("Sovereign Workshop");
    setMinimumSize(500, 500);
    setupUi();
    refreshProjects();
}

void SovereignWorkshopDialog::setupUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    auto *titleLabel = new QLabel("Sovereign Workshop");
    titleLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; color: #e2e8f0; }");
    layout->addWidget(titleLabel);

    // Create project section
    auto *createGroup = new QGroupBox("New Project");
    createGroup->setStyleSheet(
        "QGroupBox { border: 1px solid #4a5568; border-radius: 6px; "
        "margin-top: 8px; padding-top: 16px; color: #e2e8f0; }"
    );
    auto *createLayout = new QVBoxLayout(createGroup);

    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText("Project name...");
    createLayout->addWidget(m_titleEdit);

    m_descEdit = new QTextEdit(this);
    m_descEdit->setPlaceholderText("Description...");
    m_descEdit->setMaximumHeight(60);
    createLayout->addWidget(m_descEdit);

    auto *templateLayout = new QHBoxLayout();
    m_templateCombo = new QComboBox(this);
    m_templateCombo->addItems({"cpp", "python", "javascript"});
    m_createBtn = new QPushButton("Create Project", this);
    m_createBtn->setStyleSheet(
        "QPushButton { background-color: #48bb78; color: white; "
        "border-radius: 6px; padding: 8px 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #38a169; }"
    );
    templateLayout->addWidget(m_templateCombo);
    templateLayout->addWidget(m_createBtn);
    createLayout->addLayout(templateLayout);
    layout->addWidget(createGroup);

    // Project list
    m_projectList = new QListWidget(this);
    m_projectList->setStyleSheet(
        "QListWidget { background-color: #1a202c; border: 1px solid #4a5568; "
        "color: #e2e8f0; border-radius: 6px; }"
        "QListWidget::item { padding: 8px; }"
        "QListWidget::item:selected { background-color: #2d3748; }"
    );
    layout->addWidget(m_projectList, 1);

    // Bottom buttons
    auto *btnLayout = new QHBoxLayout();
    m_deleteBtn = new QPushButton("Delete Project", this);
    m_deleteBtn->setStyleSheet(
        "QPushButton { background-color: #e53e3e; color: white; "
        "border-radius: 6px; padding: 8px 16px; }"
        "QPushButton:hover { background-color: #c53030; }"
    );
    auto *closeBtn = new QPushButton("Close", this);
    closeBtn->setStyleSheet(
        "QPushButton { background-color: #4a5568; color: white; "
        "border-radius: 6px; padding: 8px 16px; }"
        "QPushButton:hover { background-color: #718096; }"
    );
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(m_createBtn, &QPushButton::clicked, this, &SovereignWorkshopDialog::onCreateProject);
    connect(m_deleteBtn, &QPushButton::clicked, this, &SovereignWorkshopDialog::onDeleteProject);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    setStyleSheet(
        "QDialog { background-color: #171923; }"
        "QLineEdit, QTextEdit, QComboBox { background-color: #2d3748; color: #e2e8f0; "
        "border: 1px solid #4a5568; border-radius: 4px; padding: 6px; }"
    );
}

void SovereignWorkshopDialog::refreshProjects()
{
    m_projectList->clear();
    auto projects = m_projects.listProjects();
    for (const auto &proj : projects) {
        auto *item = new QListWidgetItem(proj["title"].toString());
        item->setData(Qt::UserRole, proj["id"]);
        m_projectList->addItem(item);
    }
}

void SovereignWorkshopDialog::onCreateProject()
{
    QString title = m_titleEdit->text().trimmed();
    if (title.isEmpty()) return;

    QString desc = m_descEdit->toPlainText().trimmed();
    QString tmpl = m_templateCombo->currentText();

    m_projects.createProject(title, desc, tmpl);
    m_titleEdit->clear();
    m_descEdit->clear();
    refreshProjects();
}

void SovereignWorkshopDialog::onDeleteProject()
{
    auto *item = m_projectList->currentItem();
    if (!item) return;

    int id = item->data(Qt::UserRole).toInt();
    auto reply = QMessageBox::question(this, "Delete Project",
        "Are you sure you want to delete this project?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_projects.deleteProject(id);
        refreshProjects();
    }
}
