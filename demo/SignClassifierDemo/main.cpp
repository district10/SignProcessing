#include "SignClassifier.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SignClassifier w;
    w.show();

    return a.exec();
}
