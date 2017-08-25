#include "tcpipserver.h"
#include <QException>
//#include <QMutex>

TcpIpServer::TcpIpServer(QObject *parent, QString prefix, QString suffix):
    QTcpServer(parent)
{
    clientSocketList.clear();//清空客户端对象集合
    m_sPrefix = prefix;
    m_sSuffix = suffix;
}

bool TcpIpServer::openListen(const QString &address, quint16 port)
{   //IP地址 (e.g., "127.0.0.1").
    if(!this->isListening())
    {
        QHostAddress addr(address);
        if(!this->listen(addr,port))
        {
            emit serverErrorMsg(this->errorString());
            this->close();
            return false;
        }
    }
    return true;
}

void TcpIpServer::closeServerListen()
{
    this->close();
}

void TcpIpServer::incomingConnection(qintptr socketDescriptor)
{
    TcpIpClient *clientSocket = new TcpIpClient;
    if (!clientSocket->setSocketDescriptor(socketDescriptor))
    {
        emit serverErrorMsg(clientSocket->errorString());
        delete clientSocket;
        return;
    }
    clientSocket->setClientPrefix(this->m_sPrefix);
    clientSocket->setClientSuffix(this->m_sSuffix);

    connect(clientSocket,&TcpIpClient::disconnected,clientSocket,&TcpIpClient::deleteLater);
    connect(clientSocket,&TcpIpClient::clientReadData,this,&TcpIpServer::serverReadData);
    connect(clientSocket,&TcpIpClient::clientDisconnect,this,&TcpIpServer::disconnect);
    connect(clientSocket,&TcpIpClient::clientErrorMsg,this,&TcpIpServer::serverErrorMsg);

    emit serverClientConnect(clientSocket->peerAddress().toString(),(int)clientSocket->peerPort());//发送已连接的客户端信息

    clientSocketList.append(clientSocket);//将新的客户端连接对象添加到列表
}

void TcpIpServer::setServerPrefix(const QString &prefix)
{
    this->m_sPrefix = prefix;
}

void TcpIpServer::setServerSuffix(const QString &suffix)
{
    this->m_sSuffix = suffix;
}

QString TcpIpServer::getServerPrefix()
{
    return this->m_sPrefix;
}

QString TcpIpServer::getServerSuffix()
{
    return this->m_sSuffix;
}

//指定客户端连接发消息
void TcpIpServer::serverSendData(const quint16 &port, const QString &sendMsg)
{
    QByteArray sendByte = sendMsg.toLatin1();
    for (int i=0;i<clientSocketList.count();i++)
    {
        if (clientSocketList[i]->peerPort()==port)
        {
            clientSocketList[i]->write(sendByte);
            return;
        }
    }
    emit serverErrorMsg(tr("The Socket does not exist!\n"));
}

void TcpIpServer::disconnect(const QString &ip, const int &port)
{
    for (int i=0;i<clientSocketList.count();i++)
    {
        if(clientSocketList[i]->peerAddress().toString()==ip
                && clientSocketList[i]->peerPort()==port)
        {
            clientSocketList[i] = NULL;
            clientSocketList[i]->deleteLater();
            clientSocketList.removeAt(i);//从列表中移除该连接
            emit serverClientDisconnected(ip,port);
            break;
        }
    }
}

void TcpIpServer::deleteServer()
{
    for (int i=0;i<clientSocketList.count();i++)
    {
        clientSocketList[i]=NULL;
        clientSocketList[i]->deleteLater();
    }
    this->deleteLater();
    clientSocketList.clear();
}
