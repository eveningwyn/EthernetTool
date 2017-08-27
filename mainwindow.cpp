#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMutex>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QSettings>
#include <QRegExp>
#include <QMetaType>
#include <QMessageBox>

#define PRO_VERSION "V1.02"
#define BUILT_DATE "2017-08-27"
void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,NULL,QString(tr("\nVersion: %1\n"
                                            "\nBuilt on %2\n"
                                            "\n\t---evening.wen\n"))
                               .arg(PRO_VERSION).arg(BUILT_DATE));
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<QString>("QString&");
    qRegisterMetaType<SocketObj>("SocketObj");
    qRegisterMetaType<ShowMsg>("ShowMsg");

    ui->lineEdit_serverIP->setText("127.0.0.14");
    ui->lineEdit_serverPort->setText("8080");

    m_pTcpip = new TcpipObj;
    m_pThread = new QThread;

    connect(m_pThread,&QThread::started,m_pTcpip,&TcpipObj::init);

    connect(m_pTcpip,&TcpipObj::log,this,&MainWindow::log);
    connect(m_pTcpip,&TcpipObj::createSuccess,this,&MainWindow::createSuccess);
    connect(m_pTcpip,&TcpipObj::showServerSendIpPort,this,
            [this](const QString &ip, const QString &port)
    {
        ui->lineEdit_serverIP_send->setText(ip);
        ui->lineEdit_serverPort_send->setText(port);
        emit setServerSendIpPort(ip,port);
    });

    connect(this,&MainWindow::setCommFileName,m_pTcpip,&TcpipObj::setCommFileName);
    connect(this,&MainWindow::setTimingFileName,m_pTcpip,&TcpipObj::setTimingFileName);
    connect(this,&MainWindow::setIniFileName,m_pTcpip,&TcpipObj::setIniFileName);
    connect(this,&MainWindow::createObj,m_pTcpip,&TcpipObj::createObj);
    connect(this,&MainWindow::setServerSendIpPort,m_pTcpip,&TcpipObj::setServerSendIpPort);
    connect(this,&MainWindow::manualSendMsg,m_pTcpip,&TcpipObj::manualSendMsg);
    connect(this,&MainWindow::deleteObj,m_pTcpip,&TcpipObj::deleteObj);
    connect(ui->comboBox_split,&QComboBox::currentTextChanged,m_pTcpip,&TcpipObj::setRegExpPattern);

    this->init();
    m_pTcpip->moveToThread(m_pThread);
    m_pThread->start();
}

MainWindow::~MainWindow()
{
    m_pTcpip->removeFile();
    delete ui;
}

