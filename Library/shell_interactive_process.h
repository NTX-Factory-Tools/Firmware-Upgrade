#ifndef SHELL_INTERACTIVE_PROCESS_H
#define SHELL_INTERACTIVE_PROCESS_H

#include <QObject>
#include <QProcess>

#include "Shell_Interactive_global.h"
#include "shell_interactive.h"

class Shell_Interactive_Process_Private;
class SHELL_INTERACTIVE_EXPORT Shell_Interactive_Process : public Shell_Interactive
{
	Q_OBJECT

public:
	explicit Shell_Interactive_Process(QObject* parent = nullptr);
	~Shell_Interactive_Process();

	QString program() const;
	void setProgram(const QString &program);

	QStringList arguments() const;
	void setArguments(const QStringList &arguments);

	bool ShowErrorAsStandard() const;
	void setShowErrorAsStandard(bool newShowErrorAsStandard);

	QProcess::ExitStatus exitStatus() const;

	bool isRunning() const;


public slots:
	void Connect();

	void Disconnect();

	void Send_Data(QByteArray Data);

private:
	Shell_Interactive_Process_Private*	d;
};

#endif // SHELL_INTERACTIVE_PROCESS_H

