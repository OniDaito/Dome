/**
* @brief Leeds Projector Window body
* @file projector_window.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 22/06/2012
*
*/

#include "projector_window.hpp"

/*
 * Projector Windows
 */
 
ProjectorWindow::ProjectorWindow(QWidget *parent ) :  QWidget(parent) {
	mFS = false;
	
	this->setWindowFlags(Qt::Window);
	
	QPalette Pal(palette());
	// set black background
	Pal.setColor(QPalette::Background, Qt::black);
	setAutoFillBackground(true);
	setPalette(Pal);
	show();
	
    mSize = 5;
	mPoint.setX(0);
	mPoint.setY(0);
}

void ProjectorWindow::paintEvent(QPaintEvent *event){
	QPainter painter(this);
	painter.setPen(QPen(Qt::white, mSize));
	painter.drawPoints(&mPoint,1);
}

void ProjectorWindow::advance() {
	
	if (mPoint.x() + mSize > width()) {
		mPoint.setX(0);
		mPoint.setY(mPoint.y() + mSize);
	}
	else {
		if (mPoint.y() + mSize > height()){
			mPoint.setX(0);
			mPoint.setY(0);
		}
		mPoint.setX(mPoint.x() + mSize);
	}
	update();
 }
 
void ProjectorWindow::setPos(int x, int y) {
}

/*
 * Set the flash here
 * \todo causes crashes when quitting annoyingly - segfault on destruction of state
 */

void ProjectorWindow::setFlash(bool b) {
	QPalette Pal(palette());
	if (b){
		Pal.setColor(QPalette::Background, Qt::white);
	}
	else{
		Pal.setColor(QPalette::Background, Qt::black);
	}
	
	setAutoFillBackground(true);
	setPalette(Pal);
	update();
}

ProjectorWindow::~ProjectorWindow(){ }

/*
 * Handle Full screen for the projector
 */

void ProjectorWindow::handleFullScreen(){
	if (mFS)
		showNormal();
	else
		showFullScreen();
		
	mFS = !mFS;
}

/*
 * Listen for the F key for fullscreen
 */

void ProjectorWindow::keyPressEvent ( QKeyEvent * event ) {
	if(event->key() == Qt::Key_F){
		handleFullScreen();
	}
}
