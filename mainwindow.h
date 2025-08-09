#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QStateMachine>

#include "fw_upgrade.h"
#include "filecopyer.h"

#include "Library/file_searcher.h"
#include "Library/shell_interactive.h"

// test2
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

signals:
	void inputData(QByteArray Data);

private slots:
	void on_actionClose_triggered();

	void on_actionAbout_triggered();

	void on_pushButton_Size_Check_clicked();

	void on_pushButton_PowerOff_clicked();

	void on_pushButton_Burn_clicked();

	void add_FW_Files(QString fileName);

	void add_FW_Files_Done();

	void on_pushButton_Rescan_clicked();

	void on_listWidget_currentTextChanged(const QString &currentText);

	void on_radioButton_SD_clicked();

	void on_radioButton_USB_clicked();


	void finished(bool success);

	void start();

private:
	void init();

	bool checkDevice(QString device);

private:
    Ui::MainWindow*     ui;
    Shell_Interactive*  m_Shell;
    QStateMachine*      m_Machine;
		File_Searcher*      m_searcher;
		FW_Upgrade*					m_upgrade;
		FileCopyer*					m_copy;
		QThread*						m_Worker;

    QStringList         m_Devices;
    QStringList         m_Mounts;
    QString             m_FWLocation;
};
#endif // MAINWINDOW_H
