#include "tcpipclient.h"
#include <QHostAddress>

#define READ_TIME 10
TcpIpClient::TcpIpClient(QObject *parent, QString prefix, QString suffix) :
    QTcpSocket(parent)
{
    byteReadBuffer = "";
    m_sPrefix = prefix;
    m_sSuffix = suffix;
    m_pReadTimer = new QTimer(this);
    m_iReadTimeout = READ_TIME;

    connect(this,&TcpIpClient::readyRead,this,&TcpIpClient::clientReadyRead);
//    connect(this,&TcpIpClient::disconnected,this,&TcpIpClient::DisConnect);//关闭连接时，发送断开连接信号
    connect(this,&TcpIpClient::disconnected,this,
            [this](){
        emit clientDisconnect(this->peerAddress().toString(),this->peerPort());
    });//关闭连接时，发送断开连接信号


    connect(this,&TcpIpClient::disconnected,this,&TcpIpClient::deleteLater);//关闭连接时，对象自动删除
    connect(m_pReadTimer,&QTimer::timeout,this,&TcpIpClient::tcpipReadTimeout);
}

bool TcpIpClient::newConnect(const QString &address, const quint16 &port)
{
//    this->abort();//关闭已有连接
    this->connectToHost(QHostAddress(address),port);
    if (!this->waitForConnected(3000))
    {
        emit clientErrorMsg(this->errorString());
        return false;
    }
    return true;
}

void TcpIpClient::clientSendData(const QString &sendMsg)
{
    QByteArray sendByte = sendMsg.toLatin1();
    this->write(sendByte);
}

void TcpIpClient::setTcpipReadTimeoutTime(const int &msec)
{
    m_iReadTimeout = msec;
    if(READ_TIME > m_iReadTimeout)//超时时间定义为不小于预设时间(ms)
    {
        m_iReadTimeout = READ_TIME;
    }
}

void TcpIpClient::tcpipReadTimeout()
{
    if(m_pReadTimer->isActive())
    {
        m_pReadTimer->stop();
    }
    byteReadBuffer = "";
}

void TcpIpClient::setClientPrefix(const QString &prefix)
{
    this->m_sPrefix = prefix;
}

void TcpIpClient::setClientSuffix(const QString &suffix)
{
    this->m_sSuffix = suffix;
}

QString TcpIpClient::getClientPrefix()
{
    return this->m_sPrefix;
}

QString TcpIpClient::getClientSuffix()
{
    return this->m_sSuffix;
}

void TcpIpClient::clientReadyRead()
{
    QByteArray byteBuf;     //接收数据缓冲区
    byteBuf = this->readAll();
    byteReadBuffer.append(byteBuf);
    if(!m_pReadTimer->isActive())
    {
        m_pReadTimer->start(m_iReadTimeout);
    }

    if(m_sPrefix.isEmpty() && m_sSuffix.isEmpty())
    {//如果无前缀无后缀，直接读取返回
        emit clientReadData(this->peerAddress().toString(),this->peerPort(),QString(byteReadBuffer));
        tcpipReadTimeout();
        return;
    }
    QByteArray pre = m_sPrefix.toLatin1();     //获得前缀
    QByteArray suf = m_sSuffix.toLatin1();     //获得后缀

    if(1024<=byteReadBuffer.length())
    {
        emit clientReadData(this->peerAddress().toString(),this->peerPort(),QString(byteReadBuffer));
        tcpipReadTimeout();
        return;
    }
    /*判断是否接收完毕*/
    if(!m_sPrefix.isEmpty() && m_sSuffix.isEmpty())
    {   //如果有前缀无后缀
        if(0 == byteReadBuffer.indexOf(pre))
        {
            emit clientReadData(this->peerAddress().toString(),this->peerPort(),QString(byteReadBuffer));
            tcpipReadTimeout();
            return;
        }
    }
    else
    {
        if(m_sPrefix.isEmpty() && !m_sSuffix.isEmpty())
        {   //如果无前缀有后缀
            if(byteReadBuffer.size() == byteReadBuffer.lastIndexOf(suf) + suf.size())
            {
                emit clientReadData(this->peerAddress().toString(),this->peerPort(),QString(byteReadBuffer));
                tcpipReadTimeout();
                return;
            }
        }
        else
        {
            if(!m_sPrefix.isEmpty() && !m_sSuffix.isEmpty())
            {   //如果有前缀有后缀
                if(0 == byteReadBuffer.indexOf(pre)
                        && byteReadBuffer.size() == byteReadBuffer.lastIndexOf(suf) + suf.size())
                {
                    emit clientReadData(this->peerAddress().toString(),this->peerPort(),QString(byteReadBuffer));
                    tcpipReadTimeout();
                    return;
                }
            }
        }
    }
    byteBuf.clear();
}

void TcpIpClient::closeConnect()
{
    this->disconnectFromHost();
//    if(!(this->state() == QAbstractSocket::UnconnectedState || this->waitForDisconnected(1000)))
//        emit clientErrorMsg(this->errorString()+"\n");
}

//void TcpIpClient::DisConnect()
//{
//    //断开连接时，发送断开信号
//    emit clientDisconnect(this->peerAddress().toString(),this->peerPort());
//}
