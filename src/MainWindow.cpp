#include "MainWindow.h"
#include "ConversionWorker.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>
#include <QFileInfo>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setAcceptDrops(true);
    setFixedSize(500, 400);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    workerThread = new QThread(this);
    worker = new ConversionWorker();
    worker->moveToThread(workerThread);

    dropZoneLabel = new QLabel("Drag and Drop up to 10 PDF/Word files here", this);
    dropZoneLabel->setAlignment(Qt::AlignCenter);
    dropZoneLabel->setStyleSheet("border: 2px dashed #aaa; border-radius: 5px; background: #f9f9f9; padding: 20px;");

    fileList = new QListWidget(this);
    progressBar = new QProgressBar(this);
    progressBar->setValue(0);

    convertButton = new QPushButton("Convert Files", this);
    convertButton->setEnabled(false);

    downloadButton = new QPushButton("Open Downloads", this);
    downloadButton->setEnabled(false);

    layout->addWidget(dropZoneLabel);
    layout->addWidget(fileList);
    layout->addWidget(progressBar);
    layout->addWidget(convertButton);
    layout->addWidget(downloadButton);

    setCentralWidget(centralWidget);

    connect(convertButton, &QPushButton::clicked, this, &MainWindow::startConversion);
    connect(downloadButton, &QPushButton::clicked, this, &MainWindow::openDownloadsFolder);
    connect(worker, &ConversionWorker::progressUpdate, progressBar, &QProgressBar::setValue);
    connect(worker, &ConversionWorker::finished, this, &MainWindow::onConversionFinished);
    connect(worker, &ConversionWorker::errorReported, this, &MainWindow::onErrorReported);
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);

    workerThread->start();

    QString libreOfficePath = QStandardPaths::findExecutable("libreoffice");
    if (libreOfficePath.isEmpty()) {
        libreOfficePath = QStandardPaths::findExecutable("soffice");
    }

    if (libreOfficePath.isEmpty()) {
        QMessageBox::critical(this, "Dependency Missing",
            "LibreOffice is required for conversion but was not found on your system. "
            "Please install it and ensure it is added to your system PATH.");
        convertButton->setToolTip("LibreOffice is missing.");
    } else {
        this->converterExecutable = libreOfficePath;
    }
}

MainWindow::~MainWindow() {
    workerThread->quit();
    workerThread->wait();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        QStringList rejectedFiles; // To notify user of bad files

        for (const QUrl &url : urlList) {
            QString filePath = url.toLocalFile();
            QFileInfo fileInfo(filePath);

            // 1. Zero-Byte Check
            if (fileInfo.size() == 0) {
                rejectedFiles.append(fileInfo.fileName() + " (Empty file)");
                continue;
            }

            // 2. Magic Number (Spoofed Extension) Check
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray header = file.read(8);
                file.close();

                bool isPDF = header.startsWith("%PDF");
                bool isDOCX = header.startsWith("PK\x03\x04");
                bool isDOC = header.startsWith("\xD0\xCF\x11\xE0"); // Older .doc format

                if (!isPDF && !isDOCX && !isDOC) {
                    rejectedFiles.append(fileInfo.fileName() + " (Invalid file format)");
                    continue;
                }

                if (filesToConvert.size() < MAX_FILES) {
                    if (!filesToConvert.contains(filePath)) {
                        filesToConvert.append(filePath);
                        fileList->addItem(filePath);
                    }
                } else {
                    QMessageBox::warning(this, "Limit Reached", "You can only convert up to 10 files at a time.");
                    break;
                }
            }
        }

        if (!rejectedFiles.isEmpty()) {
            QMessageBox::warning(this, "Files Rejected", "The following files were rejected:\n" + rejectedFiles.join("\n"));
        }

        if (!filesToConvert.isEmpty() && !converterExecutable.isEmpty()) {
            convertButton->setEnabled(true);
        }
    }
}

void MainWindow::startConversion() {
    convertButton->setEnabled(false);
    progressBar->setValue(0);
    conversionErrors.clear();

    QMetaObject::invokeMethod(worker, "processFiles", Qt::QueuedConnection,
            Q_ARG(QStringList, filesToConvert),
            Q_ARG(QString, converterExecutable));
}

void MainWindow::onErrorReported(const QString& fileName, const QString& errorMessage) {
    conversionErrors.append(QString("<b>%1:</b> %2").arg(fileName, errorMessage));
}

void MainWindow::onConversionFinished() {
    filesToConvert.clear();
    fileList->clear();
    downloadButton->setEnabled(true);

    if (conversionErrors.isEmpty()) {
        QMessageBox::information(this, "Success", "All files have been successfully converted!");
    } else {
        QMessageBox::warning(this, "Completed with Errors",
            "Some files could not be converted:<br><br>" + conversionErrors.join("<br>"));
    }
}

void MainWindow::openDownloadsFolder() {
    QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QDesktopServices::openUrl(QUrl::fromLocalFile(downloadsPath));
}