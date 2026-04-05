#pragma once

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

class HistoryService;

class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryDialog(HistoryService &history, QWidget *parent = nullptr);

signals:
    void sessionSelected(int sessionId);
    void sessionDeleted(int sessionId);
    void sessionRenamed(int sessionId, const QString &newTitle);

private:
    void setupUi();
    void refreshList();
    void onRename();
    void onDelete();
    void onSelect();

    HistoryService &m_history;
    QListWidget *m_sessionList = nullptr;
    QLineEdit *m_renameEdit = nullptr;
    QPushButton *m_selectBtn = nullptr;
    QPushButton *m_renameBtn = nullptr;
    QPushButton *m_deleteBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;
};
