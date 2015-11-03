#include <QtGui>
#include <QList>
#include "SelectionView.h"
#include "SignCutter.h"
#include "SignTransformer.h"
#include "SignLogger.h"

const QString SignCutter::filter =  
    QObject::tr( "images (*.bmp *.png *.jpg *.jpeg *.png *.gif *.tif *.tiff)" );

SignCutter::SignCutter(QWidget *parent)
	:QDialog(parent)
{
	ui.setupUi(this);
    ui.importButton      ->setStyleSheet( "background-color:      red;   font-weight: bold" );
    ui.selectButton      ->setStyleSheet( "background-color: lightgreen; font-weight: bold" );
    ui.pushButton_helpgen->setStyleSheet( "background-color: lightblue;  font-weight: bold" );
    ui.pushButton_genpos ->setStyleSheet( "background-color: lightblue;  font-weight: bold" );
    ui.pushButton_genneg ->setStyleSheet( "background-color: lightblue;  font-weight: bold" );

	listModelWorkspace = new QStringListModel(this);
	listModelThumbnail = new QStringListModel(this);
	listModelPositive  = new QStringListModel(this);
	listModelNegative  = new QStringListModel(this);

	initializeControls();

	viewWorkspace      = ui.workspaceGraphicsView;
	viewSceneWorkspace = new QGraphicsScene(this);
	viewSceneThumbnail = new QGraphicsScene(this);
	viewSceneSelection = new QGraphicsScene(this);
	ui.workspaceGraphicsView->setScene(viewSceneWorkspace);
	ui.workspaceGraphicsView->show();
	ui.thumbnailGraphicsView->setScene(viewSceneThumbnail);
	ui.thumbnailGraphicsView->show();
	ui.realTimeCroppedImageGraphicsView->setScene(viewSceneSelection);
	ui.realTimeCroppedImageGraphicsView->show();
}

SignCutter::~SignCutter()
{
    if ( listModelWorkspace ) { delete listModelWorkspace; }
    if ( listModelThumbnail ) { delete listModelThumbnail; }
    if ( viewSceneWorkspace ) { delete viewSceneWorkspace; }
    if ( viewSceneThumbnail ) { delete viewSceneThumbnail; }
    if (viewSceneSelection) { delete viewSceneSelection; }
}

void SignCutter::initializeControls()
{
    // zoom in/out factor
	ui.zoomComboBox->setEditable( true );
	static QStringList factorZoom = ( QStringList() << FIT_TO_VIEW
                                                    << FIT_TO_WIDTH
                                                    << FIT_TO_HEIGHT
                                                    << "0.25"
                                                    << "0.50"
                                                    << "1.00"
                                                    << "1.25"
                                                    << "1.50"
                                                    << "2.00"
                                                    << "3.00"
                                                    << "5.00"
                                                    << "8.00" );
	ui.zoomComboBox->addItems( factorZoom );
	ui.zoomComboBox->setCurrentIndex( ui.zoomComboBox->findText( "1.00" ) );
	connect( ui.zoomComboBox, SIGNAL(currentIndexChanged(int)),
             this, SLOT(onZoomComboBox_IndexChanged(int)) );

	connect( listModelThumbnail, SIGNAL(modelReset()),
             this, SLOT(onCroppedObjectImageListModel_ModelReset()) );
	connect( ui.workspaceGraphicsView, SIGNAL(rubberBandSelectionFinished()),
             this, SLOT(onGraphicsViewScene_SelectionFinished()) );
	connect( ui.workspaceGraphicsView, SIGNAL(cutoutSelectedRectangle()),
             this, SLOT(on_selectButton_clicked()) );
    connect( ui.workspaceGraphicsView, SIGNAL(rubberBandOnSelection()),
             this, SLOT(onGraphicsViewScene_OnSelection()) );

	ui.sourceListView->setSelectionMode( QListView::SingleSelection );
	ui.croppedObjectImageListView->setSelectionMode( QListView::SingleSelection );
	ui.sourceListView->setEditTriggers( QListView::NoEditTriggers );
	ui.croppedObjectImageListView->setEditTriggers( QListView::NoEditTriggers );

	outdirSgn = Utils::joinPath( QDir::currentPath(), "sign" );
	outdirPos = Utils::joinPath( QDir::currentPath(), "pos" );
	outdirNeg = Utils::joinPath( QDir::currentPath(), "neg" );

	QDir dir;
    dir = QDir( outdirSgn ); if ( !dir.exists() ) { dir.mkdir( outdirSgn ); }
    dir = QDir( outdirPos ); if ( !dir.exists() ) { dir.mkdir( outdirPos ); }
    dir = QDir( outdirNeg ); if ( !dir.exists() ) { dir.mkdir( outdirNeg ); }

    ui.resultLocationToolButton_pos->setEnabled( false );
    ui.resultLocationToolButton_neg->setEnabled( false );
	ui.resultLocationLineEdit->setText( outdirSgn );
	ui.resultLocationLineEdit->setReadOnly(true);
	ui.resultLocationLineEdit_pos->setText( outdirPos );
	ui.resultLocationLineEdit_pos->setReadOnly(true);
	ui.resultLocationLineEdit_neg->setText( outdirNeg );
	ui.resultLocationLineEdit_neg->setReadOnly(true);

	listModelThumbnail->setStringList( thumbFileNames );
	ui.croppedObjectImageListView->setModel( listModelThumbnail );

    viewWorkspace->_expandingWidth  = 0;
    viewWorkspace->_expandingHeight = 0;
    viewWorkspace->_expandingWidthRatio  = 1;
    viewWorkspace->_expandingHeightRatio = 1;
}

