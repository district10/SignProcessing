#include "SelectionView.h"

SelectionView::SelectionView( QWidget *parent )
	: QGraphicsView( parent )
{
	_pRubberBand = new QRubberBand(QRubberBand::Rectangle, this);
	_pExpandingRubberBand = new QRubberBand(QRubberBand::Rectangle, this);
	_pCenterRubberBand = new CenterRubberBand(QRubberBand::Rectangle, this);

	QPalette palette;
	palette.setBrush( QPalette::Highlight, QBrush( QColor(255,0,0,10) ) );  // red
	_pRubberBand->setPalette(palette);
	palette.setBrush( QPalette::Highlight, QBrush( QColor(0,0,255,10) ) ); // blue
	_pExpandingRubberBand->setPalette(palette);
	
	_selectionFinished = false;
    _drawLine = false;
    linePath = NULL;
    pix = NULL;
    pixItem = NULL;
}

SelectionView::~SelectionView( )
{
	delete _pRubberBand;
	delete _pCenterRubberBand;
	delete _pExpandingRubberBand;
}

void SelectionView::mousePressEvent( QMouseEvent *e )
{
    if ( _drawLine ) {
        _drawLineHold = true;
        delete linePath;
        linePath = new QPainterPath;
        linePath->moveTo( e->posF() );
        e->accept();
        return;
    }

	//add
	QPoint expandingTopLeft;
	QPoint expandingTerminalPoint;

	int expandingWidth  = int(_snapWidth*_zoomRatio*(_expandingWidthRatio-1)*0.5)+_expandingWidth*_zoomRatio;
	int expandingHeight = int(_snapHeight*_zoomRatio*(_expandingHeightRatio-1)*0.5)+_expandingHeight*_zoomRatio;

	//add
	_selectionFinished = false;

	_originPoint = e->pos();

	// !* selfDefine selection, draw a rectangle with size of given pixel.
	if (_snapRatio == -1.0)
	{
		//!* from center
		if (_isFromCenter)
		{
			_pCenterRubberBand->show();
			_pCenterRubberBand->setGeometry(QRect(_originPoint-QPoint(3,3), _originPoint+QPoint(3,3)).normalized());
			_pRubberBand->setGeometry( _originPoint.x() - int(_snapWidth*_zoomRatio/2),	
				_originPoint.y() - int(_snapHeight*_zoomRatio/2), 
				_snapWidth*_zoomRatio, _snapHeight*_zoomRatio);	
			_pExpandingRubberBand->setGeometry( _originPoint.x() - int(_snapWidth*_zoomRatio/2) - expandingWidth,	
				_originPoint.y() - int(_snapHeight*_zoomRatio/2) - expandingWidth, 
				_snapWidth*_zoomRatio + expandingWidth*2, _snapHeight*_zoomRatio + expandingWidth*2);	
		}
		//!* from TopLeft
		else
		{
			_pCenterRubberBand->hide();
			_pRubberBand->setGeometry( _originPoint.x(), _originPoint.y(), 
				_snapWidth*_zoomRatio-1, _snapHeight*_zoomRatio-1);	
			_pExpandingRubberBand->setGeometry( _originPoint.x() - expandingWidth, _originPoint.y() - expandingWidth, 
				_snapWidth*_zoomRatio-1 + expandingWidth*2, _snapHeight*_zoomRatio-1 + expandingWidth*2);	
		}
	}
	else
	{
		//!* from center
		if (_isFromCenter)
		{
			_pCenterRubberBand->show();
			_pCenterRubberBand->setGeometry(QRect(_originPoint-QPoint(3,3), _originPoint+QPoint(3,3)).normalized());
		}
		else
			_pCenterRubberBand->hide();
		_pRubberBand->setGeometry(QRect(_originPoint, QSize()));
		_pExpandingRubberBand->setGeometry(QRect(_originPoint, QSize()));
	}

	_pExpandingRubberBand->show();
	_pRubberBand->show();
}	

