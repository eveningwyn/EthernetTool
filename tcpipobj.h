#ifndef TCPIPOBJ_H
#define TCPIPOBJ_H

#include <QObject>
#include "tcpipserver.h"
#include "tcpipclient.h"
#include "language.h"
#include <QEvent>
#include <QMap>

enum SocketObj
{
    TCPIP_SERVER = 0,
    TCPIP_CLIENT,
    TCPIP_NULL,
};
enum ShowMsg
{
    SHOW_SENDER = 0,
    SHOW_RECEIVE,
    SHOW_NULL,
};
class TcpipObj : public QObject
{
    Q_OBJECT
public:
    explicit TcpipObj(QObject *parent = 0);
    ~TcpipObj();

private:
    TcpIpServer *server;
    TcpIpClient *client;
    QString m_strCommFileName;
    QString m_strTimingFileName;
    SocketObj m_socketObj;
    QString m_clientIp;
    int m_clientPort;
    QString m_sPattern;
    QMap <QString, QString> m_map;

    void checkMsg(QString &msg, int &sleep_time);
    void check_timerMsg(QString &sendMsg);

protected:
    virtual void timerEvent(QTimerEvent *event);

signals:
    void log(const QString &msg, const ShowMsg &index);
    void createSuccess(const SocketObj &index, const bool &success);
    void showServerSendIpPort(const QString &ip, const QString &port);

public slots:
    void init();
    void setCommFileName(const QString &fileName);
    void setTimingFileName(const QString &fileName);
    void createObj(const QString &ip, int port, QString &prefix, QString &suffix, SocketObj index,int clientPort);
    void setServerSendIpPort(const QString &ip, const QString &port);
    void manualSendMsg(SocketObj index,const QString &msg);
    void deleteObj(SocketObj index);
    void setRegExpPattern(const QString &split);
    void clearMap();

    void server_ReadData(const QString &ip, const int &port, const QString &readMsg);

    void client_ReadData(const QString &ip, const int &port,const QString &readMsg);
};

#endif // TCPIPOBJ_H
