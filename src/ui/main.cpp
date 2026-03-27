#include <QApplication>
#include <Eigen/Dense>    // TIB-04: TEMPORARY — remove before TIB-05
#include <iostream>       // TIB-04: TEMPORARY — remove before TIB-05
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    // TIB-04: TEMPORARY — Eigen3 verification (remove before TIB-05)
    std::cout << "Eigen3 Identity Matrix (3x3):\n"
              << Eigen::Matrix3d::Identity() << std::endl;

    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
