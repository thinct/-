#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QCloseEvent>
#include <QDir>
#include <QScreen>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QFileDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include "global_macrodefinition.h"
#include "enginethread.h"
#include "logindialog.h"
#include "gpio_thread.h"
#include "Include/Dask64.h"


#pragma execution_character_set("utf-8")


#define HCPP_LEGACY_API
#define HCPP_LEGACY_NAMESPACE
#include "HalconCpp.h"
using namespace HalconCpp;


enum class WindowSelect {
    Window1,
    Window2,
    WindowAll
};
WindowSelect windowSelect = WindowSelect::WindowAll;


#define Reset_SwitchWindowSelect do {           \
    ui->actionWindow1->setChecked(false);       \
    ui->actionWindow2->setChecked(false);       \
    ui->actionAllWindows->setChecked(false);    \
    } while(0)



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loadLayout();



    { // 机密狗模块，定时检测加密狗是否存在

#ifdef USER_LOCK
        QTcpSocket* client = new QTcpSocket(this);
        client->connectToHost(QHostAddress("127.0.0.1"), 27015);
        client->waitForConnected();

        auto lisenceCheck = [=](){
            static int count = 0;
            if (QTcpSocket::SocketState::ConnectedState == client->state()) {
                char data[16]{0};
                client->write("askID", 5);
                client->read(data, 16);

                QDateTime dateTime = QDateTime::currentDateTime();
                int year, month, day;
                dateTime.date().getDate(&year, &month, &day);
                int check = (year^month^day);
                LOG_INFO("Socket %s, Local %d", data, check);

                if (data == QString::number(check)) {
                    count = 0;
                    LOG_INFO("机密狗正确应答...");
                }
            } else {
                count++;
                qDebug()<<"No askID...";
                LOG_INFO("机密狗未应答...");
            }
            if (count>3) { // 连续三次都没有检测到机密狗，则视为没有出入加密狗，直接退出
                exit(0);
            }
        };
        QTimer* timerCheckSoftValid = new QTimer();
        QObject::connect(timerCheckSoftValid, &QTimer::timeout, [=](){
            lisenceCheck();
        });
        lisenceCheck();
        timerCheckSoftValid->start(5 * 1000);
#endif // USER_LOCK
    } // 机密狗模块，定时检测加密狗是否存在


    { // 工具栏按钮初始化
        ui->mainToolBar->setWindowTitle("工具按钮");
        ui->mainToolBar->layout()->setSpacing(3);


        QPushButton* pushButton_singleImageAcq = new QPushButton("单张采集");
        connect(pushButton_singleImageAcq, &QPushButton::clicked, ui->actionSingleAcq, &QAction::triggered);
        ui->mainToolBar->addWidget(pushButton_singleImageAcq);


        QPushButton* pushButton_createMark = new QPushButton("创建Mark点");
        connect(pushButton_createMark, &QPushButton::clicked, ui->actionCreateMark, &QAction::triggered);
        ui->mainToolBar->addWidget(pushButton_createMark);


        QPushButton* pushButton_drawROI1 = new QPushButton("绘制测量区域");
        connect(pushButton_drawROI1, &QPushButton::clicked, ui->actionDrawROI1, &QAction::triggered);
        ui->mainToolBar->addWidget(pushButton_drawROI1);


        QPushButton* pushButton_drawROI_SN = new QPushButton("绘制字符区域");
        connect(pushButton_drawROI_SN, &QPushButton::clicked, ui->actionDrawROI_SN, &QAction::triggered);
        ui->mainToolBar->addWidget(pushButton_drawROI_SN);


        QPushButton* pushButton_startDetect = new QPushButton("开始检测");
        connect(pushButton_startDetect, &QPushButton::clicked, ui->actionDetectTestWitchCamera, &QAction::triggered);
        ui->mainToolBar->addWidget(pushButton_startDetect);
    } // 工具栏按钮初始化


    {// 窗口切换模式
        Reset_SwitchWindowSelect;
        ui->actionAllWindows->setChecked(true);

        connect(ui->actionWindow1, &QAction::triggered, [this](){
            Reset_SwitchWindowSelect;
            ui->actionWindow1->setChecked(true);
            windowSelect = WindowSelect::Window1;
        });
        connect(ui->actionWindow2, &QAction::triggered, [this](){
            Reset_SwitchWindowSelect;
            ui->actionWindow2->setChecked(true);
            windowSelect = WindowSelect::Window2;
        });
        connect(ui->actionAllWindows, &QAction::triggered, [this](){
            Reset_SwitchWindowSelect;
            ui->actionAllWindows->setChecked(true);
            windowSelect = WindowSelect::WindowAll;
        });
    }// 窗口切换模式

    {// 读取图片
        connect(ui->actionReadImage, &QAction::triggered, [=](){

            QString preFileDir = "";
            {// 最近有用的访问路径，方便打开最近的文件

                PHDI_DB_GROUP_LOAD("工位1配置参数");
                preFileDir = PHDI_DB_VAR("最近打开的图片路径");

                if (preFileDir.isEmpty()
                        || !QDir(preFileDir).exists()) {
                    preFileDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                }
            }// 最近有用的访问路径，方便打开最近的文件

            QString  fileName = QFileDialog::getOpenFileName(this, tr("读取图片"), preFileDir
                                                             , tr("tiff Image (*.tiff);BMP Image (*.bmp); All Files (*.*)"));

            if (!fileName.isNull()) {
                //fileName即是选择的文件名
                preFileDir = QFileInfo(fileName).absoluteDir().absolutePath();
                {// 更新最近的访问路径
                    PHDI_DB_GROUP_LOAD("工位1配置参数");
                    PHDI_DB_SET_VALUE(preFileDir, "最近打开的图片路径");
                    PHDI_DB_GROUP_SAVE("工位1配置参数");
                }// 更新最近的访问路径

                LOG_INFO("image file path: %s", QString2CharPtr(fileName));

                engineThread->imageFilePath = fileName;
                engineThread->status_RunTime = EngineThread::Status_RunTime::LoadImageFile;
                if (engineThread->isRunning()) {
                    engineThread->waitCMD->wakeAll();
                } else {
                    engineThread->start();
                }
            } else {
                return;
            }
        });
    }// 读取图片

    {// 保存图片

        connect(ui->actionWriteImage, &QAction::triggered, [=](){
            QString fileName = QFileDialog::getSaveFileName(this, tr("保存图片")
                                                            , QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
                                                            , tr("图片(*.bmp)"));
            if (!fileName.isEmpty()) {
                engineThread->imageFilePath = fileName;
                engineThread->status_RunTime = EngineThread::Status_RunTime::SaveImageFile;
                if (engineThread->isRunning()) {
                    engineThread->waitCMD->wakeAll();
                } else {
                    engineThread->start();
                }
            }
        });

    }// 保存图片


