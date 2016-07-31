#include "Utils.h"
#include "Configs.h"
#include <iostream>

#include <QtGui>
#include <QStringList>
#include <QChar>

#include <opencv2/core.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace cv;

const QChar Utils::sep = QChar('/');

QString Utils::strippedName( const QString &fullFileName, QString &fileDir )
{
    fileDir = QFileInfo( fullFileName ).absolutePath();
    return QFileInfo( fullFileName ).fileName();
}

bool Utils::isPicture( const QString &fileName )
{
    if(
        fileName.endsWith( ".jpg",  Qt::CaseInsensitive ) ||
        fileName.endsWith( ".bmp",  Qt::CaseInsensitive ) ||
        fileName.endsWith( ".png",  Qt::CaseInsensitive ) ||
        fileName.endsWith( ".gif",  Qt::CaseInsensitive ) ||
        fileName.endsWith( ".jpeg", Qt::CaseInsensitive ) ||
        fileName.endsWith( ".pgm",  Qt::CaseInsensitive ) ||
        fileName.endsWith( ".ppm",  Qt::CaseInsensitive ) ||
        fileName.endsWith( ".tif",  Qt::CaseInsensitive ) ||
        fileName.endsWith( ".tiff", Qt::CaseInsensitive ) ||
        fileName.endsWith( ".xbm",  Qt::CaseInsensitive ) ||
        fileName.endsWith( ".tiff", Qt::CaseInsensitive ) ||
        fileName.endsWith( ".xpm",  Qt::CaseInsensitive ) ||
        fileName.endsWith( ".pbm",  Qt::CaseInsensitive ) ||
        fileName.endsWith( ".xbm",  Qt::CaseInsensitive )
       ) {
        return true;
    } else {
        return false;
    }
}

QStringList Utils::openMultipleFiles( const QString &filter, QWidget *widget )
{
    QStringList fileNames;

    QFileDialog::Options options;
    options |= QFileDialog:: DontUseNativeDialog;

    fileNames = QFileDialog::getOpenFileNames( widget,
                                               QObject::tr( "select multiple image files" ),
                                               DEFAULT_INPUT_IMAGE_DIR,  // entry dir
                                               filter,                   // filter
                                               (QString *)0,             // selected filter
                                               options );
    return fileNames;
}


static QFileInfoList getFilOfDir( const QString &dirPath )
{
    QFileInfoList fil;

    QDir dir = Utils::transferDirectory( dirPath );
    QString path = dir.path();

    if( !dir.exists() ) {
        return fil;
    }

    fil = dir.entryInfoList( QDir::Dirs
                           | QDir::Files
                           | QDir::Readable
                           | QDir::Writable
                           | QDir::Hidden
                           | QDir::NoDotAndDotDot, QDir::Name );
    return fil;
}

bool Utils::deleteFolderContent( const QString &dirPath, bool deleteFolder /* = false */ )
{
    QDir dir = transferDirectory(dirPath);
    QString path = dir.path();

    QFileInfoList fil = getFilOfDir( dirPath );

    QFileInfo curFile;

    if( !dir.exists() ) {
        return true;
    }

    while( fil.size() > 0 ) {
        int infoNum = fil.size();
        for( int i = infoNum-1; i >= 0; --i ) {
            curFile = fil[i];
            fil.removeAt( i );

            if ( curFile.isFile() ) {

                // remove file
                QFile fileTemp( curFile.filePath() );
                fileTemp.remove();

            } else if( curFile.isDir() ) {

                // append files in dir
                fil.append( getFilOfDir( curFile.filePath() ) );

            } else {

                qDebug() << "I have no idea what's going on...";

            }
        }
    }

    if ( deleteFolder && fil.size() == 0 ) {
        dir.rmdir( path );
    }

    return ( fil.size() == 0 );
}

bool Utils::makeFolder( const QString &folderName )
{
    QDir dir = QDir::current();
    dir.cdUp();

    if ( dir.exists(folderName) ) {
        deleteFolderContent( folderName );
    }

    return dir.mkdir( folderName );
}

QDir Utils::transferDirectory( const QString &folderName )
{
    QDir dir = QDir::current();
    dir.cdUp();
    return QDir( dir.path() + sep + folderName );
}

QString Utils::joinPath( const QString &dirPath, const QString &basename )
{
    QString joinedPath = dirPath;

    if ( dirPath.right(1) == sep ) {
        joinedPath.chop( 1 );
    }
    return joinedPath + sep + basename;
}

QStringList Utils::basename( const QStringList &paths )
{
    QStringList basenames;
    foreach ( const QString &path, paths ) {
        basenames << basename( path );
    }
    return basenames;
}

QString Utils::basename( const QString &path )
{
    return path.split( sep ).last();
}

