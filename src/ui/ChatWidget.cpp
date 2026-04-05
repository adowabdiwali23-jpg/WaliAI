#include "ui/ChatWidget.h"

#include <QScrollBar>
#include <QTimer>
#include <QFrame>
#include <QClipboard>
#include <QApplication>
#include <QHBoxLayout>

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_messagesContainer = new QWidget();
    m_messagesLayout = new QVBoxLayout(m_messagesContainer);
    m_messagesLayout->setAlignment(Qt::AlignTop);
    m_messagesLayout->setSpacing(12);
    m_messagesLayout->setContentsMargins(16, 16, 16, 16);
    m_messagesLayout->addStretch();

    m_scrollArea->setWidget(m_messagesContainer);
    layout->addWidget(m_scrollArea);
}

void ChatWidget::addMessage(const QString &role, const QString &content)
{
    QWidget *bubble = createBubble(role, content);

    ChatBubble cb;
    cb.role = role;
    cb.content = content;
    cb.widget = bubble;
    m_bubbles.append(cb);

    int insertIndex = m_messagesLayout->count() - 1;
    m_messagesLayout->insertWidget(insertIndex, bubble);

    QTimer::singleShot(50, this, &ChatWidget::scrollToBottom);
    emit messageDisplayed(role, content);
}

void ChatWidget::appendToLastMessage(const QString &token)
{
    if (m_bubbles.isEmpty()) {
        addMessage("assistant", token);
        return;
    }

    auto &last = m_bubbles.last();
    if (last.role != "assistant") {
        addMessage("assistant", token);
        return;
    }

    last.content += token;
    updateLastBubble();
    QTimer::singleShot(50, this, &ChatWidget::scrollToBottom);
}

void ChatWidget::clear()
{
    for (auto &bubble : m_bubbles) {
        if (bubble.widget) {
            m_messagesLayout->removeWidget(bubble.widget);
            bubble.widget->deleteLater();
        }
    }
    m_bubbles.clear();
}

void ChatWidget::scrollToBottom()
{
    auto *scrollBar = m_scrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

QWidget *ChatWidget::createBubble(const QString &role, const QString &content)
{
    auto *frame = new QFrame();
    frame->setFrameShape(QFrame::StyledPanel);

    auto *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(12, 8, 12, 8);

    // Header with role label and action buttons
    auto *headerLayout = new QHBoxLayout();
    auto *roleLabel = new QLabel(role == "user" ? "You" : "Wali");
    QFont boldFont = roleLabel->font();
    boldFont.setBold(true);
    roleLabel->setFont(boldFont);
    headerLayout->addWidget(roleLabel);
    headerLayout->addStretch();

    // Copy button
    auto *copyBtn = new QPushButton("Copy");
    copyBtn->setFixedSize(50, 22);
    copyBtn->setStyleSheet(
        "QPushButton { background-color: transparent; color: #718096; "
        "border: 1px solid #4a5568; border-radius: 3px; font-size: 11px; }"
        "QPushButton:hover { background-color: #2d3748; color: #e2e8f0; }"
    );
    connect(copyBtn, &QPushButton::clicked, [content]() {
        QApplication::clipboard()->setText(content);
    });
    headerLayout->addWidget(copyBtn);

    // Speaker icon for assistant messages (TTS replay)
    if (role == "assistant") {
        auto *speakBtn = new QPushButton("Speak");
        speakBtn->setFixedSize(55, 22);
        speakBtn->setStyleSheet(
            "QPushButton { background-color: transparent; color: #718096; "
            "border: 1px solid #4a5568; border-radius: 3px; font-size: 11px; }"
            "QPushButton:hover { background-color: #2d3748; color: #e2e8f0; }"
        );
        connect(speakBtn, &QPushButton::clicked, [this, content]() {
            emit speakRequested(content);
        });
        headerLayout->addWidget(speakBtn);
    }

    layout->addLayout(headerLayout);

    auto *contentLabel = new QLabel(content);
    contentLabel->setWordWrap(true);
    contentLabel->setTextFormat(Qt::PlainText);
    contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(contentLabel);

    if (role == "user") {
        frame->setStyleSheet(
            "QFrame { background-color: #2d3748; border-radius: 12px; "
            "margin-left: 60px; margin-right: 8px; }"
            "QLabel { color: #e2e8f0; }"
        );
    } else {
        frame->setStyleSheet(
            "QFrame { background-color: #1a202c; border-radius: 12px; "
            "margin-left: 8px; margin-right: 60px; }"
            "QLabel { color: #e2e8f0; }"
        );
    }

    return frame;
}

void ChatWidget::updateLastBubble()
{
    if (m_bubbles.isEmpty()) return;
    auto &last = m_bubbles.last();
    if (!last.widget) return;

    auto *layout = last.widget->layout();
    if (layout && layout->count() >= 2) {
        auto *contentLabel = qobject_cast<QLabel *>(layout->itemAt(1)->widget());
        if (contentLabel) {
            contentLabel->setText(last.content);
        }
    }
}
