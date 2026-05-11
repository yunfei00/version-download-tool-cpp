#pragma once

#include <QAbstractTableModel>

#include "../core/RemoteFileItem.h"

class DownloadTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit DownloadTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setItems(const QList<RemoteFileItem> &items);
    QList<RemoteFileItem> items() const;
    void updateRow(int row, const QString &status, int progress);
    void setCurrentRow(int row);
    int currentRow() const;

private:
    static QString formatSize(qint64 size);
    QList<RemoteFileItem> items_;
    int m_currentRow = -1;
};