void SignCutter::onCroppedObjectImageListModel_ModelReset()
{
	ui.deleteButton->setDisabled(thumbFileNames.isEmpty());
}

void SignCutter::onGraphicsViewScene_SelectionFinished()
{
	ui.selectButton->setEnabled(true);
}

void SignCutter::on_importButton_clicked()
{
	imageFileNames.append( Utils::openMultipleFiles( filter, this ) );
	int numFiles = imageFileNames.size();
	if ( numFiles <= 0 ) {
		return;
    }

	listModelWorkspace->setStringList( Utils::basename( imageFileNames ) );
	ui.sourceListView->setModel( listModelWorkspace );

	ui.beginingButton->setDisabled(false);
	ui.preFrameButton->setDisabled(false);
	ui.nextFrameButton ->setDisabled(false);
	ui.endButton->setDisabled(false);
	ui.zoomComboBox->setDisabled(false);
	ui.zoomInToolButton ->setDisabled(false);
	ui.zoomOutToolButton->setDisabled(false);
	ui.resultLocationToolButton->setDisabled(false);

    showPicture( imageFileNames.at( 0 ) );
}

void SignCutter::on_sourceListView_clicked(const QModelIndex &index)
{
	QString	curfile = imageFileNames.at( index.row() );
	if ( imgCurFileName == curfile ) {
		return;
    }

	imgCurFileName = curfile;
	showPicture(curfile);
}

void SignCutter::on_preFrameButton_clicked()
{
	int idx = ui.sourceListView->currentIndex().row();
    idx = (--idx) % imageFileNames.size();
    showPicture( imageFileNames.at(idx) );

	ui.sourceListView->selectionModel()->setCurrentIndex( 
        ui.sourceListView->model()->index( idx, 0, ui.sourceListView->rootIndex()),
                                           QItemSelectionModel::ClearAndSelect );
}

void SignCutter:: on_nextFrameButton_clicked( )
{
	int idx = ui.sourceListView->currentIndex().row();
    idx = (++idx) % imageFileNames.size();
    showPicture( imageFileNames.at(idx) );

	ui.sourceListView->selectionModel()->setCurrentIndex( 
        ui.sourceListView->model()->index( idx, 0, ui.sourceListView->rootIndex()),
                                           QItemSelectionModel::ClearAndSelect );
}

void SignCutter::showPicture( const QString &fileName )
{	
	double zoomRatio = ui.zoomComboBox->currentText().toDouble(); 
    zoomRatio = qBound( (double)0.1, zoomRatio, (double)8.0 );
    ui.zoomComboBox->setEditText( QString:: number(zoomRatio, 'f', 2) );

	if ( Utils::isPicture( fileName )  && imgCur.load( fileName ) ) {
        imgCurFileName = fileName;
    } else {
        qDebug() << "Error loading image" << fileName;
        return;
    }

	int iw = imgCur.width();
	int ih = imgCur.height();
	int vw = ui.workspaceGraphicsView->width();
	int vh = ui.workspaceGraphicsView->height();

    static QPixmap pixmap;
	pixmap = QPixmap::fromImage(
        imgCur.scaled( iw * zoomRatio, ih * zoomRatio,
                       Qt::KeepAspectRatioByExpanding ) );
	int pw = pixmap.width();
	int ph = pixmap.height();

	viewSceneWorkspace->setSceneRect( 0, 0, pw, ph );
	viewSceneWorkspace->clear();
	viewSceneWorkspace->addPixmap( pixmap );
}

