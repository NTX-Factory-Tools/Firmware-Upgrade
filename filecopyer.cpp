#include <QDebug>
#include <QFileInfo>

#include "filecopyer.h"

FileCopyer::FileCopyer(QThread *thread, QObject *parent) : QObject(parent)
{
    moveToThread(thread);
    setChunkSize(DEFAULT_CHUNK_SIZE);

    QObject::connect(thread, &QThread::started, this, &FileCopyer::copy);
    QObject::connect(this, &FileCopyer::finished, thread, &QThread::quit);
    QObject::connect(this, &FileCopyer::finished, this, &FileCopyer::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
}

FileCopyer::~FileCopyer()
{

}

qint64 FileCopyer::chunkSize() const
{
    return m_chunkSize;
}

void FileCopyer::setChunkSize(qint64 newChunkSize)
{
    m_chunkSize = newChunkSize;
}

QStringList FileCopyer::sourcePaths() const
{
    return m_sourcePaths;
}

void FileCopyer::setSourcePaths(const QStringList newsourcePaths)
{
    m_sourcePaths = newsourcePaths;
}

QStringList FileCopyer::destinationPaths() const
{
    return m_destinationPaths;
}

void FileCopyer::setDestinationPaths(const QStringList newdestinationPaths)
{
    m_destinationPaths = newdestinationPaths;
}

void FileCopyer::copy()
{
    if (m_sourcePaths.isEmpty() || m_destinationPaths.isEmpty()) {
            qWarning() << QStringLiteral("source or destination paths are empty!");
            emit finished(false);
            return;
        }

        if (m_sourcePaths.count() != m_destinationPaths.count()) {
            qWarning() << QStringLiteral("source or destination paths doesn't match!");
            emit finished(false);
            return;
        }

        qint64 total = 0, written = 0;
        foreach(QString f , m_sourcePaths)
            total += QFileInfo(f).size();
        qInfo() << QStringLiteral("%1 bytes should be write in total").arg(total);

        int indx = 0;
        qInfo() << QStringLiteral("writing with chunk size of %1 byte").arg(chunkSize());
        while (indx < m_sourcePaths.count()) {
            const auto dstPath = m_destinationPaths.at(indx);
            QFile srcFile(m_sourcePaths.at(indx));
            QFile dstFile(dstPath);

//            if (QFile::exists(dstPath)) {
//                qInfo() << QStringLiteral("file %1 alreasy exists, overwriting...").arg(dstPath);
//                QFile::remove(dstPath);
//            }

            if (!srcFile.open(QFileDevice::ReadOnly)) {
                qWarning() << QStringLiteral("failed to open %1 (error:%1)").arg(srcFile.errorString());
                indx++;
                continue; // skip
            }

            if (!dstFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
                qWarning() << QStringLiteral("failed to open %1 (error:%1)").arg(dstFile.errorString());
                indx++;
                continue; // skip
            }

            /* copy the content in portion of chunk size */
            qint64 fSize = srcFile.size();
            while (fSize) {
                const auto data = srcFile.read(chunkSize());
                const auto _written = dstFile.write(data);
//                srcFile.flush();
//                dstFile.flush();

                if (data.size() == _written)
                {
                    written += _written;
                    fSize -= data.size();
                    emit copyProgress(written, total);
                }
                else
                {
                    qWarning() << QStringLiteral("failed to write to %1 (error:%2) %3|%4 ").arg(dstFile.fileName()).arg(dstFile.errorString()).arg(data.size()).arg(_written);
                    fSize = 0;
                    break; // skip this operation
                }
            }

            srcFile.close();
            dstFile.close();
            indx++;
        }

        if (total == written) {
            qInfo() << QStringLiteral("progress finished, %1 bytes of %2 has been written").arg(written).arg(total);
            emit finished(true);
        } else {
            emit finished(false);
        }
}
