#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QVector>
#include <QPushButton>

struct ChatBubble {
    QString role;
    QString content;
    QWidget *widget = nullptr;
};

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget *parent = nullptr);

    void addMessage(const QString &role, const QString &content);
    void appendToLastMessage(const QString &token);
    void clear();
    void scrollToBottom();

signals:
    void messageDisplayed(const QString &role, const QString &content);
    void speakRequested(const QString &text);

private:
    QWidget *createBubble(const QString &role, const QString &content);
    void updateLastBubble();

    QScrollArea *m_scrollArea = nullptr;
    QVBoxLayout *m_messagesLayout = nullptr;
    QWidget *m_messagesContainer = nullptr;
    QVector<ChatBubble> m_bubbles;
};
