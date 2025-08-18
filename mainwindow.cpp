#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDir>
#include <QMessageBox>
#include <QFinalState>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog_about.h"
#include "Shell_Interactive_Process.h"
#include "Process_State.h"

#define	FIRMWARE_INI    "Firmware.ini"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
, ui(new Ui::MainWindow)
, m_Machine(nullptr)
{
    ui->setupUi(this);
    ui->progressBar->hide();
    ui->pushButton_PowerOff->setEnabled(false);
    ui->pushButton_Size_Check->hide();

    QString Version = QCoreApplication::applicationVersion();
    this->setWindowTitle(tr("Firmware Upgrade V") + Version);

    m_Shell = new Shell_Interactive_Process(this);
    m_Shell->setProgram("/bin/bash");
    m_Shell->Connect();
    connect(this, &MainWindow::inputData, m_Shell, &Shell_Interactive::Send_Data);

    m_searcher = new File_Searcher(this);

    QTimer::singleShot(0, this, &MainWindow::init);
}

MainWindow::~MainWindow()
{
    m_Shell->Disconnect();
    delete m_Shell;
    delete m_searcher;
    delete m_Machine;

    delete ui;
}



void MainWindow::on_actionClose_triggered()
{
    QCoreApplication::quit();
}

void MainWindow::on_actionAbout_triggered()
{
    Dialog_About* About = new Dialog_About(this);
    About->show();
}

void MainWindow::on_listWidget_currentTextChanged(const QString &currentText)
{
    ui->pushButton_Burn->setEnabled(!currentText.isEmpty());

    m_FWLocation = m_Mounts[ui->radioButton_SD->isChecked()?0:1] + "/" + currentText;
}



void MainWindow::on_pushButton_Size_Check_clicked()
{

}


void MainWindow::on_pushButton_PowerOff_clicked()
{
    system("halt -p");
}


void MainWindow::on_pushButton_Burn_clicked()
{
    ui->groupBox_Source->setEnabled(false);
    ui->pushButton_Rescan->setEnabled(false);
    ui->progressBar->show();
    ui->pushButton_Burn->hide();
    ui->pushButton_PowerOff->hide();
    ui->pushButton_Size_Check->hide();
    ui->listWidget->setEnabled(false);

    connect(m_searcher, &File_Searcher::fileFound, this, &MainWindow::found_FW_File);
    m_searcher->setSearchForFiles(true);
    m_searcher->startSearch(m_FWLocation, "*.dtb;*.bin;*.ext4");
    ui->statusbar->showMessage("Prepare firmware upgrading ...");
}

void MainWindow::add_FW_Folder(QString fileName)
{
    QString relativeFilePath = QDir(m_Mounts[ui->radioButton_SD->isChecked()?0:1]).relativeFilePath(fileName);

    ui->listWidget->addItem(relativeFilePath);
}

void MainWindow::add_FW_Folder_Done()
{
    ui->statusbar->showMessage("");

    disconnect(m_searcher, &File_Searcher::fileFound, this, &MainWindow::add_FW_Folder);
    disconnect(m_searcher, &File_Searcher::searchFinished, this, &MainWindow::add_FW_Folder_Done);

    ui->groupBox_Source->setEnabled(true);
    ui->pushButton_Rescan->setEnabled(true);
}

void MainWindow::found_FW_File(QString fileName)
{
    qDebug() << "found FW file:" << fileName;

    QFileInfo info(fileName);

    if(info.suffix() == "dtb")
    {
        m_FWFiles.insert("dtb", fileName);
    }
    if(info.suffix() == "bin")
    {
        m_FWFiles.insert("bin", fileName);
    }
    if(info.suffix() == "ext4")
    {
        m_FWFiles.insert("ext4", fileName);
    }

    if(m_FWFiles.count() == 3)
        QTimer::singleShot(0, this, &MainWindow::start);
}

bool MainWindow::checkDevice(QString device)
{
    QFileInfo dev(device);

    return dev.exists();
}

