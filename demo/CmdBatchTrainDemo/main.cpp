#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv/cxcore.h>
#include "Utils.h"
 
using namespace std;
using namespace cv;

enum {
    IMGSIZE = 24,
    FEATURENUM = 24 * 6,
    OUTPUT_LAYER_SIZE = 1,
};

const float posValue = 1.0f;
const float negValue = 0.0f;

typedef QPair<QString, bool> SampleType;

void loadIndex( const QString &idxFile, QStringList &idx );

void writeLog( const QString &filename, QStringList lyrSz, int posNum, int negNum, float ratio, float accuracy, size_t time );

bool getPictureFeature( const QString &filePath, float *feature );

void trainSamples( const QList<SampleType> &samples, CvANN_MLP *bp, CvANN_MLP_TrainParams *params );

float predictSamples( const QList<SampleType> &predicts, CvANN_MLP *bp );

QStringList currentWeights( CvANN_MLP &bp );

int main( int argc, char **argv )
{
    if ( argc < 2 ) {
        return -1;
    }

    const int inputLayerSize = FEATURENUM;
    const int outputLayerSize = OUTPUT_LAYER_SIZE;
    float trainPartition = 0.75;

    QString pathBaseDir =  Utils::dirname( QString(argv[0]) );
    QString pathPosDir( "pos" );
    QString pathNegDir( "neg" );
    QString pathPosIdx( "pos.txt" );
    QString pathNegIdx( "neg.txt" );

    QStringList layerSizes = QString( argv[1] ).split(' ');
    Mat layerSizesMat = Mat_<int>(1, layerSizes.length());
    for ( int i = 0; i < layerSizes.length(); ++i ) {
        layerSizesMat.at<int>(0, i) = layerSizes.at(i).toInt();
    }

    if ( layerSizesMat.at<int>(0, 0) != inputLayerSize
         || layerSizesMat.at<int>(0, layerSizes.length()-1) != outputLayerSize ) {
             qDebug() << "Invalid Layer Sizes.";
             return -1;
    }

    QStringList posIdx, negIdx;
    loadIndex( pathPosIdx, posIdx );
    loadIndex( pathNegIdx, negIdx );

    QList<SampleType> samples;
    QList<SampleType> predicts;

    for ( int i = 0; i < negIdx.length(); ++i ) {
        SampleType sample( QString("%1/%2/%3").arg( pathBaseDir )
                                              .arg( pathNegDir )
                                              .arg( negIdx.at(i) ), false );
        if ( 0 != i % 4 ) {
            samples << sample;
        } else {
            predicts << sample;
        }
    }

    for ( int i = 0; i < posIdx.length(); ++i ) {
        SampleType sample( QString("%1/%2/%3").arg( pathBaseDir )
                                              .arg( pathPosDir )
                                              .arg( posIdx.at(i) ), true );
        // if ( i < posIdx.length() * trainPartition ) {
        if ( 0 != i % 4 ) {
            samples << sample;
        } else {
            predicts << sample;
        }
    }

    qDebug() << QString( "With %1 pos, %2 neg, %3 as training data, %4 to predict." )
                    .arg( posIdx.length() )
                    .arg( negIdx.length() )
                    .arg( (posIdx.length()+negIdx.length())* trainPartition )
                    .arg( (posIdx.length()+negIdx.length())*(1-trainPartition) );

    CvANN_MLP *bp = new CvANN_MLP;

    // bp.create( layerSizesMat, CvANN_MLP::SIGMOID_SYM );
    bp->create( layerSizesMat, CvANN_MLP::SIGMOID_SYM, 1.0, 1.0 );
    // bp.create( layerSizesMat, CvANN_MLP::SIGMOID_SYM, 0.0, 0.0 );

    CvANN_MLP_TrainParams *params = new CvANN_MLP_TrainParams;
    params->train_method = CvANN_MLP_TrainParams::BACKPROP;
    params->term_crit.type = 3;
    params->term_crit.max_iter = 5000;
    params->term_crit.epsilon = 0.01;


    // train
    /*
    for ( int i = 0; i < samples.length(); ) {
        const static int batchSize = 100;
        if ( i + batchSize < samples.length() ) {
            trainSamples( samples.mid( i, batchSize ), bp, params );
            i += batchSize;
        } else {
            QList<SampleType> allLasts = samples.mid( i );
            trainSamples( allLasts, bp, params );
            i += allLasts.length();
        }
    }
    */
    trainSamples( samples, bp, params );

    bp->save( "bp.xml" );

    // predict
    float accuracy = predictSamples( predicts, bp );
    qDebug() << "accuracy:" << accuracy;

    return 0;
}

