#include "DownloadManager.h"

#include "DownloadTask.h"

DownloadManager::DownloadManager(QObject *parent) : QObject(parent) {}

bool DownloadManager::isDownloading() const { return currentTask_ != nullptr || (currentIndex_ >= 0 && currentIndex_ < items_.size()); }

DownloadManager::Statistics DownloadManager::statistics() const { return stats_; }

void DownloadManager::start(const QList<RemoteFileItem> &items, const QString &localRoot) {
    items_ = items;
    localRoot_ = localRoot;
    currentIndex_ = -1;
    stopped_ = false;
    resetStatistics();
    stats_.totalFiles = items_.size();
    for (int i = 0; i < items_.size(); ++i) {
        const RemoteFileItem &item = items_.at(i);
        if (item.size >= 0) {
            stats_.totalKnownBytes += item.size;
        }
    }
    elapsedTimer_.start();
    connect(&statsTimer_, &QTimer::timeout, this, &DownloadManager::updateStatisticsTick, Qt::UniqueConnection);
    statsTimer_.start(1000);
    emitStatistics();
    startNext();
}

void DownloadManager::stop() {
    stopped_ = true;
    if (currentTask_) currentTask_->cancel();
    emit logMessage(QStringLiteral("收到停止下载请求"));
}

void DownloadManager::startNext() {
    if (stopped_) {
        statsTimer_.stop();
        updateStatisticsTick();
        emit allFinished();
        return;
    }

    ++currentIndex_;
    if (currentIndex_ >= items_.size()) {
        statsTimer_.stop();
        updateStatisticsTick();
        emit logMessage(QStringLiteral("全部下载任务已处理完成"));
        emit allFinished();
        return;
    }

    emit currentRowChanged(currentIndex_);
    emit rowStatusChanged(currentIndex_, QStringLiteral("下载中"), 0);
    emit logMessage(QStringLiteral("开始下载：%1").arg(items_[currentIndex_].relativePath));

    currentTask_ = new DownloadTask(items_[currentIndex_], localRoot_, &network_, this);
    connect(currentTask_, &DownloadTask::progressChanged, this, [this](int p) {
        emit rowStatusChanged(currentIndex_, QStringLiteral("下载中"), p);
    });
    connect(currentTask_, &DownloadTask::bytesReceivedDelta, this, [this](qint64 delta) {
        stats_.downloadedBytes += delta;
        bytesSinceLastTick_ += delta;
        emitStatistics();
    });
    connect(currentTask_, &DownloadTask::finished, this, [this](bool ok, const QString &msg, bool skipped) {
        if (skipped) {
            emit rowStatusChanged(currentIndex_, QStringLiteral("已存在"), 100);
            emit logMessage(QStringLiteral("跳过：%1（%2）").arg(items_[currentIndex_].relativePath, msg));
            ++stats_.skippedFiles;
            ++stats_.finishedFiles;
            if (items_[currentIndex_].size > 0) stats_.downloadedBytes += items_[currentIndex_].size;
        } else if (ok) {
            emit rowStatusChanged(currentIndex_, QStringLiteral("完成"), 100);
            emit logMessage(QStringLiteral("完成：%1").arg(items_[currentIndex_].relativePath));
            ++stats_.finishedFiles;
        } else {
            emit rowStatusChanged(currentIndex_, QStringLiteral("失败"), 0);
            emit logMessage(QStringLiteral("失败：%1，原因：%2").arg(items_[currentIndex_].relativePath, msg));
            ++stats_.failedFiles;
        }
        emitStatistics();
        currentTask_->deleteLater();
        currentTask_ = nullptr;
        startNext();
    });
    currentTask_->start();
}

void DownloadManager::resetStatistics() {
    stats_ = Statistics{};
    bytesSinceLastTick_ = 0;
}

void DownloadManager::updateStatisticsTick() {
    stats_.elapsedSeconds = elapsedTimer_.isValid() ? elapsedTimer_.elapsed() / 1000 : 0;
    stats_.currentSpeedBytesPerSecond = bytesSinceLastTick_;
    bytesSinceLastTick_ = 0;
    if (stats_.totalKnownBytes > 0 && stats_.currentSpeedBytesPerSecond > 0 && stats_.downloadedBytes <= stats_.totalKnownBytes) {
        stats_.estimatedRemainingSeconds = (stats_.totalKnownBytes - stats_.downloadedBytes) / stats_.currentSpeedBytesPerSecond;
    } else {
        stats_.estimatedRemainingSeconds = -1;
    }
    emitStatistics();
}

void DownloadManager::emitStatistics() { emit statisticsChanged(stats_); }
