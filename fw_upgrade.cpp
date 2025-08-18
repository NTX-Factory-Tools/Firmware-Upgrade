#include <QDebug>
#include <QDir>

#include "fw_upgrade.h"

//#support model are:
//# 1: spada2e
//# 2: spada2l
#if HARDWARE_MODEL == 1
    #define EMMC_BLOCK          "/dev/mmcblk1"
    #define SD_BLOCK            "/dev/mmcblk0"
#else
    #define EMMC_BLOCK          "/dev/mmcblk0"
    #define SD_BLOCK            "/dev/mmcblk1"
#endif

#define SD_BOOT_PARTITION   SD_BLOCK"p1"
#define SD_ROOT_PARTITION   SD_BLOCK"p2"
#define MMC_BOOT_PARTITION  EMMC_BLOCK"p1"
#define MMC_ROOT_PARTITION  EMMC_BLOCK"p2"




FW_Upgrade::FW_Upgrade(QString Source_Dir, QObject *parent) : QObject(parent)
{
    m_Source_Dir = Source_Dir;

    m_dtb = Filename("dtb");
    m_kernel = Filename("bin");
    m_ext4 = Filename("ext4");
    m_md5sum = Filename("md5sum");

    qDebug() << m_dtb << m_kernel << m_ext4 << m_md5sum;

    connect(&m_Run, &QProcess::readyReadStandardOutput, this, &FW_Upgrade::QProcess_Message);
    connect(&m_Run, &QProcess::readyReadStandardError, this, &FW_Upgrade::QProcess_Message);

}

QString FW_Upgrade::Filename(QString Extention)
{
    QDir dir(m_Source_Dir);
    QFileInfoList files = dir.entryInfoList(QStringList() << QString("*.%1").arg(Extention), QDir::Files|QDir::NoDotAndDotDot);

    if(files.count()==0)    return "";
    return files[0].absoluteFilePath();
}

void FW_Upgrade::QProcess_Message()
{
    static int Stage = 0;

    QString Message = m_Run.readAllStandardOutput() + "\n" + m_Run.readAllStandardError();
    qDebug() << Stage << "QProcess_Message:" << Message;

    if(Stage == 0 && Message.contains("Command (m for help):"))
    {
        Stage++;
        m_Run.write("o\n");
    }

    // first partition
    else if(Stage == 1 && Message.contains("Command (m for help):"))
    {
        Stage++;
        m_Run.write("n\n");
    }
    else if(Stage == 2 && Message.contains("Select (default p):"))
    {
        Stage++;
        m_Run.write("p\n");
    }
    else if(Stage == 3 && Message.contains("Partition number"))
    {
        Stage++;
        m_Run.write("1\n");
    }
    else if(Stage == 4 && Message.contains("First sector"))
    {
        Stage++;
        m_Run.write("2048\n");
    }
    else if(Stage == 5 && Message.contains("Last sector,"))
    {
        Stage++;
        m_Run.write("206847\n");
    }

    // second partition
    else if(Stage == 6 && Message.contains("Command (m for help):"))
    {
        Stage = 10;
        m_Run.write("n\n");
    }
    else if(Stage == 10 && Message.contains("Select (default p):"))
    {
        Stage++;
        m_Run.write("p\n");
    }
    else if(Stage == 11 && Message.contains("Partition number"))
    {
        Stage++;
        m_Run.write("2\n");
    }
    else if(Stage == 12 && Message.contains("First sector"))
    {
        Stage++;
        m_Run.write("206848\n");
    }
    else if(Stage == 13 && Message.contains("Last sector,"))
    {
        Stage++;
        m_Run.write("\n");
    }

    //active partition
    else if(Stage == 14 && Message.contains("Command (m for help):"))
    {
        Stage = 20;
        m_Run.write("a\n");
    }
    else if(Stage == 20 && Message.contains("Partition number"))
    {
        Stage++;
        m_Run.write("1\n");
    }

    //partition type change
    else if(Stage == 21 && Message.contains("Command (m for help):"))
    {
        Stage = 30;
        m_Run.write("t\n");
    }
    else if(Stage == 30 && Message.contains("Partition number"))
    {
        Stage++;
        m_Run.write("1\n");
    }
    else if(Stage == 31 && Message.contains("Hex code (type L to list all codes):"))
    {
        Stage++;
        m_Run.write("c\n");
    }

    //save and quit
    else if(Stage == 32 && Message.contains("Command (m for help):"))
    {
        Stage = 0;
        m_Run.write("wq\n");
    }
}

// const QString &FW_Upgrade::ext4() const
// {
//     return m_ext4;
// }

const QString FW_Upgrade::emmc_root() const
{
    return QString(MMC_ROOT_PARTITION);
}

// const QString &FW_Upgrade::kernel() const
// {
//     return m_kernel;
// }

// const QString &FW_Upgrade::dtb() const
// {
//     return m_dtb;
// }

bool FW_Upgrade::fdisk()
{
    qDebug() << "Run fdisk()";

    m_Run.start("fdisk", QStringList() << EMMC_BLOCK);
    m_Run.waitForFinished();


    QProcess::ExitStatus Status = m_Run.exitStatus();
    qDebug() << "fdisk:" << (Status == 0);

    return (Status == 0);
}

bool FW_Upgrade::mk_boot()
{
    m_Run.start("mkfs.vfat", QStringList() << "-n" << "\"BOOT\"" << MMC_BOOT_PARTITION);
    m_Run.waitForFinished();

    QProcess::ExitStatus Status = m_Run.exitStatus();
    if(Status == 0)
    {
        m_Run.start("mkdir", QStringList() << "/tmp/disk_boot");
        m_Run.waitForFinished();

        m_Run.start("mount", QStringList() << MMC_BOOT_PARTITION << "/tmp/disk_boot");
        m_Run.waitForFinished();
    }

    qDebug() << "mk_boot:" << (Status == 0);
    return (Status == 0);
}

bool FW_Upgrade::mk_root()
{
    m_Run.start("mkfs.ext4", QStringList() << "-F" << "-L" << "\"ROOT\"" << MMC_ROOT_PARTITION);
//    m_Run.write("y\n");
    QProcess::ExitStatus Status = m_Run.exitStatus();
    qDebug() << "mkfs:" << (Status == 0);

    bool result = resize2fs();
    qDebug() << "mk_root:" << result;

    return result;
}

void FW_Upgrade::final()
{
    resize2fs();

    m_Run.start("umount", QStringList() << "/tmp/disk_boot");
    m_Run.waitForFinished();

    m_Run.start("umount", QStringList() << "/boot");
    m_Run.waitForFinished();
}

bool FW_Upgrade::resize2fs()
{
    qDebug() << "e2fsck" << "-y" << "-f" << MMC_ROOT_PARTITION;

    m_Run.start("e2fsck", QStringList() << "-y" << "-f" << MMC_ROOT_PARTITION);
    m_Run.waitForFinished();

    qDebug() << "resize2fs" << MMC_ROOT_PARTITION;

    m_Run.start("resize2fs", QStringList() << "-f" << MMC_ROOT_PARTITION);
    m_Run.waitForFinished();

    return true;
}

