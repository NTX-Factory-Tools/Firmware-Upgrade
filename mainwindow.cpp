#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDir>
#include <QMessageBox>
#include <QFinalState>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog_about.h"
#include "Library/shell_interactive_process.h"
#include "Library/Process_state.h"


#define	FIRMWARE_INI	"Firmware.ini"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
, ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString Version = QCoreApplication::applicationVersion();
    this->setWindowTitle(tr("Firmware Upgrade V") + Version);

    m_Shell = new Shell_Interactive_Process(this);
    m_Shell->setProgram("/bin/bash");
    m_Shell->Connect();

    m_searcher = new File_Searcher(this);

    connect(this, &MainWindow::inputData,
    m_Shell, &Shell_Interactive::Send_Data);
    connect(m_searcher, &File_Searcher::fileFound,
    this, &MainWindow::add_FW_Files);
    connect(m_searcher, &File_Searcher::searchFinished,
    this, &MainWindow::add_FW_Files_Done);

    m_Machine = new QStateMachine(this);

    QTimer::singleShot(0, this, &MainWindow::init);
}

MainWindow::~MainWindow()
{
	m_Shell->Disconnect();

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

	m_FWLocation = currentText;
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

    QTimer::singleShot(1000, this, &MainWindow::start);
}

void MainWindow::add_FW_Files(QString fileName)
{
    QString relativeFilePath = QDir(m_Mounts[ui->radioButton_SD->isChecked()?0:1]).relativeFilePath(fileName);

    ui->listWidget->addItem(relativeFilePath);
}

void MainWindow::add_FW_Files_Done()
{
    qDebug() << "搜尋結束";

    ui->groupBox_Source->setEnabled(true);
    ui->pushButton_Rescan->setEnabled(true);
}

bool MainWindow::checkDevice(QString device)
{
	QFileInfo dev(device);

	return dev.exists();
}

void MainWindow::on_pushButton_Rescan_clicked()
{
	ui->listWidget->clear();
    ui->pushButton_Rescan->setEnabled(false);
    ui->groupBox_Source->setEnabled(false);

    delete m_Machine;
    m_Machine = new QStateMachine(this);

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
    QString configPath = QCoreApplication::applicationDirPath() + "/" + FIRMWARE_INI;
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

    ui->statusbar->showMessage("");
    m_Worker = new QThread;
    m_copy = new FileCopyer(m_Worker);
    QObject::connect(m_copy, &FileCopyer::finished, this, &MainWindow::finished);
    QObject::connect(m_copy, &FileCopyer::copyProgress, [=](qint64 copy, qint64 total) {
        qDebug() << QStringLiteral("PROGRESS => %1").arg(qreal(copy) / qreal(total) * 100.0);
        ui->progressBar->setValue(qreal(copy) / qreal(total) * 100.0);
    });
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
     ui->pushButton_Size_Check->show();
     ui->listWidget->setEnabled(true);
}

void MainWindow::start()
{
    QFileInfo* info;
    QStringList source;
    QStringList destination;

    m_upgrade = new FW_Upgrade(m_FWLocation);
    m_upgrade->fdisk();
    m_upgrade->mk_boot();

    info = new QFileInfo(m_upgrade->dtb());
    source << info->absoluteFilePath();
    destination << QString("/tmp/disk_boot/%1").arg(info->fileName());

    info = new QFileInfo(m_upgrade->kernel());
    source << info->absoluteFilePath();
    destination << QString("/tmp/disk_boot/%1").arg(info->fileName());

    info = new QFileInfo(m_upgrade->ext4());
    source << info->absoluteFilePath();
    destination << m_upgrade->emmc_root();

    m_copy->setSourcePaths(source);
    m_copy->setDestinationPaths(destination);
    m_Worker->start();
}