void SelectionView::mouseMoveEvent( QMouseEvent *e )
{
    if ( _drawLine && _drawLineHold ) {
        cur = e->posF();
        linePath->lineTo( e->posF() );
        qDebug() << "mid: " << e->posF();
        return;
    }

	QPoint topLeft;
	QPoint pointTemp;

	if ( _snapRatio == -1.0 )
		return;

	if (_pRubberBand)
	{
		if (_isFromCenter)
		{
			//*! free ratio
			if (_snapRatio == 0.0)
			{
				pointTemp = e ->pos();
			}
			
			//*!  a ratio is given
			if (_snapRatio != 0.0 && _snapRatio != -1.0 )
			{
				//!* keep the x coordinate the same as the current point while mouse is moving
				//!* make the perpendicular coordinate adjust to the ratio.
				pointTemp.rx() = e->pos().x();
				pointTemp.ry() = _originPoint.y() + (e->pos().x() - _originPoint.x()) * (_snapRatio);
			}
			
			topLeft = _originPoint + _originPoint - pointTemp;
		}
		//!* from the TopLeft
		else
		{
			//!* free ratio
			if (_snapRatio == 0.0)
			{

				topLeft = _originPoint;
				pointTemp = e->pos();
			}
			
			//!* specified ratio
			if (_snapRatio != 0.0 && _snapRatio != -1.0)
			{
				pointTemp.rx() = e->pos().x();
				pointTemp.ry() = _originPoint.y() + (e->pos().x() - _originPoint.x()) * _snapRatio;
				topLeft = _originPoint;
			}
		}


		//!* from center
		if (_isFromCenter)
		{
			_pCenterRubberBand->show();
			_pCenterRubberBand->setGeometry(QRect(_originPoint-QPoint(3,3), _originPoint+QPoint(3,3)).normalized());
		}
		else
			_pCenterRubberBand->hide();

		_pRubberBand->setGeometry(QRect(topLeft, pointTemp).normalized());

		//add
		QPoint expandingTopLeft;
		QPoint expandingpointTemp;

		int expandingWidth  = int(_pRubberBand->width()*(_expandingWidthRatio-1)*0.5)+_expandingWidth*_zoomRatio;
		int expandingHeight = int(_pRubberBand->height()*(_expandingHeightRatio-1)*0.5)+_expandingHeight*_zoomRatio;

		_terminalPoint = pointTemp;

		//add
		if ((topLeft.x()-pointTemp.x())*(topLeft.y()-pointTemp.y())<0)
		{
			int y;
			y = topLeft.y();
			topLeft.ry() = pointTemp.y();
			pointTemp.ry() = y;
		}

		if (topLeft.x()<pointTemp.x())
		{
			expandingTopLeft.rx() = topLeft.x()-expandingWidth;
			expandingTopLeft.ry() = topLeft.y()-expandingHeight;
			expandingpointTemp.rx() = pointTemp.x()+expandingWidth;
			expandingpointTemp.ry() = pointTemp.y()+expandingHeight;
		}

		else
		{
			expandingTopLeft.rx() = pointTemp.x()-expandingWidth;
			expandingTopLeft.ry() = pointTemp.y()-expandingHeight;
			expandingpointTemp.rx() = topLeft.x()+expandingWidth;
			expandingpointTemp.ry() = topLeft.y()+expandingHeight;
		}

		_pExpandingRubberBand->setGeometry(QRect(expandingTopLeft, expandingpointTemp).normalized());
	}

	emit rubberBandOnSelection();
}