void MainWindow::init()
{
    QRegExp regExpIP("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    QRegExp regExpNetPort("((6553[0-5])|[655[0-2][0-9]|65[0-4][0-9]{2}|6[0-4][0-9]{3}|[1-5][0-9]{4}|[1-9][0-9]{3}|[1-9][0-9]{2}|[1-9][0-9]|[0-9])");

    ui->lineEdit_serverIP->setValidator(new QRegExpValidator(regExpIP,this));
    ui->lineEdit_serverPort->setValidator(new QRegExpValidator(regExpNetPort,this));
    ui->lineEdit_clientPort->setValidator(new QRegExpValidator(regExpNetPort,this));
    ui->lineEdit_serverIP_send->setValidator(new QRegExpValidator(regExpIP,this));
    ui->lineEdit_serverPort_send->setValidator(new QRegExpValidator(regExpNetPort,this));
    m_strLogFileName = "";
    ui->mainToolBar->hide();
    ui->comboBox_split->setCurrentIndex(1);
    ui->label_clientPort->hide();
    ui->checkBox_clientPort->hide();
    ui->lineEdit_clientPort->hide();
    ui->pushButton_send->setEnabled(false);
    ui->pushButton_delete->setEnabled(false);
}

void MainWindow::log(const QString &msg, const ShowMsg &index)
{
    static QMutex mutexLog;
    mutexLog.lock();
    QString time = QDateTime::currentDateTime().toString("yyyyMMdd_hh:mm:ss_zzz");
    QString senderStr;
    switch (index) {
    case SHOW_SENDER:
        senderStr = "Send to";
        break;
    case SHOW_RECEIVE:
        senderStr = "Receive from";
        break;
    case SHOW_NULL:
        senderStr = "";
        break;
    default:
        senderStr = "";
        break;
    }
    QString strMsg = QString("%1 %2:%3\n").arg(time).arg(senderStr).arg(msg);
    if(!ui->checkBox_pauseShow->isChecked())
    {
//        ui->textBrowser_show_msg->moveCursor(QTextCursor::End);
//        ui->textBrowser_show_msg->insertPlainText(strMsg);
//        ui->textBrowser_show_msg->moveCursor(QTextCursor::End);
        ui->textBrowser_show_msg->append(strMsg);
    }
    if(ui->checkBox_saveLog->isChecked())
    {
        saveLog(strMsg);
    }
    mutexLog.unlock();
}

void MainWindow::saveLog(const QString &msg)
{
    if(!m_strLogFileName.isEmpty())
    {
        QFile file(m_strLogFileName);
        if(file.open(QFile::Append | QIODevice::Text))
        {
            QTextStream out(&file);
            out << msg;
            if(!file.flush())
            {
                qWarning("log文件刷新失败!");
            }
            file.close();
        }
    }
}

void MainWindow::on_checkBox_saveLog_clicked()
{
    if(ui->checkBox_saveLog->isChecked())
    {
        m_strLogFileName = QFileDialog::getSaveFileName(this,tr("选择存储路径"),
                                                        "..\\Message_log.txt",
                                                        tr("Text files (*.txt)"));
        if(m_strLogFileName.isEmpty())
        {
            m_strLogFileName = "";
            ui->checkBox_saveLog->setChecked(false);
            return;
        }
        log(tr("通讯信息将保存到文件%1当中!").arg(m_strLogFileName),SHOW_NULL);
    }
    else
    {
        m_strLogFileName = "";
    }
}

void MainWindow::on_pushButton_clear_clicked()
{
    ui->textBrowser_show_msg->clear();
}

void MainWindow::on_pushButton_loadFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("load"),
                                                     "..\\Message_list.txt",
                                                     tr("Text files (*.txt)"));
    if(!fileName.isEmpty())
    {
        log(tr("加载通讯文件 %1成功!").arg(fileName),SHOW_NULL);
    }
    else
    {
        log(tr("已取消加载通讯文件!"),SHOW_NULL);
    }
    emit setCommFileName(fileName);
}

void MainWindow::on_pushButton_timer_clicked()
{
    QString timerFileName = QFileDialog::getOpenFileName(this,tr("load"),
                                                         "..\\timer_list.txt",
                                                         tr("Text Files (*.txt)"));
    if(!timerFileName.isEmpty())
    {
        QString iniFileName = QFileDialog::getSaveFileName(this,NULL,"ForTiming.ini",tr("ini Files (*.ini)"));
        if(!iniFileName.isEmpty())
        {
            log(tr("加载定时文件 %1成功!").arg(timerFileName),SHOW_NULL);
            QSettings *configWrite = new QSettings(iniFileName, QSettings::IniFormat);
            configWrite->clear();
            delete configWrite;
            emit setTimingFileName(timerFileName);
            emit setIniFileName(iniFileName);
            return;
        }
    }
    emit setTimingFileName("");
    emit setIniFileName("");
    log(tr("已取消加载定时文件!"),SHOW_NULL);
}

