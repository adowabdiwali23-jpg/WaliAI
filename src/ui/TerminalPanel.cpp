#include "ui/TerminalPanel.h"
#include "execution/SandboxManager.h"
#include "execution/CommandExecutor.h"

#include <QScrollBar>

TerminalPanel::TerminalPanel(SandboxManager &sandbox, QWidget *parent)
    : QWidget(parent), m_sandbox(sandbox)
{
    setupUi();
}

void TerminalPanel::setupUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    // Header
    auto *headerLayout = new QHBoxLayout();
    auto *titleLabel = new QLabel("Terminal");
    titleLabel->setStyleSheet("QLabel { color: #e2e8f0; font-weight: bold; padding: 4px; }");
    m_clearBtn = new QPushButton("Clear");
    m_clearBtn->setFixedSize(60, 24);
    m_clearBtn->setStyleSheet(
        "QPushButton { background-color: #4a5568; color: #e2e8f0; "
        "border-radius: 4px; font-size: 11px; }"
        "QPushButton:hover { background-color: #718096; }"
    );
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_clearBtn);
    layout->addLayout(headerLayout);

    // Output area
    m_output = new QTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setStyleSheet(
        "QTextEdit { background-color: #0d1117; color: #c9d1d9; "
        "font-family: 'Courier New', monospace; font-size: 12px; "
        "border: 1px solid #30363d; padding: 8px; }"
    );
    layout->addWidget(m_output, 1);

    // Input line
    auto *inputLayout = new QHBoxLayout();
    auto *promptLabel = new QLabel("$");
    promptLabel->setStyleSheet("QLabel { color: #58a6ff; font-family: monospace; font-weight: bold; }");
    m_input = new QLineEdit(this);
    m_input->setPlaceholderText("Enter command...");
    m_input->setStyleSheet(
        "QLineEdit { background-color: #161b22; color: #c9d1d9; "
        "font-family: monospace; font-size: 12px; "
        "border: 1px solid #30363d; border-radius: 4px; padding: 6px; }"
    );
    inputLayout->addWidget(promptLabel);
    inputLayout->addWidget(m_input, 1);
    layout->addLayout(inputLayout);

    connect(m_input, &QLineEdit::returnPressed, this, &TerminalPanel::onCommandEntered);
    connect(m_clearBtn, &QPushButton::clicked, this, &TerminalPanel::clear);

    setStyleSheet("QWidget { background-color: #0d1117; }");
}

void TerminalPanel::onCommandEntered()
{
    QString command = m_input->text().trimmed();
    if (command.isEmpty()) return;

    appendOutput("$ " + command);
    m_input->clear();

    CommandResult result = m_sandbox.executor().execute(command);
    if (!result.output.isEmpty()) {
        appendOutput(result.output);
    }
    if (!result.error.isEmpty()) {
        appendOutput("[Error] " + result.error);
    }

    emit commandSubmitted(command);
}

void TerminalPanel::appendOutput(const QString &text)
{
    m_output->append(text);
    auto *scrollBar = m_output->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void TerminalPanel::clear()
{
    m_output->clear();
}
