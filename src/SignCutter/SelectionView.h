#ifndef	SELECTION_VIEW_H
#define SELECTION_VIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QtGui>

class CenterRubberBand;

class SelectionView: public QGraphicsView
{
	Q_OBJECT

public:
	static QRubberBand* _pRubberBand;
	static QRubberBand* _pExpandingRubberBand;
	CenterRubberBand*   _pCenterRubberBand;
	QPoint _originPoint;
	QPoint _terminalPoint;

signals:
	void rubberBandSelectionFinished();
	void cutoutSelectedRectangle();
	void rubberBandOnSelection();

public:
	SelectionView( QWidget *parent = 0 ); 
	~SelectionView( );
	
	void mousePressEvent( QMouseEvent *e );		
	void mouseMoveEvent( QMouseEvent *e );      
	void mouseReleaseEvent( QMouseEvent *e );
	void keyPressEvent( QKeyEvent *e );
	
	 QRect getSelectedRect();
	 QRect getRealTimeSelectedRect();
	
	//!* return the rubber band for control the rubber's visibility
	 QRubberBand* getRubberBand();
	 QRubberBand* getExpandingRubberBand();
	 CenterRubberBand* getCenterRubberBand();

	 static  bool   _isFromCenter;
	 static  int  	_snapHeight;
	 static  int    _snapWidth;
	 static  double	_snapRatio;
	 static  double _zoomRatio;
	 static  int    _expandingHeight;
	 static  int    _expandingWidth;
	 static  double	_expandingHeightRatio;
	 static  double _expandingWidthRatio;

	 bool _selectionFinished;
     bool _drawLine;
     bool _drawLineHold;
     QPointF lst, cur;
     QPainterPath *linePath;

     void dealWithPixmap( QPixmap pm );

     void setPix( QPixmap *p ) { pix = p; }
     QPixmap *getPix( ) { return pix; }
     QPixmap *pix;
     QGraphicsPixmapItem *pixItem;
};

class CenterRubberBand : public QRubberBand
{
public:
	CenterRubberBand( Shape s, QWidget *p = 0 ) : QRubberBand( s, p ) 
	{
		QPalette palette;
		palette.setBrush( QPalette::WindowText, QBrush( Qt::red ) );
		setPalette( palette );
	}
	~CenterRubberBand( ) { }

protected:
	virtual void paintEvent( QPaintEvent * ) 
	{
		QStylePainter painter( this );
		QStyleOptionFocusRect option;
		option.initFrom( this );
		painter.drawControl( QStyle::CE_FocusFrame, option );
	}

};

#endif  // SELECTION_VIEW