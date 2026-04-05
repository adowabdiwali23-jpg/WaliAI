#pragma once

#include <QObject>
#include <QString>
#include <QQueue>
#include <QNetworkAccessManager>
#include <QNetworkReply>

struct WebResult {
    QString url;
    QString title;
    QString content;
    bool success = false;
    QString error;
};

class HiddenBrowser : public QObject
{
    Q_OBJECT

public:
    explicit HiddenBrowser(QObject *parent = nullptr);

    void fetchPage(const QString &url);
    void searchWeb(const QString &query);

    bool isBusy() const;

signals:
    void pageFetched(const WebResult &result);
    void searchCompleted(const QVector<WebResult> &results);
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QString sanitizeHtml(const QString &html);
    QString extractText(const QString &html);
    QString extractTitle(const QString &html);

    QNetworkAccessManager m_networkManager;
    QQueue<QString> m_requestQueue;
    bool m_busy = false;

    void processQueue();
};
