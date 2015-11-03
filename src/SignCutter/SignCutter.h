#ifndef SIGN_CUTTER_H
#define SIGN_CUTTER_H

#include <QDialog>
#include <QModelIndex>
#include <Qstringlistmodel>
#include <QRectF>
#include <QModelIndex>
#include <QGraphicsScene>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QGraphicsItem>
#include <QPainter>
#include <QStringList>
#include "Utils.h"
#include "Config.h"
#include "ui_SignCutter.h"

#define FIT_TO_VIEW                 ( "Fit to View" )
#define FIT_TO_HEIGHT               ( "Fit to Height" )
#define FIT_TO_WIDTH                ( "Fit to Width" )
#define SELF_DEFINED_RATIO          ( "Self Defined Ratio" )
#define SELF_DEFINED                ( "Self Defined" )
#define ZOOM_IN_OUT_RATIO           ( 1.25 )

class SignCutter : public QDialog
{
	Q_OBJECT

private:
	Ui_signCutter ui;
	SelectionView *viewWorkspace;
	SelectionView *viewWorkspaceEx;

	QStringListModel *listModelWorkspace;
	QStringListModel *listModelThumbnail;
	QStringListModel *listModelPositive;
	QStringListModel *listModelNegative;

	QStringList imageFileNames;
	QStringList thumbFileNames;
	QStringList positiveFileNames;
	QStringList negativeFileNames;
	QString imgCurFileName;
	QImage imgCur;

	static const QString filter;
	QString  outdirSgn;
	QString  outdirPos;
	QString  outdirNeg;
	
	QGraphicsScene *viewSceneWorkspace;
	QGraphicsScene *viewSceneThumbnail;
	QGraphicsScene *viewSceneSelection;

private slots:
	void on_importButton_clicked();

	void on_sourceListView_clicked(const QModelIndex &index);
	void on_croppedObjectImageListView_clicked(const QModelIndex &index);
	void on_positiveSign_clicked(const QModelIndex &index);
	void on_negativeSign_clicked(const QModelIndex &index);

	void on_preFrameButton_clicked();
	void on_nextFrameButton_clicked();
	void on_beginingButton_clicked();
	void on_endButton_clicked();
	void on_selectButton_clicked();
	void on_deleteButton_clicked();

	void on_resultLocationToolButton_clicked();
	void on_zoomInToolButton_clicked();
	void on_zoomOutToolButton_clicked();
	void onZoomComboBox_IndexChanged(int currentIndex);

	void onCroppedObjectImageListModel_ModelReset();
	void onGraphicsViewScene_SelectionFinished();

	void onGraphicsViewScene_OnSelection();
	void closeEvent(QCloseEvent *e);

    void on_pushButton_helpgen_clicked();
    void on_pushButton_genpos_clicked();
    void on_pushButton_genneg_clicked();

public:
	SignCutter(QWidget *parent = 0);
	~SignCutter();

	void initializeControls();
    void showPicture( const QString &fileName );
	void showThumbnail( const QString &fileName );
    void setBaseDir( const QString &basedir );
	QAction *_selectedAciton;
};

#endif  // SIGN_CUTTER_H