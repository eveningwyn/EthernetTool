#ifndef TCPIPSERVER_H
#define TCPIPSERVER_H

/*需要在.pro文件添加QT += network*/
#include <QTcpServer>
#include "tcpipclient.h"

class TcpIpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpIpServer(QObject *parent = 0, QString prefix = "", QString suffix = "");
    bool openListen(const QString &address, quint16 port); //监听连接
    void closeServerListen();//关闭监听
    void deleteServer();//删除服务器对象
    void setServerPrefix(const QString &prefix);
    void setServerSuffix(const QString &suffix);
    QString getServerPrefix();
    QString getServerSuffix();
    void serverSendData(const quint16 &port, const QString &sendMsg);    //发送数据

private:
    QList<TcpIpClient *> clientSocketList;  //保存客户端对象集合
    QString m_sPrefix;
    QString m_sSuffix;

protected:
    void incomingConnection(qintptr socketDescriptor);  //虚函数，有Tcp请求时会触发

signals:
    void serverReadData(const QString &ip, const int &port, const QString &readMsg);   //读取数据信号
    void serverClientConnect(const QString &ip, const int &port);                    //发送已连接的客户端IP地址和Port信息
    void serverClientDisconnected(const QString &ip, const int &port);               //发送已断开连接的客户端IP地址和Port信息
    void serverErrorMsg(const QString &errorMsg);                        //错误信号

private slots:
    void disconnect(const QString &ip, const int &port);                      //获得已断开的客户端信息，并发送信号给上层调用

};

#endif // TCPIPSERVER_H
