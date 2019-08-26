#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>
#include <QTimer>
#include <QFile>
#include <QProcess>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>
#include <QDebug>

#include "global_macrodefinition.h"
#include "logindialog.h"



#pragma execution_character_set("utf-8")

class CommonHelper
{
public:
    static void setStyle(const QString &style) {
        QFile qss(style);
        qss.open(QFile::ReadOnly);
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }


    static void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        Q_UNUSED(context); Q_UNUSED(type);

        static bool first = true;
        if (first) {
            first = false;
            QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
            QString current_date = QString("(%1)").arg(current_date_time);
            qDebug("\r\n\r\n\r\n");
            qDebug("******************************************************");
            qDebug(QString("   日志文件 " + current_date).toStdString().data());
            qDebug("******************************************************");
        }

        static QMutex mutex;
        mutex.lock();

        QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd");
        QFile file(QString(QCoreApplication::applicationDirPath()+"/Log/%1.txt")
                   .arg(current_date_time));
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream text_stream(&file);
        text_stream << msg << "\r\n";
        file.flush();
        file.close();

        mutex.unlock();
    }
};

#include <QFileInfo>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef OUTPUT_LOGFILE
    //注册MessageHandler
    qInstallMessageHandler(CommonHelper::outputMessage);
#endif

    // 加载QSS样式
    CommonHelper::setStyle(":/skin.qss");

    {// 打开第三方进程
        QLocalServer* server = new QLocalServer();
        server->listen("PHDI_Vision");

        QProcess process;
        process.startDetached(QApplication::applicationDirPath() + "/removefiles.exe");
    }// 打开第三方进程


#ifdef USER_LOCK
    QProcess process;
    process.startDetached(QApplication::applicationDirPath() + "/demo.exe");
    QApplication::processEvents();
#endif

    MainWindow w;
    w.show();
    w.initUIComponents();

    return a.exec();
}
