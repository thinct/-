#ifndef FLOWWIDGET_H
#define FLOWWIDGET_H

#include <QObject>
#include <QTableWidget>
#include <QHeaderView>

#include <QStyledItemDelegate>

class NoFocusDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    NoFocusDelegate(){};
    ~NoFocusDelegate(){};

    void NoFocusDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
    {
        QStyleOptionViewItem itemOption(option);
        if (itemOption.state & QStyle::State_HasFocus)
        {
            itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
        }
        QStyledItemDelegate::paint(painter, itemOption, index);
    }
};




class FlowWidget : public QObject
{
    Q_OBJECT

public:
    FlowWidget(QTableWidget* tableWidgetParam) {
        tableWidget = tableWidgetParam;
        tableWidget->horizontalHeader()->setVisible(false);
        tableWidget->verticalHeader()->setVisible(false);
        tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tableWidget->setGridStyle(Qt::NoPen);
        tableWidget->setItemDelegate(new NoFocusDelegate());
        tableWidget->setStyleSheet("QTableWidget::item:selected { background-color: rgb(255, 255, 255) }");


        connect(tableWidget, &QTableWidget::cellClicked, [this](int row, int column){
            foreach (QWidget* widgetItem, listWidget) {
                widgetItem->setStyleSheet("border: 3px solid #D7D7D7; border-radius:15px;"
                                      "font: 20pt \"Arial\";");
            }

            QWidget* widget = tableWidget->cellWidget(row, column);
            if (widget) {
                widget->setStyleSheet("border: 3px solid #12BD75; border-radius:15px;"
                                      "font: 20pt \"Arial\";");
            }
        });
    }
    ~FlowWidget(){}

public:
    void addWidget(QWidget* widget) {

        listWidget.append(widget);

        { // 清空并且设置新的行列

//            tableWidget->setRowCount(0);
//            tableWidget->setColumnCount(0);

            const int rowCount = listWidget.count() / columnCount + 1;
            tableWidget->setRowCount(rowCount);
            tableWidget->setColumnCount(columnCount);
        } // 清空并且设置新的行列


        const int rowCount = tableWidget->rowCount();
        for (int row=0; row<rowCount; row++) {
            tableWidget->setRowHeight(row, 100);

            { // 如果是最后一列,可能是部分显示

                if (row == rowCount-1) {
                    for (int column=0; column<(listWidget.count() % columnCount); column++) {
                        tableWidget->setCellWidget(row, column, listWidget[row*columnCount+column]);
                    }
                    break;
                }
            } // 如果是最后一列,可能是部分显示

            for (int column=0; column<columnCount; column++) {
                tableWidget->setCellWidget(row, column, listWidget[row*columnCount+column]);
            }
        }
    }

private:
    QTableWidget* tableWidget;
    const int columnCount = 4;
    QList<QWidget*> listWidget;
};

#endif // FLOWWIDGET_H
