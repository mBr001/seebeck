#include <QtGui/QApplication>
#include <QTextCodec>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForTr(codec);

    a.setApplicationName("Seebeck");
    a.setOrganizationDomain("www.vscht.cz");
    a.setOrganizationName("VŠCHT");

    MainWindow w;
    w.show();

    return a.exec();
}
