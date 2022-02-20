#include "mainwindow.h"
#include <QApplication>




int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QWidget* window = new QWidget;
    window->resize(500, window->height());

    Chat* chat = new Chat;
    window->setLayout(chat->get_base());
    window->show();
    chat->ConnectToServer();

    return a.exec();
}










