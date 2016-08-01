#include "SignProcessor.h"
#include "ui_SignProcessor.h"

#include <QFileDialog>
#include <QTemporaryFile>
#include <QDir>
#include <QDebug>
#include <QQueue>
#include <QStandardItemModel>

#include "Utils.h"

#include <cv.h>
#include <opencv2/core.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

#include "Configs.h"

using namespace std;
using namespace cv;

static QString TextSign     = QObject::tr( "Pos" );
static QString TextNotSign  = QObject::tr( "Neg" );
static QString TextUnknown  = QObject::tr( "???" );

SignProcessor::SignProcessor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SignProcessor)
{
    ui->setupUi(this);
    ui->image_label->setStyleSheet("border: 1px solid gray;");
    ui->tabWidget->setDisabled( true );
    ui->pushButton_loadxml->setStyleSheet( "background-color:      red;   font-weight: bold" );
    // ui->pushButton_loadxml->setStyleSheet( "background-color: lightgreen; font-weight: bold" );

    // mlp.loadXML( ":/data/output/4.xml" );
    connect( this, SIGNAL(init_model()),
             this, SLOT(initModel()) );
    emit init_model();

    unknowns = new QStandardItemModel;
    QListView *view = ui->listView;
    view->setModel( unknowns );
    view->setFlow( QListView::LeftToRight );
    view->setWrapping ( true );
    view->setViewMode( QListView::IconMode );
    view->setIconSize( QSize( 24, 24 ) );
    view->setUniformItemSizes ( true );
    view->setSpacing( 5 );
    view->setResizeMode ( QListView::Adjust );
    view->setWordWrap ( true );
    view->setEditTriggers( QListView::NoEditTriggers );
}

SignProcessor::~SignProcessor()
{
    delete ui;
}

void SignProcessor::loadImage( QString filePath )
{
    setWindowTitle( filePath );
    // ui->image_label->loadImage( filePath );
}

void SignProcessor::on_pushButton_loadxml_clicked()
{
    QString dir = QString(CMAKE_SOURCE_DIR) + QString("/data/output");
    QString fileName = QFileDialog::getOpenFileName( 0, QString(), dir,
                                                     tr("XML Model File (*.xml)") );
    mlp.loadXML( fileName );
    ui->pushButton_loadxml->setStyleSheet( "background-color: lightgreen; font-weight: bold" );
    ui->tabWidget->setDisabled( false );
}

void SignProcessor::on_pushButton_selectimgs_clicked()
{
    float feature[FEATURENUM];
    QStringList files = Utils::openMultipleFiles( QString(), this );
    if ( !files.isEmpty() ) {
        foreach ( const QString file, files ) {
            if ( !unknownsImgPath.contains( file ) ) {
                Mat img = imread( file.toStdString(), cv::IMREAD_COLOR ), mat;
                cv::resize( img, mat, cv::Size(IMGSIZE, IMGSIZE), 0.0, 0.0, cv::INTER_AREA );
                Utils::mat2feature( mat, feature );
                Mat featureMat(1, FEATURENUM, CV_32F, feature);
                QPixmap thumb = QPixmap::fromImage( Utils::cvMat2QImage(mat) );
                if ( !thumb.isNull() ) {
                    unknownsImgPath << file;
                    QStandardItem *item;
                    if( mlp.predictFeatureMat(featureMat) ) {
                        item = new QStandardItem( QIcon(thumb), TextSign );
                    } else {
                        item = new QStandardItem( QIcon(thumb), TextNotSign );
                    }
                    item->setToolTip( file );
                    unknowns->appendRow( item );
                }
            }
        }
    }
}

