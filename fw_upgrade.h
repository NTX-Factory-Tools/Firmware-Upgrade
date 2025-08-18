#ifndef FW_UPGRADE_H
#define FW_UPGRADE_H

#include <QObject>
#include <QProcess>

class FW_Upgrade : public QObject
{
    Q_OBJECT
public:
    explicit FW_Upgrade(QString Source_Dir, QObject *parent = nullptr);

    const QString emmc_root() const;

public slots:
    bool fdisk();
    bool mk_boot();
    bool mk_root();
    void final();

    bool resize2fs();

signals:

private slots:
    QString Filename(QString Extention);

    void QProcess_Message();

private:
    QString     m_Source_Dir;

    QString     m_dtb;    //=`find /boot/FW -name "Image-*.dtb" | head -n 1`
    QString     m_kernel; //=`find /boot/FW -name "Image-*.bin" | head -n 1`
    QString     m_ext4;   //=`find /boot/FW -name "*-spada2e*.ext4" | head -n 1`
    QString     m_md5sum; //=`find /boot/FW -name "fw.md5sum"`

    QProcess    m_Run;
};

#endif // FW_UPGRADE_H
