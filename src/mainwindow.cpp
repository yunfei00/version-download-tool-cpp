#include "mainwindow.h"

#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QSettings>
#include <QTableView>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include "core/DownloadManager.h"
#include "core/RemoteScanner.h"
#include "ui/DownloadTableModel.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
    loadSettings();

    scanner_ = new RemoteScanner(this);
    downloader_ = new DownloadManager(this);

    connect(scanner_, &RemoteScanner::logMessage, this, &MainWindow::appendLog);
    connect(scanner_, &RemoteScanner::scanFinished, this, [this](const QList<RemoteFileItem> &items) {
        model_->setItems(items);
        applyState(UiState::Ready);
        if (autoStartDownloadAfterScan_) {
            autoStartDownloadAfterScan_ = false;
            startDownload();
        }
    });

    connect(downloader_, &DownloadManager::logMessage, this, &MainWindow::appendLog);
    connect(downloader_, &DownloadManager::rowStatusChanged, this, [this](int row, const QString &status, int progress) {
        model_->updateRow(row, status, progress);
    });
    connect(downloader_, &DownloadManager::currentRowChanged, this, [this](int row) {
        fileTable_->scrollTo(model_->index(row, 0), QAbstractItemView::PositionAtCenter);
    });
    connect(downloader_, &DownloadManager::statisticsUpdated, this, &MainWindow::updateStatsUi);
    connect(downloader_, &DownloadManager::allFinished, this, [this]() { applyState(UiState::Finished); });
    applyState(UiState::Idle);
}

MainWindow::~MainWindow() { saveSettings(); }

void MainWindow::setupUi() {
    setWindowTitle(QStringLiteral("版本下载工具"));
    resize(1100, 700);

    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);
    auto *topLayout = new QHBoxLayout();

    auto *configWidget = new QWidget(central);
    auto *configLayout = new QVBoxLayout(configWidget);
    configLayout->setContentsMargins(0, 0, 0, 0);

    auto *remoteLabel = new QLabel(QStringLiteral("远程版本地址"), configWidget);
    remoteUrlEdit_ = new QLineEdit(configWidget);
    auto *localLabel = new QLabel(QStringLiteral("本地保存目录"), configWidget);
    localDirEdit_ = new QLineEdit(configWidget);
    chooseDirButton_ = new QPushButton(QStringLiteral("选择目录"), configWidget);
    scanButton_ = new QPushButton(QStringLiteral("扫描"), configWidget);
    downloadButton_ = new QPushButton(QStringLiteral("开始下载"), configWidget);
    stopButton_ = new QPushButton(QStringLiteral("停止下载"), configWidget);

    configLayout->addWidget(remoteLabel);
    configLayout->addWidget(remoteUrlEdit_);
    configLayout->addWidget(localLabel);
    configLayout->addWidget(localDirEdit_);
    configLayout->addWidget(chooseDirButton_);
    configLayout->addWidget(scanButton_);
    configLayout->addWidget(downloadButton_);
    configLayout->addWidget(stopButton_);
    configLayout->addStretch();

    model_ = new DownloadTableModel(this);
    fileTable_ = new QTableView(central);
    fileTable_->setModel(model_);
    fileTable_->verticalHeader()->setVisible(false);
    fileTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    fileTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    fileTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    fileTable_->horizontalHeader()->setStretchLastSection(true);

    topLayout->addWidget(configWidget, 0);
    topLayout->addWidget(fileTable_, 1);

    logOutput_ = new QPlainTextEdit(central);
    logOutput_->setReadOnly(true);
    statsLabel_ = new QLabel(central);
    overallProgress_ = new QProgressBar(central);
    overallProgress_->setRange(0, 100);

    mainLayout->addLayout(topLayout, 3);
    mainLayout->addWidget(statsLabel_);
    mainLayout->addWidget(overallProgress_);
    mainLayout->addWidget(logOutput_, 1);

    connect(chooseDirButton_, &QPushButton::clicked, this, &MainWindow::chooseDirectory);
    connect(scanButton_, &QPushButton::clicked, this, &MainWindow::scanVersions);
    connect(downloadButton_, &QPushButton::clicked, this, &MainWindow::startDownload);
    connect(stopButton_, &QPushButton::clicked, this, &MainWindow::stopDownload);
}

void MainWindow::appendLog(const QString &message) {
    const QString ts = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    logOutput_->appendPlainText(QStringLiteral("[%1] %2").arg(ts, message));
}

void MainWindow::chooseDirectory() {
    const QString selected = QFileDialog::getExistingDirectory(this, QStringLiteral("选择保存目录"), localDirEdit_->text());
    if (!selected.isEmpty()) localDirEdit_->setText(selected);
}

void MainWindow::scanVersions() {
    QUrl url;
    if (!validateInputs(&url)) return;
    model_->setItems({});
    applyState(UiState::Scanning);
    scanner_->scan(url);
}