void SelectionView::mouseReleaseEvent( QMouseEvent *e )
{
    if ( _drawLine ) {
        QPainter painter( getPix() );
        painter.setPen(QPen(Qt::red, 12, Qt::DashDotLine, Qt::RoundCap));
        painter.drawPath( *linePath );
        pixItem->setPixmap( *getPix() );
        // scene()->addPixmap( *getPix() );
        _drawLineHold = false;
        return;
    }

	//!* self-defined pixel
	if ( _snapRatio == -1.0)
	{
		if (_isFromCenter)
		{
			_originPoint.rx() = e ->pos().x()  - int(_snapWidth*_zoomRatio/2+0.5);
			_originPoint.ry() = e ->pos().y()  - int(_snapHeight*_zoomRatio/2+0.5);
			_terminalPoint.rx() = e ->pos().x() + int(_snapWidth*_zoomRatio/2+0.5);
			_terminalPoint.ry() = e ->pos().y() + int(_snapHeight*_zoomRatio/2+0.5);
		}
		else
		{
			_originPoint = e->pos();
			_terminalPoint.rx() = e ->pos().x() + int(_snapWidth*_zoomRatio+0.5)  - 1;
			_terminalPoint.ry() = e ->pos().y() + int(_snapHeight*_zoomRatio+0.5) - 1;
		}
	}
	//!* specify ratio
	else
	{
		if (_isFromCenter)
		{
			_originPoint = _originPoint + _originPoint - _terminalPoint;  
		}
	}


	QRect  selectedRect = getSelectedRect();
	QPoint topLeft = selectedRect.topLeft() / _zoomRatio;
	QPoint bottomRight = selectedRect.bottomRight() / _zoomRatio;

	if ( qAbs(bottomRight.x()-topLeft.x()) > 2 && qAbs(bottomRight.y()-topLeft.y()) > 2 )
			emit rubberBandSelectionFinished();

	_selectionFinished = true;
}

