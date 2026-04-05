#include "ui/HistoryDialog.h"
#include "service/HistoryService.h"

#include <QMessageBox>

HistoryDialog::HistoryDialog(HistoryService &history, QWidget *parent)
    : QDialog(parent), m_history(history)
{
    setWindowTitle("Chat History");
    setMinimumSize(400, 500);
    setupUi();
    refreshList();
}

void HistoryDialog::setupUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(8);

    auto *titleLabel = new QLabel("Session History");
    titleLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; color: #e2e8f0; }");
    layout->addWidget(titleLabel);

    m_sessionList = new QListWidget(this);
    m_sessionList->setStyleSheet(
        "QListWidget { background-color: #1a202c; border: 1px solid #4a5568; "
        "color: #e2e8f0; border-radius: 6px; }"
        "QListWidget::item { padding: 8px; }"
        "QListWidget::item:selected { background-color: #2d3748; }"
    );
    layout->addWidget(m_sessionList, 1);

    // Rename section
    auto *renameLayout = new QHBoxLayout();
    m_renameEdit = new QLineEdit(this);
    m_renameEdit->setPlaceholderText("New session name...");
    m_renameEdit->setStyleSheet(
        "QLineEdit { background-color: #2d3748; color: #e2e8f0; "
        "border: 1px solid #4a5568; border-radius: 4px; padding: 6px; }"
    );
    m_renameBtn = new QPushButton("Rename", this);
    m_renameBtn->setStyleSheet(
        "QPushButton { background-color: #4299e1; color: white; "
        "border-radius: 4px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #3182ce; }"
    );
    renameLayout->addWidget(m_renameEdit, 1);
    renameLayout->addWidget(m_renameBtn);
    layout->addLayout(renameLayout);

    // Action buttons
    auto *btnLayout = new QHBoxLayout();
    m_selectBtn = new QPushButton("Open Session", this);
    m_selectBtn->setStyleSheet(
        "QPushButton { background-color: #48bb78; color: white; "
        "border-radius: 4px; padding: 8px 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #38a169; }"
    );
    m_deleteBtn = new QPushButton("Delete", this);
    m_deleteBtn->setStyleSheet(
        "QPushButton { background-color: #e53e3e; color: white; "
        "border-radius: 4px; padding: 8px 16px; }"
        "QPushButton:hover { background-color: #c53030; }"
    );
    m_closeBtn = new QPushButton("Close", this);
    m_closeBtn->setStyleSheet(
        "QPushButton { background-color: #4a5568; color: white; "
        "border-radius: 4px; padding: 8px 16px; }"
        "QPushButton:hover { background-color: #718096; }"
    );
    btnLayout->addWidget(m_selectBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_closeBtn);
    layout->addLayout(btnLayout);

    connect(m_selectBtn, &QPushButton::clicked, this, &HistoryDialog::onSelect);
    connect(m_deleteBtn, &QPushButton::clicked, this, &HistoryDialog::onDelete);
    connect(m_renameBtn, &QPushButton::clicked, this, &HistoryDialog::onRename);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    setStyleSheet("QDialog { background-color: #171923; }");
}

void HistoryDialog::refreshList()
{
    m_sessionList->clear();
    auto sessions = m_history.listSessions();
    for (const auto &session : sessions) {
        auto *item = new QListWidgetItem(session["title"].toString());
        item->setData(Qt::UserRole, session["id"]);
        m_sessionList->addItem(item);
    }
}

void HistoryDialog::onSelect()
{
    auto *item = m_sessionList->currentItem();
    if (item) {
        emit sessionSelected(item->data(Qt::UserRole).toInt());
        accept();
    }
}

void HistoryDialog::onDelete()
{
    auto *item = m_sessionList->currentItem();
    if (!item) return;

    int id = item->data(Qt::UserRole).toInt();
    auto reply = QMessageBox::question(this, "Delete Session",
        "Are you sure you want to delete this session?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        emit sessionDeleted(id);
        refreshList();
    }
}

void HistoryDialog::onRename()
{
    auto *item = m_sessionList->currentItem();
    if (!item) return;

    QString newTitle = m_renameEdit->text().trimmed();
    if (newTitle.isEmpty()) return;

    int id = item->data(Qt::UserRole).toInt();
    emit sessionRenamed(id, newTitle);
    m_renameEdit->clear();
    refreshList();
}
