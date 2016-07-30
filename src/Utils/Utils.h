#ifndef UTILS_H
#define UTILS_H

#include <QtGui>
#include <QStringList>
#include <QChar>

namespace cv {
    class Mat;
}

class QImage;

class Utils
{
public:
    static QString strippedName( const QString &fullFileName, QString &fileDir );
    static bool isPicture( const QString &fileName );
    static QStringList openMultipleFiles( const QString &filter, QWidget *widget );
    static bool deleteFolderContent( const QString &dirPath, bool deleteFolder = false );
    static bool makeFolder( const QString &folderName );
    static QDir transferDirectory( const QString &folderName );
    static QString joinPath( const QString &shortPath, const QString &suffix );
    static QStringList basename( const QStringList &paths );
    static QString basename( const QString &path );
    static QString basenameWithoutExt( const QString &path );
    static QString dirname( const QString &path );
    static void sleep( const quint16 &t );

    static QImage cvMat2QImage( const cv::Mat &mat );
    static cv::Mat QImage2cvMat( const QImage &image );

    static bool img2feature( const char *filePath, float *feature );

private:
    static const QChar sep;
     Utils( ) { }
     ~Utils( ) { }
};

#endif  // UTILS_H