void SelectionView::keyPressEvent( QKeyEvent *e )
{
	if (_selectionFinished == true)
	{
		switch (e->key())
		{
		case Qt::Key_Space:
			{
				e->accept();
				emit cutoutSelectedRectangle();
				break;
			}

		case Qt::Key_Up:
			{
				_pCenterRubberBand->hide();
				_pRubberBand->hide();
				_pExpandingRubberBand->hide();
				e->accept();
				_originPoint.ry() = _originPoint.y()-1;
				_terminalPoint.ry() = _terminalPoint.y()-1;

				if (_isFromCenter)
				{

					QPoint center;
					center.rx() = _originPoint.x() + (_terminalPoint.x()-_originPoint.x())*0.5;
					center.ry() = _originPoint.y() + (_terminalPoint.y()-_originPoint.y())*0.5;

					//add
					QPoint expandingTopLeft;
					QPoint expandingTerminalPoint;

					int expandingWidth  = int(_pRubberBand->width()*(_expandingWidthRatio-1)*0.5)+_expandingWidth*_zoomRatio;
					int expandingHeight = int(_pRubberBand->height()*(_expandingHeightRatio-1)*0.5)+_expandingHeight*_zoomRatio;

					//expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
					//expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
					//expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
					//expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;

					//add
					if ((_originPoint.x()-_terminalPoint.x())*(_originPoint.y()-_terminalPoint.y())<0)
					{
						int y;
						y = _originPoint.y();
						_originPoint.ry() = _terminalPoint.y();
						_terminalPoint.ry() = y;
					
					}

					if (_originPoint.x()<_terminalPoint.x())
					{
						expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;
					}

					else
					{
						expandingTopLeft.rx() = _terminalPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _terminalPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _originPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _originPoint.y()+expandingHeight;
					}

					_pExpandingRubberBand->setGeometry(QRect(expandingTopLeft, expandingTerminalPoint).normalized());

					//_pExpandingRubberBand->setGeometry(QRect(_originPoint, _terminalPoint).normalized());
					_pExpandingRubberBand->show();

					_pRubberBand->setGeometry(QRect(_originPoint, _terminalPoint).normalized());
					_pRubberBand->show();

					_pCenterRubberBand->setGeometry(QRect(center-QPoint(3,3), center+QPoint(3,3)).normalized());
					_pCenterRubberBand->show();

				}

				else
				{
					//add
					QPoint expandingTopLeft;
					QPoint expandingTerminalPoint;

					int expandingWidth  = int(_pRubberBand->width()*(_expandingWidthRatio-1)*0.5)+_expandingWidth*_zoomRatio;
					int expandingHeight = int(_pRubberBand->height()*(_expandingHeightRatio-1)*0.5)+_expandingHeight*_zoomRatio;

	/*				expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
					expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
					expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
					expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;*/

					if ((_originPoint.x()-_terminalPoint.x())*(_originPoint.y()-_terminalPoint.y())<0)
					{
						int y;
						y = _originPoint.y();
						_originPoint.ry() = _terminalPoint.y();
						_terminalPoint.ry() = y;

					}

					if (_originPoint.x()<_terminalPoint.x())
					{
						expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;
					}

					else
					{
						expandingTopLeft.rx() = _terminalPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _terminalPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _originPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _originPoint.y()+expandingHeight;
					}

					_pExpandingRubberBand->setGeometry(QRect(expandingTopLeft, expandingTerminalPoint).normalized());
					_pExpandingRubberBand->show();

					_pRubberBand->setGeometry(QRect(_originPoint, _terminalPoint).normalized());
					_pRubberBand->show();

				}

				emit rubberBandOnSelection();

				break;	

			}
		case Qt::Key_Down:
			{
				_pCenterRubberBand->hide();
				_pRubberBand->hide();
				_pExpandingRubberBand->hide();

				e->accept();
				//printf("Down!");


				_originPoint.ry() = _originPoint.y()+1;
				_terminalPoint.ry() = _terminalPoint.y()+1;

				if (_isFromCenter)
				{

					QPoint center;
					center.rx() = _originPoint.x() + (_terminalPoint.x()-_originPoint.x())*0.5;
					center.ry() = _originPoint.y() + (_terminalPoint.y()-_originPoint.y())*0.5;

					//add
					QPoint expandingTopLeft;
					QPoint expandingTerminalPoint;

					int expandingWidth  = int(_pRubberBand->width()*(_expandingWidthRatio-1)*0.5)+_expandingWidth*_zoomRatio;
					int expandingHeight = int(_pRubberBand->height()*(_expandingHeightRatio-1)*0.5)+_expandingHeight*_zoomRatio;

					//expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
					//expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
					//expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
					//expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;

					if ((_originPoint.x()-_terminalPoint.x())*(_originPoint.y()-_terminalPoint.y())<0)
					{
						int y;
						y = _originPoint.y();
						_originPoint.ry() = _terminalPoint.y();
						_terminalPoint.ry() = y;

					}

					if (_originPoint.x()<_terminalPoint.x())
					{
						expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;
					}

					else
					{
						expandingTopLeft.rx() = _terminalPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _terminalPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _originPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _originPoint.y()+expandingHeight;
					}

					_pExpandingRubberBand->setGeometry(QRect(expandingTopLeft, expandingTerminalPoint).normalized());

					//_pExpandingRubberBand->setGeometry(QRect(_originPoint, _terminalPoint).normalized());
					_pExpandingRubberBand->show();

					_pRubberBand->setGeometry(QRect(_originPoint, _terminalPoint).normalized());
					_pRubberBand->show();

					_pCenterRubberBand->setGeometry(QRect(center-QPoint(3,3), center+QPoint(3,3)).normalized());
					_pCenterRubberBand->show();

				}

				else
				{
					//add
					QPoint expandingTopLeft;
					QPoint expandingTerminalPoint;

					int expandingWidth  = int(_pRubberBand->width()*(_expandingWidthRatio-1)*0.5)+_expandingWidth*_zoomRatio;
					int expandingHeight = int(_pRubberBand->height()*(_expandingHeightRatio-1)*0.5)+_expandingHeight*_zoomRatio;

					//expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
					//expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
					//expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
					//expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;
					if ((_originPoint.x()-_terminalPoint.x())*(_originPoint.y()-_terminalPoint.y())<0)
					{
						int y;
						y = _originPoint.y();
						_originPoint.ry() = _terminalPoint.y();
						_terminalPoint.ry() = y;

					}

					if (_originPoint.x()<_terminalPoint.x())
					{
						expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;
					}

					else
					{
						expandingTopLeft.rx() = _terminalPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _terminalPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _originPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _originPoint.y()+expandingHeight;
					}

					_pExpandingRubberBand->setGeometry(QRect(expandingTopLeft, expandingTerminalPoint).normalized());
					_pExpandingRubberBand->show();

					_pRubberBand->setGeometry(QRect(_originPoint, _terminalPoint).normalized());
					_pRubberBand->show();

				}

				emit rubberBandOnSelection();

				break;	
			}
		case Qt::Key_Left:
			{
				_pCenterRubberBand->hide();
				_pRubberBand->hide();
				_pExpandingRubberBand->hide();

				e->accept();
				//printf("Left!");


				_originPoint.rx() = _originPoint.x()-1;
				_terminalPoint.rx() = _terminalPoint.x()-1;

				if (_isFromCenter)
				{

					QPoint center;
					center.rx() = _originPoint.x() + (_terminalPoint.x()-_originPoint.x())*0.5;
					center.ry() = _originPoint.y() + (_terminalPoint.y()-_originPoint.y())*0.5;

					//add
					QPoint expandingTopLeft;
					QPoint expandingTerminalPoint;

					int expandingWidth  = int(_pRubberBand->width()*(_expandingWidthRatio-1)*0.5)+_expandingWidth*_zoomRatio;
					int expandingHeight = int(_pRubberBand->height()*(_expandingHeightRatio-1)*0.5)+_expandingHeight*_zoomRatio;

					//expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
					//expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
					//expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
					//expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;
					if ((_originPoint.x()-_terminalPoint.x())*(_originPoint.y()-_terminalPoint.y())<0)
					{
						int y;
						y = _originPoint.y();
						_originPoint.ry() = _terminalPoint.y();
						_terminalPoint.ry() = y;
					}

					if (_originPoint.x()<_terminalPoint.x())
					{
						expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;
					}

					else
					{
						expandingTopLeft.rx() = _terminalPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _terminalPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _originPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _originPoint.y()+expandingHeight;
					}

					_pExpandingRubberBand->setGeometry(QRect(expandingTopLeft, expandingTerminalPoint).normalized());

					//_pExpandingRubberBand->setGeometry(QRect(_originPoint, _terminalPoint).normalized());
					_pExpandingRubberBand->show();

					_pRubberBand->setGeometry(QRect(_originPoint, _terminalPoint).normalized());
					_pRubberBand->show();

					_pCenterRubberBand->setGeometry(QRect(center-QPoint(3,3), center+QPoint(3,3)).normalized());
					_pCenterRubberBand->show();

				}

				else
				{
					//add
					QPoint expandingTopLeft;
					QPoint expandingTerminalPoint;

					int expandingWidth  = int(_pRubberBand->width()*(_expandingWidthRatio-1)*0.5)+_expandingWidth*_zoomRatio;
					int expandingHeight = int(_pRubberBand->height()*(_expandingHeightRatio-1)*0.5)+_expandingHeight*_zoomRatio;

					//expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
					//expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
					//expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
					//expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;

					if ((_originPoint.x()-_terminalPoint.x())*(_originPoint.y()-_terminalPoint.y())<0)
					{
						int y;
						y = _originPoint.y();
						_originPoint.ry() = _terminalPoint.y();
						_terminalPoint.ry() = y;

					}

					if (_originPoint.x()<_terminalPoint.x())
					{
						expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;
					}

					else
					{
						expandingTopLeft.rx() = _terminalPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _terminalPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _originPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _originPoint.y()+expandingHeight;
					}

					_pExpandingRubberBand->setGeometry(QRect(expandingTopLeft, expandingTerminalPoint).normalized());
					_pExpandingRubberBand->show();

					_pRubberBand->setGeometry(QRect(_originPoint, _terminalPoint).normalized());
					_pRubberBand->show();

				}

				emit rubberBandOnSelection();

				break;	
			}
		case Qt::Key_Right:
			{
				_pCenterRubberBand->hide();
				_pRubberBand->hide();
				_pExpandingRubberBand->hide();

				e->accept();
				//printf("Left!");


				_originPoint.rx() = _originPoint.x()+1;
				_terminalPoint.rx() = _terminalPoint.x()+1;

				if (_isFromCenter)
				{

					QPoint center;
					center.rx() = _originPoint.x() + (_terminalPoint.x()-_originPoint.x())*0.5;
					center.ry() = _originPoint.y() + (_terminalPoint.y()-_originPoint.y())*0.5;

					//add
					QPoint expandingTopLeft;
					QPoint expandingTerminalPoint;

					int expandingWidth  = int(_pRubberBand->width()*(_expandingWidthRatio-1)*0.5)+_expandingWidth*_zoomRatio;
					int expandingHeight = int(_pRubberBand->height()*(_expandingHeightRatio-1)*0.5)+_expandingHeight*_zoomRatio;

					//expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
					//expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
					//expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
					//expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;

					if ((_originPoint.x()-_terminalPoint.x())*(_originPoint.y()-_terminalPoint.y())<0)
					{
						int y;
						y = _originPoint.y();
						_originPoint.ry() = _terminalPoint.y();
						_terminalPoint.ry() = y;

					}

					if (_originPoint.x()<_terminalPoint.x())
					{
						expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;
					}

					else
					{
						expandingTopLeft.rx() = _terminalPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _terminalPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _originPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _originPoint.y()+expandingHeight;
					}

					_pExpandingRubberBand->setGeometry(QRect(expandingTopLeft, expandingTerminalPoint).normalized());

					//_pExpandingRubberBand->setGeometry(QRect(_originPoint, _terminalPoint).normalized());
					_pExpandingRubberBand->show();

					_pRubberBand->setGeometry(QRect(_originPoint, _terminalPoint).normalized());
					_pRubberBand->show();

					_pCenterRubberBand->setGeometry(QRect(center-QPoint(3,3), center+QPoint(3,3)).normalized());
					_pCenterRubberBand->show();

				}

				else
				{
					//add
					QPoint expandingTopLeft;
					QPoint expandingTerminalPoint;

					int expandingWidth  = int(_pRubberBand->width()*(_expandingWidthRatio-1)*0.5)+_expandingWidth*_zoomRatio;
					int expandingHeight = int(_pRubberBand->height()*(_expandingHeightRatio-1)*0.5)+_expandingHeight*_zoomRatio;

					//expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
					//expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
					//expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
					//expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;

					if ((_originPoint.x()-_terminalPoint.x())*(_originPoint.y()-_terminalPoint.y())<0)
					{
						int y;
						y = _originPoint.y();
						_originPoint.ry() = _terminalPoint.y();
						_terminalPoint.ry() = y;

					}

					if (_originPoint.x()<_terminalPoint.x())
					{
						expandingTopLeft.rx() = _originPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _originPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _terminalPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _terminalPoint.y()+expandingHeight;
					}

					else
					{
						expandingTopLeft.rx() = _terminalPoint.x()-expandingWidth;
						expandingTopLeft.ry() = _terminalPoint.y()-expandingHeight;
						expandingTerminalPoint.rx() = _originPoint.x()+expandingWidth;
						expandingTerminalPoint.ry() = _originPoint.y()+expandingHeight;
					}

					_pExpandingRubberBand->setGeometry(QRect(expandingTopLeft, expandingTerminalPoint).normalized());
					_pExpandingRubberBand->show();

					_pRubberBand->setGeometry(QRect(_originPoint, _terminalPoint).normalized());
					_pRubberBand->show();

				}

				emit rubberBandOnSelection();

				break;	
			}
		default:
			{
				e->ignore();
				return;
			}
 
		}

	}

}

