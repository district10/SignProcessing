#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

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
    Mat layerSizes=(Mat_<int>(1,5) << 5,2,2,2,5);
    bp.create( layerSizes, CvANN_MLP::SIGMOID_SYM );

    // int CvANN_MLP::train(const Mat& inputs,
    //                      const Mat& outputs,
    //                      const Mat& sampleWeights,
    //                      const Mat& sampleIdx=Mat(),
    //                      CvANN_MLP_TrainParams params=CvANN_MLP_TrainParams(),
    //                      int flags=0 )

    float trainingData[3][5] = {
        {   1,   2,   3,   4,  5 },
        { 111, 112, 113, 114, 115},
        {  21,  22,  23,  24,  25},
    };
    Mat trainingDataMat(3, 5, CV_32FC1, trainingData);

    float labels[3][5] = {
        { 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0 },
        { 1, 1, 1, 1, 1 },
    };
    Mat labelsMat(3, 5, CV_32FC1, labels);

    bp.train( trainingDataMat, labelsMat, Mat(), Mat(), params );

    // Data for visual representation
    int width = 512, height = 512;
    Mat image = Mat::zeros( height, width, CV_8UC3 );
    Vec3b green( 0, 255, 0 ), blue ( 255, 0, 0 );

    // Show the decision regions given by the SVM
    for ( int i = 0; i < image.rows; ++i ) {
        for ( int j = 0; j < image.cols; ++j ) {
            Mat sampleMat = (Mat_<float>(1,5) << i,j,0,0,0);
            Mat responseMat;
            bp.predict( sampleMat, responseMat );
            float *p = responseMat.ptr<float>(0);
            float response = 0.0f;
            for( int k=0; k < 5; k++ ) {
                response += p[k];
            }
            if ( response > 2 ) {
                image.at<Vec3b>(j, i)  = green;
            } else {
                image.at<Vec3b>(j, i)  = blue;
            }
        }
    }

    // Show the training data
    int thickness = -1;
    int lineType = 8;
    circle( image, Point(501,  10), 5, Scalar(  0,   0,   0), thickness, lineType);
    circle( image, Point(255,  10), 5, Scalar(255, 255, 255), thickness, lineType);
    circle( image, Point(501, 255), 5, Scalar(255, 255, 255), thickness, lineType);
    circle( image, Point( 10, 501), 5, Scalar(255, 255, 255), thickness, lineType);

    imwrite("result.png", image);        // save the image
    imshow("BP Simple Example", image); // show it to the user
    cvWaitKey(0);
}
