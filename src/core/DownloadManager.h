#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QElapsedTimer>
#include <QTimer>

#include "RemoteFileItem.h"

class DownloadTask;

class DownloadManager : public QObject {
    Q_OBJECT
public:
    struct Statistics {
        int totalFiles = 0;
        int finishedFiles = 0;
        int failedFiles = 0;
        int skippedFiles = 0;
        qint64 totalKnownBytes = 0;
        qint64 downloadedBytes = 0;
        qint64 currentSpeedBytesPerSecond = 0;
        qint64 elapsedSeconds = 0;
        qint64 estimatedRemainingSeconds = -1;
    };

    explicit DownloadManager(QObject *parent = nullptr);

    void start(const QList<RemoteFileItem> &items, const QString &localRoot);
    void stop();
    bool isDownloading() const;
    Statistics statistics() const;

signals:
    void logMessage(const QString &message);
    void rowStatusChanged(int row, const QString &status, int progress);
    void currentRowChanged(int row);
    void allFinished();
    void statisticsUpdated(const DownloadManager::Statistics &stats);

private:
    void startNext();
    void resetStatistics();
    void updateStatisticsTick();
    void emitStatistics();

    QNetworkAccessManager network_;
    QList<RemoteFileItem> items_;
    QString localRoot_;
    int currentIndex_ = -1;
    bool stopped_ = false;
    DownloadTask *currentTask_ = nullptr;
    Statistics stats_;
    QTimer statsTimer_;
    QElapsedTimer elapsedTimer_;
    qint64 bytesSinceLastTick_ = 0;
};