#define WindowSelect_Current(Up, Bottom)                                            \
    EngineThread::Status_RunTime runtime = EngineThread::Status_RunTime::None;      \
    if (WindowSelect::Window1 == windowSelect) {                                    \
        runtime = Up;                                                               \
    } else if (WindowSelect::Window2 == windowSelect) {                             \
        runtime = Bottom;                                                           \
    } else {                                                                        \
        return;                                                                     \
    }



    { // 单张采集

        connect(ui->actionSingleAcq, &QAction::triggered, [=](){

            engineThread->status_RunTime = EngineThread::Status_RunTime::SingleAcq;
            if (engineThread->isRunning()) {
                engineThread->waitCMD->wakeAll();
            } else {
                engineThread->start();
            }
        });

    } // 单张采集

    { // 连续采集

        connect(ui->actionLooperAcq, &QAction::triggered, [=](){

            engineThread->status_RunTime = EngineThread::Status_RunTime::LooperAcq;
            if (engineThread->isRunning()) {
                engineThread->waitCMD->wakeAll();
            } else {
                engineThread->start();
            }
        });

    } // 连续采集

    { // 创建Mark点

        connect(ui->actionCreateMark, &QAction::triggered, [=](){

            WindowSelect_Current(EngineThread::Status_RunTime::CreateMarkModel_1
                                 , EngineThread::Status_RunTime::CreateMarkModel_2);
            engineThread->status_RunTime = runtime;
            if (engineThread->isRunning()) {
                engineThread->waitCMD->wakeAll();
            } else {
                engineThread->start();
            }
        });

    } // 创建Mark点

    { // 绘制测量区域

        connect(ui->actionDrawROI1, &QAction::triggered, [=](){

            WindowSelect_Current(EngineThread::Status_RunTime::DrawROI1_Up,
                                 EngineThread::Status_RunTime::DrawROI1_Bottom);
            engineThread->status_RunTime = runtime;
            if (engineThread->isRunning()) {
                engineThread->waitCMD->wakeAll();
            } else {
                engineThread->start();
            }
        });

    } // 绘制测量区域

    { // 绘制字符区域

        connect(ui->actionDrawROI_SN, &QAction::triggered, [=](){

            WindowSelect_Current(EngineThread::Status_RunTime::DrawROI_SN_Up,
                                 EngineThread::Status_RunTime::DrawROI_SN_Bottom);
            engineThread->status_RunTime = runtime;
            if (engineThread->isRunning()) {
                engineThread->waitCMD->wakeAll();
            } else {
                engineThread->start();
            }
        });

    } // 绘制字符区域

    { // 检测测试

        connect(ui->actionDetectTest, &QAction::triggered, [=](){

            engineThread->status_RunTime = EngineThread::Status_RunTime::Detection_Manual;
            if (engineThread->isRunning()) {
                engineThread->waitCMD->wakeAll();
            } else {
                engineThread->start();
            }
        });
    } // 检测测试

    { // 检测测试(新图片)

        connect(ui->actionDetectTestWitchCamera, &QAction::triggered, [=](){

            engineThread->status_RunTime = EngineThread::Status_RunTime::Detection_Auto;
            if (engineThread->isRunning()) {
                engineThread->waitCMD->wakeAll();
            } else {
                engineThread->start();
            }
        });
    } // 检测测试(新图片)

    { // 参数保存

        { // 是否允许修改参数
            const bool isCheckedFlag = ui->checkBox_ModifyParams->isChecked();
            ui->lineEdit_Cam1_GapThreshold->setEnabled(isCheckedFlag);
            ui->lineEdit_Cam2_GapThreshold->setEnabled(isCheckedFlag);
            ui->lineEdit_Cam1_Ratio->setEnabled(isCheckedFlag);
            ui->lineEdit_Cam2_Ratio->setEnabled(isCheckedFlag);
            ui->tableWidget->setEnabled(isCheckedFlag);
            ui->lineEdit_Count_OK->setEnabled(isCheckedFlag);
            ui->lineEdit_Count_NG->setEnabled(isCheckedFlag);
            ui->lineEdit_Count_Total->setEnabled(isCheckedFlag);
            ui->lineEdit_Count_Ratio->setEnabled(isCheckedFlag);
            connect(ui->checkBox_ModifyParams, &QCheckBox::clicked, [this](){


                //************************登录窗口******************************
#ifdef LOGIN_DIALOG
                static bool loginFlag = false;
                if (!loginFlag) {
                    LogInDialog loginDlg;
                    loginDlg.exec();
                    if (!loginDlg.validFlag) {
                        ui->checkBox_ModifyParams->setChecked(false);
                        return;
                    }
                    loginFlag = true;
                }
#endif
                //************************登录窗口******************************

                const bool isCheckedFlag = ui->checkBox_ModifyParams->isChecked();
                ui->lineEdit_Cam1_GapThreshold->setEnabled(isCheckedFlag);
                ui->lineEdit_Cam2_GapThreshold->setEnabled(isCheckedFlag);
                ui->lineEdit_Cam1_Ratio->setEnabled(isCheckedFlag);
                ui->lineEdit_Cam2_Ratio->setEnabled(isCheckedFlag);
                ui->tableWidget->setEnabled(isCheckedFlag);
                ui->lineEdit_Count_OK->setEnabled(isCheckedFlag);
                ui->lineEdit_Count_NG->setEnabled(isCheckedFlag);
                ui->lineEdit_Count_Total->setEnabled(isCheckedFlag);
                ui->lineEdit_Count_Ratio->setEnabled(isCheckedFlag);
            });
        } // 是否允许修改参数

        PHDI_DB_GROUP_LOAD("用户参数");
        QString cam1_Gap_ThresholdValue = PHDI_DB_VAR("相机1海绵缝隙阈值");
        ui->lineEdit_Cam1_GapThreshold->setText(cam1_Gap_ThresholdValue);
        connect(ui->lineEdit_Cam1_GapThreshold, &QLineEdit::textChanged, [this](){
            QString gap_ThresholdValue = ui->lineEdit_Cam1_GapThreshold->text();
            PHDI_DB_GROUP_LOAD("用户参数");
            PHDI_DB_SET_VALUE(gap_ThresholdValue, "相机1海绵缝隙阈值");
            PHDI_DB_GROUP_SAVE("用户参数")
        });


        QString cam2_Gap_ThresholdValue = PHDI_DB_VAR("相机2海绵缝隙阈值");
        ui->lineEdit_Cam2_GapThreshold->setText(cam2_Gap_ThresholdValue);
        connect(ui->lineEdit_Cam2_GapThreshold, &QLineEdit::textChanged, [this](){
            QString gap_ThresholdValue = ui->lineEdit_Cam2_GapThreshold->text();
            PHDI_DB_GROUP_LOAD("用户参数");
            PHDI_DB_SET_VALUE(gap_ThresholdValue, "相机2海绵缝隙阈值");
            PHDI_DB_GROUP_SAVE("用户参数")
        });


        QString cam1_Ratio = PHDI_DB_VAR("相机1系数");
        ui->lineEdit_Cam1_Ratio->setText(cam1_Ratio);
        connect(ui->lineEdit_Cam1_Ratio, &QLineEdit::textChanged, [this](){
            QString ratio = ui->lineEdit_Cam1_Ratio->text();
            PHDI_DB_GROUP_LOAD("用户参数");
            PHDI_DB_SET_VALUE(ratio, "相机1系数");
            PHDI_DB_GROUP_SAVE("用户参数")
        });


        QString cam2_Ratio = PHDI_DB_VAR("相机2系数");
        ui->lineEdit_Cam2_Ratio->setText(cam2_Ratio);
        connect(ui->lineEdit_Cam2_Ratio, &QLineEdit::textChanged, [this](){
            QString ratio = ui->lineEdit_Cam2_Ratio->text();
            PHDI_DB_GROUP_LOAD("用户参数");
            PHDI_DB_SET_VALUE(ratio, "相机2系数");
            PHDI_DB_GROUP_SAVE("用户参数")
        });


        QString CountOK = PHDI_DB_VAR("计数-良品");
        ui->lineEdit_Count_OK->setText(CountOK);
        connect(ui->lineEdit_Count_OK, &QLineEdit::textChanged, [this](){
            PHDI_DB_GROUP_LOAD("用户参数");
            PHDI_DB_SET_VALUE(ui->lineEdit_Count_OK->text(), "计数-良品");
            PHDI_DB_GROUP_SAVE("用户参数");

            int cnt_ok = ui->lineEdit_Count_OK->text().toInt();
            int cnt_ng = ui->lineEdit_Count_NG->text().toInt();
            int total = cnt_ok + cnt_ng;
            ui->lineEdit_Count_Total->setText(QString::number(total));
            if (total>0) {
                ui->lineEdit_Count_Ratio->setText(QString::asprintf("%.2f%%", (float)((float)cnt_ok / total)*100));
            }
        });


        QString CountNG = PHDI_DB_VAR("计数-不良品");
        ui->lineEdit_Count_NG->setText(CountNG);
        connect(ui->lineEdit_Count_NG, &QLineEdit::textChanged, [this](){
            PHDI_DB_GROUP_LOAD("用户参数");
            PHDI_DB_SET_VALUE(ui->lineEdit_Count_NG->text(), "计数-不良品");
            PHDI_DB_GROUP_SAVE("用户参数");

            int cnt_ok = ui->lineEdit_Count_OK->text().toInt();
            int cnt_ng = ui->lineEdit_Count_NG->text().toInt();
            int total = cnt_ok + cnt_ng;
            ui->lineEdit_Count_Total->setText(QString::number(total));
            if (total>0) {
                ui->lineEdit_Count_Ratio->setText(QString::asprintf("%.2f%%", (float)((float)cnt_ok / total)*100));
            }
        });


        int total = CountNG.toInt() + CountOK.toInt();
        ui->lineEdit_Count_Total->setText(QString::number(total));
        if (total>0) {
            int cnt_ok = CountOK.toInt();
            ui->lineEdit_Count_Ratio->setText(QString::asprintf("%.2f%%", (float)((float)cnt_ok / total)*100));
        }


        PHDI_DB_GROUP_LOAD("用户参数");
        QTableWidget* tableWidget = ui->tableWidget;
        tableWidget->verticalHeader()->hide();
        tableWidget->setRowCount(8);
        tableWidget->setColumnCount(3);
        tableWidget->setHorizontalHeaderLabels(QStringList()<<"位置"<<"标准值"<<"允许偏差");

        QString value;
        int row = 0;
        {
            QString tag = "A>上";
            tableWidget->setItem(row, 0, new QTableWidgetItem(tag));
            value = PHDI_DB_VAR(tag + " 标准值");
            tableWidget->setItem(row, 1, new QTableWidgetItem(value));
            value = PHDI_DB_VAR(tag + " 允许偏差");
            tableWidget->setItem(row, 2, new QTableWidgetItem(value));
        }

        row++;
        {
            QString tag = "A>下";
            tableWidget->setItem(row, 0, new QTableWidgetItem(tag));
            value = PHDI_DB_VAR(tag + " 标准值");
            tableWidget->setItem(row, 1, new QTableWidgetItem(value));
            value = PHDI_DB_VAR(tag + " 允许偏差");
            tableWidget->setItem(row, 2, new QTableWidgetItem(value));
        }

        row++;
        {
            QString tag = "A>左";
            tableWidget->setItem(row, 0, new QTableWidgetItem(tag));
            value = PHDI_DB_VAR(tag + " 标准值");
            tableWidget->setItem(row, 1, new QTableWidgetItem(value));
            value = PHDI_DB_VAR(tag + " 允许偏差");
            tableWidget->setItem(row, 2, new QTableWidgetItem(value));
        }

        row++;
        {
            QString tag = "A>右";
            tableWidget->setItem(row, 0, new QTableWidgetItem(tag));
            value = PHDI_DB_VAR(tag + " 标准值");
            tableWidget->setItem(row, 1, new QTableWidgetItem(value));
            value = PHDI_DB_VAR(tag + " 允许偏差");
            tableWidget->setItem(row, 2, new QTableWidgetItem(value));
        }


        row++;
        {
            QString tag = "B>上";
            tableWidget->setItem(row, 0, new QTableWidgetItem(tag));
            value = PHDI_DB_VAR(tag + " 标准值");
            tableWidget->setItem(row, 1, new QTableWidgetItem(value));
            value = PHDI_DB_VAR(tag + " 允许偏差");
            tableWidget->setItem(row, 2, new QTableWidgetItem(value));
        }

        row++;
        {
            QString tag = "B>下";
            tableWidget->setItem(row, 0, new QTableWidgetItem(tag));
            value = PHDI_DB_VAR(tag + " 标准值");
            tableWidget->setItem(row, 1, new QTableWidgetItem(value));
            value = PHDI_DB_VAR(tag + " 允许偏差");
            tableWidget->setItem(row, 2, new QTableWidgetItem(value));
        }

        row++;
        {
            QString tag = "B>左";
            tableWidget->setItem(row, 0, new QTableWidgetItem(tag));
            value = PHDI_DB_VAR(tag + " 标准值");
            tableWidget->setItem(row, 1, new QTableWidgetItem(value));
            value = PHDI_DB_VAR(tag + " 允许偏差");
            tableWidget->setItem(row, 2, new QTableWidgetItem(value));
        }

        row++;
        {
            QString tag = "B>右";
            tableWidget->setItem(row, 0, new QTableWidgetItem(tag));
            value = PHDI_DB_VAR(tag + " 标准值");
            tableWidget->setItem(row, 1, new QTableWidgetItem(value));
            value = PHDI_DB_VAR(tag + " 允许偏差");
            tableWidget->setItem(row, 2, new QTableWidgetItem(value));
        }


        connect(tableWidget, &QTableWidget::itemChanged, [this](QTableWidgetItem *item) {
            LOG_INFO("%s", QString2CharPtr(item->text()));

            {// 刷新数据

                PHDI_DB_GROUP_LOAD("用户参数");
                QTableWidget* tableWidget = ui->tableWidget;

                QString value;
                int row = 0;
                {
                    QString tag = "A>上";

                    value = tableWidget->item(row, 1)->text();
                    PHDI_DB_SET_VALUE(value, (tag + " 标准值"));
                    value = tableWidget->item(row, 2)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 允许偏差");
                }

                row++;
                {
                    QString tag = "A>下";

                    value = tableWidget->item(row, 1)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 标准值");
                    value = tableWidget->item(row, 2)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 允许偏差");
                }

                row++;
                {
                    QString tag = "A>左";

                    value = tableWidget->item(row, 1)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 标准值");
                    value = tableWidget->item(row, 2)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 允许偏差");
                }

                row++;
                {
                    QString tag = "A>右";

                    value = tableWidget->item(row, 1)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 标准值");
                    value = tableWidget->item(row, 2)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 允许偏差");
                }


                row++;
                {
                    QString tag = "B>上";

                    value = tableWidget->item(row, 1)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 标准值");
                    value = tableWidget->item(row, 2)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 允许偏差");
                }

                row++;
                {
                    QString tag = "B>下";

                    value = tableWidget->item(row, 1)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 标准值");
                    value = tableWidget->item(row, 2)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 允许偏差");
                }

                row++;
                {
                    QString tag = "B>左";

                    value = tableWidget->item(row, 1)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 标准值");
                    value = tableWidget->item(row, 2)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 允许偏差");
                }

                row++;
                {
                    QString tag = "B>右";

                    value = tableWidget->item(row, 1)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 标准值");
                    value = tableWidget->item(row, 2)->text();
                    PHDI_DB_SET_VALUE(value, tag + " 允许偏差");
                }

                PHDI_DB_GROUP_SAVE("用户参数");
            }// 刷新数据
        });

    } // 参数保存
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{

    QMessageBox msgbox(QMessageBox::Question, "退出", "强制退出?");
    msgbox.setStandardButtons (QMessageBox::Yes|QMessageBox::No);
    msgbox.setButtonText (QMessageBox::Yes,QString("是"));
    msgbox.setButtonText (QMessageBox::No,QString("否"));
    if (QMessageBox::Yes == msgbox.exec() ) {

        { // 左窗口大小
            QRect rc = ui->widget_Up->geometry();
            PHDI_DB_GROUP_LOAD("工位1配置参数");
            PHDI_DB_SET_VALUE(QString("%1,%2,%3,%4").arg(rc.left()).arg(rc.top())
                              .arg(rc.width()).arg(rc.height()), "视觉窗口尺寸");
            PHDI_DB_GROUP_SAVE("工位1配置参数");
        } // 左窗口大小

        { // 右窗口大小
            QRect rc = ui->widget_Bottom->geometry();
            PHDI_DB_GROUP_LOAD("工位2配置参数");
            PHDI_DB_SET_VALUE(QString("%1,%2,%3,%4").arg(rc.left()).arg(rc.top())
                              .arg(rc.width()).arg(rc.height()), "视觉窗口尺寸");
            PHDI_DB_GROUP_SAVE("工位2配置参数");
        } // 右窗口大小


        PHDI_DB_GROUP_LOAD("用户参数");
        int cardNum1 = PHDI_DB_VAR("第一张信号控制卡").toInt();
        DO_WritePort(cardNum1, 0, 0);
        saveLayout();
        exit(0);
    } else {
        event->ignore();
    }
}

