#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class EngineThread;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;


public slots:

public:
    void initUIComponents();

private:
    void saveLayout();
    void loadLayout();

private:
    Ui::MainWindow *ui;
    EngineThread* engineThread;
    int signalIndex;
};

#endif // MAINWINDOW_H
