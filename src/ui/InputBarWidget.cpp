#include "ui/InputBarWidget.h"

#include <QKeyEvent>

InputBarWidget::InputBarWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void InputBarWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 4, 12, 8);
    mainLayout->setSpacing(4);

    auto *layout = new QHBoxLayout();
    layout->setSpacing(8);

    m_voiceButton = new QPushButton("Mic", this);
    m_voiceButton->setFixedSize(40, 40);
    m_voiceButton->setStyleSheet(
        "QPushButton { background-color: #4a5568; color: #e2e8f0; "
        "border-radius: 20px; font-size: 12px; }"
        "QPushButton:hover { background-color: #718096; }"
    );

    m_textEdit = new QTextEdit(this);
    m_textEdit->setPlaceholderText("Ask Wali anything...");
    m_textEdit->setMaximumHeight(100);
    m_textEdit->setMinimumHeight(40);
    m_textEdit->setStyleSheet(
        "QTextEdit { background-color: #2d3748; color: #e2e8f0; "
        "border: 1px solid #4a5568; border-radius: 8px; padding: 8px; "
        "font-size: 14px; }"
        "QTextEdit:focus { border-color: #63b3ed; }"
    );
    m_textEdit->installEventFilter(this);

    m_sendButton = new QPushButton("Send", this);
    m_sendButton->setFixedSize(60, 40);
    m_sendButton->setStyleSheet(
        "QPushButton { background-color: #4299e1; color: white; "
        "border-radius: 8px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background-color: #3182ce; }"
        "QPushButton:disabled { background-color: #4a5568; }"
    );

    m_stopButton = new QPushButton("Stop", this);
    m_stopButton->setFixedSize(60, 40);
    m_stopButton->setStyleSheet(
        "QPushButton { background-color: #e53e3e; color: white; "
        "border-radius: 8px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background-color: #c53030; }"
    );
    m_stopButton->setVisible(false);

    layout->addWidget(m_voiceButton);
    layout->addWidget(m_textEdit, 1);
    layout->addWidget(m_sendButton);
    layout->addWidget(m_stopButton);
    mainLayout->addLayout(layout);

    // Character count label
    m_charCount = new QLabel("0 / 4000", this);
    m_charCount->setStyleSheet("QLabel { color: #718096; font-size: 11px; }");
    m_charCount->setAlignment(Qt::AlignRight);
    mainLayout->addWidget(m_charCount);

    connect(m_sendButton, &QPushButton::clicked, this, &InputBarWidget::onSendClicked);
    connect(m_voiceButton, &QPushButton::clicked, this, &InputBarWidget::voiceButtonClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &InputBarWidget::stopRequested);
    connect(m_textEdit, &QTextEdit::textChanged, this, &InputBarWidget::updateCharCount);
}

bool InputBarWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_textEdit && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return && !(keyEvent->modifiers() & Qt::ShiftModifier)) {
            onSendClicked();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void InputBarWidget::onSendClicked()
{
    QString msg = m_textEdit->toPlainText().trimmed();
    if (msg.isEmpty()) return;
    if (msg.length() > MAX_INPUT_LENGTH) {
        msg = msg.left(MAX_INPUT_LENGTH);
    }

    emit messageSent(msg);
    m_textEdit->clear();
}

void InputBarWidget::updateCharCount()
{
    int count = m_textEdit->toPlainText().length();
    m_charCount->setText(QString::number(count) + " / " + QString::number(MAX_INPUT_LENGTH));
    if (count > MAX_INPUT_LENGTH) {
        m_charCount->setStyleSheet("QLabel { color: #e53e3e; font-size: 11px; }");
    } else {
        m_charCount->setStyleSheet("QLabel { color: #718096; font-size: 11px; }");
    }
}

QString InputBarWidget::text() const { return m_textEdit->toPlainText(); }
void InputBarWidget::clear() { m_textEdit->clear(); }

void InputBarWidget::setEnabled(bool enabled)
{
    m_textEdit->setEnabled(enabled);
    m_sendButton->setEnabled(enabled);
    m_voiceButton->setEnabled(enabled);
}

void InputBarWidget::setPlaceholderText(const QString &text)
{
    m_textEdit->setPlaceholderText(text);
}

void InputBarWidget::setProcessing(bool processing)
{
    m_isProcessing = processing;
    m_sendButton->setVisible(!processing);
    m_stopButton->setVisible(processing);
    m_textEdit->setEnabled(!processing);
}
