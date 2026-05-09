#pragma once

#include <QMainWindow>

class QLineEdit;
class QPushButton;
class QPlainTextEdit;
class QTableWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void chooseDirectory();
    void scanVersions();
    void startDownload();

private:
    void setupUi();
    void appendLog(const QString &message);

    QLineEdit *remoteUrlEdit_ = nullptr;
    QLineEdit *localDirEdit_ = nullptr;
    QPushButton *chooseDirButton_ = nullptr;
    QPushButton *scanButton_ = nullptr;
    QPushButton *downloadButton_ = nullptr;
    QTableWidget *fileTable_ = nullptr;
    QPlainTextEdit *logOutput_ = nullptr;
};
