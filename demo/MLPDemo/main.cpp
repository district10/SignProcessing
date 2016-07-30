#include "MLP.h"
#include "Configs.h"

#include <QStringList>
#include <QDebug>
#include <QPair>
#include <QList>

using namespace std;

int main( int argc, char **argv )
{
    QString trainIndex      = QString(CMAKE_SOURCE_DIR) + QString("/data/input/4.csv");
    QString xmlfile         = QString(CMAKE_SOURCE_DIR) + QString("/data/output/4.xml");
    QString predictIndex    = QString(CMAKE_SOURCE_DIR) + QString("/data/input/predicts.txt");
    if ( argc >= 2 ) {
        trainIndex  = QString(CMAKE_SOURCE_DIR) + QString("/data/input/%1.csv").arg(argv[1]);
        xmlfile     = QString(CMAKE_SOURCE_DIR) + QString("/data/output/%1.xml").arg(argv[1]);
    }

    qDebug() << trainIndex << "-->" << xmlfile;

    MLP mlp;
    mlp.loadCSV( trainIndex );
    mlp.train();
    mlp.saveXML(xmlfile);
    return 0;

    QStringList toPredict = MLP::loadImagePaths( predictIndex );
    QList<QPair<QString, bool> > outputs = mlp.predictImages(toPredict);
    // foreach( QPair<QString, bool> p, outputs) { }
    for (int i = 0; i < outputs.length(); ++i) {
        QPair<QString, bool > &p = outputs[i];
        qDebug() << p.second << p.first;
    }

    return 0;
}