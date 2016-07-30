#include "SignTransformer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SignTransformer w;
    w.show();

    return a.exec();
}
