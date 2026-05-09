#include "DownloadTask.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

DownloadTask::DownloadTask(const RemoteFileItem &item, const QString &localRoot, QNetworkAccessManager *network, QObject *parent)
    : QObject(parent), m_item(item), m_localRoot(localRoot), m_manager(network) {}

void DownloadTask::start() {
    const QString target = QDir(m_localRoot).filePath(m_item.relativePath);
    const QString parentDir = QFileInfo(target).path();
    if (!QDir().mkpath(parentDir)) {
        emit finished(false, QStringLiteral("创建目录失败：%1").arg(parentDir), false);
        return;
    }

    QFileInfo info(target);
    if (info.exists() && m_item.size >= 0 && info.size() == m_item.size) {
        emit finished(true, QStringLiteral("文件已存在且大小一致，跳过"), true);
        return;
    }

    m_lastReceived = 0;
    m_canceled = false;

    m_outputFile.setFileName(target);
    if (!m_outputFile.open(QIODevice::WriteOnly)) {
        emit finished(false, QStringLiteral("无法写入文件：%1").arg(target), false);
        return;
    }

    m_reply = m_manager->get(QNetworkRequest(m_item.url));
    connect(m_reply, &QNetworkReply::readyRead, this, [this]() { m_outputFile.write(m_reply->readAll()); });
    connect(m_reply, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64 total) {
        const qint64 delta = received - m_lastReceived;
        if (delta > 0) emit bytesReceivedDelta(delta);
        m_lastReceived = received;
        if (total > 0) emit progressChanged(static_cast<int>((received * 100) / total));
    });
    connect(m_reply, &QNetworkReply::finished, this, [this]() {
        m_outputFile.write(m_reply->readAll());
        m_outputFile.close();
        if (m_canceled) {
            m_outputFile.remove();
            emit finished(false, QStringLiteral("下载已停止"), false);
        } else if (m_reply->error() != QNetworkReply::NoError) {
            m_outputFile.remove();
            emit finished(false, m_reply->errorString(), false);
        } else {
            emit progressChanged(100);
            emit finished(true, QStringLiteral("下载完成"), false);
        }
        m_reply->deleteLater();
        m_reply = nullptr;
    });
}

void DownloadTask::cancel() {
    m_canceled = true;
    if (m_reply) m_reply->abort();
}
