#include "SignTransformer.h"
#include "ui_SignTransformer.h"
#include "Utils.h"
#include "ThirdPartyUtils.h"
#include "Configs.h"
#include <QString>
#include <QDateTime>

const QString SignTransformer::filter =  
    QObject::tr( "images (*.bmp *.png *.jpg *.jpeg *.png *.gif *.tif *.tiff)" );
QString SignTransformer::_demoPath;

SignTransformer::SignTransformer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignTransformer)
{
    ui->setupUi(this);
    ui->graphicsView_ws->setScene( &scene );
    ui->graphicsView_ws->show( );

    outdir = QDir::currentPath();
    ui->lineEdit_outdir->setText( outdir );
}

SignTransformer::~SignTransformer()
{
    delete ui;
}

QStringList SignTransformer::processSign(const QString &path)
{
    QStringList outs;
    QString dstPath;

    Twin m2( path, outdir );

    for (int i = 0; i < 360; i += 45 ) {
        m2.rotate( i ); outs << m2.save( QString("-rot%1").arg(i) );
    }

    m2.flipLeftRight(); outs << m2.save();
    for (int i = 0; i < 360; i += 45 ) {
        m2.rotate( i ); outs << m2.save( QString("-rot%1").arg(i) );
    }

    m2.flipUpdown();    outs << m2.save();
    for (int i = 0; i < 360; i += 45 ) {
        m2.rotate( i ); outs << m2.save( QString("-rot%1").arg(i) );
    }

    return outs;
}

void SignTransformer::showImage(const QString &path)
{
    if ( Utils::isPicture( path ) && !pixmap.load( path ) ) {
        scene.clear();
        qDebug() << "error loading image.";
        return;
    }

    int vw = ui->graphicsView_ws->width();
    int vh = ui->graphicsView_ws->height();

    if ( (double)pixmap.height()/(double)vh > (double)pixmap.width()/(double)vw ) {
        pixmap = pixmap.scaledToHeight(vh);
    } else {
        pixmap = pixmap.scaledToWidth(vw);
    }

    scene.setSceneRect( 0, 0, pixmap.width(), pixmap.height() );
    scene.clear();
    scene.addPixmap( pixmap );
}

static QString getSuffix( int i ) {
    QString suffix;
    if ( 0 == i ) {
        suffix = "___";
    } else if ( 1 == i ) {
        suffix = "U-D";
    } else if ( 2 == i ) {
        suffix = "L-R";
    } else {
        qDebug() << "I have no idea what suffix you want.";
        suffix = "XXX";
    }
    return suffix;
}

static void drawRect( cv::Mat &mat, const cv::RotatedRect &rr, cv:: Scalar &color, int thickness = 1 ) {
    cv::Point2f rect_points[4];
    rr.points( rect_points );
    for( int j = 0; j < 4; ++j ) {
        cv::line( mat, rect_points[j], rect_points[(j+1)%4], color, thickness, CV_AA );
    }
}