void MainWindow::on_pushButton_Rescan_clicked()
{
    ui->statusbar->showMessage("Scanning firmware files ...");
    ui->listWidget->clear();
    ui->pushButton_Rescan->setEnabled(false);
    ui->groupBox_Source->setEnabled(false);

    if(m_Machine != nullptr)
        delete m_Machine;
    m_Machine = new QStateMachine(this);

    connect(m_searcher, &File_Searcher::fileFound, this, &MainWindow::add_FW_Folder);
    connect(m_searcher, &File_Searcher::searchFinished, this, &MainWindow::add_FW_Folder_Done);

    QFinalState* Done = new QFinalState(m_Machine);
    Process_State* checkMount = new Process_State("checkMount", m_Shell, 2000, m_Machine);
    Process_State* mountDisk = new Process_State("mountDisk", m_Shell, m_Machine);
    Process_State* fwSearch = new Process_State("fwSearch", m_Shell, m_Machine);

    checkMount->on_Succeeded(fwSearch);
    if(ui->radioButton_SD->isChecked())
    checkMount->on_Timeout(mountDisk);
    else
    checkMount->on_Timeout(Done);
    checkMount->setStart([this]() -> void {
        m_Shell->Send_Data("df\n");
    });
    checkMount->setFunction([this](const QString &line) -> Process_State::SWITCH_STATE
    {
        Process_State::SWITCH_STATE result = Process_State::SWITCH_STATE::STATE_KEEP_GOING;

        if(line.startsWith(m_Devices[ui->radioButton_SD->isChecked()?0:1]))
        {
            result = Process_State::SWITCH_STATE::STATE_SUCCESS;

            static const QRegularExpression sep("\\s+");
            QStringList columns = line.split(sep, Qt::SkipEmptyParts);

            if (columns.size() >= 6) {
                m_Mounts[ui->radioButton_SD->isChecked()?0:1] = columns[5];
            }
        }

        return result;
    });

    mountDisk->on_Succeeded(fwSearch);
    mountDisk->setStart([this]() -> void {
        QDir().mkpath(m_Mounts[ui->radioButton_SD->isChecked()?0:1]);
        QString command = QString("mount %1 %2\n")
        .arg(m_Devices[ui->radioButton_SD->isChecked()?0:1])
        .arg(m_Mounts[ui->radioButton_SD->isChecked()?0:1]);
        qDebug() << command;
        m_Shell->Send_Data(command.toLocal8Bit());
        m_Shell->Send_Data("echo $?\n");
    });
    mountDisk->setFunction([](const QString &line) -> Process_State::SWITCH_STATE
    {
        Q_UNUSED(line)
        return Process_State::SWITCH_STATE::STATE_SUCCESS;
    });

    fwSearch->on_Succeeded(Done);
    fwSearch->setStart([this]() -> void {
        m_Shell->Send_Data("echo $?\n");
    });
    fwSearch->setFunction([this](const QString &line) -> Process_State::SWITCH_STATE
    {
        Process_State::SWITCH_STATE result = Process_State::SWITCH_STATE::STATE_KEEP_GOING;

        if(line == "0")
        {
            result = Process_State::SWITCH_STATE::STATE_SUCCESS;

            m_searcher->startSearch(m_Mounts[ui->radioButton_SD->isChecked()?0:1], "FW_*");
        }

        return result;
    });

    m_Machine->setInitialState(checkMount);
    m_Machine->start();
}

void MainWindow::on_radioButton_SD_clicked()
{
    on_pushButton_Rescan_clicked();
}

void MainWindow::on_radioButton_USB_clicked()
{
    on_pushButton_Rescan_clicked();
}

void MainWindow::init()
{
    QString configPath = "/spada/config/" FIRMWARE_INI;
    QSettings Config(configPath, QSettings::IniFormat, this);

    // check ini file settings
    Config.beginGroup("device");
    m_Devices.append(Config.value("sdcard").toString());
    m_Devices.append(Config.value("usb").toString());
    if(!checkDevice(m_Devices[0]))
    {
        ui->radioButton_SD->setEnabled(false);
        ui->radioButton_USB->setChecked(true);
    }
    Config.endGroup();
    Config.beginGroup("mount");
    m_Mounts.append(Config.value("firmwareDisk").toString());
    m_Mounts.append("");
    Config.endGroup();

    on_pushButton_Rescan_clicked();
}


void MainWindow::finished(bool success)
{
    qDebug() << (success ? "FINISHED" : "FAILED");

    if(success)
    {
        m_upgrade->final();

        QMessageBox::information(this, tr("Done"), tr("Firmware upgraded"));
    }
    else
    {
        QMessageBox::information(this, tr("Fail"), tr("Firmware upgrade fail"));
    }

    ui->groupBox_Source->setEnabled(true);
    ui->pushButton_Rescan->setEnabled(true);
    ui->progressBar->hide();
    ui->pushButton_Burn->show();
    ui->pushButton_PowerOff->show();
    ui->pushButton_PowerOff->setEnabled(true);
    // ui->pushButton_Size_Check->show();
    ui->listWidget->setEnabled(true);
}

