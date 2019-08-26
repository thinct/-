#ifndef GLOBAL_MACRODEFINITION_H
#define GLOBAL_MACRODEFINITION_H


#include <QDate>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QDebug>

#define FLOAT_ZERO 1e-6

// 调试日志信息
#define LOG_INFO(format, ...)                                                                     \
    do {                                                                                          \
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd"); \
    QString current_date = QString("(%1)").arg(current_date_time);                                \
    char buf[1024]{0};                                                                            \
    sprintf(buf, "[INFO] [%s %s:%s:%d]\n\t" format "",                                            \
    current_date.toStdString().data(), __FILE__, __FUNCTION__ , __LINE__, ##__VA_ARGS__);         \
    qDebug()<<buf;                                                                                \
    } while (0)

/*****************************************************

*****************************************************/

// 错误日志信息
#define LOG_ERROR(format, ...)                                                                    \
    do {                                                                                          \
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd"); \
    QString current_date = QString("(%1)").arg(current_date_time);                                \
    char buf[1024]{0};                                                                            \
    sprintf(buf, "[ERROR] [%s %s:%s:%d]\n\t" format "",                                           \
    current_date.toStdString().data(), __FILE__, __FUNCTION__ , __LINE__, ##__VA_ARGS__);         \
    qDebug()<<buf;                                                                                \
    } while (0)


/*****************************************************

*****************************************************/

// 异常日志信息
#define LOG_EXCEPTION                                                                             \
    do {                                                                                          \
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd"); \
    QString current_date = QString("(%1)").arg(current_date_time);                                \
    char buf[1024]{0};                                                                            \
    sprintf(buf, "[EXCEPTION] [%s %s:%s:%d]\n\t",                                                 \
    current_date.toStdString().data(), __FILE__, __FUNCTION__ , __LINE__);                        \
    qDebug()<<buf;                                                                                \
    } while (0)


/*****************************************************

*****************************************************/

// 异常处理
#define EXCEPTION_BEGIN try {
#define EXCEPTION_END } catch (...) { LOG_EXCEPTION; }



/*****************************************************

*****************************************************/

// 读取文件
#define FILE_READ_BEGIN(filePath)                                 \
QFile file(filePath);                                             \
if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {           \
    QTextStream in(&file);

#define FILE_READ_END file.close(); }  else { LOG_ERROR("file path not found"); };



/*****************************************************

*****************************************************/

// 写入文件
#define FILE_WRITE_BEGIN(filePath)                                    \
QFile file(filePath);                                                 \
if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {              \
    QTextStream out(&file);

#define FILE_WRITE_END file.close(); }  else { LOG_ERROR("file path not found"); };

/*****************************************************

*****************************************************/

// 删除制定文件夹下所有文件

#define REMOVEFILES(folderDir)                                                      \
    do {                                                                            \
    QDir dir(folderDir);                                                            \
    QFileInfoList fileList=dir.entryInfoList(QDir::Dirs|QDir::Files                 \
                                             |QDir::Readable|QDir::Writable         \
                                             |QDir::Hidden|QDir::NoDotAndDotDot     \
                                             ,QDir::Name);                          \
    while(fileList.size()>0) {                                                      \
        int infoNum=fileList.size();                                                \
        for(int i=infoNum-1;i>=0;i--) {                                             \
            QFileInfo curFile=fileList[i];                                          \
            if(curFile.isFile()) {                                                  \
                QFile fileTemp(curFile.filePath());                                 \
                fileTemp.remove();                                                  \
                fileList.removeAt(i);                                               \
            }                                                                       \
        }                                                                           \
    }                                                                               \
    } while (0);


/*****************************************************

*****************************************************/

// QString转char*
#define QString2CharPtr(qstr) (QString(qstr)).toStdString().data()
// QString转wchar*
#define QString2WCharPtr(s) (W2A(QString(s).toStdWString().data()))


static QMap<QString,QString> PHDI_DB_map_vars;
#define PHDI_DB_SET_VALUE(var, info) \
    if (QString(info).isEmpty()) { LOG_ERROR("第二个参数不能为空!"); } \
    LOG_INFO("%s", QString2CharPtr(QString(""#var" = ")+ \
    QVariant(var).toString()+QString("\t\\\\")+info)); \
    PHDI_DB_map_vars[info] = QVariant(var).toString();

#define PHDI_DB_VAR(info) PHDI_DB_map_vars[info]
#define PHDI_DB_VAR_PRINT(info) LOG_INFO("%s:%s", QString2CharPtr(info), QString2CharPtr(PHDI_DB_map_vars[info]));


static QMap<QString, QMap<QString,QString>> PHDI_DB_groupVars;
#define PHDI_DB_GROUP_LOAD(group) PHDI_DB_CLEAR;PHDI_DB_LOAD;PHDI_DB_map_vars = PHDI_DB_groupVars[group];
#define PHDI_DB_GROUP_SAVE(group) PHDI_DB_groupVars[group] = PHDI_DB_map_vars; PHDI_DB_SAVE;

#define PHDI_DB_CLEAR PHDI_DB_groupVars.clear();

#define PHDI_DB_SAVE \
    do {                                                                            \
        QStringList groupFiles;                                                     \
        foreach (QString group, PHDI_DB_groupVars.keys()) {                         \
            FILE_WRITE_BEGIN(QApplication::applicationDirPath()+"/"+group)          \
            {                                                                       \
                groupFiles<<group;                                                  \
                foreach (QString info, PHDI_DB_groupVars[group].keys()) {           \
                    out<<info<<":"<<PHDI_DB_groupVars[group].value(info)<<endl;     \
                }                                                                   \
            }                                                                       \
            FILE_WRITE_END;                                                         \
        }                                                                           \
        FILE_WRITE_BEGIN(QApplication::applicationDirPath()+"/PHDI_DB_SYSTEM")      \
        {                                                                           \
            foreach (QString groupName, groupFiles) {                               \
                out<<groupName<<endl;                                               \
            }                                                                       \
        }                                                                           \
        FILE_WRITE_END;                                                             \
    } while (0);


#define PHDI_DB_LOAD \
    do {                                                                              \
        FILE_READ_BEGIN(QApplication::applicationDirPath()+"/PHDI_DB_SYSTEM")         \
        {                                                                             \
            QStringList groupFiles = in.readAll().split("\n");                        \
            foreach (QString group, groupFiles) {                                     \
                QString groupFilePath = QApplication::applicationDirPath()+"/"+group; \
                if (!group.isEmpty() && QFileInfo(groupFilePath).exists()) {          \
                    FILE_READ_BEGIN(groupFilePath)                                    \
                    {                                                                 \
                        QMap<QString,QString> mapVars;                                \
                        QStringList vars = in.readAll().split("\n");                  \
                        foreach (QString var, vars) {                                 \
                            if (!var.isEmpty()) {                                     \
                                QString varName = var.section(':',0,0);               \
                                QString varValue = var.section(':',1);                \
                                mapVars[varName] = varValue;                          \
                            }                                                         \
                        }                                                             \
                        PHDI_DB_groupVars[group] = mapVars;                           \
                    }                                                                 \
                    FILE_READ_END;                                                    \
                }                                                                     \
            }                                                                         \
        }                                                                             \
        FILE_READ_END;                                                                \
    } while (0);



#endif // GLOBAL_MACRODEFINITION_H
