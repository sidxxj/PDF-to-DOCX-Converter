#ifndef CONVERSIONWORKER_H
#define CONVERSIONWORKER_H

#include <QObject>
#include <QStringList>
#include <QProcess>
#include <QThread>
#include <QFileInfo>
#include <QStandardPaths>
#include <QFile>
#include <QDir>

class ConversionWorker : public QObject {
    Q_OBJECT

public:
    explicit ConversionWorker(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void processFiles(const QStringList& files, const QString& executablePath) {
        int total = files.size();

        for (int i = 0; i < total; ++i) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }

            QString currentFile = files[i];
            QFileInfo fileInfo(currentFile);
            QString extension = fileInfo.suffix().toLower();
            QString outDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

            // 1. Source File Lock & Password Check
            QFile sourceFile(currentFile);
            if (!sourceFile.open(QIODevice::ReadOnly)) {
                emit errorReported(fileInfo.fileName(), "File is locked. Close it in other programs.");
                continue;
            }

            // Heuristic check for PDF Encryption
            if (extension == "pdf") {
                QByteArray headerData = sourceFile.read(4096);
                if (headerData.contains("/Encrypt")) {
                    emit errorReported(fileInfo.fileName(), "File is password protected/encrypted.");
                    sourceFile.close();
                    continue;
                }
            }
            sourceFile.close();

            // 2. Target File Overwrite Lock Check
            QString targetExt = (extension == "pdf") ? ".docx" : ".pdf";
            QString targetPath = QDir(outDir).filePath(fileInfo.completeBaseName() + targetExt);
            QFile targetFile(targetPath);

            if (targetFile.exists()) {
                if (!targetFile.open(QIODevice::ReadWrite | QIODevice::Append)) {
                    emit errorReported(fileInfo.fileName(), "Destination file is open in another program. Please close it.");
                    continue;
                }
                targetFile.close();
            }

            // 3. Dynamic Timeout Calculation (60s base + 30s per 10MB)
            qint64 fileSizeMB = fileInfo.size() / (1024 * 1024);
            int timeoutMs = 60000 + (fileSizeMB / 10) * 30000;

            QProcess process;
            QStringList arguments;
            arguments << "--headless";

            if (extension == "pdf") {
                arguments << "--infilter=writer_pdf_import" << "--convert-to" << "docx";
            } else if (extension == "docx" || extension == "doc") {
                arguments << "--convert-to" << "pdf";
            }

            arguments << "--outdir" << outDir << currentFile;
            process.start(executablePath, arguments);

            if (!process.waitForFinished(timeoutMs)) {
                emit errorReported(fileInfo.fileName(), "Process timed out. The file may be too complex.");
                process.kill();
                continue;
            }

            if (process.exitStatus() == QProcess::CrashExit) {
                emit errorReported(fileInfo.fileName(), "The conversion engine crashed.");
            } else if (process.exitCode() != 0) {
                QString errorOutput = QString::fromUtf8(process.readAllStandardError());
                if (errorOutput.isEmpty()) {
                    errorOutput = "Unknown error occurred (Exit code " + QString::number(process.exitCode()) + ").";
                }
                emit errorReported(fileInfo.fileName(), errorOutput.trimmed());
            }

            emit progressUpdate(((i + 1) * 100) / total);
        }

        emit finished();
    }

signals:
    void progressUpdate(int percentage);
    void finished();
    void errorReported(const QString& fileName, const QString& errorMessage);
};

#endif // CONVERSIONWORKER_H