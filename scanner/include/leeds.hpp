/**
* @brief Main Program based header
* @file leeds.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 26/04/2012
*
*/

// http://opencv.itseez.com/modules/gpu/doc/camera_calibration_and_3d_reconstruction.html
// http://stackoverflow.com/questions/5987285/what-is-an-acceptable-return-value-from-cvcalibratecamera


#ifndef _LEEDS_HPP_
#define _LEEDS_HPP_

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>
#include <QTimer>
#include <QMouseEvent>
#include <QLabel>
#include <QWheelEvent>

#include "tinyxml.h"

#include <stdint.h> 
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/ptr_container/ptr_deque.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/shared_ptr.hpp>

#include <opencv/highgui.h>
#include <gpu/gpu.hpp>
#include "opencv2/calib3d/calib3d.hpp"

#include "camera_manager.hpp"
#include "utils.hpp"
#include "projector_window.hpp"
#include "config.hpp"
#include "drawer.hpp"
#include "states.hpp"

#undef Success //Needed for PCL stuff I believe?

#include "mesh.hpp"

#include <QtOpenGL/QGLWidget>




/*
 * Small class to hold a reference between name and function
 */
 
class CameraControlRef {
public:
	CameraControlRef(std::string n, boost::function<void (int x)> f ){ name = n; callBack = f; };
	
	boost::function<void (unsigned int x)> callBack;
	std::string name;
};


/*
 * Main class that parses input and loads the other classes for
 * calibration and similar. Exports to the UI our functionality
 */
 

class Leeds : public QGLWidget {
	
Q_OBJECT

public:
	Leeds(QWidget *parent, ProjectorWindow *, QGLFormat format);
	~Leeds();
	
	void setup();
	void draw();
	
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();
	void mousePressEvent(QMouseEvent *) {}
	void mouseReleaseEvent(QMouseEvent *) {}

	
	void parseXML();
	void stop();
	
	void pause() { mPaused = !mPaused; if (mPaused) mTimer.stop(); else mTimer.start(); } 
	
	float getFPS() {return mFPS;};
	
	// Toggleable states for QT and other UI hooks
	void toggleShowCameras();
	void toggleScanning();
	void toggleDetected();
	void toggleDrawMesh();
	void toggleTexturing();
	void toggleToolView();
	
	void clearMesh();
	void generateMesh();
	void calibrateCameras();
	void calibrateWorld();
	std::string getStatus() { if (pInfo) return pInfo->getStatus(); return "Leeds - Nonestate"; } 
	void save();
	void load(std::string filename="./data/test.pcd");
	void loadFromSTL(std::string filename) {mM.loadFromSTL(filename, mManager.getCams()); };


	// Small Callbacks for UVC Cameras - exposed for UI
	void brightness(int value){setAllCameras(BRIGHTNESS,(unsigned int) value); };
	void contrast(int value){ setAllCameras(CONTRAST,(unsigned int) value); };
	void saturation(int value){ setAllCameras(SATURATION,(unsigned int) value); };
	void gain(int value){ setAllCameras(GAIN,(unsigned int) value); };
	void sharpness(int value){ setAllCameras(SHARPNESS,(unsigned int) value); };
	void autoexposure(int value){ setAllCameras(AUTO_EXPOSURE,(unsigned int) value); };
	void exposure(int value){ setAllCameras(EXPOSURE,(unsigned int) value); };
	void focus(int value){ setAllCameras(FOCUS,(unsigned int) value); };
	void autofocus(int value){ setAllCameras(AUTO_FOCUS,(unsigned int) value); };
	
	GlobalConfig mConfig;
	
protected:

	void _update(); 
	void wheelEvent(QWheelEvent *event);
	void mouseMoveEvent (QMouseEvent *event );
	void keyPressEvent ( QKeyEvent * event );

	SharedInfo pInfo;	// Holds the state which we pass to the running states

	// State Control
	// Annoyingly we can't have copy/value semantics here - STL copies only the base class portion
	
	LeedsQueue qState;
	LeedsList lState;	

	// Clock timing functions
	void startClock(int idx) { gettimeofday (&start[idx],NULL);} ;
	double sampleClock(int idx) { 
			gettimeofday(&end[idx],NULL); 
			double ds = (start[idx].tv_sec * 1000.0) + (start[idx].tv_usec * 0.001);
			double de = (end[idx].tv_sec * 1000.0) + (end[idx].tv_usec * 0.001);
			double d = de - ds; 
			gettimeofday(&start[idx],NULL);
			return d * 0.001; 
		};


	// Controls all the camera functions and basic calibration features
	CameraManager mManager;
	
	// GUI
	std::vector<CameraControlRef> vCamControls;
	bool mDG;
	bool mPaused;
	
	// Drawer - does all the drawing
	Drawer mD;
	
	// Mesh class that does a lot of hard work with the points
	LeedsMesh mM;
	
	QTimer mTimer;
	
	// Mouse state - mainly for picking
	bool mPicked;
	   
    // Main Loop Functions
    bool mGo;
    boost::thread *pUpdateThread;
    
    // Label for FPS
    QLabel *pFPSLabel;
    
    // The window we use to project our scanner
    ProjectorWindow* pProject;
	
	// call to set all the connected cameras
	void setAllCameras(CameraControl c, unsigned int v);

	timeval start[5],end[5];
	
	size_t mH, mW;
	
	float mFPS;
	
protected slots:
	void processing();

};

#endif