void MainWindow::start()
{
    QFileInfo* info;

    ui->statusbar->showMessage("Checking partition ...");

    m_upgrade = new FW_Upgrade(m_FWLocation);
    m_upgrade->fdisk();
    m_upgrade->mk_boot();

    ui->statusbar->showMessage("Programing firmware ...");

    m_Copy = new File_Copy(this);
    QObject::connect(m_Copy, &File_Copy::progress, this, &MainWindow::on_CopyProgress);
    QObject::connect(m_Copy, &File_Copy::finished, this, &MainWindow::on_CopyFinished);
    // QObject::connect(m_Copy, &File_Copy::error, this, &MainWindow::on_CopyError);

    m_Copy->setOverwrite(true);

    m_Copy->setSource(m_FWFiles.value("ext4"));
    m_FWSize.insert("ext4", m_Copy->fileSize());
    m_Copy->setSource(m_FWFiles.value("bin"));
    m_FWSize.insert("bin", m_Copy->fileSize());
    m_Copy->setSource(m_FWFiles.value("dtb"));
    m_FWSize.insert("dtb", m_Copy->fileSize());
    m_FWSize.insert("total", m_FWSize.value("dtb") + m_FWSize.value("bin") + m_FWSize.value("ext4"));
    qDebug() << m_FWSize.value("total");

    info = new QFileInfo(m_Copy->source());
    m_Copy->setDestination(QString("/tmp/disk_boot/%1").arg(info->fileName()));
    // m_Copy->setDestination(QString("/tmp/%1").arg(info->fileName()));
    m_Copy->copy();
}

void MainWindow::on_CopyProgress(const qint64 size, const qint64 total)
{
    Q_UNUSED(total)

    QFileInfo* info = new QFileInfo(m_Copy->source());
    quint64 copied = size;
    quint64 copied_total = m_FWSize.value("total");;

    if(info->suffix() == "bin")
    {
        copied += m_FWSize.value("dtb");
    }
    if(info->suffix() == "ext4")
    {
        copied += m_FWSize.value("dtb");
        copied += m_FWSize.value("bin");
    }

    qDebug() << "copy size:" << size << "total %" << (copied*100/copied_total);
    ui->progressBar->setValue((copied*100/copied_total));
}

void MainWindow::on_CopyFinished(bool status)
{
    qDebug() << "copy status:" << status;

    if(!status)
    {
        finished(status);
        return;
    }

    QFileInfo* info = new QFileInfo(m_Copy->source());
    if(info->suffix() == "ext4")
    {
        qDebug() << "done";
        finished(status);
    }

    if(info->suffix() == "bin")
    {
        m_Copy->setSource(m_FWFiles.value("ext4"));
        info = new QFileInfo(m_Copy->source());
        m_Copy->setDestination(m_upgrade->emmc_root());
        // m_Copy->setDestination("/dev/null");
        m_Copy->copy();
    }

    if(info->suffix() == "dtb")
    {
        m_Copy->setSource(m_FWFiles.value("bin"));
        info = new QFileInfo(m_Copy->source());
        m_Copy->setDestination(QString("/tmp/disk_boot/%1").arg(info->fileName()));
        // m_Copy->setDestination(QString("/tmp/%1").arg(info->fileName()));
        m_Copy->copy();
    }


}

// void MainWindow::on_CopyError(QString error)
// {
//     qDebug() << error;
// }








// #define KB  (1024)
// #define MB  (1024*KB)
// #define GB  (1024*MB)

// void MainWindow::on_pushButton_Size_Check_clicked()
// {
//     qint64 Size = 0, minSize = 3.0 * GB;
//     m_disk.mount_root();
//     Size = m_disk.root_disk_size();
//     m_disk.umount_root();

//     qDebug() << "disk size:" << Size << "KB";
//     QMessageBox::information(this, tr("Disk Size"), QString("eMMC root disk size: %1 KB").arg(Size));

//     if(Size < minSize)
//     {
//         if(QMessageBox::warning(this, tr("Disk Size"), "Resize eMMC root disk?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
//         {
//             m_upgrade = new FW_Upgrade(m_FW_Location);
//             m_upgrade->resize2fs();
//             on_pushButton_Size_Check_clicked();
//         }
//     }
// }
