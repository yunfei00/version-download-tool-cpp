#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QQueue>
#include <QSet>

#include "RemoteFileItem.h"

class RemoteScanner : public QObject {
    Q_OBJECT
public:
    explicit RemoteScanner(QObject *parent = nullptr);

    void scan(const QUrl &baseUrl);

signals:
    void logMessage(const QString &message);
    void scanFinished(const QList<RemoteFileItem> &items);

private:
    struct PendingDir {
        QUrl url;
        QString relativePrefix;
    };

    void requestNextDirectory();
    void handleDirectoryResponse(const QUrl &dirUrl, const QString &relativePrefix, const QByteArray &content);
    void resolveFileSize(RemoteFileItem &item);
    static bool isLikelyDirectoryLink(const QString &href);

    QNetworkAccessManager network_;
    QQueue<PendingDir> pendingDirs_;
    QSet<QString> visitedDirs_;
    QList<RemoteFileItem> items_;
    QUrl baseUrl_;
};