QStringList SignTransformer::genPositives( const int &idx,
                                           const QString &path, 
                                           double x, double y,
                                           double dx, double dy, 
                                           const QString &outdir, 
                                           const QStringList &outs,
                                           const QList<int> &angles,
                                           const QList<int> &shiftsX,
                                           const QList<int> &shiftsY )
{
    static cv::Scalar posColor( 0, 255, 0 );
    QStringList dsts;
    cv::Mat img = cv::imread( path.toStdString(), CV_LOAD_IMAGE_UNCHANGED );
    if ( !img.data ) {
        qDebug() << "cannot load image" << path;
        return dsts;
    }

    cv::Mat imgLR, imgUD;
    cv::flip( img, imgUD, 0 );
    cv::flip( img, imgLR, 1 );

    double imgX =  x * img.cols;
    double imgY =  y * img.rows;
    double imgW = dx * img.cols;
    double imgH = dy * img.rows;

    MatType mt[3];
    cv::Mat dst;
    mt[0].m = &img;    mt[0].imgX = (    x)*img.cols; mt[0].imgY = (    y)*img.rows;
    mt[1].m = &imgUD;  mt[1].imgX = (    x)*img.cols; mt[1].imgY = (1.0-y)*img.rows;
    mt[2].m = &imgLR;  mt[2].imgX = (1.0-x)*img.cols; mt[2].imgY = (    y)*img.rows;
    mt[0].imgW = imgW; mt[0].imgH = imgH;
    mt[1].imgW = imgW; mt[0].imgH = imgH;
    mt[2].imgW = imgW; mt[0].imgH = imgH;
    cv::RotatedRect rr0( cv::Point2f( mt[0].imgX, mt[0].imgY ), cv::Size( imgW, imgH ), 0 );
    cv::RotatedRect rr1( cv::Point2f( mt[1].imgX, mt[1].imgY ), cv::Size( imgW, imgH ), 0 );
    cv::RotatedRect rr2( cv::Point2f( mt[2].imgX, mt[2].imgY ), cv::Size( imgW, imgH ), 0 );
    mt[0].rr = &rr0;
    mt[1].rr = &rr1;
    mt[2].rr = &rr2;

    cv::Mat tmp( rr0.size, mt[0].m->type() );
    QString prefix = QString().sprintf( "%s/%04d-%s", qPrintable( outdir ), idx, 
                                        qPrintable( Utils::basenameWithoutExt(path) ) );
    QString demoPath = QString( "%1/sgn/%2-demo.bmp" ).arg( Utils::dirname(outdir) )
                                                      .arg( Utils::basenameWithoutExt(path) );
    cv::Mat demo = cv::imread( demoPath.toStdString() );
    if ( !demo.data ) {
        img.copyTo( demo );
    }

    // base image, upside down, or left right mirrored
    for ( int i = 0; i < 3; ++i ) {
        QString cur = 
            QString().sprintf( "%s-%s.bmp", 
                               qPrintable( prefix ), 
                               qPrintable( getSuffix(i) ) );
        if ( outs.contains( cur ) || dsts.contains( cur ) ) {
            continue;
        }
        mt[i].rr->angle = 0.0f;
        ThirdPartyUtils::getRotRectImg( *mt[i].rr, *mt[i].m, tmp );
        cv::resize( tmp, dst, cv::Size( STANDARD_SIZE, STANDARD_SIZE ) );
        if ( 0 == i ) { drawRect( demo, *mt[i].rr, posColor ); }
        if ( cv::imwrite( cur.toStdString(), dst ) ) {
            dsts << cur;
        } else {
            qDebug() << "error writing" << cur;
        }
    }
    
    // rotation
    foreach( const int &angle, angles ) {
        for ( int i = 0; i < 3; ++i ) {
            QString cur = 
                QString().sprintf( "%s-%s-r%03d.bmp", 
                                   qPrintable( prefix ),
                                   qPrintable( getSuffix(i) ),
                                   angle );
            if ( outs.contains( cur ) || dsts.contains( cur ) ) {
                continue;
            }

            mt[i].rr->angle = angle;
            ThirdPartyUtils::getRotRectImg( *mt[i].rr, *mt[i].m, tmp );
            cv::resize( tmp, dst, cv::Size( STANDARD_SIZE, STANDARD_SIZE ) );
            if ( 0 == i ) { drawRect( demo, *mt[i].rr, posColor ); }
            if ( cv::imwrite( cur.toStdString(), dst ) ) {
                dsts << cur;
            } else {
                qDebug() << "error writing" << cur;
            }
        }
    }

    // translation
    QList<int> xFactor, yFactor;
    xFactor <<  1 << -1;
    yFactor <<  1 << -1;
    foreach ( const int &sx, shiftsX ) {
        foreach ( const int &sy, shiftsY ) {
            if ( sx <= 0 && sy <= 0 ) { continue; }
            foreach ( const int &fx, xFactor ) {
                foreach ( const int &fy, yFactor ) {

                    int xx = (double)sx / 100.0 * imgW;
                    int yy = (double)sy / 100.0 * imgH;
                    QString cur = 
                        QString().sprintf( "%s-shift-sx%03d-sy%03d.bmp", 
                                           qPrintable( prefix ),
                                           xx*fx, yy*fy );
                    if ( outs.contains( cur ) || dsts.contains( cur ) ) {
                        continue;
                    }

                    // !!! shit
                    cv::RotatedRect rr( cv::Point2f( imgX+xx*fx, imgY+yy*fy ), cv::Size( imgW, imgH ), 0 );
                    cv::Mat tmp( rr.size, img.type() );
                    ThirdPartyUtils::getRotRectImg( rr, img, tmp );
                    cv::resize( tmp, dst, cv::Size( STANDARD_SIZE, STANDARD_SIZE ) );
                    drawRect( demo, rr, posColor ); 
                    if ( cv::imwrite( cur.toStdString(), dst ) ) {
                        dsts << cur;
                    } else {
                        qDebug() << "error writing" << cur;
                    }

                }
            }
        }
    }
    
    mt[0].rr->angle = 0;
    drawRect( demo, *mt[0].rr, cv::Scalar( 0, 0, 255 ), 5 );
    if ( cv::imwrite( demoPath.toStdString(), demo ) ) {
        qDebug() << "write demo to" << demoPath;
    } else {
        qDebug() << "demo not generated";
    }
    _demoPath = demoPath;
    return dsts;
}

