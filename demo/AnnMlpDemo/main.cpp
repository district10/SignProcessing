#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <iostream>
#include <string>
#include <QDateTime>
#include <QtGui>
#include <QHash>

using namespace std;
using namespace cv;

enum Pattern {
    Circle,
    Square,
};

enum {
    Width = 100,
    Height = 100,
    PatternSize = 60,
    TrainSize = 200,
};

bool isPos( const int &x, const int &y, Pattern pattern = Circle )
{
    return ( qAbs<int>(x - Width/2) < PatternSize / 2 &&
             qAbs<int>(y - Height/2) < PatternSize / 2 );
}

void pattern( Mat &data, Mat &labels, QHash<int, QPair<int, int> > &pts, Pattern pattern = Circle )
{
    qsrand( QDateTime::currentMSecsSinceEpoch() );

    float trainingData[TrainSize][2];
    float trainingLabels[TrainSize][1];
    pts.clear();

    // QHash<int, QPair<int, int> > pts;
    for ( int i = 0; i < TrainSize; ++i ) {
        int x, y;
        do {
            x = qrand() % Height;
            y = qrand() % Width;
        } while ( pts.values().contains( QPair<int, int>(x, y) ) );
        pts.insert( i, QPair<int, int>(x, y) );

        trainingData[i][0] = x;
        trainingData[i][1] = y;
        /*
        if ( ( qPow((x - Width/2), 2)
             + qPow((y - Height/2), 2) ) < qPow(PatternSize/2, 2) ) {
             */
        if ( qAbs<int>(x - Width/2) < PatternSize / 2 &&
             qAbs<int>(y - Height/2) < PatternSize / 2 ) {
            trainingLabels[i][0] =  1;
        } else {
            trainingLabels[i][0] = -1;
        } 
    }

    data = Mat( TrainSize, 2, CV_32FC1, trainingData );
    labels = Mat( TrainSize, 1, CV_32FC1, trainingLabels );
}

int main()
{
    CvANN_MLP bp;

    //  CvANN_MLP_TrainParams::CvANN_MLP_TrainParams()
    //  {
    //      term_crit = cvTermCriteria( 
    //              CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 1000, 0.01 );
    //      train_method = RPROP;
    //      bp_dw_scale = bp_moment_scale = 0.1;
    //      rp_dw0 = 0.1; rp_dw_plus = 1.2; rp_dw_minus = 0.5;
    //      rp_dw_min = FLT_EPSILON; rp_dw_max = 50.;
    //  }
    CvANN_MLP_TrainParams params;

    // CvANN_MLP_TrainParams::BACKPROP The back-propagation algorithm.
    // CvANN_MLP_TrainParams::RPROP The RPROP algorithm.
    params.train_method = CvANN_MLP_TrainParams::BACKPROP;

    // Strength of the weight gradient term. The recommended value is about 0.1.
    // params.bp_dw_scale = 0.1;

    // Strength of the momentum term (the difference between weights on the 2
    // previous iterations). This parameter provides some inertia to smooth the
    // random fluctuations of the weights. It can vary from 0 (the feature is
    // disabled) to 1 and beyond. The value 0.1 or so is good enough
    // params.bp_moment_scale = 0.1;

    // Initial value \Delta_0 of update-values \Delta_{ij}.
    // params.rp_dw0 = 0.1;

    // Increase factor \eta^+. It must be >1.
    // params.rp_dw_plus = 1.2;

    // Decrease factor \eta^-. It must be <1.
    // params.rp_dw_minus = 0.5;

    // Update-values lower limit \Delta_{min}. It must be positive.
    // params.rp_dw_min = FLT_EPSILON;

    // Update-values upper limit \Delta_{max}. It must be >1.
    // params.rp_dw_max = 50.;

    // void CvANN_MLP::create(const Mat& layerSizes, 
    //                        int activateFunc=CvANN_MLP::SIGMOID_SYM, 
    //                        double fparam1=0, 
    //                        double fparam2=0 )
    // * layerSizes 每 #neurons in each layer including the input/output layers
    // * activateFunc 每 Parameter specifying the activation function for each neuron
    //      CvANN_MLP::IDENTITY
    //      CvANN_MLP::SIGMOID_SYM
    //      CvANN_MLP::GAUSSIAN
    // * fparam1 每 Free parameter of the activation function, \alpha
    // * fparam2 每 Free parameter of the activation function, \beta
    bp.create( ( Mat_<int>(1,5) << 2, 4, 6, 4, 1),
                CvANN_MLP::SIGMOID_SYM );

    // int CvANN_MLP::train(const Mat& inputs,
    //                      const Mat& outputs,
    //                      const Mat& sampleWeights,
    //                      const Mat& sampleIdx=Mat(),
    //                      CvANN_MLP_TrainParams params=CvANN_MLP_TrainParams(),
    //                      int flags=0 )

    Mat data, labels;
    QHash<int, QPair<int, int> > pts;
    pattern( data, labels, pts );
    bp.train( data, labels, Mat(), Mat(), params );

    // Data for visual representation
    Mat image = Mat::zeros( Width, Height, CV_8UC3 );
    Vec3b green( 0, 0, 0 ), blue ( 0, 0, 0 );

    // Show the decision regions given by the SVM
    for ( int i = 0; i < image.rows; ++i ) {
        for ( int j = 0; j < image.cols; ++j ) {
            Mat sampleMat = (Mat_<float>(1,2) << i,j);
            Mat responseMat;
            bp.predict( sampleMat, responseMat );
            float *p = responseMat.ptr<float>(0);
            qDebug() << *p;
            if ( *p > 0 ) {
                image.at<Vec3b>(j, i)  = green;
            } else {
                image.at<Vec3b>(j, i)  = blue;
            }
        }
    }

    // Show the training data
    for ( int i = 0; i < TrainSize; ++i ) {
        QPair<int, int> pt = pts.value( i );
        circle( image, Point( pt.first, pt.second), 5, 
                isPos( pt.first, pt.second )
                ? Scalar(0,0,255)
                : Scalar(0,255,0) );  
    }

    imwrite("result.png", image);
    imshow("BP Simple Example", image);
    cvWaitKey(0);
}
