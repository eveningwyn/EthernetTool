#ifndef TCPIPCLIENT_H
#define TCPIPCLIENT_H

/*需要在.pro文件添加QT += network*/
#include <QTcpSocket>
#include <QTimer>

class TcpIpClient : public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpIpClient(QObject *parent = 0, QString prefix = "", QString suffix = "");

    bool newConnect(const QString &address, const quint16 &port);//连接服务器
    void setClientPrefix(const QString &prefix);
    void setClientSuffix(const QString &suffix);
    QString getClientPrefix();
    QString getClientSuffix();
    void clientSendData(const QString &sendMsg);//客户端发送数据
    void closeConnect();//断开连接

private:
    QString m_sPrefix;
    QString m_sSuffix;
    QByteArray byteReadBuffer;//接收的字符串信息
    QTimer *m_pReadTimer;
    int m_iReadTimeout;

signals:
    void clientReadData(const QString &ip, const int &port,const QString &readMsg);//客户端接收数据之后，发送信号给上层调用，根据客户端信息解析
    void clientDisconnect(const QString &ip, const int &port);//客户端断开连接之后，发送断开的客户端信息信号给上层调用
    void clientErrorMsg(const QString &errorMsg);

private slots:
    void clientReadyRead();//客户端接收数据
    void tcpipReadTimeout();
//    void DisConnect();

public slots:
    void setTcpipReadTimeoutTime(const int &msec);

};

#endif // TCPIPCLIENT_H