void SignCutter::on_deleteButton_clicked()
{
	if ( thumbFileNames.isEmpty() ) {
		return;
    }

	QModelIndex idx = ui.croppedObjectImageListView->currentIndex();
    if ( !idx.isValid() ) {
        return; 
    }

    // remove file in fs
	QDir dir = QDir( outdirSgn );
	dir.remove( thumbFileNames.at(idx.row()) );

    // remove file in string list
	thumbFileNames.removeAt( idx.row() );

    // refresh list view
	listModelThumbnail ->setStringList( Utils::basename( thumbFileNames ) );
	ui.croppedObjectImageListView->setModel( listModelThumbnail );

	if ( thumbFileNames.isEmpty() ) {
		viewSceneThumbnail->clear();
		return;
	}

    int cur = qBound( (int)0, idx.row() - 1, (int)thumbFileNames.size()-1 );
	QModelIndex index = ui.croppedObjectImageListView->model()->index( cur, 0, ui.croppedObjectImageListView->rootIndex() ); 
	ui.croppedObjectImageListView->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect );

	showThumbnail( thumbFileNames.at(cur) );
}

void SignCutter::on_beginingButton_clicked()
{
	if ( imageFileNames.isEmpty() ) {
		return;
    }

	imgCurFileName = imageFileNames.at(0);
	showPicture( imgCurFileName );

	QModelIndex index = ui.sourceListView->model()->index( 0, 0, ui.sourceListView->rootIndex() ); 
	ui.sourceListView->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect );
}

void SignCutter::on_endButton_clicked()
{
	if ( imageFileNames.isEmpty() ) {
		return;
    }

	int idx = imageFileNames.size() - 1;
    showPicture( imageFileNames.at(idx) );

	ui.sourceListView->selectionModel()->setCurrentIndex( 
        ui.sourceListView->model()->index( idx, 0, ui.sourceListView->rootIndex()),
                                           QItemSelectionModel::ClearAndSelect );
}

void SignCutter::onZoomComboBox_IndexChanged(int currentIndex)
{
	double zoomRatio, h_zoomRatio, w_zoomRatio;
	QString currentText = ui.zoomComboBox->itemText(currentIndex);
    QSize gvSize = ui.workspaceGraphicsView->size();

    h_zoomRatio = double(gvSize.height())/double(imgCur.height());
    w_zoomRatio = double(gvSize.width())/double(imgCur.width());
	if ( FIT_TO_VIEW == currentText ) {
		zoomRatio = w_zoomRatio < h_zoomRatio
                  ? w_zoomRatio
                  : h_zoomRatio;
	} else if ( FIT_TO_HEIGHT == currentText ) {
		if ( h_zoomRatio > w_zoomRatio ) {
			int bar_width = ui.workspaceGraphicsView->horizontalScrollBar()->height();
            if ( bar_width <= 0 ) { bar_width = 17; }
			zoomRatio = double(ui.workspaceGraphicsView->height()-bar_width)/double(imgCur.height());
		} else {
			zoomRatio = h_zoomRatio;
        }
	} else if ( FIT_TO_WIDTH == currentText ) {
		if ( h_zoomRatio < w_zoomRatio )
		{
			int bar_width = ui.workspaceGraphicsView->verticalScrollBar()->width();
            if ( bar_width <= 0 ) { bar_width = 17; }
			zoomRatio = double(ui.workspaceGraphicsView->width()-bar_width)/double(imgCur.width());
		} else {
			zoomRatio = w_zoomRatio;
        }
    } else {
		zoomRatio = currentText.toDouble();
	}

	zoomRatio = double(int(zoomRatio*100.0))/100.0;
    zoomRatio = qBound( (double)0.1, zoomRatio, (double)8.0 );
	ui.zoomComboBox->setEditText(QString:: number(zoomRatio, 'f', 2));
	viewWorkspace->_zoomRatio = zoomRatio;
	showPicture( imgCurFileName );
}