QRect SelectionView::getSelectedRect()
{
	QRect selectedRect;
	//!* transfer the view coordinate system to the scene coordinate system
	//!* the coordinates of the rectangle is in the scene coordinate system
	//!* is the same with the position in the  pixmap shown in the GraphicsView
	QPointF pointF1 = mapToScene(_originPoint);
	QPointF pointF2 = mapToScene(_terminalPoint);


	QPoint pt1 = QPoint(qRound(pointF1.x()), qRound(pointF1.y()));
	QPoint pt2 = QPoint(qRound(pointF2.x()), qRound(pointF2.y()));


	if ((_originPoint.x()-_terminalPoint.x())*(_originPoint.y()-_terminalPoint.y())<0)
	{
		int y;
		y = pt1.y();
		pt1.ry() = pt2.y();
		pt2.ry() = y;
	}


	if ( pt1.x() < pt2.x() )		
		selectedRect = QRect(pt1, pt2);	
	else
		selectedRect = QRect(pt2, pt1);
		
	
	return selectedRect;
}

QRect SelectionView::getRealTimeSelectedRect()
{
	//QPoint originPoint;
	//QPoint terminalPoint;

	//if ( _snapRatio == -1.0)
	//{
	//	if (_isFromCenter)
	//	{
	//		originPoint.rx() = _terminalPoint.x() - int(_snapWidth*_zoomRatio/2+0.5);
	//		originPoint.ry() = _terminalPoint.x() - int(_snapHeight*_zoomRatio/2+0.5);
	//		terminalPoint.rx() = _terminalPoint.x() + int(_snapWidth*_zoomRatio/2+0.5);
	//		terminalPoint.ry() = _terminalPoint.y() + int(_snapHeight*_zoomRatio/2+0.5);
	//	}

	//}
	////!* specify ratio
	//else
	//{
	//	if (_isFromCenter)
	//	{
	//		originPoint = originPoint + originPoint - terminalPoint;  
	//	}
	//}

	QRect selectedRect;
	//!* transfer the view coordinate system to the scene coordinate system
	//!* the coordinates of the rectangle is in the scene coordinate system
	//!* is the same with the position in the  pixmap shown in the GraphicsView
	QPointF pointF1 = mapToScene(_originPoint-(_terminalPoint-_originPoint));
	QPointF pointF2 = mapToScene(_terminalPoint);

	QPoint pt1 = QPoint(qRound(pointF1.x()), qRound(pointF1.y()));
	QPoint pt2 = QPoint(qRound(pointF2.x()), qRound(pointF2.y()));

	if ((_originPoint.x()-_terminalPoint.x())*(_originPoint.y()-_terminalPoint.y())<0)
	{
		int y;
		y = pt1.y();
		pt1.ry() = pt2.y();
		pt2.ry() = y;
	}

	if ( pt1.x() < pt2.x() )
		selectedRect = QRect(pt1, pt2);	
	else
		selectedRect = QRect(pt2, pt1);
		

	return selectedRect;
}

