#include "DownloadTableModel.h"

DownloadTableModel::DownloadTableModel(QObject *parent) : QAbstractTableModel(parent) {}

int DownloadTableModel::rowCount(const QModelIndex &parent) const { return parent.isValid() ? 0 : items_.size(); }
int DownloadTableModel::columnCount(const QModelIndex &parent) const { return parent.isValid() ? 0 : 5; }

QVariant DownloadTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) return {};
    const auto &item = items_.at(index.row());
    switch (index.column()) {
        case 0: return index.row() + 1;
        case 1: return item.relativePath;
        case 2: return formatSize(item.size);
        case 3: return item.status;
        case 4: return QStringLiteral("%1%").arg(item.progress);
        default: return {};
    }
}

QVariant DownloadTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return QStringLiteral("序号");
            case 1: return QStringLiteral("文件名/相对路径");
            case 2: return QStringLiteral("大小");
            case 3: return QStringLiteral("状态");
            case 4: return QStringLiteral("进度");
            default: return {};
        }
    }
    return {};
}

Qt::ItemFlags DownloadTableModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void DownloadTableModel::setItems(const QList<RemoteFileItem> &items) {
    beginResetModel();
    items_ = items;
    endResetModel();
}

QList<RemoteFileItem> DownloadTableModel::items() const { return items_; }

void DownloadTableModel::updateRow(int row, const QString &status, int progress) {
    if (row < 0 || row >= items_.size()) return;
    items_[row].status = status;
    items_[row].progress = progress;
    emit dataChanged(index(row, 3), index(row, 4));
}

QString DownloadTableModel::formatSize(qint64 size) {
    if (size < 0) return QStringLiteral("未知");
    double value = static_cast<double>(size);
    QString unit = QStringLiteral("B");
    if (value >= 1024.0) { value /= 1024.0; unit = QStringLiteral("KB"); }
    if (value >= 1024.0) { value /= 1024.0; unit = QStringLiteral("MB"); }
    if (value >= 1024.0) { value /= 1024.0; unit = QStringLiteral("GB"); }
    return QStringLiteral("%1 %2").arg(QString::number(value, 'f', value < 10 ? 2 : 1), unit);
}
