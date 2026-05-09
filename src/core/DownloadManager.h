#pragma once

#include <QNetworkAccessManager>
#include <QObject>

#include "RemoteFileItem.h"

class DownloadTask;

class DownloadManager : public QObject {
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = nullptr);

    void start(const QList<RemoteFileItem> &items, const QString &localRoot);
    void stop();

signals:
    void logMessage(const QString &message);
    void rowStatusChanged(int row, const QString &status, int progress);
    void currentRowChanged(int row);
    void allFinished();

private:
    void startNext();

    QNetworkAccessManager network_;
    QList<RemoteFileItem> items_;
    QString localRoot_;
    int currentIndex_ = -1;
    bool stopped_ = false;
    DownloadTask *currentTask_ = nullptr;
};
