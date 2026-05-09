#include "DownloadManager.h"

#include "DownloadTask.h"

DownloadManager::DownloadManager(QObject *parent) : QObject(parent) {}

void DownloadManager::start(const QList<RemoteFileItem> &items, const QString &localRoot) {
    items_ = items;
    localRoot_ = localRoot;
    currentIndex_ = -1;
    stopped_ = false;
    startNext();
}

void DownloadManager::stop() {
    stopped_ = true;
    if (currentTask_) currentTask_->cancel();
    emit logMessage(QStringLiteral("收到停止下载请求"));
}

void DownloadManager::startNext() {
    if (stopped_) {
        emit allFinished();
        return;
    }

    ++currentIndex_;
    if (currentIndex_ >= items_.size()) {
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
    connect(currentTask_, &DownloadTask::finished, this, [this](bool ok, const QString &msg, bool skipped) {
        if (skipped) {
            emit rowStatusChanged(currentIndex_, QStringLiteral("已存在"), 100);
            emit logMessage(QStringLiteral("跳过：%1（%2）").arg(items_[currentIndex_].relativePath, msg));
        } else if (ok) {
            emit rowStatusChanged(currentIndex_, QStringLiteral("完成"), 100);
            emit logMessage(QStringLiteral("完成：%1").arg(items_[currentIndex_].relativePath));
        } else {
            emit rowStatusChanged(currentIndex_, QStringLiteral("失败"), 0);
            emit logMessage(QStringLiteral("失败：%1，原因：%2").arg(items_[currentIndex_].relativePath, msg));
        }
        currentTask_->deleteLater();
        currentTask_ = nullptr;
        startNext();
    });
    currentTask_->start();
}
