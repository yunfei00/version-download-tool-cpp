#pragma once

#include <QObject>

#include "RemoteFileItem.h"

class QNetworkAccessManager;
class QNetworkReply;

class DownloadTask : public QObject {
    Q_OBJECT
public:
    DownloadTask(const RemoteFileItem &item, const QString &localRoot, QNetworkAccessManager *network, QObject *parent = nullptr);

    void start();
    void cancel();

signals:
    void progressChanged(int percent);
    void bytesReceivedDelta(qint64 delta);
    void finished(bool ok, const QString &message, bool skipped);

private:
    RemoteFileItem item_;
    QString localRoot_;
    QNetworkAccessManager *network_ = nullptr;
    QNetworkReply *reply_ = nullptr;
    qint64 lastReceived_ = 0;
    bool canceled_ = false;
};