void MainWindow::on_pushButton_creat_clicked()
{
    QString server_IP = ui->lineEdit_serverIP->text();
    QString port = ui->lineEdit_serverPort->text();
    int server_Port;
    bool ok;
    server_Port = port.toInt(&ok,10);
    QString prefix = ui->comboBox_prefix->currentText();
    QString suffix = ui->comboBox_suffix->currentText();
    suffix.replace("\\r","\r");
    suffix.replace("\\n","\n");
    if(server_IP.isEmpty() || port.isEmpty() || !ok)
    {
        log(tr("请正确设置IP地址和端口号!"),SHOW_NULL);
        return;
    }
    if(0==ui->comboBox_server_client->currentIndex())//服务器
    {
        emit createObj(server_IP,server_Port,prefix,suffix,TCPIP_SERVER);
    }
    else
    {
        if(1==ui->comboBox_server_client->currentIndex())//客户端
        {
            int clientPort = ui->lineEdit_clientPort->text().toInt();
            if(ui->checkBox_clientPort->isChecked()
                    && 0 < clientPort
                    && "" != ui->lineEdit_clientPort->text())
            {
                emit createObj(server_IP,server_Port,prefix,suffix,TCPIP_CLIENT,clientPort);
            }
            else
            {
                emit createObj(server_IP,server_Port,prefix,suffix,TCPIP_CLIENT);
            }
        }
    }
}

void MainWindow::on_pushButton_delete_clicked()
{
    emit deleteObj((SocketObj) ui->comboBox_server_client->currentIndex());
    ui->lineEdit_serverIP_send->clear();
    ui->lineEdit_serverPort_send->clear();
}

void MainWindow::on_pushButton_send_clicked()
{
    QString strMsg = ui->lineEdit_input->text();
    if(!strMsg.isEmpty())
    {
        emit manualSendMsg((SocketObj)ui->comboBox_server_client->currentIndex(),strMsg);
    }
}

void MainWindow::createSuccess(const SocketObj &index, const bool &success)
{
    ui->pushButton_creat->setDisabled(success);
    ui->comboBox_server_client->setDisabled(success);
    ui->pushButton_delete->setDisabled(!success);
    ui->lineEdit_serverIP->setDisabled(success);
    ui->lineEdit_serverPort->setDisabled(success);
    ui->comboBox_prefix->setDisabled(success);
    ui->comboBox_suffix->setDisabled(success);
    ui->checkBox_clientPort->setDisabled(success);
    ui->lineEdit_clientPort->setDisabled(success);
    if(!ui->checkBox_clientPort->isChecked())
    {
        ui->lineEdit_clientPort->setDisabled(true);
    }

    ui->pushButton_send->setEnabled(success);
    ui->pushButton_delete->setEnabled(success);
}

void MainWindow::on_lineEdit_serverIP_send_returnPressed()
{
    QString strIp = ui->lineEdit_serverIP_send->text();
    QString strPort = ui->lineEdit_serverPort_send->text();
    if(!strIp.isEmpty() && !strPort.isEmpty())
    {
        emit setServerSendIpPort(strIp,strPort);
    }
}

void MainWindow::on_lineEdit_serverPort_send_returnPressed()
{
    QString strIp = ui->lineEdit_serverIP_send->text();
    QString strPort = ui->lineEdit_serverPort_send->text();
    if(!strIp.isEmpty() && !strPort.isEmpty())
    {
        emit setServerSendIpPort(strIp,strPort);
    }
}

void MainWindow::on_comboBox_server_client_currentIndexChanged(int index)
{
    switch (index) {
    case TCPIP_SERVER:
        ui->label_serverIP_send->show();
        ui->label_serverPort_send->show();
        ui->lineEdit_serverIP_send->show();
        ui->lineEdit_serverPort_send->show();
        ui->label_clientPort->hide();
        ui->checkBox_clientPort->hide();
        ui->lineEdit_clientPort->hide();
        break;
    case TCPIP_CLIENT:
        ui->label_serverIP_send->hide();
        ui->label_serverPort_send->hide();
        ui->lineEdit_serverIP_send->hide();
        ui->lineEdit_serverPort_send->hide();
        ui->label_clientPort->show();
        ui->checkBox_clientPort->show();
        ui->lineEdit_clientPort->show();
        break;
    default:
        break;
    }
}

void MainWindow::on_checkBox_clientPort_clicked()
{
    ui->lineEdit_clientPort->setEnabled(ui->checkBox_clientPort->isChecked());
}