QRubberBand *SelectionView::getRubberBand()
{
	return _pRubberBand;
}

QRubberBand *SelectionView::getExpandingRubberBand()
{
	return _pExpandingRubberBand;
}

CenterRubberBand* SelectionView::getCenterRubberBand()
{
	return _pCenterRubberBand;
}

QRubberBand *SelectionView::_pRubberBand          = NULL;
QRubberBand *SelectionView::_pExpandingRubberBand = NULL;
bool		 SelectionView::_isFromCenter         = false;
int		     SelectionView::_snapHeight           = 1;
int		     SelectionView::_snapWidth            = 1;
double       SelectionView::_snapRatio            = 0.0;
double       SelectionView::_zoomRatio            = 1.0;
int          SelectionView::_expandingHeight      = 10;
int          SelectionView::_expandingWidth       = 10;
double	     SelectionView::_expandingHeightRatio = 1.1;
double       SelectionView::_expandingWidthRatio  = 1.1;

void SelectionView::dealWithPixmap(QPixmap pm)
{
    if ( !scene() ) {
        qDebug() << "No scene!";
        return;
    }

    scene()->setSceneRect(0, 0, pm.width(), pm.height());
    scene()->clear();
    setPix( &pm );
    pixItem = scene()->addPixmap( pm );
    qDebug() << QString().sprintf( "pix: %p, pixItem: %p", getPix(), pixItem );
}
