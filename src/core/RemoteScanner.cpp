#include "RemoteScanner.h"

#include <QNetworkReply>
#include <QRegularExpression>

namespace {
QString normalizeHref(const QString &href) {
    return href.section('#', 0, 0).section('?', 0, 0).trimmed();
}

bool shouldSkipHref(const QString &href) {
    if (href.isEmpty()) return true;
    if (href.startsWith('#') || href.startsWith('?')) return true;
    if (href == QStringLiteral("../") || href == QStringLiteral("..")) return true;
    if (href.contains(QStringLiteral("?C="), Qt::CaseInsensitive)) return true;
    return false;
}
}

RemoteScanner::RemoteScanner(QObject *parent) : QObject(parent) {}

void RemoteScanner::scan(const QUrl &baseUrl) {
    items_.clear();
    pendingDirs_.clear();
    visitedDirs_.clear();

    baseUrl_ = baseUrl;
    if (!baseUrl_.path().endsWith('/')) {
        QString path = baseUrl_.path();
        path.append('/');
        baseUrl_.setPath(path);
    }

    emit logMessage(QStringLiteral("开始扫描：%1").arg(baseUrl_.toString()));
    pendingDirs_.enqueue({baseUrl_, QString()});
    requestNextDirectory();
}

void RemoteScanner::requestNextDirectory() {
    if (pendingDirs_.isEmpty()) {
        emit logMessage(QStringLiteral("扫描完成，发现文件数量：%1").arg(items_.size()));
        emit scanFinished(items_);
        return;
    }

    const PendingDir dir = pendingDirs_.dequeue();
    const QString dirKey = dir.url.toString(QUrl::RemoveFragment | QUrl::NormalizePathSegments);
    if (visitedDirs_.contains(dirKey)) {
        requestNextDirectory();
        return;
    }
    visitedDirs_.insert(dirKey);

    auto *reply = network_.get(QNetworkRequest(dir.url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, dir]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit logMessage(QStringLiteral("扫描目录失败：%1，原因：%2").arg(dir.url.toString(), reply->errorString()));
            reply->deleteLater();
            requestNextDirectory();
            return;
        }

        const QByteArray content = reply->readAll();
        reply->deleteLater();
        handleDirectoryResponse(dir.url, dir.relativePrefix, content);
        requestNextDirectory();
    });
}

void RemoteScanner::handleDirectoryResponse(const QUrl &dirUrl, const QString &relativePrefix, const QByteArray &content) {
    const QString html = QString::fromUtf8(content);
    static const QRegularExpression rowRegex(
        QStringLiteral(R"(<a\s+[^>]*href\s*=\s*["']([^"']+)["'][^>]*>([^<]*)</a>([^\n\r<]*)?)"),
        QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatchIterator it = rowRegex.globalMatch(html);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const QString href = match.captured(1).trimmed();
        if (shouldSkipHref(href)) {
            continue;
        }

        const QString cleaned = normalizeHref(href);
        if (cleaned.isEmpty() || cleaned == QStringLiteral(".") || cleaned == QStringLiteral("..")) {
            continue;
        }

        const QUrl resolved = dirUrl.resolved(QUrl(href));
        if (!resolved.isValid()) {
            continue;
        }
        if (resolved.scheme() != QStringLiteral("http") && resolved.scheme() != QStringLiteral("https")) {
            continue;
        }
        if (resolved.host().compare(baseUrl_.host(), Qt::CaseInsensitive) != 0
            || resolved.port(baseUrl_.port()) != baseUrl_.port(resolved.port())) {
            continue;
        }

        if (isLikelyDirectoryLink(cleaned)) {
            QString dirName = cleaned;
            if (dirName.endsWith('/')) dirName.chop(1);
            dirName = dirName.section('/', -1);
            pendingDirs_.enqueue({resolved, relativePrefix + dirName + '/'});
            continue;
        }

        RemoteFileItem item;
        item.url = resolved;
        item.name = cleaned.section('/', -1);
        item.relativePath = relativePrefix + item.name;
        item.size = -1;

        const QString trailing = match.captured(3);
        QRegularExpression sizeRegex(QStringLiteral(R"((\d+(?:\.\d+)?)\s*([KMGT]?B))"), QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch sizeMatch = sizeRegex.match(trailing);
        if (sizeMatch.hasMatch()) {
            double v = sizeMatch.captured(1).toDouble();
            const QString unit = sizeMatch.captured(2).toUpper();
            if (unit == QStringLiteral("KB")) v *= 1024.0;
            else if (unit == QStringLiteral("MB")) v *= 1024.0 * 1024.0;
            else if (unit == QStringLiteral("GB")) v *= 1024.0 * 1024.0 * 1024.0;
            else if (unit == QStringLiteral("TB")) v *= 1024.0 * 1024.0 * 1024.0 * 1024.0;
            item.size = static_cast<qint64>(v);
        }

        items_.append(item);
    }
}

void RemoteScanner::resolveFileSize(RemoteFileItem &) {}

bool RemoteScanner::isLikelyDirectoryLink(const QString &href) {
    return href.endsWith('/');
}
