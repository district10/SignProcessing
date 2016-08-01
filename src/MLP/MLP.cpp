#include "MLP.h"

#include <iostream>
#include <cstdio>
#include <fstream>
#include "Utils.h"
#include "Configs.h"

#include <QDebug>

using namespace std;

MLP::MLP()
{
    mlp = cv::ml::ANN_MLP::create();
    mlp->setLayerSizes( (cv::Mat)(cv::Mat_<int>(1,5)
                                    << FEATURENUM, FEATURENUM / 2, FEATURENUM / 6, FEATURENUM / 24, 1) );
    mlp->setActivationFunction( cv::ml::ANN_MLP::SIGMOID_SYM, 1.0, 1.0 );
    mlp->setTrainMethod( cv::ml::ANN_MLP::BACKPROP, 0.1, 0.1 );
    mlp->setTermCriteria( cv::TermCriteria(
                                cv::TermCriteria::COUNT + cv::TermCriteria::EPS
                                , 5000
                                , 0.01 ) );
}

void MLP::loadXML( const QString &xml  )
{
    pos.clear();
    neg.clear();
    trained.clear();

    mlp->clear();
    mlp = cv::Algorithm::load<cv::ml::ANN_MLP>( xml.toStdString() );
}

void MLP::saveXML( const QString &xml  )
{
    mlp->save( qPrintable(xml) );
}

void MLP::loadCSV( const QString &csv )
{
    QString basedir = Utils::dirname(csv);
    qDebug() << "basedir : " << basedir;

    ifstream data( qPrintable(csv) );
    string line;
    if ( !data.is_open() ) { return; }
    while ( getline( data, line ) ) {
        QStringList pair = QString(line.c_str()).split(",");
        if ( pair.length() == 2 ) {
            int flag = -1;
            sscanf( qPrintable(pair.at(1)), "%d", &flag );
            switch (flag) {
            case 0: neg << (basedir +"/"+ pair.at(0)); break;
            case 1: pos << (basedir +"/"+ pair.at(0)); break;
            default:
                qDebug() << "You are fucked.";
            }
        }
    }
    qDebug() << "grabed " << pos.length() + neg.length() << "images";
}

void MLP::train()
{
    int np = pos.length();
    int nn = neg.length();
    if ( np + nn <= 0 ) { return; }

    std::cout << "Training" << np << "pos(s) &" << nn << "neg(s)...";

    float *features = new float[FEATURENUM*(np + nn)];
    float *flags = new float[np + nn];

    for ( int i = 0; i < np; ++i ) {
        Utils::img2feature(qPrintable(pos.at(i)), features + FEATURENUM*i);
        *(flags + i) = 1.0f;
    }
    for ( int i = 0; i < nn; ++i ) {
        Utils::img2feature(qPrintable(neg.at(i)), features + FEATURENUM*np+FEATURENUM*i);
        *(flags + np + i) = -1.0f;
    }

    cv::Mat featureMat( np+nn, FEATURENUM, CV_32F, features );
    cv::Mat flagMat( np+nn, 1, CV_32F, flags );
    cv::Ptr<cv::ml::TrainData> trainSet = cv::ml::TrainData::create( featureMat, cv::ml::ROW_SAMPLE, flagMat );
    mlp->train( trainSet );

    trained << pos << neg;
    pos.clear();
    neg.clear();

    std::cout << "Training" << np << "pos(s) &" << nn << "neg(s)... done.";

    delete[] features;
    delete[] flags;
}

bool MLP::predictFeatureMat( const cv::Mat &featureMat )
{
    cv::Mat result( 1, 1, CV_32FC1 );
    mlp->predict( featureMat, result );
    float *p = result.ptr<float>(0);
    return *p > 0;
}

QList<QPair<QString, bool> > MLP::predictImages( const QStringList &images )
{
    QList<QPair<QString, bool> > output;

    float feature[FEATURENUM];
    foreach( const QString &img, images ) {
        Utils::img2feature( qPrintable(img), feature );
        cv::Mat featureMat( 1, FEATURENUM, CV_32F, feature );
        cv::Mat result( 1, 1, CV_32FC1 );
        mlp->predict( featureMat, result );
        float *p = result.ptr<float>(0);
        output << QPair<QString, bool>( img, *p>0 );
    }

    return output;
}

QStringList MLP::loadImagePaths( const QString &path )
{
    QString basedir = Utils::dirname( path );

    QStringList output;
    ifstream data( qPrintable(path) );
    string line;
    if ( !data.is_open() ) { return QStringList(); }
    while ( getline( data, line ) ) {
        QString p = QString(line.c_str()).simplified();
        if ( !p.isEmpty() ) {
            output << (basedir +"/"+ p);
        }
    }
    return output;
}