QStringList SignTransformer::genNegatives( const int &idx, 
                                           const QString &path, 
                                           double x, double y,
                                           double dx, double dy, 
                                           const QString &outdir, 
                                           const QStringList &outs )
{
    static cv::Scalar negColor( 255, 0, 0 );
    QStringList dsts;

    static QList<int> idxProcessed;
    if ( idxProcessed.contains(idx) ) {
        return dsts;
    } else {
        idxProcessed << idx;
    }

    cv::Mat img = cv::imread( path.toStdString(), CV_LOAD_IMAGE_UNCHANGED );
    if ( !img.data ) {
        qDebug() << "cannot load image" << path;
        return dsts;
    }

    double imgX =  x * img.cols;
    double imgY =  y * img.rows;
    double imgW = dx * img.cols;
    double imgH = dy * img.rows;

    int rangX = img.cols - 3 * imgW; // 3 = (shift_width=1 + center_offset=0.5)
    int rangY = img.rows - 3 * imgH;
    if ( rangX < 0 || rangY < 0 ) {
        qDebug() << "input image too small.";
        return dsts;
    }

    cv::Mat dst;

    QString prefix = QString().sprintf( "%s/%04d-%s-rand", qPrintable( outdir ), idx, 
                                        qPrintable( Utils::basenameWithoutExt(path) ) );
    QString demoPath = QString( "%1/sgn/%2-demo.bmp" ).arg( Utils::dirname(outdir) )
                                                  .arg( Utils::basenameWithoutExt(path) );
    cv::Mat demo = cv::imread( demoPath.toStdString() );
    if ( !demo.data ) {
        img.copyTo( demo );
    }

    qsrand( QDateTime::currentMSecsSinceEpoch() );
    for ( int i = 0; i < 700; ++i ) { // pos: 68 --> 1:10 -> 700

        int cx = 1.5 * imgW + qrand() % rangX;         // center x
        int cy = 1.5 * imgH + qrand() % rangY;
        int ra = qrand() % 360;                        // rotation angle
        int sw = qrand() % (int)imgW;                  // shift pixel
        int sh = qrand() % (int)imgH;
        if ( 0 == qrand() % 2) { sw *= -1; }
        if ( 0 == qrand() % 2) { sh *= -1; }
        if ( qAbs((int)imgX-(cx+sw)) < imgW || qAbs((int)imgY-(cx+sw)) < imgH ) {
            continue;
        }

        QString cur = QString().sprintf( "%s-cx%04d-cy%04d-r%03d.bmp", 
                                         qPrintable( prefix ),
                                         cx+sw, cy+sh, ra );
        if ( outs.contains( cur ) || dsts.contains( cur ) ) {
            continue;
        }

        cv::RotatedRect rr( cv::Point2f( cx+sw, cy+sh ), cv::Size( imgW, imgH ), ra );
        cv::Mat tmp( rr.size, img.type() );
        ThirdPartyUtils::getRotRectImg( rr, img, tmp );
        cv::resize( tmp, dst, cv::Size( STANDARD_SIZE, STANDARD_SIZE ) );
        drawRect( demo, rr, negColor );
        if ( cv::imwrite( cur.toStdString(), dst ) ) {
            dsts << cur;
        } else {
            qDebug() << "error writing" << cur;
        }
    }

    if ( cv::imwrite( demoPath.toStdString(), demo ) ) {
        qDebug() << "write demo to" << demoPath;
    } else {
        qDebug() << "demo not generated";
    }
    _demoPath = demoPath;
    return dsts;
}

