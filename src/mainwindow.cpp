#include "mainwindow.h"

#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Qt Desktop App");
    setCentralWidget(new QLabel("Qt 6 Widgets application", this));
    resize(640, 480);
}
