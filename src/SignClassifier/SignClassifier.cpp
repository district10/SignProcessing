#include "SignClassifier.h"
#include "ui_SignClassifier.h"
#include "Utils.h"
#include "Configs.h"
#include <QDebug>
#include <QMessageBox>

static QString TextSign     = QObject::tr( "Pos" );
static QString TextNotSign  = QObject::tr( "Neg" );
static QString TextUnknown  = QObject::tr( "???" );

static float        LabelPosRaw[1][1] = { {  1 }, };
static float        LabelNegRaw[1][1] = { { -1 }, };
static cv::Mat      LabelPosMat( 1, 1, CV_32FC1, LabelPosRaw );
static cv::Mat      LabelNegMat( 1, 1, CV_32FC1, LabelNegRaw );

SignClassifier::SignClassifier( QWidget *parent )
    : QDialog(parent)
    , ui(new Ui::SignClassifier)
{
    ui->setupUi(this);
    ui->predict->setEnabled( false );

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

    // cv ann mlp
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
    ui->train->setEnabled( true );
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
    ui->train->setEnabled( true );
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
    ui->train->setEnabled( false );
    QStringList newPos;
    QStringList newNeg;

    foreach( const QString &file, signsPos ) {
        if ( !signPairs.keys().contains(file) ) {
            newPos << file;
        }
    }

    foreach( const QString &file, signsNeg ) {
        if ( !signPairs.keys().contains(file) ) {
            newNeg << file;
        }
    }

    int np = newPos.length();
    int nn = newNeg.length();
    if ( np + nn <= 0 ) { return; }

    qDebug() << "Extra" << np << "pos(s) &" << nn << "neg(s) to train.";

    float *features = new float[FEATURENUM*(np + nn)];
    float *flags = new float[np + nn];
    for (int i = 0; i < np; ++i) {
        Utils::img2feature(qPrintable(newPos.at(i)), features + FEATURENUM*i);
        *(flags + i) = 1.0f;
    }
    for (int i = 0; i < nn; ++i) {
        Utils::img2feature(qPrintable(newNeg.at(i)), features + FEATURENUM*np+FEATURENUM*i);
        *(flags + np + i) = -1.0f;
    }
    cv::Mat featureMat( np+nn, FEATURENUM, CV_32F, features);
    cv::Mat flagMat( np+nn, 1, CV_32F, flags);
    cv::Ptr<cv::ml::TrainData> trainSet
            = cv::ml::TrainData::create( featureMat, cv::ml::ROW_SAMPLE, flagMat );
    mlp->train(trainSet);
    QMessageBox::information( this, tr("Done"), tr("done training %1 samples.").arg(np+nn) );

    ui->predict->setEnabled( true );
    ui->train->setEnabled( true );
    delete []features;
    delete []flags;
}

void SignClassifier::on_save_clicked()
{
    QString fileName = QFileDialog::getSaveFileName( this, tr("Save to XML"),
                                                     QString(CMAKE_SOURCE_DIR) + QString("/data/output"),
                                                     tr("XML (*.xml)") );
    mlp->save( qPrintable(fileName) );
    QMessageBox::information( this, tr("Save trained XML Success"),
                              tr("saved to %1").arg(fileName) );
}

void SignClassifier::on_load_clicked()
{
    ui->train->setEnabled( false );
    ui->load->setEnabled( false );
    ui->predict->setEnabled( true );

    QString fileName = QFileDialog::getOpenFileName( this, tr("Load XML"),
                                                     QString(CMAKE_SOURCE_DIR) + QString("/data/output"),
                                                     tr("XML (*.xml)") );
    mlp->clear();
    mlp = cv::Algorithm::load<cv::ml::ANN_MLP>( fileName.toStdString() );
    QMessageBox::information( this, tr("Load trained XML Success"),
                              tr("loaded %1").arg(fileName) );
}

void SignClassifier::on_predict_clicked()
{
    QList<QStandardItem *> unknowns = signsModelUnknown->findItems(TextUnknown);
    int length = unknowns.length();
    if ( !length ) {
        QMessageBox::information( this, tr("Wrong action"),
                                  tr("Select some unknowns then press [predict].") );
    }

    QProgressDialog progress(this);
    progress.setLabelText( tr("Predicting %1th image...").arg( length ) );
    progress.setRange( 0, length );
    progress.setModal( true );
    progress.show();

    int idx = 0;
    foreach( QStandardItem *item, unknowns ) {
        QString file = item->toolTip();
        progress.setValue( ++idx );
        if ( isSign( file ) ) {
            item->setText( TextSign );
        } else {
            item->setText( TextNotSign );
        }
    }
}

bool SignClassifier::isSign(const QString &file)
{
    bool isSign = false;
    float feature[FEATURENUM];
    if ( Utils::img2feature( qPrintable(file), feature ) ) {
        cv::Mat featureMat( 1, FEATURENUM, CV_32F, feature );
        cv::Mat result( 1, 1, CV_32FC1 );
        mlp->predict( featureMat, result );
        float *p = result.ptr<float>(0);
        isSign = *p > 0;
    } else {
        qDebug() << "failed one predict";
    }
    return isSign;
}