void SignTransformer::on_pushButton_open_signs_clicked()
{
    signs.append( Utils::openMultipleFiles( filter, this ) );
    signs.removeDuplicates();
    signsModel.setStringList( Utils::basename( signs ) );
    ui->listView_ins->setModel( &signsModel );

    if ( !signs.empty() ) {
        showImage( signs.at(0) );
    }
}

void SignTransformer::on_pushButton_tx_signs_clicked()
{
    foreach( const QString &sign, signs ) {
        signXs.append( processSign(sign) );
    }

    signXs.removeDuplicates();
    signXsModel.setStringList( Utils::basename( signXs ) );
    ui->listView_outs->setModel( &signXsModel );
}

void SignTransformer::on_listView_ins_clicked(const QModelIndex &index)
{
    showImage( signs.at( index.row() ) );
}

void SignTransformer::on_listView_outs_clicked(const QModelIndex &index)
{
    showImage( signXs.at( index.row() ) );
}

void SignTransformer::on_pushButton_out_dir_clicked()
{
    outdir = QFileDialog::getExistingDirectory( this, 
                                                tr( "Save File Directory" ),
                                                DEFAULT_OUTPUT_IMAGE_DIR,
                                                QFileDialog::ShowDirsOnly | 
                                                QFileDialog::DontResolveSymlinks );
    ui->lineEdit_outdir->setText( outdir );
}

Twin::Twin(const QString &path, const QString &dir)
{
    idx = 0;
    inPath = path;
    outDir = dir;

    m[src()] = cv::imread( inPath.toStdString(), CV_LOAD_IMAGE_UNCHANGED );
    if ( !m[src()].data ) {
        valid = false;
    } else {
        valid = true;
    }

    m[src()].copyTo( m[dst()] );
}

QString Twin::save( QString suffix /* = "" */ )
{
    if ( !valid ) { return ""; }
    QString outPath = 
        QString( "%1/%2-%3%4.bmp" ).arg( outDir )
                                   .arg( QFileInfo( inPath ).completeBaseName() )
                                   .arg( log ).arg( suffix );
    return cv::imwrite( outPath.toStdString(), m[dst()] ) ? outPath : "";
}

void Twin::flipLeftRight()
{
    if ( !valid ) { return; }
    oxo();
    cv::flip( m[src()], m[dst()], 1 );
    log += "-flipLeftRight";
}

void Twin::flipUpdown()
{
    if ( !valid ) { return; }
    oxo();
    cv::flip( m[src()], m[dst()], 0 );
    log += "-flipUpDown";
}

void Twin::rotate(int angle0 /* = 0 */)
{
    if ( 0 == angle0 ) {
        return;
    }

    double angle = (double)angle0;
    cv::Point2f center( m[src()].cols / 2.0, m[src()].rows / 2.0 );
    cv::Mat rot = cv::getRotationMatrix2D( center, angle, 1.0 );
    cv::Rect bbox = cv::RotatedRect( center, m[src()].size(), angle ).boundingRect();
    rot.at<double>(0,2) += bbox.width/2.0 - center.x;
    rot.at<double>(1,2) += bbox.height/2.0 - center.y;
    cv::warpAffine( m[src()], m[dst()], rot, bbox.size() );
}

void Twin::translate(double wp, double hp)
{
    cv::Mat trans( 2, 3, CV_32FC1 );  
    trans.at<double>( 0, 0 ) = 1.0;
    trans.at<double>( 0, 1 ) = 0.0;
    trans.at<double>( 1, 0 ) = 0.0;
    trans.at<double>( 1, 1 ) = 1.0;
    trans.at<double>( 0, 2 ) = wp; //  * m[src()].size().width;
    trans.at<double>( 1, 2 ) = hp; //  * m[src()].size().height;
    cv::warpAffine( m[src()], m[dst()], trans, m[dst()].size() );
    
    /*
    int dx = wp * m[src()].cols;
    int dy = wp * m[src()].rows;

    m[dst()] = cv::Mat::zeros( m[src()].size(), m[src()].type() );
    m[src()]( cv::Rect( dy, dx, m[src()].cols - dy, m[src()].rows -dx)).copyTo(
        m[dst()](cv::Rect( 0, 0, m[src()].cols-dy, m[src()].rows-dx ) ));
    */
}