void SignCutter::on_resultLocationToolButton_clicked( )
{
	QString basedir = QFileDialog::getExistingDirectory( this, 
                                                         tr( "Save File Directory" ),
                                                         DEFAULT_OUTPUT_IMAGE_DIR,
                                                         QFileDialog::ShowDirsOnly | 
                                                         QFileDialog::DontResolveSymlinks );
    setBaseDir( basedir );
}

void SignCutter::on_selectButton_clicked( )
{
	ui.selectButton->setDisabled( true );

    // hide all rubber bands
	if ( ui.workspaceGraphicsView->getRubberBand() ) {
		ui.workspaceGraphicsView->getRubberBand()->hide();
    }

	if ( ui.workspaceGraphicsView->getExpandingRubberBand() ) {
		ui.workspaceGraphicsView->getExpandingRubberBand()->hide();
    }

	if ( ui.workspaceGraphicsView->getCenterRubberBand() ) {
		ui.workspaceGraphicsView->getCenterRubberBand()->hide();
    }

	QRect selectedRect = ui.workspaceGraphicsView->getSelectedRect();
	double zoomRatio = ui.zoomComboBox->currentText().toDouble();

	int expandingWidth  = int(selectedRect.width()*(viewWorkspace->_expandingWidthRatio-1)*0.5)+viewWorkspace->_expandingWidth*zoomRatio;
	int expandingHeight = int(selectedRect.height()*(viewWorkspace->_expandingHeightRatio-1)*0.5)+viewWorkspace->_expandingHeight*zoomRatio;

	QPoint ptTL, ptBR; // point TopLeft, BottomRight
	ptTL.rx() = selectedRect.topLeft().x() - expandingWidth;
	ptTL.ry() = selectedRect.topLeft().y() - expandingHeight;
	ptBR.rx() = selectedRect.bottomRight().x() + expandingWidth;
	ptBR.ry() = selectedRect.bottomRight().y() + expandingHeight;

	double heightWidthRatio = (double)selectedRect.height() / (double)selectedRect.width();
	int width = qRound( (double)(ptBR.x()-ptTL.x()+1) / zoomRatio );
	int height = qRound( heightWidthRatio * width );

	if ( SelectionView::_isFromCenter ) {
		QPoint cp = QPoint( 
            qRound( selectedRect.center().x() / zoomRatio ), 
            qRound( selectedRect.center().y() / zoomRatio ) );
		ptTL = QPoint( cp.x()-width/2,  cp.y()-height/2 );
	} else {
		ptTL = QPoint( 
            qRound( ptTL.x() / zoomRatio ), 
            qRound( ptTL.y() / zoomRatio ) );
	}
	ptBR = QPoint(
        ptTL.x() + width, 
        ptTL.y() + height );

    QRect selection = QRect(ptTL, ptBR);
	QImage croppedImage = imgCur.copy( selection );
	if ( croppedImage.isNull() || 
         croppedImage.width() <= 1 || 
         croppedImage.height() <= 1 ) {
		return;
    }

    /*
    qDebug() << "[" << imgCur.width() << "x" << imgCur.height() << "]"
             << "->" 
             << "{" << selection.width() << "x" << selection.height() << "}"
             << "-->"
             << "(" << (double)selection.center().x() / (double)imgCur.width()
             << "x" << (double)selection.center().y() / (double)imgCur.height()
             << ")";
    */

    static int _croppedObjectImageOrdinal = 0;
	QString cur = QString( "%1-%2.%3" ).arg( CROPPING_PREFIX )
                                       .arg( ++_croppedObjectImageOrdinal, 5, 10, QChar('0') )
                                       .arg( CROPPING_SUFFIX );
	QString fullCur = Utils::joinPath( outdirSgn, cur );

    double  x = (double)selection.center().x() / (double)imgCur.width();
    double  y = (double)selection.center().y() / (double)imgCur.height();
    double dx = (double)selection.width()      / (double)imgCur.width(); 
    double dy = (double)selection.height()     / (double)imgCur.height(); 
    if ( SignLogger::logging( Log( imgCurFileName, x, y, dx, dy ) ) ) {
        ui.label_msg->setStyleSheet( "background-color: white;" );
        ui.label_msg->setText( QString("#logs: %1").arg( SignLogger::getCount() ) );
    } else {
        ui.label_msg->setStyleSheet( "background-color: red;" );
        ui.label_msg->setText( QString("Ugly Rectangle!!" ) );
        --_croppedObjectImageOrdinal;
        return;
    }

	croppedImage.save( fullCur );
	thumbFileNames << fullCur;
	listModelThumbnail->setStringList( Utils::basename( thumbFileNames ) );
	ui.croppedObjectImageListView->setModel( listModelThumbnail );
	showThumbnail( fullCur );

	QModelIndex index = 
        ui.croppedObjectImageListView->model()->index( thumbFileNames.size()-1, 0, 
                                                       ui.croppedObjectImageListView->rootIndex() ); 
	ui.croppedObjectImageListView->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect);
}

