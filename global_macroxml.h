#ifndef GLOBAL_MACROXML_H
#define GLOBAL_MACROXML_H
#include <QtXml>

#pragma execution_character_set("utf-8")

#define WRITE_XML_BEGIN(fileName)              do{                                                                \
    const QString filePath = QCoreApplication::applicationDirPath() + "/" + fileName+".xml";        \
    QFile file(filePath);                                                                                                       \
    if(!file.open(QFile::WriteOnly|QFile::Truncate)) {                                                            \
        LOG_ERROR("failed to write xml file: %s", QString2CharPtr(filePath));            \
        break;                                                                                                                                       \
    }                                                                                                                                                 \
    QDomDocument doc;                                                                                                                                 \
    QDomProcessingInstruction instruction;                                                                                              \
    instruction=doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");           \
    doc.appendChild(instruction);                                                                                                                     \
                                                                                                                                                      \
    QDomElement rootDomElem = doc.createElement("Application");                                     \
    doc.appendChild(rootDomElem);                                                                                          \


    #define WRITE_XML_END                \
    QTextStream out_stream(&file);       \
    doc.save(out_stream,4);              \
    file.close(); }while(0)


#define READ_XML_BEGIN(fileName)              do{        \
        const QString filePath = QCoreApplication::applicationDirPath() + "/" + fileName+".xml";      \
        QFile file(filePath);                                                                         \
        if (!file.open(QIODevice::ReadOnly)){                                                         \
            LOG_ERROR("failed to write xml file: %s", QString2CharPtr(filePath));                     \
            break;                                                                                    \
        }                                                                                             \
                                                                                                      \
        QDomDocument doc;                                                                             \
        if (!doc.setContent(&file)) {                                                                 \
            LOG_ERROR("failed set doc content");                                                      \
            break;                                                                                    \
        }                                                                                             \
        file.close();                                                                                 \
                                                                                                      \
        QDomElement root = doc.documentElement();


#define READ_XML_END     } while(0);



#define MODIFY_XML_BEGIN(fileName, tagName)              do{        \
        const QString filePath = QCoreApplication::applicationDirPath() + "/" + fileName+".xml";      \
        QFile file(filePath);                                                                         \
        if (!file.open(QIODevice::ReadOnly)){                                                         \
            LOG_ERROR("failed to modify xml file: %s", QString2CharPtr(filePath));                     \
            break;                                                                                    \
        }                                                                                             \
        file.close();                                                                                 \
                                                                                                     \
        QDomDocument doc;                                                                             \
        if (!doc.setContent(&file)) {                                                                 \
            LOG_ERROR("failed set doc content");                                                      \
            break;                                                                                    \
        }                                                                                             \
                                                                                                      \
        QDomElement root = doc.documentElement();                                      \
        QDomNodeList domNodes = root.elementsByTagName(tagName);                       \
        if (1 == domNodes.count()) {                                                   \
            QDomNode _domNodeMatch_ =  domNodes.item(0);



#define MODIFY_XML_END                                                                                            \
            QFile file(filePath);                                                                                                  \
            if(!file.open(QFile::WriteOnly|QFile::Truncate)) {                                               \
                LOG_ERROR("failed to write xml file: %s", QString2CharPtr(filePath));    \
                break;                                                                                                             \
            }                                                                                                                      \
            QTextStream out_stream(&file);                                                                      \
            doc.save(out_stream,4);                                                                                \
            file.close();                                                                            \
        } else {                                                                                     \
            LOG_ERROR("XML数据结构出错");                                                                  \
        }                                                                                                                          \
    } while(0);




#define MODIFY_XML_VALUE(var, value)                                       \
    do {                                                                      \
        QDomNodeList domNodes2 = _domNodeMatch_.toElement().elementsByTagName(var);    \
        if (1 == domNodes2.count()) {                                                   \
            if (domNodes2.item(0).hasChildNodes()) {                                    \
                QDomNode oldnode =  domNodes2.item(0).firstChild();                     \
                domNodes2.item(0).firstChild().setNodeValue(value);                     \
                QDomNode newnode =  domNodes2.item(0).firstChild();                     \
                domNodes2.item(0).replaceChild(newnode,oldnode);                        \
            } else {                                                                    \
                QDomText text = doc.createTextNode(value);                              \
                domNodes2.item(0).appendChild(text);                                    \
                text.clear();                                                           \
            }                                                                           \
        } else {                                                                        \
            LOG_ERROR("XML数据结构出错");                                                     \
        }                                                                               \
    } while (0)



#define ADD_XML_NEW(var, value)                      \
    do {                                                        \
        QDomElement elemText = doc.createElement(var);          \
        QDomText nodeText = doc.createTextNode(value);          \
        elemText.appendChild(nodeText);                         \
        _domNodeMatch_.appendChild(elemText);                   \
    } while (0)



#define DELETE_XML_OLD(var)                      \
    do {                                           \
        QDomNodeList domNodes2 = _domNodeMatch_.toElement().elementsByTagName(var); \
        if (1 == domNodes2.count()) {                                                \
            _domNodeMatch_.removeChild(domNodes2.item(0));                           \
        } else {                                                                     \
            LOG_ERROR("XML数据结构出错");                                                  \
        }                                                          \
    } while (0)


#define DELETE_SELF_XML_OLD do { _domNodeMatch_.parentNode().removeChild(_domNodeMatch_); } while(0)


#define MATCH_XML_ELEM_BEGIN(var)                                       \
do {                                                                \
    QDomNodeList domNodes2 = _domNodeMatch_.toElement().elementsByTagName(var); \
    if (1 == domNodes2.count()) {                  \
        QString _value_ = domNodes2.item(0).toElement().text();


#define MATCH_XML_ELEM_END          \
    }  else {                          \
        LOG_ERROR("XML数据结构出错");      \
    } }while(0)



#define SAVE_XML(fileName)                                                                                                     \
    do {                                                                                                                                           \
        const QString filePath = QCoreApplication::applicationDirPath() + "/" + fileName+".xml";   \
        QFile file(filePath);                                                                                                                      \
        if(!file.open(QFile::WriteOnly|QFile::Truncate)) {                                                                       \
            LOG_ERROR("failed to write xml file: %s", QString2CharPtr(filePath));                           \
            break;                                                                                                                                 \
        }                                                                                                                                          \
        QTextStream out_stream(&file);                                                                                              \
        doc.save(out_stream,4);                                                                                                        \
        file.close();                                                                                                                              \
    } while (0)


#endif // GLOBAL_MACROXML_H
