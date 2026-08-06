#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QProgressBar>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QThread>

class ConversionWorker;

class MainWindow : public QMainWindow {
    Q_OBJECT;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private Q_SLOTS:
    void startConversion();
    void openDownloadsFolder();
    void onConversionFinished();
    void onErrorReported(const QString& fileName, const QString& errorMessage);

private:
    QLabel *dropZoneLabel;
    QListWidget *fileList;
    QProgressBar *progressBar;
    QPushButton *convertButton;
    QPushButton *downloadButton;

    QStringList filesToConvert;
    QStringList conversionErrors; // Added to collect errors
    const int MAX_FILES = 10;

    QThread *workerThread;
    ConversionWorker *worker;
    QString converterExecutable;
};

#endif // MAINWINDOW_H