void SignCutter::on_croppedObjectImageListView_clicked( const QModelIndex &index )
{
	showThumbnail( thumbFileNames.at( index.row() ) );
}

void SignCutter::on_positiveSign_clicked(const QModelIndex &index)
{
	showThumbnail( positiveFileNames.at( index.row() ) );
}

void SignCutter::on_negativeSign_clicked(const QModelIndex &index)
{
	showThumbnail( negativeFileNames.at( index.row() ) );
}

void SignCutter::showThumbnail(const QString &fileName)
{
	QPixmap pix;
	pix.load( fileName );

	int vw = ui.thumbnailGraphicsView->width();
	int vh = ui.thumbnailGraphicsView ->height();

	if ( (double)pix.height()/(double)vh > (double)pix.width()/(double)vw ) {
		pix = pix.scaledToHeight(vh);   // too much height, so scaled to height
    } else {
		pix = pix.scaledToWidth(vw);    // too much width
    }

	viewSceneThumbnail->setSceneRect( 0, 0, pix.width(), pix.height() );
	viewSceneThumbnail->clear();
	viewSceneThumbnail->addPixmap( pix );
}

void SignCutter::setBaseDir(const QString &basedir)
{
	outdirSgn = Utils::joinPath( basedir, "sgn" );
	outdirPos = Utils::joinPath( basedir, "pos" );
	outdirNeg = Utils::joinPath( basedir, "neg" );

	QDir dir;
    dir = QDir( outdirSgn ); if ( !dir.exists() ) { dir.mkdir( outdirSgn ); }
    dir = QDir( outdirPos ); if ( !dir.exists() ) { dir.mkdir( outdirPos ); }
    dir = QDir( outdirNeg ); if ( !dir.exists() ) { dir.mkdir( outdirNeg ); }
	ui.resultLocationLineEdit    ->setText( outdirSgn );
	ui.resultLocationLineEdit_pos->setText( outdirPos );
	ui.resultLocationLineEdit_neg->setText( outdirNeg );
}

void SignCutter::onGraphicsViewScene_OnSelection()
{

	QRect  selectedRect;

	if (viewWorkspace->_isFromCenter == true&&viewWorkspace->_selectionFinished == false)
		selectedRect = ui.workspaceGraphicsView->getRealTimeSelectedRect();
	else
		selectedRect = ui.workspaceGraphicsView->getSelectedRect();

	double zoomRatio = ui.zoomComboBox->currentText().toDouble();

	zoomRatio = viewWorkspace->_zoomRatio;

	double heightWidthRatio = (double)selectedRect.height() / (double)selectedRect.width();
	int width = qRound( (double)selectedRect.width() / zoomRatio );
	int height = qRound( heightWidthRatio * width );

	QPoint topLeft, bottomRight;
	if ( SelectionView::_isFromCenter )
	{
		QPoint centerPoint = QPoint( qRound(selectedRect.center().x() / zoomRatio), qRound(selectedRect.center().y() / zoomRatio) );
		topLeft = QPoint( centerPoint.x()-width/2,  centerPoint.y()-height/2 );
	}
	else
	{
		topLeft = QPoint( qRound(selectedRect.topLeft().x() / zoomRatio), qRound(selectedRect.topLeft().y() / zoomRatio) );
	}
	bottomRight = QPoint(topLeft.x()+width, topLeft.y()+height);


	QImage croppedImage = imgCur.copy(QRect(topLeft, bottomRight));
	if ( croppedImage.isNull() || croppedImage.width() <= 1 || croppedImage.height() <= 1 )
		return;


	QPixmap *realTimePixmap = new QPixmap;
	*realTimePixmap = QPixmap::fromImage(croppedImage);

	int viewWidth = ui.realTimeCroppedImageGraphicsView->width();
	int viewHeight = ui.realTimeCroppedImageGraphicsView ->height();

	if ( (double)realTimePixmap->height()/(double)viewHeight > (double)realTimePixmap->width()/(double)viewWidth )
		*realTimePixmap = realTimePixmap->scaledToHeight(viewHeight);
	else	
		*realTimePixmap = realTimePixmap->scaledToWidth(viewWidth);

	int pixWidth = realTimePixmap->width();
	int pixHeight = realTimePixmap->height();

	//keep the pixmap staying in the central 
	viewSceneSelection->setSceneRect(0, 0, pixWidth, pixHeight);
	//clear the previous scene
	viewSceneSelection->clear();
	//add the image to scene
	viewSceneSelection->addPixmap(*realTimePixmap);

}

