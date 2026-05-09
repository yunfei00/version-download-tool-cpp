#pragma once

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

#include "RemoteFileItem.h"

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
    RemoteFileItem m_item;
    QString m_localRoot;
    QNetworkReply *m_reply = nullptr;
    QFile m_outputFile;
    QNetworkAccessManager *m_manager = nullptr;
    qint64 m_lastReceived = 0;
    bool m_canceled = false;
};