void SignProcessor::on_pushButton_selectdir_clicked()
{
    QString dirName =
        QFileDialog::getExistingDirectory(this, tr("Open a Dir thas Contains Images to Test"),
            QString(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QStringList dirEntries, imgEntries;
    QDir dir(dirName);
    QQueue<QDir> queue;
    queue.enqueue(dir);
    while (!queue.isEmpty()) {
        QDir d = queue.dequeue();
        dirEntries << d.absolutePath();
        foreach(const QFileInfo &info, d.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            QDir subdir(info.absoluteFilePath());
            queue.enqueue(subdir);
        }
        foreach(const QFileInfo &info, d.entryInfoList(QDir::Files)) {
            if (Utils::isPicture(info.absoluteFilePath())) {
                imgEntries << info.absoluteFilePath();
            }
        }
    }
    ui->textEdit->setText(QString("To Predict %1 images.\n\n").arg(imgEntries.length()));

    QDir posDir(Utils::dirname(dirName) + QString("/pos"));
    QDir negDir(Utils::dirname(dirName) + QString("/neg"));
    if (!posDir.exists()) { posDir.mkdir(posDir.absolutePath()); }
    if (!negDir.exists()) { negDir.mkdir(negDir.absolutePath()); }
    foreach ( const QString &img, imgEntries ) {
        ui->textEdit->setText(ui->textEdit->toPlainText() + QString("\n\tprocessing %1...").arg(img));
        float feature[FEATURENUM];
        Utils::img2feature( qPrintable(img), feature );
        Mat featureMat( 1, FEATURENUM, CV_32F, feature );
        if (mlp.predictFeatureMat(featureMat)) {
            ui->textEdit->setText(ui->textEdit->toPlainText() + QString(" done. assigned to [pos]"));
            QFile::copy( img, posDir.filePath(Utils::basename(img)) );
        } else {
            ui->textEdit->setText(ui->textEdit->toPlainText() + QString(" done. assigned to [neg]"));
            QFile::copy( img, negDir.filePath(Utils::basename(img)) );
        }
    }
}

void SignProcessor::initModel()
{
    QFile xml(":/data/output/4.xml");
    QTemporaryFile tmpXML;
    if ( !xml.open(QIODevice::ReadOnly | QIODevice::Text) || !tmpXML.open() ) {
        return;
    }
    {
        QTextStream out(&tmpXML);
        out << QString(xml.readAll());
    }
    QString xmlPath = tmpXML.fileName();
    tmpXML.close();
    mlp.loadXML( xmlPath );
    ui->pushButton_loadxml->setStyleSheet( "background-color: lightgreen; font-weight: bold" );
    ui->tabWidget->setDisabled( false );
}

void SignProcessor::on_pushButton_loadimg_clicked()
{
    QString dir = QString(CMAKE_SOURCE_DIR) + QString("/data/input/tests");
    QString fileName = QFileDialog::getOpenFileName( 0, QString(), dir,
                                                     tr("Images (*.png *.jpg *.jpeg *.bmp *.gif)") );

    Mat img = imread( fileName.toStdString(), cv::IMREAD_COLOR );
    if ( img.empty() ) {
        ui->image_label->setText( "Invalid Image" );
        return;
    }

    QImage qimg = Utils::cvMat2QImage(img);
    int image_width  = qimg.width();
    int image_height = qimg.height();
    int label_width  = ui->image_label->width();
    int label_height = ui->image_label->height();

    if ( image_width > label_width || image_height > label_height ) {
        if( (float)image_width/image_height > (float)label_width/label_height ) {
            qimg = qimg.scaledToWidth( 0.9f * label_width );
        } else {
            qimg = qimg.scaledToHeight( 0.9f * label_height );
        }
    }
    ui->image_label->setPixmap( QPixmap::fromImage( qimg ) );

    ui->label_result->setText( "???" );
    ui->label_result->setStyleSheet( "background-color: lightgray; font-weight: bold" );

    Mat mat;
    cv::resize( img, mat, cv::Size( IMGSIZE, IMGSIZE ), 0.0, 0.0, cv::INTER_AREA ); // interpolation method
    float feature[FEATURENUM];
    Utils::mat2feature( mat, feature );
    Mat featureMat( 1, FEATURENUM, CV_32F, feature );

    if ( mlp.predictFeatureMat(featureMat) ) {
        ui->label_result->setText("Sign (pos)");
        ui->label_result->setStyleSheet("background-color: lightblue; font-weight: bold");
    } else {
        ui->label_result->setText("Not Sign (neg)");
        ui->label_result->setStyleSheet("background-color:       red; font-weight: bold");
    }
}