void SignCutter::on_zoomInToolButton_clicked()
{
	double zoomRatio = ui.zoomComboBox->currentText().toDouble() * ZOOM_IN_OUT_RATIO;
	if (zoomRatio >= 8.0 )  zoomRatio = 8.0;
	zoomRatio = double(int(zoomRatio*100.0))/100.0;
	ui.zoomComboBox->setEditable(true);
	ui.zoomComboBox ->setEditText(QString::number(zoomRatio, 'f', 2));
	viewWorkspace->_zoomRatio = zoomRatio;
	showPicture( imgCurFileName );
}

void SignCutter::on_zoomOutToolButton_clicked()
{
	double zoomRatio = ui.zoomComboBox->currentText().toDouble() / ZOOM_IN_OUT_RATIO;
	if (zoomRatio <= 0.10 )  zoomRatio = 0.10;
	zoomRatio = double(int(zoomRatio*100.0))/100.0;
	ui.zoomComboBox->setEditable(true);
	ui.zoomComboBox ->setEditText(QString::number(zoomRatio, 'f', 2));
	viewWorkspace->_zoomRatio = zoomRatio;
	showPicture(imgCurFileName);
}

//!* delete the temporary folder created for storing the frames before exit the dialog
void SignCutter::closeEvent(QCloseEvent *event)
{
	QString fileName;
	for (int i =0; i<imageFileNames.size(); i++)
	{
		fileName = QString::number(i);
		//!* set the second parameter "true"  to delete the folder itself at the same time.
		Utils::deleteFolderContent(fileName, true);	
	}
	this->close();
}

void SignCutter::on_pushButton_helpgen_clicked()
{
    QStringList logs;
    foreach ( const Log &log, SignLogger::getLogs() ) {
        logs << log.save();
    }
    QMessageBox::information( this, 
                              "S i g n   I n f o",
                              "<patch id> <img path> <cx> <cy> <width> <height>\n" + logs.join( "\n" ) );
}

void SignCutter::on_pushButton_genpos_clicked()
{
    foreach ( const Log &log, SignLogger::getLogs() ) {
        QString path = log.path;
        int idx = log.idx;
        double x = log.x, y = log.y, dx = log.dx, dy = log.dy;

        QList<int> angles, shiftX, shiftY;
        for ( int i = 0; i < 360; i += 30 ) {
            angles << i;
        }

        for ( int i = 0; i < 40; i += 10 ) {
            shiftX << i;
            shiftY << i;
        }

        positiveFileNames << SignTransformer::genPositives( idx, path, x, y, dx, dy, 
                                                            outdirPos, positiveFileNames,
                                                            angles, shiftX, shiftY );
    }

    showPicture( SignTransformer::_demoPath );
    qSort( positiveFileNames.begin(), positiveFileNames.end() );
	listModelPositive->setStringList( Utils::basename( positiveFileNames ) );
	ui.positiveSign->setModel( listModelPositive );
}

void SignCutter::on_pushButton_genneg_clicked()
{
    foreach ( const Log &log, SignLogger::getLogs() ) {
        QString path = log.path;
        int idx = log.idx;
        double x = log.x, y = log.y, dx = log.dx, dy = log.dy;
        negativeFileNames << SignTransformer::genNegatives( idx, path, x, y, dx, dy, 
                                                            outdirNeg, positiveFileNames );
    }

    showPicture( SignTransformer::_demoPath );
    // qSort( negativeFileNames.begin(), negativeFileNames.end() );
	listModelNegative->setStringList( Utils::basename( negativeFileNames ) );
	ui.negativeSign->setModel( listModelNegative );
}
