#pragma once

#include <QString>
#include <QUrl>

struct RemoteFileItem {
    QString name;
    QString relativePath;
    QUrl url;
    qint64 size = -1; // -1 means unknown
    QString status = QStringLiteral("待下载");
    int progress = 0;
};
