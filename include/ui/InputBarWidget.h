#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>

class InputBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InputBarWidget(QWidget *parent = nullptr);

    QString text() const;
    void clear();
    void setEnabled(bool enabled);
    void setPlaceholderText(const QString &text);

signals:
    void messageSent(const QString &message);
    void voiceButtonClicked();
    void stopRequested();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUi();
    void onSendClicked();
    void updateCharCount();

    QTextEdit *m_textEdit = nullptr;
    QPushButton *m_sendButton = nullptr;
    QPushButton *m_voiceButton = nullptr;
    QPushButton *m_stopButton = nullptr;
    QLabel *m_charCount = nullptr;
    bool m_isProcessing = false;

    static constexpr int MAX_INPUT_LENGTH = 4000;

public slots:
    void setProcessing(bool processing);
};
