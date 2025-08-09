#ifndef SHELL_INTERACTIVE_H
#define SHELL_INTERACTIVE_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "Shell_Interactive_global.h"

class SHELL_INTERACTIVE_EXPORT Shell_Interactive : public QObject
{
	Q_OBJECT

public:
	explicit Shell_Interactive(QObject* parent = nullptr) : QObject(parent) {}
	virtual ~Shell_Interactive() {}

	virtual void setProgram(const QString& program) = 0;
	virtual QString program() const = 0;

	virtual void setArguments(const QStringList& arguments) = 0;
	virtual QStringList arguments() const = 0;

	virtual void Connect() = 0;
	virtual void Disconnect() = 0;

	virtual void Send_Data(QByteArray Data) = 0;
	virtual bool isRunning() const = 0;

signals:
	void Data_Ready(const QString& message);
	void Error_Message(const QString& message);
};

#endif // SHELL_INTERACTIVE_H
