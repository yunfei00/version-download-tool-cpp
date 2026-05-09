#pragma once

#include <QMainWindow>

#include "core/RemoteFileItem.h"

class QLineEdit;
class QPushButton;
class QPlainTextEdit;
class QTableView;
class DownloadTableModel;
class RemoteScanner;
class DownloadManager;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void chooseDirectory();
    void scanVersions();
    void startDownload();
    void stopDownload();

private:
    void setupUi();
    void appendLog(const QString &message);
    void loadSettings();
    void saveSettings();

    QLineEdit *remoteUrlEdit_ = nullptr;
    QLineEdit *localDirEdit_ = nullptr;
    QPushButton *chooseDirButton_ = nullptr;
    QPushButton *scanButton_ = nullptr;
    QPushButton *downloadButton_ = nullptr;
    QPushButton *stopButton_ = nullptr;
    QTableView *fileTable_ = nullptr;
    QPlainTextEdit *logOutput_ = nullptr;

    DownloadTableModel *model_ = nullptr;
    RemoteScanner *scanner_ = nullptr;
    DownloadManager *downloader_ = nullptr;
};
