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
    });

    connect(downloader_, &DownloadManager::logMessage, this, &MainWindow::appendLog);
    connect(downloader_, &DownloadManager::rowStatusChanged, this, [this](int row, const QString &status, int progress) {
        model_->updateRow(row, status, progress);
    });
    connect(downloader_, &DownloadManager::currentRowChanged, this, [this](int row) {
        fileTable_->scrollTo(model_->index(row, 0), QAbstractItemView::PositionAtCenter);
    });
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

    mainLayout->addLayout(topLayout, 3);
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
    const QString text = remoteUrlEdit_->text().trimmed();
    remoteUrlEdit_->setText(text);
    if (text.isEmpty()) {
        appendLog(QStringLiteral("参数错误：远程地址为空"));
        return;
    }
    QUrl url = QUrl::fromUserInput(text);
    if (!url.isValid() || (url.scheme() != QStringLiteral("http") && url.scheme() != QStringLiteral("https"))) {
        appendLog(QStringLiteral("参数错误：请输入有效 HTTP/HTTPS 地址"));
        return;
    }
    scanner_->scan(url);
}

void MainWindow::startDownload() {
    const QString dir = localDirEdit_->text().trimmed();
    localDirEdit_->setText(dir);
    if (dir.isEmpty()) {
        appendLog(QStringLiteral("参数错误：本地目录为空"));
        return;
    }
    if (model_->items().isEmpty()) {
        appendLog(QStringLiteral("没有可下载文件，请先扫描"));
        return;
    }
    if (!QDir().mkpath(dir)) {
        appendLog(QStringLiteral("无法创建目录：%1").arg(dir));
        return;
    }
    downloader_->start(model_->items(), dir);
}

void MainWindow::stopDownload() { downloader_->stop(); }

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
