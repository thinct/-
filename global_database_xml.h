#ifndef GLOBAL_DATABASE_XML_H
#define GLOBAL_DATABASE_XML_H

#include <QtXml> //也可以include <QDomDocument>
#include "global_macrodefinition.h"



void WriteXMLRecord(QDomDocument &doc, QDomElement &domElem, const QStringList& elementList, int deep=0);
void WriteXML(QString fileName, const QVector<QStringList>& vElementList, QString rootName="Application");
bool ReadXMLRecord(QDomNode node, QStringList &elementList);
void ReadXML(QString fileName, QVector<QStringList>& vElementList, QString rootName="Application");

//写xml
void WriteXML(QString fileName, const QVector<QStringList>& vElementList, QString rootName/*="Application"*/)
{
    //打开或创建文件
    const QString filePath = QCoreApplication::applicationDirPath() + "/" + fileName+".xml";
    LOG_INFO("write xml file: %s", QString2CharPtr(filePath));
    QFile file(filePath); //相对路径、绝对路径、资源路径都可以
    if(!file.open(QFile::WriteOnly|QFile::Truncate)) { //可以用QIODevice，Truncate表示清空原来的内容
        LOG_ERROR("无法创建XML文件:%s", QString2CharPtr(filePath));
        return;
    }


    QDomDocument doc;
    //写入xml头部
    QDomProcessingInstruction instruction; //添加处理命令
    instruction=doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(instruction);

    QDomElement rootDomElem = doc.createElement(rootName);
    doc.appendChild(rootDomElem);

    foreach (QStringList elementList, vElementList) {
        WriteXMLRecord(doc, rootDomElem, elementList);
    }


    //输出到文件
    QTextStream out_stream(&file);
    doc.save(out_stream,4); //缩进4格
    file.close();
}


//写xml
void WriteXMLRecord(QDomDocument& doc, QDomElement& domElem, const QStringList &elementList, int deep/*=0*/)
{
    // QStringList()<<"library"<<"book(id=\"1\",time=\"2013/6/13\")"<<"title:C++ primer"<<"author:Stanley Lippman"

    //添加根节点
    const int eleCnt = elementList.size();
    if (eleCnt>0 && deep<eleCnt) {

        QString elemName = elementList[deep];
        QDomElement elemDom;

        if (elemName.contains(QRegExp("\(.*\)"))) { // 可有可无的判断，不过暂时先保留起来
            int start = elemName.indexOf("\(");
            int end = elemName.indexOf("\)", start+1);
            QString elemNameNew = elemName.mid(0, elemName.indexOf("\("));
            elemDom=doc.createElement(elemNameNew);
            QString attrs = elemName.mid(start+1, end-start-1);
            QStringList attrList = attrs.split(",");
            foreach (QString attr, attrList) {
                QStringList attrItem = attr.split("=");
                if (2 == attrItem.count()) {
                    elemDom.setAttribute(attrItem[0], attrItem[1]);
                }
            }
        }

        deep++;
        while (deep<eleCnt) {
            elemName = elementList[deep];
            if (elemName.contains(":")) {
                QStringList elemTextList =  elemName.split(":");
                if (2 == elemTextList.count()) {
                    QDomElement textElem = doc.createElement(elemTextList[0]);
                    QDomText text = doc.createTextNode(elemTextList[1]);
                    textElem.appendChild(text);

                    elemDom.appendChild(textElem);
                }
            } else {
                WriteXMLRecord(doc, elemDom, elementList, deep);
                break;
            }
            deep++;
        }

        domElem.appendChild(elemDom);
    }
}



//读xml
void ReadXML(QString fileName, QVector<QStringList>& vElementList, QString rootName/*="Application"*/)
{
    //打开或创建文件
    const QString filePath = QCoreApplication::applicationDirPath() + "/" + fileName+".xml";
    LOG_INFO("write xml file: %s", QString2CharPtr(filePath));
    QFile file(filePath); //相对路径、绝对路径、资源路径都可以
    if(!file.open(QFile::ReadOnly)) {
        LOG_ERROR("cannot open file: %s", QString2CharPtr(filePath));
        return;
    }

    QDomDocument doc;
    if(!doc.setContent(&file))  {
        file.close();
        return;
    }
    file.close();


    QDomElement root=doc.documentElement(); //返回根节点
    if (rootName != root.nodeName()) {
        LOG_ERROR("文件格式出错");
        return;
    }


    QDomNode node=root.firstChild(); //获得第一个子节点

    while(!node.isNull()) { //如果节点不空

        if(node.isElement()) { //如果节点是元素

            QStringList elementList;
            ReadXMLRecord(node, elementList);
            vElementList.append(elementList);
        }
        node=node.nextSibling(); //下一个兄弟节点,nextSiblingElement()是下一个兄弟元素，都差不多
    }
}


bool ReadXMLRecord(QDomNode node, QStringList& elementList)
{
    QDomElement e=node.toElement(); //转换为元素，注意元素和节点是两个数据结构，其实差不多
    QDomNodeList list=e.childNodes();
    const int listCnt = list.count();
    QDomNamedNodeMap mapNode = e.attributes();
    QString nodes;
    for(int i=0; i< mapNode.size(); ++i) {
        QDomNode nodeItem = mapNode.item(i);
        nodes += nodeItem.nodeName() + "=" + nodeItem.nodeValue() + ",";
    }
    if (!nodes.isEmpty()) {
        nodes = nodes.remove(nodes.length()-1, 1);
        elementList.append(QString("%1(%2)").arg(e.tagName()).arg(nodes));
    } else {
        elementList.append(QString("%1").arg(e.tagName()));
    }


    for(int i=0;i<listCnt;i++) { //遍历子元素，count和size都可以用,可用于标签数计数

        QDomNode n = list.item(i);

        if (n.isElement()) {
            { // 回调之前判断是否是叶子节点
                QDomElement eLeaf=n.toElement();
                if (!eLeaf.isNull() && eLeaf.isElement()) {

                    QDomNodeList listLeafNode=eLeaf.childNodes();
                    if (1 == listLeafNode.size()) {
                        QDomNode nLeaf = listLeafNode.item(0);
                        if (!nLeaf.isElement()) { // 非叶子节点
                            if ("#text" == nLeaf.nodeName()) { // 叶子节点
                                QString pairElem = n.nodeName()+":"+n.toElement().text();
                                elementList.append(pairElem);
                            } else {
                                ReadXMLRecord(n, elementList);
                            }
                        }
                    } else { // 一定是非叶子节点
                        ReadXMLRecord(n, elementList);
                    }
                }
            } // 回调之前判断是否是叶子节点
        } else {
        }
    }

    return true;
}


#endif // GLOBAL_DATABASE_XML_H
