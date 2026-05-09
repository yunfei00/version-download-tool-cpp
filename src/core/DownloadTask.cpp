#include "DownloadTask.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

DownloadTask::DownloadTask(const RemoteFileItem &item, const QString &localRoot, QNetworkAccessManager *network, QObject *parent)
    : QObject(parent), item_(item), localRoot_(localRoot), network_(network) {}

void DownloadTask::start() {
    const QString target = QDir(localRoot_).filePath(item_.relativePath);
    QDir().mkpath(QFileInfo(target).path());

    QFileInfo info(target);
    if (info.exists() && item_.size >= 0 && info.size() == item_.size) {
        emit finished(true, QStringLiteral("文件已存在且大小一致，跳过"), true);
        return;
    }

    QFile *file = new QFile(target, this);
    if (!file->open(QIODevice::WriteOnly)) {
        emit finished(false, QStringLiteral("无法写入文件：%1").arg(target), false);
        return;
    }

    reply_ = network_->get(QNetworkRequest(item_.url));
    connect(reply_, &QNetworkReply::readyRead, this, [this, file]() { file->write(reply_->readAll()); });
    connect(reply_, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64 total) {
        if (total > 0) emit progressChanged(static_cast<int>((received * 100) / total));
    });
    connect(reply_, &QNetworkReply::finished, this, [this, file]() {
        file->write(reply_->readAll());
        file->close();
        if (canceled_) {
            file->remove();
            emit finished(false, QStringLiteral("下载已停止"), false);
        } else if (reply_->error() != QNetworkReply::NoError) {
            file->remove();
            emit finished(false, reply_->errorString(), false);
        } else {
            emit progressChanged(100);
            emit finished(true, QStringLiteral("下载完成"), false);
        }
        reply_->deleteLater();
    });
}

void DownloadTask::cancel() {
    canceled_ = true;
    if (reply_) reply_->abort();
}
