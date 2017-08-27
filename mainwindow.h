#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "tcpipobj.h"
#include <QThread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void setCommFileName(const QString &fileName);
    void setTimingFileName(const QString &fileName);
    void createObj(const QString &ip, int port, QString &prefix, QString &suffix, SocketObj index, int clientPort = 0);
    void setServerSendIpPort(const QString &strIp, const QString &strPort);
    void manualSendMsg(SocketObj index,const QString &msg);
    void deleteObj(SocketObj index);

public slots:
    void log(const QString &msg, const ShowMsg &index);
    void createSuccess(const SocketObj &index, const bool &success);

private slots:
    void on_checkBox_saveLog_clicked();

    void on_pushButton_clear_clicked();

    void on_pushButton_loadFile_clicked();

    void on_pushButton_timer_clicked();

    void on_pushButton_creat_clicked();

    void on_pushButton_delete_clicked();

    void on_pushButton_send_clicked();

    void on_lineEdit_serverIP_send_returnPressed();

    void on_lineEdit_serverPort_send_returnPressed();

    void on_comboBox_server_client_currentIndexChanged(int index);

    void on_checkBox_clientPort_clicked();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
    TcpipObj *m_pTcpip;
    QThread *m_pThread;
    QString m_strLogFileName;

    void init();
    void saveLog(const QString &msg);
};

#endif // MAINWINDOW_H
