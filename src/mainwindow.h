#pragma once

#include <QMainWindow>

#include "core/RemoteFileItem.h"
#include "core/DownloadManager.h"

class QLineEdit;
class QPushButton;
class QPlainTextEdit;
class QTableView;
class QLabel;
class QProgressBar;
class DownloadTableModel;
class RemoteScanner;

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
    enum class UiState { Idle, Scanning, Ready, Downloading, Stopping, Finished, Failed };

    void setupUi();
    void appendLog(const QString &message);
    bool validateInputs(QUrl *urlOut = nullptr);
    void applyState(UiState state);
    void updateStatsUi(const DownloadManager::Statistics &stats);
    static QString formatBytes(qint64 bytes);
    static QString formatDuration(qint64 seconds);
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
    QLabel *statsLabel_ = nullptr;
    QProgressBar *overallProgress_ = nullptr;

    DownloadTableModel *model_ = nullptr;
    RemoteScanner *scanner_ = nullptr;
    DownloadManager *downloader_ = nullptr;
    UiState state_ = UiState::Idle;
    bool autoStartDownloadAfterScan_ = false;
};
