/**
* @brief Leeds Main Window header
* @file mainwindow.h
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 11/06/2012
*
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "leeds.hpp"

#include <QMainWindow>
#include <QScrollArea>
#include <QGridLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QContextMenuEvent>
#include <QDialog>
#include <QFileDialog>
#include "ui_controlwindow.h"

 namespace Ui {
	class MainWindow;
}

/*
 * Main Window class for the master window rendering the 3D
 */

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
	void handleExit();
	
	void handleMenuScan();
	void handleMenuSave();
	void handleMenuLoad();
	void handleMenuShowCameras();
	void handleFullScreen();
	void handleMenuCalibrateCameras();
	void handleMenuCalibrateWorld();
	void handleMenuSTLLoad();
	void handleToolView() {pLeedsWidget->toggleToolView(); };
	
	void handleDrawMesh() { pLeedsWidget->toggleDrawMesh(); }
	void handleMenuDetected() { pLeedsWidget->toggleDetected(); };
	void handleMenuGenerateMesh() {pLeedsWidget->generateMesh(); };
	void handleMenuClearMesh() {pLeedsWidget->clearMesh(); };
	
	// Assuming the min/max and step are all set in the ui
	void handleBrightness(int value) { pLeedsWidget->brightness(value); } ;
	void handleContrast(int value) { pLeedsWidget->contrast(value); } ;
	void handleExposure(int value) { pLeedsWidget->exposure(value); } ;
	void handleSaturation(int value) { pLeedsWidget->saturation(value); } ;
	void handleGain(int value) { pLeedsWidget->gain(value); } ;
	void handleFocus(int value) { pLeedsWidget->focus(value); } ;
	void handleAutoFocus(int value) { pLeedsWidget->autofocus(value); } ;
	void handleAutoExposure(int value) { pLeedsWidget->autoexposure(value); } ;
	void handleSharpness(int value) { pLeedsWidget->sharpness(value); } ;
	
	
	// Handle OpenCV Tab
	
	void handleOpenCVThreshold(int value) {  pLeedsWidget->mConfig.pointThreshold = (static_cast<double_t>(value) / 100.0f); };
	void handleOpenCVSpeed(int value) {  pLeedsWidget->mConfig.scanInterval = (static_cast<double_t>(value) / 100.0f); };
	
protected:
	void createActions();
	void createMenus();
	void contextMenuEvent(QContextMenuEvent *event);
    void keyPressEvent ( QKeyEvent * event );
    void paintEvent(QPaintEvent *event);


    
private:

	QWidget 		*centralWidget;
	QScrollArea 	*glWidgetArea;    
    Ui::MainWindow	*ui;
    Ui::Controls	*mCW;
    
	Leeds 			*pLeedsWidget;
	
	// Menu
	QMenu 			*pFileMenu;
	QAction 		*pFileMenuScan;
	QAction 		*pFileMenuLoad;
	QAction 		*pFileMenuSave;
	QAction 		*pFileMenuExit;
	
	QMenu 			*pViewMenu;
	QAction 		*pViewMenuCameras;
	QTimer			mTimer;
	QLabel			*pFPSLabel;
	QLabel			*pStatusLabel;
	
	// Projector Window
	ProjectorWindow	*pProjector;
	
	bool 			mFS; //is fullscreen?
	
};

#endif
