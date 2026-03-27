#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Tibyan");
    resize(800, 600);
}

MainWindow::~MainWindow() {
}

void MainWindow::closeEvent(QCloseEvent *event) {
    event->accept();
}
