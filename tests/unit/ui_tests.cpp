#include <gtest/gtest.h>
#include <QApplication>
#include "MainWindow.h"

TEST(MainWindowTest, InstantiateMainWindow) {
    int argc = 0;
    char *argv[] = {nullptr};
    QApplication app(argc, argv);

    MainWindow window(nullptr);
    EXPECT_NE(nullptr, &window);
}
