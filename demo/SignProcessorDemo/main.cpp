#include <QApplication>
#include "SignProcessor.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    SignProcessor sp;
    sp.show();
    return a.exec();
}