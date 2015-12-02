#include "SignClassifier.h"
#include "ui_SignClassifier.h"
#include "Utils.h"
#include <QDebug>
#include <QMessageBox>

static QString TextSign = QObject::tr( "Pos" );
static QString TextNotSign = QObject::tr( "Neg" );
static QString TextUnknown = QObject::tr( "XXX" );

using namespace cv;

static float LabelPosRaw[1][1] = {
    {  1 },
};

static float LabelNegRaw[1][1] = {
    { -1 },
};

static Mat LabelPosMat( 1, 1, CV_32FC1, LabelPosRaw );
static Mat LabelNegMat( 1, 1, CV_32FC1, LabelNegRaw );

SignClassifier::SignClassifier(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignClassifier)
{
    ui->setupUi(this);

    signsModelPos = new QStandardItemModel;
    ui->listView_pos->setModel( signsModelPos );

    signsModelNeg = new QStandardItemModel;
    ui->listView_neg->setModel( signsModelNeg );

    signsModelUnknown = new QStandardItemModel;
    ui->listView_unknown->setModel( signsModelUnknown );

    foreach( QListView *view, findChildren<QListView *>() ) {
        if ( view ) {
            view->setFlow( QListView::LeftToRight );
            view->setWrapping ( true );
            view->setViewMode( QListView::IconMode );
            view->setIconSize( QSize( 24, 24 ) );
            view->setUniformItemSizes ( true );
            view->setSpacing( 3 );
            view->setResizeMode ( QListView::Adjust );
            view->setWordWrap ( true );
            view->setEditTriggers( QListView::NoEditTriggers );
        }
    }

    params.train_method = CvANN_MLP_TrainParams::BACKPROP;
    bp.create((Mat_<int>(1,5) << 24*24*3, 24*24, 6*6, 8, 1),
               CvANN_MLP::SIGMOID_SYM );
}

SignClassifier::~SignClassifier()
{
    delete ui;
}

void SignClassifier::on_filepos_clicked()
{
    QStringList files = Utils::openMultipleFiles( QString(), this );
    if ( !files.isEmpty() ) {
        foreach ( const QString file, files ) {
            if ( !signsPos.contains( file ) ) {
                QPixmap *thumb = new QPixmap( file );
                if ( !thumb->isNull() ) {
                    signsPos << file;
                    QStandardItem *item = new QStandardItem( QIcon(*thumb), TextSign );
                    item->setToolTip( file );
                    signsModelPos->appendRow( item );
                }
            }
        }
    }
}

void SignClassifier::on_fileneg_clicked()
{
    QStringList files = Utils::openMultipleFiles( QString(), this );
    if ( !files.isEmpty() ) {
        foreach ( const QString file, files ) {
            if ( !signsNeg.contains( file ) ) {
                QPixmap *thumb = new QPixmap( file );
                if ( !thumb->isNull() ) {
                    signsNeg << file;
                    QStandardItem *item = new QStandardItem( QIcon(*thumb), TextNotSign );
                    item->setToolTip( file );
                    signsModelNeg->appendRow( item );
                }
            }
        }
    }
}

void SignClassifier::on_fileunknown_clicked()
{
    QStringList files = Utils::openMultipleFiles( QString(), this );
    if ( !files.isEmpty() ) {
        foreach ( const QString file, files ) {
            if ( !signsUnknown.contains( file ) ) {
                QPixmap *thumb = new QPixmap( file );
                if ( !thumb->isNull() ) {
                    signsUnknown << file;
                    QStandardItem *item = new QStandardItem( QIcon(*thumb), TextUnknown );
                    item->setToolTip( file );
                    signsModelUnknown->appendRow( item );
                }
            }
        }
    }
}

void SignClassifier::on_train_clicked()
{
    QStringList newPos;
    QStringList newNeg;

    foreach ( const QString &file, signsPos ) {
        if ( !signPairs.keys().contains( file ) ) {
            newPos << file;
        }
    }

    foreach ( const QString &file, signsNeg ) {
        if ( !signPairs.keys().contains( file) ) {
            newNeg << file;
        }
    }

    qDebug() << "Extra" 
             << newPos.length() << "pos(s) &" 
             << newNeg.length() << "neg(s) to train.";

    int np = newPos.length();
    int nn = newNeg.length();
    QProgressDialog progress(this);
    progress.setLabelText(tr("Training with %1 images.").arg( np + nn ) );
    progress.setRange(0, np + nn);
    int idx = 0;
    progress.setModal(true);
    progress.show();

    foreach ( const QString &file, newPos ) {
        progress.setValue( ++idx );
        signPairs.insert( file, Sign );
        train( file, Sign );
    }

    foreach ( const QString &file, newNeg ) {
        progress.setValue( ++idx );
        signPairs.insert( file, NotSign );
        train( file, NotSign );
    }
}

void SignClassifier::on_predict_clicked()
{
    QList<QStandardItem *> unknowns = signsModelUnknown->findItems( TextUnknown );
    qDebug() << "#unknown: " << unknowns.length();
    foreach( QStandardItem *item, unknowns ) {
        QString file = item->toolTip();
        if ( isSign( file ) ) {
            item->setText( TextSign );
        } else {
            item->setText( TextNotSign );
        }
    }
}

void SignClassifier::train( const QString &file, enum SignFlag flag )
{
    Mat input;
    if ( img2mat( file, input ) ) {
        bp.train( input, 
                  Sign == flag ? LabelPosMat : LabelNegMat, 
                  Mat(), 
                  Mat(), 
                  params );
    } else {
        qDebug() << "failed one train";
    }
}

bool SignClassifier::img2mat( const QString &file, 
                              cv::Mat &mat, 
                              TransConfig config /*= NORMAL */ )
{
    QImage img( file );
    float pixels[ Sign_Width * Sign_Height * 3 ];
    if ( !img.isNull() && Sign_Width == img.width() && Sign_Height == img.height() ) {
        for ( int i = 0; i < Sign_Width; ++i ) {
            for ( int j = 0; j < Sign_Height; ++j ) {
                const QRgb pixel = img.pixel( i, j );
                pixels[ 0 * Sign_Width * Sign_Height + i * Sign_Width + j ] = int(pixel & (0xFF << 0));
                pixels[ 1 * Sign_Width * Sign_Height + i * Sign_Width + j ] = int(pixel & (0xFF << 2));
                pixels[ 2 * Sign_Width * Sign_Height + i * Sign_Width + j ] = int(pixel & (0xFF << 4));
            }
        }
        mat = Mat( 1, Sign_Width*Sign_Height*3, CV_32FC1, pixels );
        return true;
    } else {
        return false;
    }
}

bool SignClassifier::isSign(const QString &file)
{
    Mat result;
    Mat input;
    if ( !img2mat( file, input ) ) {
        return false;
    }

    bp.predict( input, result );
    float *p = result.ptr<float>( 0 );
    if ( *p > 0 ) {
        return true;
    } else {
        return false;
    }
}
