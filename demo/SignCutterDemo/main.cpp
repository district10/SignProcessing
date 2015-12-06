#include <QtGui>
#include <QDialog>
#include <QApplication>
#include "SignCutter.h"
#include "Utils.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    SignCutter signCutter;
    signCutter.setBaseDir( Utils::dirname( QString(argv[0]) ) );
    signCutter.show();

    return app.exec();
}
