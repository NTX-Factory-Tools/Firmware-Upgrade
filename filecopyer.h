/*
* This is a minimal example to show thread-based QFile copy operation with progress notfication.
* See here for QFile limitations: https://doc.qt.io/qt-5/qfile.html#platform-specific-issues
* Copyright (C) 2019 Iman Ahmadvand
*
* This is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* It is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*/

#ifndef FILECOPYER_H
#define FILECOPYER_H

#include <QObject>
#include <QVector>
#include <QThread>


static const int DEFAULT_CHUNK_SIZE = 1024 * 2024 * 1;

class FileCopyer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qint64 chunksize READ chunkSize WRITE setChunkSize)
    Q_PROPERTY(QStringList sourcePaths READ sourcePaths WRITE setSourcePaths)
    Q_PROPERTY(QStringList destinationPaths READ destinationPaths WRITE setDestinationPaths)


public:
    explicit FileCopyer(QThread* thread, QObject *parent = nullptr);
    ~FileCopyer();

    qint64 chunkSize() const;
    void setChunkSize(qint64 newChunkSize);
    QStringList sourcePaths() const;
    void setSourcePaths(const QStringList newsourcePaths);
    QStringList destinationPaths() const;
    void setDestinationPaths(const QStringList newdestinationPaths);


signals:
    void copyProgress(qint64 bytesCopied, qint64 bytesTotal);
    void finished(bool success);

private:
    QStringList m_sourcePaths;
    QStringList m_destinationPaths;
    qint64      m_chunkSize;


protected slots:
    void copy();

};

#endif // FILECOPYER_H