void MainWindow::startDownload() {
    QUrl url;
    if (!validateInputs(&url)) return;
    const QString dir = localDirEdit_->text().trimmed();
    if (state_ == UiState::Downloading || state_ == UiState::Scanning) {
        appendLog(QStringLiteral("当前流程正在执行，请勿重复启动"));
        return;
    }
    if (!QDir().mkpath(dir)) {
        appendLog(QStringLiteral("无法创建目录：%1").arg(dir));
        return;
    }
    if (model_->items().isEmpty() || state_ == UiState::Finished || state_ == UiState::Idle || state_ == UiState::Failed) {
        autoStartDownloadAfterScan_ = true;
        model_->setItems({});
        applyState(UiState::Scanning);
        scanner_->scan(url);
        return;
    }
    applyState(UiState::Downloading);
    downloader_->start(model_->items(), dir);
}

void MainWindow::stopDownload() {
    applyState(UiState::Stopping);
    downloader_->stop();
}

bool MainWindow::validateInputs(QUrl *urlOut) {
    const QString text = remoteUrlEdit_->text().trimmed();
    const QString dir = localDirEdit_->text().trimmed();
    remoteUrlEdit_->setText(text);
    localDirEdit_->setText(dir);
    if (text.isEmpty()) { appendLog(QStringLiteral("参数错误：远程地址为空")); return false; }
    if (dir.isEmpty()) { appendLog(QStringLiteral("参数错误：本地目录为空")); return false; }
    QUrl url = QUrl::fromUserInput(text);
    if (!url.isValid() || (url.scheme() != QStringLiteral("http") && url.scheme() != QStringLiteral("https"))) {
        appendLog(QStringLiteral("参数错误：请输入有效 HTTP/HTTPS 地址"));
        return false;
    }
    if (urlOut) *urlOut = url;
    return true;
}

void MainWindow::applyState(UiState state) { state_ = state; }

void MainWindow::updateStatsUi(const DownloadManager::Statistics &stats) {
    QString totalSize = formatBytes(stats.totalKnownBytes);
    if (stats.totalFiles > 0 && stats.totalKnownBytes >= 0) {
        int unknown = 0;
        for (const auto &item : model_->items()) if (item.size < 0) ++unknown;
        if (unknown > 0) totalSize += QStringLiteral(" + %1个未知文件").arg(unknown);
    }
    statsLabel_->setText(QStringLiteral("文件 %1/%2 失败:%3 跳过:%4 | 总大小:%5 | 已下载:%6 | 速度:%7/s | 已用:%8 | 剩余:%9")
                             .arg(stats.finishedFiles).arg(stats.totalFiles).arg(stats.failedFiles).arg(stats.skippedFiles)
                             .arg(totalSize).arg(formatBytes(stats.downloadedBytes)).arg(formatBytes(stats.currentSpeedBytesPerSecond))
                             .arg(formatDuration(stats.elapsedSeconds)).arg(stats.estimatedRemainingSeconds >= 0 ? formatDuration(stats.estimatedRemainingSeconds) : QStringLiteral("未知")));
    int progress = 0;
    if (stats.totalKnownBytes > 0) progress = static_cast<int>((stats.downloadedBytes * 100) / stats.totalKnownBytes);
    else if (stats.totalFiles > 0) progress = (stats.finishedFiles * 100) / stats.totalFiles;
    overallProgress_->setValue(qBound(0, progress, 100));
}

QString MainWindow::formatBytes(qint64 bytes) {
    if (bytes < 0) return QStringLiteral("未知");
    double value = static_cast<double>(bytes);
    const char *units[] = {"B", "KB", "MB", "GB"};
    int i = 0;
    while (value >= 1024.0 && i < 3) { value /= 1024.0; ++i; }
    return i == 0 ? QStringLiteral("%1 B").arg(static_cast<qint64>(value)) : QStringLiteral("%1 %2").arg(QString::number(value, 'f', 1), units[i]);
}

QString MainWindow::formatDuration(qint64 seconds) {
    return QStringLiteral("%1:%2").arg(seconds / 60, 2, 10, QLatin1Char('0')).arg(seconds % 60, 2, 10, QLatin1Char('0'));
}

void MainWindow::loadSettings() {
    QSettings s(QStringLiteral("VersionDownloadTool"), QStringLiteral("VersionDownloadTool"));
    remoteUrlEdit_->setText(s.value(QStringLiteral("remoteUrl")).toString());
    localDirEdit_->setText(s.value(QStringLiteral("localDir")).toString());
}

void MainWindow::saveSettings() {
    QSettings s(QStringLiteral("VersionDownloadTool"), QStringLiteral("VersionDownloadTool"));
    s.setValue(QStringLiteral("remoteUrl"), remoteUrlEdit_->text().trimmed());
    s.setValue(QStringLiteral("localDir"), localDirEdit_->text().trimmed());
}