void loadIndex( const QString &idxFile, QStringList &idx )
{
    QFile file( idxFile );
    if ( !file.open(QIODevice::ReadOnly) ) {
        return;
    }

    QTextStream in( &file );
    idx = in.readAll().split('\n');
}

bool getPictureFeature( const QString &filePath, float *feature )
{
    IplImage* img = cvLoadImage( qPrintable(filePath), 1 );
    if( !img ) {
        qDebug() << "no image";
        return false;
    }
    int rowCountR[IMGSIZE]={0}, rowCountG[IMGSIZE]={0}, rowCountB[IMGSIZE]={0};
    int colCountR[IMGSIZE]={0}, colCountG[IMGSIZE]={0}, colCountB[IMGSIZE]={0};
    int allCountR=0,allCountG=0,allCountB=0;
    CvScalar s;
    for(int i=0;i<IMGSIZE;i++){
        for(int j=0;j<IMGSIZE;j++){
            s=cvGet2D(img,i,j);
            rowCountR[i] += s.val[0];
            rowCountG[i] += s.val[1];
            rowCountB[i] += s.val[2];

            allCountR += s.val[0];
            allCountG += s.val[1];
            allCountB += s.val[2];
        }
    }
    for(int i=0;i<IMGSIZE;i++){
        for(int j=0;j<IMGSIZE;j++){
            s=cvGet2D(img,i,j);
            colCountR[i] += s.val[0];
            colCountG[i] += s.val[1];
            colCountB[i] += s.val[2];
        }
    }
    cvReleaseImage(&img);

    for (int i=0;i<IMGSIZE;i++)
    {
        feature[i*6+0] = rowCountR[i]/(1.0*allCountR);
        feature[i*6+1] = rowCountG[i]/(1.0*allCountG);
        feature[i*6+2] = rowCountB[i]/(1.0*allCountB);

        feature[i*6+3] = colCountR[i]/(1.0*allCountR);
        feature[i*6+4] = colCountG[i]/(1.0*allCountG);
        feature[i*6+5] = colCountB[i]/(1.0*allCountB);
    }
    return true;
}

void trainSamples( const QList<SampleType> &samples, CvANN_MLP *bp, CvANN_MLP_TrainParams *params )
{
    qDebug() << "training" << samples.length() << "samples";
    int sz = samples.length();
    float *data = new float[ sz * FEATURENUM ];
    float *flag = new float[ sz ];
    for ( int i = 0; i < sz; ++i ) {
        getPictureFeature( samples.at(i).first, &data[i * FEATURENUM] );
        flag[i] = samples.at(i).second ? posValue : negValue;
        qDebug() << (samples.at(i).second ? "POs!!" : "Neg");
    }

    // Mat trainingDataMat( sz, FEATURENUM, CV_32FC1, data );
    // Mat trainingFlagMat( sz,          1, CV_32FC1, flag );
    Mat trainingDataMat( sz, FEATURENUM, CV_32F, data );
    Mat trainingFlagMat( sz,          1, CV_32F, flag );

    bp->train( trainingDataMat, trainingFlagMat, Mat(), Mat(), *params );
    // qDebug() << currentWeights( bp ).join("\n");

    delete data;
    delete flag;
}

float predictSamples( const QList<SampleType> &predicts, CvANN_MLP *bp )
{
    qDebug() << "predicting" << predicts.length() << "samples";
    int sz = predicts.length();
    float *data = new float[ sz * FEATURENUM ];
    for ( int i = 0; i < sz; ++i ) {
        getPictureFeature( predicts.at(i).first, &data[i * FEATURENUM] );
    }

    Mat dataMat( sz, FEATURENUM, CV_32FC1, data );
    Mat results( sz,          1, CV_32FC1 );

    bp->predict( dataMat, results );

    int hits = 0;
    for ( int i = 0; i < sz; ++i ) {
        qDebug() << "predicting: " << (predicts.at(i).second ? "Pos" : "Neg");
        float value = results.at<float>( i, 0 );
        if ( ( qAbs( value - posValue ) < qAbs( value - negValue ) )
            && predicts.at(i).second ) {
                ++hits;
        } else {
            qDebug() << value;
        }
    }

    delete data;
    return (float)hits/sz;
}

QStringList currentWeights( CvANN_MLP &bp )
{
    QStringList output;
    int nlayers = bp.get_layer_count();
    Mat sizes = bp.get_layer_sizes();
    for ( int i = 0; i < nlayers; ++i ) {
        QStringList thisLayer;
        double *ws = bp.get_weights( i );
        for ( int j = 0; j < sizes.at<int>(0, i); ++j ) {
            thisLayer << QString::number(*(ws+j));
        }
        output << thisLayer.join("\t");
    }
    return output;
}

void writeLog( const QString &filename, QStringList lyrSz, int posNum, int negNum, float ratio, float accuracy, size_t time )
{

}
