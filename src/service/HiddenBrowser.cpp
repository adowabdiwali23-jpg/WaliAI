#include "service/HiddenBrowser.h"
#include "service/Logger.h"

#include <QUrl>
#include <QRegularExpression>

HiddenBrowser::HiddenBrowser(QObject *parent)
    : QObject(parent)
{
    connect(&m_networkManager, &QNetworkAccessManager::finished,
            this, &HiddenBrowser::onReplyFinished);
}

void HiddenBrowser::fetchPage(const QString &url)
{
    m_requestQueue.enqueue(url);
    if (!m_busy) {
        processQueue();
    }
}

void HiddenBrowser::searchWeb(const QString &query)
{
    // Use DuckDuckGo HTML-only endpoint for web search
    QString searchUrl = "https://html.duckduckgo.com/html/?q=" + QUrl::toPercentEncoding(query);
    fetchPage(searchUrl);
}

bool HiddenBrowser::isBusy() const
{
    return m_busy;
}

void HiddenBrowser::processQueue()
{
    if (m_requestQueue.isEmpty()) {
        m_busy = false;
        return;
    }

    m_busy = true;
    QString url = m_requestQueue.dequeue();

    QUrl requestUrl(url);
    QNetworkRequest request{requestUrl};
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      "Mozilla/5.0 (X11; Linux x86_64) WaliAI/1.0");
    request.setRawHeader("Accept", "text/html,application/xhtml+xml");

    m_networkManager.get(request);
}

void HiddenBrowser::onReplyFinished(QNetworkReply *reply)
{
    WebResult result;
    result.url = reply->url().toString();

    if (reply->error() == QNetworkReply::NoError) {
        QString html = QString::fromUtf8(reply->readAll());
        result.title = extractTitle(html);
        result.content = extractText(html);
        result.success = true;
        Logger::instance().log("Fetched page: " + result.url);
    } else {
        result.error = reply->errorString();
        result.success = false;
        Logger::instance().warning("Failed to fetch: " + result.url + " - " + result.error);
        emit errorOccurred(result.error);
    }

    reply->deleteLater();
    emit pageFetched(result);

    // Process next item in queue
    processQueue();
}

QString HiddenBrowser::sanitizeHtml(const QString &html)
{
    QString cleaned = html;
    // Remove script and style tags with content
    cleaned.remove(QRegularExpression("<script[^>]*>[\\s\\S]*?</script>", QRegularExpression::CaseInsensitiveOption));
    cleaned.remove(QRegularExpression("<style[^>]*>[\\s\\S]*?</style>", QRegularExpression::CaseInsensitiveOption));
    return cleaned;
}

QString HiddenBrowser::extractText(const QString &html)
{
    QString cleaned = sanitizeHtml(html);
    // Remove all HTML tags
    cleaned.remove(QRegularExpression("<[^>]+>"));
    // Normalize whitespace
    cleaned = cleaned.simplified();
    // Limit length
    if (cleaned.length() > 8000) {
        cleaned = cleaned.left(8000) + "...";
    }
    return cleaned;
}

QString HiddenBrowser::extractTitle(const QString &html)
{
    QRegularExpression titleRe("<title[^>]*>([^<]*)</title>",
                                QRegularExpression::CaseInsensitiveOption);
    auto match = titleRe.match(html);
    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }
    return "Untitled";
}