void MainWindow::initUIComponents()
{

    engineThread = new EngineThread();


    { // 上面产品窗口
        QRect recentRectWindow(0, 0, 512, 512);
        {
            PHDI_DB_GROUP_LOAD("工位1配置参数");
            QStringList rcInfo = PHDI_DB_VAR("视觉窗口尺寸").split(",");
            if (4 != rcInfo.size()) {
                showMaximized();
                QMessageBox::warning(this, "警告", "请更新窗口退出尺寸");
                return;
            } else {
                recentRectWindow = QRect(rcInfo[0].toInt(), rcInfo[1].toInt()
                        , rcInfo[2].toInt(), rcInfo[3].toInt());
            }
        }

        LOG_INFO("工位1:rcInfo: (%d, %d, %d, %d)"
                 , recentRectWindow.left(), recentRectWindow.top()
                 , recentRectWindow.width(), recentRectWindow.height());

        SetWindowAttr("background_color", "white");  // 如果打开之后是白色的，说明相机没有打开(因为准备状态，工位光源是关闭的)
        HTuple hv_WindowHandle;
        const int width = recentRectWindow.width();
        const int height = recentRectWindow.height();
        OpenWindow(5,5,width-5*2, height-5*2,(Hlong)ui->widget_Up->winId(),"","",&hv_WindowHandle);

        engineThread->windowHandle_Up = hv_WindowHandle;
    } // 上面产品窗口


    { // 下面产品窗口
        QRect recentRectWindow(0, 0, 512, 512);
        {
            PHDI_DB_GROUP_LOAD("工位1配置参数");
            QStringList rcInfo = PHDI_DB_VAR("视觉窗口尺寸").split(",");
            if (4 != rcInfo.size()) {
                showMaximized();
                QMessageBox::warning(this, "警告", "请更新窗口退出尺寸");
                return;
            } else {
                recentRectWindow = QRect(rcInfo[0].toInt(), rcInfo[1].toInt()
                        , rcInfo[2].toInt(), rcInfo[3].toInt());
            }
        }

        LOG_INFO("工位1:rcInfo: (%d, %d, %d, %d)"
                 , recentRectWindow.left(), recentRectWindow.top()
                 , recentRectWindow.width(), recentRectWindow.height());

        SetWindowAttr("background_color", "white");  // 如果打开之后是白色的，说明相机没有打开(因为准备状态，工位光源是关闭的)
        HTuple hv_WindowHandle;
        const int width = recentRectWindow.width();
        const int height = recentRectWindow.height();
        OpenWindow(5,5,width-5*2, height-5*2,(Hlong)ui->widget_Bottom->winId(),"","",&hv_WindowHandle);

        engineThread->windowHandle_Bottom = hv_WindowHandle;
    } // 下面产品窗口


    {// 状态栏初始化
        statusBar()->addWidget(new QLabel("运行状态: "));
        QLineEdit* lineEdit_CurrentStatus = new QLineEdit("已准备");
        lineEdit_CurrentStatus->setReadOnly(true);
        lineEdit_CurrentStatus->setAlignment(Qt::AlignLeft);
        statusBar()->addWidget(lineEdit_CurrentStatus);
        statusBar()->addWidget(new QLabel("上一次运行状态: "));
        QLineEdit* lineEdit_PreStatus = new QLineEdit();
        lineEdit_PreStatus->setReadOnly(true);
        lineEdit_PreStatus->setAlignment(Qt::AlignLeft);
        statusBar()->addWidget(lineEdit_PreStatus);
        connect(engineThread, &EngineThread::updateStatusbar, this, [=](QString status){
            lineEdit_PreStatus->setText(lineEdit_CurrentStatus->text());
            lineEdit_CurrentStatus->setText(status);
            qDebug()<<status;
        }, Qt::BlockingQueuedConnection);
    }// 状态栏初始化

    { // 测试结果计数

        connect(engineThread, &EngineThread::detectResult, this, [=](bool result){
            if (result) {
                int cnt_ok = ui->lineEdit_Count_OK->text().toInt();
                ui->lineEdit_Count_OK->setText(QString::number(cnt_ok+1));
            } else {
                int cnt_ng = ui->lineEdit_Count_NG->text().toInt();
                ui->lineEdit_Count_NG->setText(QString::number(cnt_ng+1));
            }
        }, Qt::BlockingQueuedConnection);

    } // 测试结果计数


    engineThread->start();


    { // GPIO初始化

        PHDI_DB_GROUP_LOAD("用户参数");

        int cardNum1 = PHDI_DB_VAR("第一张信号控制卡").toInt();
        LOG_INFO("第一张信号控制卡: %d", cardNum1);
        int cardID = Register_Card(PCI_7230, cardNum1);
        if (cardID<0) {
            LOG_INFO("Register_Card Error = %d", cardID);
            return;
        }
        engineThread->cardID = cardID;

        signalIndex = PHDI_DB_VAR("触发信号").toInt();
        LOG_INFO("触发信号: %d", signalIndex);
        GPIO_Thread* thread = new GPIO_Thread(cardID);
        connect(thread, &GPIO_Thread::newSignal, [this](int value) {
            QMap<int,int> index;
            index[1] = 1;
            index[2] = 2;
            index[4] = 3;
            index[8] = 4;
            index[16] = 5;
            index[32] = 6;

#ifdef CAMERA_HALCON
            emit ui->actionDetectTestWitchCamera->triggered();
#else
            emit ui->actionDetectTestWitchCamera->triggered();
#endif
        });

        thread->start();

    } // GPIO初始化
}


