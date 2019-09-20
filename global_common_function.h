#ifndef GLOBAL_COMMON_FUNCTION_H
#define GLOBAL_COMMON_FUNCTION_H
#include <QStringList>
#include <QRegularExpression>


/*
QString strTest = "(12345,4567)@(2,3)";
QStringList xList = extractSubStr(strTest, "\\((?<X>\\d+),");
QStringList yList = extractSubStr(strTest, ",(?<Y>\\d+)\\)");
*/
QStringList extractSubStr(const QString& stringOrigin, const QString& stringRegExp)
{
    QStringList resultList;
    int index_start = stringRegExp.indexOf("<");
    int index_end = stringRegExp.indexOf(">");
    if (index_start>=0 && index_end>=0 && index_end>index_start) {
        const QString keyWord = stringRegExp.mid(index_start+1, index_end-index_start-1);
        QRegularExpression re(stringRegExp);
        int index = 0;
        while(index < stringOrigin.length()) {
            QRegularExpressionMatch match = re.match(stringOrigin, index);
            if (match.hasMatch()) {
                index = match.capturedEnd();
                resultList.append(match.captured(keyWord));
            } else {
                break;
            }
        }
    }
    return resultList;
}


#endif // GLOBAL_COMMON_FUNCTION_H
