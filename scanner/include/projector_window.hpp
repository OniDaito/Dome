/**
* @brief Leeds Projector Window Header
* @file projector_window.h
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 22/06/2012
*
*/

#ifndef PROJECTOR_WINDOW_H
#define PROJECTOR_WINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include <QGridLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QContextMenuEvent>
#include <QDialog>
#include <QFileDialog>
 
/*
 * Projector Window for dealing with signals for the projector
 * Maybe make OpenGL based later
 */
 
class ProjectorWindow : public QWidget {
Q_OBJECT

public:
	explicit ProjectorWindow(QWidget *parent = 0);
	~ProjectorWindow();
	
	void setPos(int x, int y);
	void setFlash(bool b);
	void advance();
	
public slots:
	void handleFullScreen();

protected slots:
	void processing() {update();};
	
protected:
	void paintEvent(QPaintEvent *event);
	void keyPressEvent ( QKeyEvent * event );
	
	bool 			mFS; // is Fullscreen?
	int				mSize;
	QPoint			mPoint;

};


#endif
