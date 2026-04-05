#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

class SandboxManager;

class TerminalPanel : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalPanel(SandboxManager &sandbox, QWidget *parent = nullptr);

    void appendOutput(const QString &text);
    void clear();

signals:
    void commandSubmitted(const QString &command);

private:
    void setupUi();
    void onCommandEntered();
    bool askUserPermission(const QString &command);
    void runCommand(const QString &command);

    SandboxManager &m_sandbox;
    QTextEdit *m_output = nullptr;
    QLineEdit *m_input = nullptr;
    QPushButton *m_clearBtn = nullptr;
};
