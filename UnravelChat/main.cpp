#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("UnravelChat");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Unravel Studio");

    qDebug() << "UnravelChat 启动中...";

    MainWindow window;
    window.show();

    qDebug() << "主界面显示完成";

    return app.exec();
}