QString Utils::basenameWithoutExt(const QString &path)
{
    QString bn = basename( path );
    bn.chop( 1 + bn.split('.').last().length() );
    return bn;
}

QString Utils::dirname(const QString &path)
{
    QString dir = path;
    QStringList s1 = path.split( QChar('/') );
    QStringList s2 = path.split( QChar('\\') );
    if ( s1.length() != 1 ) {
        dir.chop( s1.last().length() + 1 );
    } else if ( s2.length() != 1 ) {
        dir.chop( s2.last().length() + 1 );
    } else {
        qDebug() << "chopping failed...";
        dir = QDir::currentPath();
    }
    return dir;
}

void Utils::sleep( int t )
{
    QTime dieTime = QTime::currentTime().addMSecs(t);
    while( QTime::currentTime() < dieTime ) {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}

// from: http://blog.csdn.net/liyuanbhu/article/details/46662115
QImage Utils::cvMat2QImage( const cv::Mat& mat )
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if( mat.type() == CV_8UC1 ) {

        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for( int i = 0; i < 256; i++ )  {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for( int row = 0; row < mat.rows; row++ ) {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;

    } else if(mat.type() == CV_8UC3) {

        // 8-bits unsigned, NO. OF CHANNELS = 3
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();

    } else if( mat.type() == CV_8UC4 ) {

        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();

    } else {
        return QImage();
    }
}

cv::Mat Utils::QImage2cvMat( const QImage &image )
{
    cv::Mat mat;
    switch(image.format()) {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, CV_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat;
}

bool Utils::img2feature( const char *filePath, float *feature )
{
    Mat src = imread( filePath, cv::IMREAD_COLOR );
    if ( src.empty() ) {
        qDebug() << __FUNCDNAME__ << ",\tempty image:" << "\tfilepath:" << filePath;
    }

    Mat mat;
    resize( src, mat, cv::Size( IMGSIZE, IMGSIZE ), 0.0, 0.0, cv::INTER_AREA ); // interpolation method
    return mat2feature( mat, feature );
}

bool Utils::mat2feature( const cv::Mat &mat, float *feature )
{
    if ( mat.empty() || mat.cols != IMGSIZE || mat.rows != IMGSIZE || mat.channels() != 3 ) {
        return false;
    }

    int allCountR          =   0  , allCountG          =   0  , allCountB          =   0  ;
    int rowCountR[IMGSIZE] = { 0 }, rowCountG[IMGSIZE] = { 0 }, rowCountB[IMGSIZE] = { 0 };
    int colCountR[IMGSIZE] = { 0 }, colCountG[IMGSIZE] = { 0 }, colCountB[IMGSIZE] = { 0 };
    int tempR[4]           = { 0 }, tempG[4]           = { 0 }, tempB[4]           = { 0 };

    for( int i = 0; i < IMGSIZE; ++i ) {
        for( int j = 0; j < IMGSIZE; ++j ) {

            int b = mat.at<cv::Vec3b>(i,j)[0];
            int g = mat.at<cv::Vec3b>(i,j)[1];
            int r = mat.at<cv::Vec3b>(i,j)[2];

            allCountR += r;
            allCountG += g;
            allCountB += b;

            rowCountR[i] += r;
            rowCountG[i] += g;
            rowCountB[i] += b;

            colCountR[j] += r;
            colCountG[j] += g;
            colCountB[j] += b;

            int index = 2 * (i < IMGSIZE / 2 ? 0 : 1) + (j < IMGSIZE / 2 ? 0 : 1);
            tempR[index] += r;
            tempG[index] += g;
            tempB[index] += b;
        }
    }

    for ( int i=0; i < IMGSIZE; ++i ) {
        feature[i*6+0] = (float)rowCountR[i]/allCountR;
        feature[i*6+1] = (float)rowCountG[i]/allCountG;
        feature[i*6+2] = (float)rowCountB[i]/allCountB;
        feature[i*6+3] = (float)colCountR[i]/allCountR;
        feature[i*6+4] = (float)colCountG[i]/allCountG;
        feature[i*6+5] = (float)colCountB[i]/allCountB;
    }
    feature[IMGSIZE*6+0] = (float)allCountR / (allCountR + allCountG +allCountB);
    feature[IMGSIZE*6+1] = (float)allCountG / (allCountR + allCountG +allCountB);
    feature[IMGSIZE*6+2] = (float)allCountB / (allCountR + allCountG +allCountB);

    for ( int i = 0; i < 4; ++i ) {
        int total = tempR[i] + tempG[i] + tempB[i];
        feature[IMGSIZE*6+3 + 3*i+0] = (float)tempR[i] / total;
        feature[IMGSIZE*6+3 + 3*i+1] = (float)tempG[i] / total;
        feature[IMGSIZE*6+3 + 3*i+2] = (float)tempB[i] / total;
    }
    return true;
}