void MainWindow::saveLayout()
{
    const QString fileName = QCoreApplication::applicationDirPath() + "/RecentLayout";
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QString msg = tr("Failed to open %1\n%2")
                .arg(QDir::toNativeSeparators(fileName), file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }

    QByteArray geo_data = saveGeometry();
    QByteArray layout_data = saveState();

    bool ok = file.putChar((uchar)geo_data.size());
    if (ok) {
        ok = file.write(geo_data) == geo_data.size();
    }
    if (ok) {
        ok = file.write(layout_data) == layout_data.size();
    }

    if (!ok) {
        QString msg = tr("Error writing to %1\n%2")
                .arg(QDir::toNativeSeparators(fileName), file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }
}

void MainWindow::loadLayout()
{
    const QString fileName = QCoreApplication::applicationDirPath() + "/RecentLayout";
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QString msg = tr("Failed to open %1\n%2")
                .arg(QDir::toNativeSeparators(fileName), file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }

    uchar geo_size;
    QByteArray geo_data;
    QByteArray layout_data;

    bool ok = file.getChar((char*)&geo_size);
    if (ok) {
        geo_data = file.read(geo_size);
        ok = geo_data.size() == geo_size;
    }
    if (ok) {
        layout_data = file.readAll();
        ok = layout_data.size() > 0;
    }

    if (ok) {
        ok = restoreGeometry(geo_data);
    }
    if (ok) {
        ok = restoreState(layout_data);
    }

    if (!ok) {
        QString msg = tr("Error reading %1").arg(QDir::toNativeSeparators(fileName));
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }
}

