#ifndef FLASH_STATE_H
#define FLASH_STATE_H

#include <QState>
#include <QTimer>
#include <functional>

#include "Shell_Interactive_global.h"

class Shell_Interactive;
class Process_State_Private;
class SHELL_INTERACTIVE_EXPORT Process_State : public QState
{
    Q_OBJECT
public:
    typedef enum SWITCH_STATE {
        STATE_FAIL,
        STATE_SUCCESS,
        STATE_KEEP_GOING,
        STATE_TIMTOUT,
        STATE_USER1,
        STATE_USER2,
        STATE_USER3,
        STATE_USER4,
        STATE_USER5,
        STATE_USER6,
        STATE_USER7,
        STATE_USER8,
        STATE_USER9,
    }switchState;
    Q_ENUM(SWITCH_STATE)

public:
    Process_State(QString name, Shell_Interactive* shell, QState* parent = nullptr);
    Process_State(QString name, Shell_Interactive* shell, ulong timeOut, QState* parent = nullptr);
    ~Process_State();

    void setFunction(std::function<SWITCH_STATE(const QString &)> handler);
    void setStart(std::function<void(void)> handler);

public slots:
    void on_Succeeded(QAbstractState* success);
    void on_Fail(QAbstractState* fail);
    void on_Timeout(QAbstractState* timeout);
    void on_User1(QAbstractState* user1);
    void on_User2(QAbstractState* user2);
    void on_User3(QAbstractState* user3);
    void on_User4(QAbstractState* user4);
    void on_User5(QAbstractState* user5);
    void on_User6(QAbstractState* user6);
    void on_User7(QAbstractState* user7);
    void on_User8(QAbstractState* user8);
    void on_User9(QAbstractState* user9);

signals:
    void taskSucceeded();
    void taskFailed();
    void taskTimeout();
    void taskUser1();
    void taskUser2();
    void taskUser3();
    void taskUser4();
    void taskUser5();
    void taskUser6();
    void taskUser7();
    void taskUser8();
    void taskUser9();

protected:
  void onEntry(QEvent *event) override;
  void onExit(QEvent *event) override;

private:
  std::unique_ptr<Process_State_Private> d;
};

#endif // FLASH_STATE_H
