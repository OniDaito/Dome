/**
* @brief Leeds main program body
* @file leeds.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 26/04/2012
*
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"


/*
 * Main Window Consstructor
 */

MainWindow::MainWindow(QWidget *parent ) :  QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    
    // Create Projection Window - almost handled by this class
	pProjector = new ProjectorWindow(this);
	pProjector->show();
	
       
    QGLFormat format;
	format.setVersion(4,2);
	
    pLeedsWidget = new Leeds(parent,pProjector,format);
    pLeedsWidget->setMouseTracking(true);
        
    setWindowTitle(tr("Leeds"));	
  
	centralWidget = new QWidget;
	setCentralWidget(centralWidget);
	glWidgetArea = new QScrollArea;
	glWidgetArea->setWidget(pLeedsWidget);
	glWidgetArea->setWidgetResizable(true);
	glWidgetArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	glWidgetArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	glWidgetArea->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	glWidgetArea->setMinimumSize(50, 50);
 
	QGridLayout *centralLayout = new QGridLayout;
	centralLayout->addWidget(glWidgetArea, 0, 0);
	centralWidget->setLayout(centralLayout);
  
	pStatusLabel = new QLabel();
	pStatusLabel->resize(550,35);
	statusBar()->addWidget(pStatusLabel);


	pFPSLabel = new QLabel();
	pFPSLabel->resize(150,35);
	statusBar()->addWidget(pFPSLabel);
	
	mTimer.setInterval(100);
    QObject::connect(&mTimer, SIGNAL(timeout()), this, SLOT(update()));
    mTimer.start(0);
    
    // Create controls window
    mCW = new Ui::Controls(this);
    mCW->show();
    
    createMenus();

}


void MainWindow::handleMenuScan() {
	pLeedsWidget->toggleScanning();
}

void MainWindow::handleMenuSave() {
	pLeedsWidget->save();
}

void MainWindow::handleMenuLoad() {
	
	pLeedsWidget->pause();
	
	
	QFileDialog dlg(this);
	dlg.setFileMode(QFileDialog::ExistingFile);
	int result = dlg.exec();
	QStringList l = dlg.selectedFiles();
	
	/*QString filename = QFileDialog::getOpenFileName(this,
                                tr("Load PCD File"), QDir::currentPath());

	*/
	if (l.size() > 0){
		QString filename = l.at(0);

		if (filename != "")
			pLeedsWidget->load(filename.toStdString());
		else
			pLeedsWidget->load();
	}
		
	pLeedsWidget->pause();
}

void MainWindow::handleMenuCalibrateCameras() {
	pLeedsWidget->calibrateCameras();
}

void MainWindow::handleMenuCalibrateWorld() {
	pLeedsWidget->calibrateWorld();
}


void MainWindow::handleMenuShowCameras(){
	pLeedsWidget->toggleShowCameras();
}

void MainWindow::handleMenuSTLLoad() {
	pLeedsWidget->pause();
	
	
	QFileDialog dlg(this);
	dlg.setFileMode(QFileDialog::ExistingFile);
	int result = dlg.exec();
	QStringList l = dlg.selectedFiles();
	
	/*QString filename = QFileDialog::getOpenFileName(this,
                                tr("Load PCD File"), QDir::currentPath());

	*/
	if (l.size() > 0){
		QString filename = l.at(0);

		if (filename != "")
			pLeedsWidget->loadFromSTL(filename.toStdString());
	}
		
	pLeedsWidget->pause();
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event) {
	QMenu menu(this);
	menu.exec(event->globalPos());
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::createMenus(){
	
	// All these buttons and actions are contained in the automatically generated header via moc
	
	connect(ui->actionScan_Mesh, SIGNAL(triggered()), this, SLOT(handleMenuScan()));
	connect(ui->actionLoad_Mesh, SIGNAL(triggered()), this, SLOT(handleMenuLoad()));
	connect(ui->actionSave_Mesh, SIGNAL(triggered()), this, SLOT(handleMenuSave()));
	connect(ui->actionLoad_from_STL, SIGNAL(triggered()), this, SLOT(handleMenuSTLLoad())); 
	
	connect(ui->actionToggle_Cameras, SIGNAL(triggered()), this, SLOT(handleMenuShowCameras()));
	connect(ui->actionToggle_Detected_Points, SIGNAL(triggered()), this, SLOT(handleMenuDetected()));
	connect(ui->actionToggle_Tool_View,  SIGNAL(triggered()), this, SLOT(handleToolView()));
	
	connect(ui->actionCalibrate_Cameras, SIGNAL(triggered()), this, SLOT(handleMenuCalibrateCameras()));
	connect(ui->actionCalibrate_World, SIGNAL(triggered()), this, SLOT(handleMenuCalibrateWorld()));
	
	connect(ui->actionGenerate_Mesh, SIGNAL(triggered()), this, SLOT(handleMenuGenerateMesh()));
	connect(ui->actionClear_Mesh, SIGNAL(triggered()), this, SLOT(handleMenuClearMesh()));
	connect(ui->actionDraw_FIlled_Mesh, SIGNAL(triggered()), this, SLOT(handleDrawMesh()));
	
	// Camera dials
	connect(mCW->dialExposure, SIGNAL(valueChanged(int)), this, SLOT(handleExposure(int)));
	connect(mCW->dialBrightness, SIGNAL(valueChanged(int)), this, SLOT(handleBrightness(int)));
	connect(mCW->dialGain, SIGNAL(valueChanged(int)), this, SLOT(handleGain(int)));
	connect(mCW->dialSaturation, SIGNAL(valueChanged(int)), this, SLOT(handleSaturation(int)));
	connect(mCW->dialFocus, SIGNAL(valueChanged(int)), this, SLOT(handleFocus(int)));
	connect(mCW->dialContrast, SIGNAL(valueChanged(int)), this, SLOT(handleContrast(int)));
    connect(mCW->dialAutoFocus, SIGNAL(valueChanged(int)), this, SLOT(handleAutoFocus(int)));
    connect(mCW->dialAutoExposure, SIGNAL(valueChanged(int)), this, SLOT(handleAutoExposure(int)));
    connect(mCW->dialSharpness, SIGNAL(valueChanged(int)), this, SLOT(handleSharpness(int)));
    
   	
	// OpenCV Sliders
	connect(mCW->sliderOpenCVThreshold, SIGNAL(valueChanged(int)), this, SLOT(handleOpenCVThreshold(int)));
	connect(mCW->sliderOpenCVSpeed, SIGNAL(valueChanged(int)), this, SLOT(handleOpenCVSpeed(int)));
	
	
}


void MainWindow::handleFullScreen(){
	if (mFS)
		showNormal();
	else
		showFullScreen();
	mFS = !mFS;
}

void MainWindow::paintEvent(QPaintEvent *event) {
	std::stringstream Num;
	std::string str;
	Num << pLeedsWidget->getFPS();
	str = Num.str();
	pFPSLabel->setText(tr(str.c_str()));
	
	pStatusLabel->setText(tr( pLeedsWidget->getStatus().c_str() ));
}


/*
 * Listen for the F key for fullscreen
 */

void MainWindow::keyPressEvent ( QKeyEvent * event ) {
	if(event->key() == Qt::Key_F){
		handleFullScreen();
	}
	else if(event->key() == Qt::Key_T){
		pLeedsWidget->toggleTexturing();
	}
}

void MainWindow::handleExit() {
	cout << "Leeds - shutting down" << endl;
	pLeedsWidget->stop();
}
