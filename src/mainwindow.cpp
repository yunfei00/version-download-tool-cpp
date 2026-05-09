#include "mainwindow.h"

#include <QDateTime>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
}

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
    remoteUrlEdit_->setPlaceholderText(QStringLiteral("请输入版本列表地址"));

    auto *localLabel = new QLabel(QStringLiteral("本地保存目录"), configWidget);
    localDirEdit_ = new QLineEdit(configWidget);
    localDirEdit_->setPlaceholderText(QStringLiteral("请选择保存目录"));

    chooseDirButton_ = new QPushButton(QStringLiteral("选择目录"), configWidget);
    scanButton_ = new QPushButton(QStringLiteral("扫描"), configWidget);
    downloadButton_ = new QPushButton(QStringLiteral("开始下载"), configWidget);

    configLayout->addWidget(remoteLabel);
    configLayout->addWidget(remoteUrlEdit_);
    configLayout->addWidget(localLabel);
    configLayout->addWidget(localDirEdit_);
    configLayout->addWidget(chooseDirButton_);
    configLayout->addWidget(scanButton_);
    configLayout->addWidget(downloadButton_);
    configLayout->addStretch();

    fileTable_ = new QTableWidget(0, 5, central);
    fileTable_->setHorizontalHeaderLabels({
        QStringLiteral("序号"),
        QStringLiteral("文件名"),
        QStringLiteral("大小"),
        QStringLiteral("状态"),
        QStringLiteral("进度")
    });
    fileTable_->horizontalHeader()->setStretchLastSection(true);
    fileTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    fileTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    fileTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    topLayout->addWidget(configWidget, 0);
    topLayout->addWidget(fileTable_, 1);

    logOutput_ = new QPlainTextEdit(central);
    logOutput_->setReadOnly(true);
    logOutput_->setPlaceholderText(QStringLiteral("日志输出..."));
    logOutput_->setMaximumBlockCount(1000);

    mainLayout->addLayout(topLayout, 3);
    mainLayout->addWidget(logOutput_, 1);

    connect(chooseDirButton_, &QPushButton::clicked, this, &MainWindow::chooseDirectory);
    connect(scanButton_, &QPushButton::clicked, this, &MainWindow::scanVersions);
    connect(downloadButton_, &QPushButton::clicked, this, &MainWindow::startDownload);
}

void MainWindow::appendLog(const QString &message) {
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    logOutput_->appendPlainText(QStringLiteral("[%1] %2").arg(timestamp, message));
}

void MainWindow::chooseDirectory() {
    const QString selected = QFileDialog::getExistingDirectory(this, QStringLiteral("选择保存目录"), localDirEdit_->text());
    if (!selected.isEmpty()) {
        localDirEdit_->setText(selected);
        appendLog(QStringLiteral("已选择目录：%1").arg(selected));
    } else {
        appendLog(QStringLiteral("取消选择目录"));
    }
}

void MainWindow::scanVersions() {
    appendLog(QStringLiteral("点击扫描，远程地址：%1").arg(remoteUrlEdit_->text()));

    fileTable_->setRowCount(3);
    const QStringList names = {QStringLiteral("core.zip"), QStringLiteral("assets.zip"), QStringLiteral("patch.zip")};
    const QStringList sizes = {QStringLiteral("15 MB"), QStringLiteral("42 MB"), QStringLiteral("8 MB")};
    for (int i = 0; i < names.size(); ++i) {
        fileTable_->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        fileTable_->setItem(i, 1, new QTableWidgetItem(names[i]));
        fileTable_->setItem(i, 2, new QTableWidgetItem(sizes[i]));
        fileTable_->setItem(i, 3, new QTableWidgetItem(QStringLiteral("待下载")));
        fileTable_->setItem(i, 4, new QTableWidgetItem(QStringLiteral("0%")));
    }
}

void MainWindow::startDownload() {
    appendLog(QStringLiteral("点击开始下载，保存目录：%1").arg(localDirEdit_->text()));

    for (int row = 0; row < fileTable_->rowCount(); ++row) {
        if (auto *statusItem = fileTable_->item(row, 3)) {
            statusItem->setText(QStringLiteral("已完成（模拟）"));
        }
        if (auto *progressItem = fileTable_->item(row, 4)) {
            progressItem->setText(QStringLiteral("100%"));
        }
    }
}
