#include "tcpipobj.h"
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QThread>

TcpipObj::TcpipObj(QObject *parent) : QObject(parent)
{

}
TcpipObj::~TcpipObj()
{
     removeFile();
}
void TcpipObj::init()
{
    m_strCommFileName = "";
    m_strTimingFileName = "";
    m_strIniFileName = "";
    m_socketObj = TCPIP_NULL;
    m_clientIp = "";
    m_clientPort = 0;
//    m_sPattern = QString("(.*)%1(.*)%2(.*)").arg(",").arg(",");
}
void TcpipObj::setCommFileName(const QString &fileName)
{
    m_strCommFileName = fileName;
}
void TcpipObj::setTimingFileName(const QString &fileName)
{
    m_strTimingFileName = fileName;
}
void TcpipObj::setIniFileName(const QString &fileName)
{
    if(fileName.isEmpty())
    {
        QFile::remove(m_strIniFileName);
    }
    m_strIniFileName = fileName;
}
void TcpipObj::setRegExpPattern(const QString &split)
{
    if(!split.isEmpty())
    {
        m_sPattern = QString("(.*)%1(.*)%2(.*)").arg(split).arg(split);
    }
}
void TcpipObj::createObj(const QString &ip, int port,
                         QString &prefix, QString &suffix,
                         SocketObj index, int clientPort)
{
    bool connSuccess = false;
    if(TCPIP_SERVER == index)//服务器
    {
        server = new TcpIpServer(this,prefix,suffix);
        connect(server,&TcpIpServer::serverReadData,this,&TcpipObj::server_ReadData);
        connect(server,&TcpIpServer::serverClientConnect,this,
                [this](const QString &ip, const int &port)
        {
            emit log(tr("客户端%1 %2已连接!").arg(ip).arg(port),SHOW_NULL);
            emit showServerSendIpPort(ip,QString("%1").arg(port));
        });
        connect(server,&TcpIpServer::serverClientDisconnected,this,
                [this](const QString &ip, const int &port)
        {
            emit log(tr("断开服务器%1 %2连接!").arg(ip).arg(port),SHOW_NULL);
            emit showServerSendIpPort("","");
        });
        connect(server,&TcpIpServer::serverErrorMsg,this,
                [this](const QString &errorMsg)
        {
            emit log(tr("%1").arg(errorMsg),SHOW_NULL);
        });

        if(server->openListen(ip,(quint16)port))
        {
            m_socketObj = TCPIP_SERVER;
            emit log(tr("%1 %2服务器创建成功!").arg(ip).arg(port),SHOW_NULL);
            connSuccess = true;
        }
        else
        {
            m_socketObj = TCPIP_NULL;
            emit log(tr("%1 %2服务器创建失败!").arg(ip).arg(port),SHOW_NULL);
            connSuccess = false;
        }
    }
    else
    {
        if(TCPIP_CLIENT == index)//客户端
        {
            client = new TcpIpClient(this,prefix,suffix);
            connect(client,&TcpIpClient::clientReadData,this,&TcpipObj::client_ReadData);
            connect(client,&TcpIpClient::clientDisconnect,this,
                    [this,index](const QString &ip, const int &port)
            {
                emit log(tr("服务器%1 %2断开连接!").arg(ip).arg(port),SHOW_NULL);
                emit createSuccess(index,false);
            });
            connect(client,&TcpIpClient::clientErrorMsg,this,
                    [this](const QString &errorMsg)
            {
                emit log(tr("%1").arg(errorMsg),SHOW_NULL);
            });
            if(0 < clientPort)
                client->bind(clientPort);

            if(client->newConnect(ip, (quint16&)port))
            {
//                client_disconn = false;
                m_socketObj = TCPIP_CLIENT;
                emit log(tr("连接服务器%1 %2成功!").arg(ip).arg(port),SHOW_NULL);
                connSuccess = true;
            }
            else
            {
//                client_disconn = true;
                m_socketObj = TCPIP_NULL;
                emit log(tr("连接服务器%1 %2失败!").arg(ip).arg(port),SHOW_NULL);
                connSuccess = false;
            }
        }
        else
        {
            m_socketObj = TCPIP_NULL;
        }
    }
    emit createSuccess(index,connSuccess);
    if(!m_strIniFileName.isEmpty() && connSuccess)
    {
        QFile::remove(m_strIniFileName);
    }
}
void TcpipObj::server_ReadData(const QString &ip, const int &port, const QString &readMsg)
{
    QString strReadData = readMsg;
    emit log(tr("%1 %2:%3").arg(ip).arg(port).arg(strReadData),SHOW_RECEIVE);
    strReadData.replace(server->getServerPrefix(),"");
    strReadData.replace(server->getServerSuffix(),"");
    int sleep_time = 0;
    if(!m_strCommFileName.isEmpty())
    {
        checkMsg(strReadData,sleep_time);
        if(!strReadData.isEmpty())
        {
            QThread::msleep(sleep_time);
            QString strSendMsg = QString("%1%2%3")
                    .arg(server->getServerPrefix()).arg(strReadData).arg(server->getServerSuffix());
            server->serverSendData((quint16&) port,strSendMsg);
            emit log(tr("%1 %2:%3").arg(ip).arg(port).arg(strSendMsg),SHOW_SENDER);
            if(!m_strTimingFileName.isEmpty())
            {
                check_timerMsg(strReadData);
            }
        }
    }
}
void TcpipObj::client_ReadData(const QString &ip, const int &port, const QString &readMsg)
{
    QString strReadData = readMsg;
    emit log(tr("%1 %2:%3").arg(ip).arg(port).arg(strReadData),SHOW_RECEIVE);
    strReadData.replace(client->getClientPrefix(),"");
    strReadData.replace(client->getClientSuffix(),"");
    int sleep_time = 0;
    if(!m_strCommFileName.isEmpty())
    {
        checkMsg(strReadData,sleep_time);
        if(!strReadData.isEmpty())
        {
            QThread::msleep(sleep_time);
            QString strSendMsg = QString("%1%2%3")
                    .arg(client->getClientPrefix()).arg(strReadData).arg(client->getClientSuffix());
            client->clientSendData(strSendMsg);
            emit log(tr("%1 %2:%3").arg(ip).arg(port).arg(strSendMsg),SHOW_SENDER);
            if(!m_strTimingFileName.isEmpty())
            {
                check_timerMsg(strReadData);
            }
        }
    }
}
void TcpipObj::checkMsg(QString &msg, int &sleep_time)
{
    QString strTemp = msg;
    QFile file(m_strCommFileName);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream txtInput(&file);
        QString strLine;
        QRegExp regMsg(m_sPattern);
        while(!txtInput.atEnd())
        {
            strLine = txtInput.readLine();
            if(0 <= strLine.indexOf(regMsg))
            {
                if(regMsg.cap(1).contains(msg))
                {
                    msg = regMsg.cap(2);
                    bool ok;
                    sleep_time = regMsg.cap(3).toInt(&ok,10);
                    if(!ok)
                    {
                        sleep_time = 500;
                    }
                    break;
                }
            }
        }
        file.close();
    }
    if(strTemp == msg)
    {
        msg = "";
    }
    if(0 >= sleep_time)
    {
        sleep_time = 500;
    }
}
void TcpipObj::check_timerMsg(QString &sendMsg)
{
    QFile file(m_strTimingFileName);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream txtInput(&file);
        QString strLine;
        QRegExp regMsg(m_sPattern);
        while(!txtInput.atEnd())
        {
            strLine = txtInput.readLine();
            if(0 <= strLine.indexOf(regMsg))
            {
                if(regMsg.cap(1).contains(sendMsg))
                {
                    QString msg_temp = regMsg.cap(2);
                    bool ok;
                    int timer_time = regMsg.cap(3).toInt(&ok,10);
                    if(0>=timer_time || !ok)
                    {
                        timer_time = 1000;
                    }
                    QString strID = QString("%1").arg(this->startTimer(timer_time));
                    QSettings *configWrite = new QSettings(m_strIniFileName, QSettings::IniFormat);
                    configWrite->setValue(strID, msg_temp);
                    delete configWrite;
                    break;
                }
            }
        }
        file.close();
    }
}
void TcpipObj::timerEvent(QTimerEvent *event)
{
    QString strTimerID = QString("%1").arg(event->timerId());
    QSettings *configRead = new QSettings(m_strIniFileName, QSettings::IniFormat);
    QString strMsg = configRead->value(strTimerID).toString();
    delete configRead;
    if(!strMsg.isEmpty())
    {
        if(TCPIP_SERVER==m_socketObj)
        {
            QString strSendMsg = QString("%1%2%3")
                    .arg(server->getServerPrefix()).arg(strMsg).arg(server->getServerSuffix());
            server->serverSendData((quint16&) m_clientPort,strSendMsg);
            emit log(strSendMsg,SHOW_SENDER);
            if(!m_strTimingFileName.isEmpty())
            {
                check_timerMsg(strSendMsg);
            }
        }
        else
        {
            if(TCPIP_CLIENT==m_socketObj)
            {
                QString strSendMsg = QString("%1%2%3")
                        .arg(client->getClientPrefix()).arg(strMsg).arg(client->getClientSuffix());
                client->clientSendData(strSendMsg);
                emit log(strSendMsg,SHOW_SENDER);
                if(!m_strTimingFileName.isEmpty())
                {
                    check_timerMsg(strSendMsg);
                }
            }
        }
    }
    this->killTimer(event->timerId());
}
void TcpipObj::setServerSendIpPort(const QString &ip, const QString &port)
{
    bool ok;
    int iPort = port.toInt(&ok,10);
    if(ok)
    {
        m_clientIp = ip;
        m_clientPort = iPort;
    }
}
void TcpipObj::manualSendMsg(SocketObj index, const QString &msg)
{
    if(msg.isEmpty())
        return;
    if(TCPIP_SERVER==index)
    {
        if(m_clientIp.isEmpty() || 0==m_clientPort)
            return;
        QString strSendMsg = QString("%1%2%3")
                .arg(server->getServerPrefix()).arg(msg).arg(server->getServerSuffix());
        server->serverSendData((quint16&) m_clientPort,strSendMsg);
        emit log(strSendMsg,SHOW_SENDER);
        if(!m_strTimingFileName.isEmpty())
        {
            check_timerMsg(strSendMsg);
        }
    }
    else
    {
        if(TCPIP_CLIENT==index)
        {
            QString strSendMsg = QString("%1%2%3")
                    .arg(client->getClientPrefix()).arg(msg).arg(client->getClientSuffix());
            client->clientSendData(strSendMsg);
            emit log(strSendMsg,SHOW_SENDER);
            if(!m_strTimingFileName.isEmpty())
            {
                check_timerMsg(strSendMsg);
            }
        }
    }
}
void TcpipObj::deleteObj(SocketObj index)
{
    if(TCPIP_SERVER==index)
    {
        server->deleteServer();
        emit log(tr("服务器已删除!"),SHOW_NULL);
    }
    else
    {
        if(TCPIP_CLIENT==index)
        {
            client->closeConnect();
            emit log(tr("客户端已删除!"),SHOW_NULL);
        }
    }
    emit createSuccess(index,false);
    removeFile();
    init();
}
void TcpipObj::removeFile()
{
    if(!m_strIniFileName.isEmpty())
    {
        QFile::remove(m_strIniFileName);
    }
}
