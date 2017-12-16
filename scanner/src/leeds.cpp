/**
* @brief Leeds main program body
* @file leeds.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 26/04/2012
*
*/

#include "leeds.hpp"

using namespace cv;
using namespace std;
using namespace boost;

namespace po = boost::program_options;
	

/*
 * Set a control for all the cameras
 */

void Leeds::setAllCameras(CameraControl c, unsigned int v){
	mManager.setControl(c,v);
}


/*
 * Update Function - Called as often as possible to update state - threaded - renamed to not clash with QT
 */

void Leeds::_update(){
	
	startClock(0);
		
	pInfo->mx = 0;
	pInfo->my = 0;
	pInfo->sr = false;
	
	double pt = 0;
	
	while (mGo){		
		// Here rather than in state though we could move this. 
		if (pInfo->dt > 0 && pInfo->ml)
			mD.rotateCamera(pInfo->dx,pInfo->dy,pInfo->dt);
		
		double dt = sampleClock(0);
		pInfo->dt = dt;
				
		if (qState.size() > 0){
			if (qState.back().mF){
				qState.pop_back();
			}
			else
				qState.back().update();
		}
				
		// update the list states
		for (state_iterator it = lState.begin(); it != lState.end(); ++it){
			it->update();
		}
		
		
		// Update the Projector Window
		if (pt >= mConfig.scanInterval){
			pt = 0;
			pProject->advance();
		} else {
			pt+= dt;
		}
	}
	
}


/*
 * Exit function. Perform tidying up
 */

void Leeds::stop(){
	mGo = false;
	
	
	vector<string> fn;
	
	TiXmlDocument doc( "./data/settings.xml" );
	bool loadOkay = doc.LoadFile();
	
	if (loadOkay){
		
		try{
			TiXmlElement *pRoot = doc.FirstChildElement( "leeds" );
			
			TiXmlElement *pP;
			if ( pRoot ) {
				// Deal with the Cameras
				
				TiXmlElement *pCameras= pRoot->FirstChildElement("cameras");
				
				// Grab all the cameras
				TiXmlElement *pCam = pCameras->FirstChildElement("cam");
				while (pCam) {
					pP = pCam->FirstChildElement("in"); 	
					fn.push_back(string(pP->GetText()));
					pCam = pCam->NextSiblingElement();
				}
			}
		}
		catch(...){
			cerr << "Leeds - Error in saving" << endl;
		}
	}
	cout << "Leeds - Saving Camera Settings" << endl;
	mManager.saveSettings(fn);
	mManager.shutdown();
					
}

/// Leeds main class

Leeds::Leeds(QWidget *parent,ProjectorWindow *pP, QGLFormat format) : QGLWidget(format, parent) {
	makeCurrent();
	pProject = pP;
	mPaused = false;
}

Leeds::~Leeds() {
}

void Leeds::parseXML(){
	TiXmlDocument doc( "./data/settings.xml" );
	bool loadOkay = doc.LoadFile();
	if (loadOkay){
		
		try{
		
			TiXmlElement *pRoot = doc.FirstChildElement( "leeds" );
			if ( pRoot ) {
				
				// Deal with the Cameras
				
				TiXmlElement *pCameras= pRoot->FirstChildElement("cameras");
				
				TiXmlElement *pP;
				// Grab basic camera details
				pP = pCameras->FirstChildElement("width"); mConfig.camSize.width = fromStringS9<int>(string(pP->GetText()));
				pP = pCameras->FirstChildElement("height"); mConfig.camSize.height = fromStringS9<int>(string(pP->GetText()));
				pP = pCameras->FirstChildElement("fps"); mConfig.fps = fromStringS9<int>(string(pP->GetText()));
				
				// Load the camera manager
				mManager.setup(mConfig);
				
				// Grab all the cameras
				TiXmlElement *pCam = pCameras->FirstChildElement("cam");
				while (pCam) {
					pP = pCam->FirstChildElement("dev");	string dev = string(pP->GetText());
					pP = pCam->FirstChildElement("in"); 	string in = string(pP->GetText());
					
					shared_ptr<LeedsCam> m0 = mManager.addCamera(dev,in);

					pCam = pCam->NextSiblingElement();
				}
				
				// Deal with the Chessboard
				TiXmlElement *pChess = pRoot->FirstChildElement("chess");
				
				pP = pChess->FirstChildElement("width"); mConfig.boardSize.width = fromStringS9<int>(string(pP->GetText()));
				pP = pChess->FirstChildElement("height"); mConfig.boardSize.height = fromStringS9<int>(string(pP->GetText()));
				//pP = pChess->FirstChildElement("size");
				pP = pChess->FirstChildElement("maximages"); mConfig.maxImages = fromStringS9<int>(string(pP->GetText()));
				pP = pChess->FirstChildElement("interval"); mConfig.interval = fromStringS9<float>(string(pP->GetText()));
				//pP = pChess->FirstChildElement("startpair"); 
				//pP = pChess->FirstChildElement("endpair");
				pP = pChess->FirstChildElement("startcam"); mConfig.startCam = fromStringS9<int>(string(pP->GetText()));
				pP = pChess->FirstChildElement("endcam"); mConfig.endCam = fromStringS9<int>(string(pP->GetText()));
				
				// Deal with World
				
				TiXmlElement *pWorld = pRoot->FirstChildElement("world");
				TiXmlElement *pSize = pWorld->FirstChildElement("size");
				
				pP = pSize->FirstChildElement("xs"); mConfig.xs = fromStringS9<float>(string(pP->GetText()));
				pP = pSize->FirstChildElement("ys"); mConfig.ys = fromStringS9<float>(string(pP->GetText()));
				pP = pSize->FirstChildElement("zs"); mConfig.zs = fromStringS9<float>(string(pP->GetText()));
				pP = pSize->FirstChildElement("xe"); mConfig.xe = fromStringS9<float>(string(pP->GetText()));
				pP = pSize->FirstChildElement("ye"); mConfig.ye = fromStringS9<float>(string(pP->GetText()));
				pP = pSize->FirstChildElement("ze"); mConfig.ze = fromStringS9<float>(string(pP->GetText()));
				
				TiXmlElement *pMesh = pWorld->FirstChildElement("mesh");
				pSize = pMesh->FirstChildElement("size");
				
				pP = pSize->FirstChildElement("x"); mConfig.meshResolution.x = fromStringS9<float>(string(pP->GetText()));
				pP = pSize->FirstChildElement("y"); mConfig.meshResolution.y = fromStringS9<float>(string(pP->GetText()));
				pP = pSize->FirstChildElement("z"); mConfig.meshResolution.z = fromStringS9<float>(string(pP->GetText()));
				
				// Deal with UVC
				
				TiXmlElement *pUVC = pRoot->FirstChildElement("uvc");
				
				pP = pUVC->FirstChildElement("brightness"); brightness(fromStringS9<int>(string(pP->GetText())));
				pP = pUVC->FirstChildElement("contrast"); contrast(fromStringS9<int>(string(pP->GetText())));
				pP = pUVC->FirstChildElement("saturation"); saturation(fromStringS9<int>(string(pP->GetText())));
				pP = pUVC->FirstChildElement("focus"); focus(fromStringS9<int>(string(pP->GetText())));
				pP = pUVC->FirstChildElement("exposure"); exposure(fromStringS9<int>(string(pP->GetText())));
				pP = pUVC->FirstChildElement("gain"); gain(fromStringS9<int>(string(pP->GetText())));
				pP = pUVC->FirstChildElement("sharpness"); sharpness(fromStringS9<int>(string(pP->GetText())));
				
				// Deal with Poisson
				TiXmlElement *pPoisson = pRoot->FirstChildElement("poisson");
				pP = pPoisson->FirstChildElement("depth"); mConfig.poissonDepth = fromStringS9<int>(string(pP->GetText()));
				pP = pPoisson->FirstChildElement("samples"); mConfig.poissonSamples = fromStringS9<float>(string(pP->GetText()));
				pP = pPoisson->FirstChildElement("scale"); mConfig.poissonScale = fromStringS9<float>(string(pP->GetText()));
				
				
				// Deal with PCL
				TiXmlElement *pPCL = pRoot->FirstChildElement("pcl");
				pP = pPCL->FirstChildElement("filterk"); mConfig.pclFilterMeanK = fromStringS9<float>(string(pP->GetText()));
				pP = pPCL->FirstChildElement("filterthresh"); mConfig.pclFilterThresh = fromStringS9<float>(string(pP->GetText()));
				pP = pPCL->FirstChildElement("searchradius"); mConfig.pclSearchRadius = fromStringS9<float>(string(pP->GetText()));
				pP = pPCL->FirstChildElement("polynomial"); mConfig.pclPolynomialOrder = fromStringS9<float>(string(pP->GetText()));
				pP = pPCL->FirstChildElement("sampleradius"); mConfig.pclUpsamplingRadius = fromStringS9<float>(string(pP->GetText()));
				pP = pPCL->FirstChildElement("stepsize"); mConfig.pclUpsamplingStepSize = fromStringS9<float>(string(pP->GetText()));
				
				
				// Deal with OpenCV
				TiXmlElement *pOpenCV = pRoot->FirstChildElement("opencv");
				pP = pOpenCV->FirstChildElement("threshold"); mConfig.pointThreshold = fromStringS9<float>(string(pP->GetText()));
				
				
			
			}
		}
		catch(...){
			cerr << "Leeds - Failed to load XML - quitting" << endl;
			stop();
			exit(0);
		}
		
	}
	else{
		cerr << "Leeds - Failed to load ./data/settings.xml" << endl;
		stop();
		exit(0);
	}
}

void Leeds::setup(){
	
	mConfig.scanInterval = 0.2;
	
	// Fire up the drawer				
	mD.setup(mConfig);	
		
	// Mesh starting
	mM.setup(mConfig);

	pInfo = shared_ptr<StateInfo>(new StateInfo(mManager,mD,mM, *pProject));

	// Configure States
	StackState<BaseState> l(qState,pInfo);
	l();
		
	// Launch update thread
	mGo = true;
	pUpdateThread =  new boost::thread(&Leeds::_update, this);

}

void Leeds::initializeGL(){
	glewExperimental = TRUE;
	GLenum err=glewInit();
	
	if(err!=GLEW_OK) {
		//Problem: glewInit failed, something is seriously wrong.
		cout << "Leeds - GLEWInit failed, aborting." << endl;
	}
	
	cout << glGetString(GL_VERSION) << endl;
	
	parseXML();
	setup();
	
	mFPS = 0;
	mPicked = false;

	mTimer.setInterval(0);
    QObject::connect(&mTimer, SIGNAL(timeout()), this, SLOT(processing()));
    mTimer.start(0);
    
    startClock(1);
}

/*
 * Callback to update the OpenGL context BUT only once other events have been processed
 */

void Leeds::processing(){
    updateGL();
    
    QPoint p = mapFromGlobal(QCursor::pos());
    pInfo->dx = p.x() >=0 ? p.x() - pInfo->mx : pInfo->dx;
	pInfo->dy = p.y() >=0 ? p.y() - pInfo->my : pInfo->dy;
	pInfo->mx = p.x() >=0 ? p.x() : pInfo->mx;
	pInfo->my = p.y() >=0 ? p.y() : pInfo->my;

}

/*
 * Mouse move tracking
 */
 
void Leeds::mouseMoveEvent ( QMouseEvent * event ){
	
	// Mouse and timing data for the states
	pInfo->ml = event->buttons() & Qt::LeftButton;
	pInfo->mr = event->buttons() & Qt::RightButton;
	pInfo->mm = event->buttons() & Qt::MidButton;

		
	if (pInfo->dt > 0 && pInfo->ml){
		
		if (mPicked) {
			mD.moveGripper(-pInfo->dx, -pInfo->dy, pInfo->dt);
		} else{
			mD.rotateCamera(pInfo->dx,pInfo->dy,pInfo->dt);
		}
		
		if (mD.picked(pInfo->mx, mH - pInfo->my, 12, glm::vec3(1.0,0.0,0.0) )){
			mPicked = true;
			
		} else {
			mPicked = false;
			
		}
	}
	
	if (pInfo->dt > 0 && pInfo->mm){
		// move look and camera pos	
		mD.moveCamera(pInfo->dx,pInfo->dy,pInfo->dt);
	}

}

/*
 * Keypress event
 */
 
void Leeds::keyPressEvent ( QKeyEvent * event ){
}

/*
 * Resize function
 */

void Leeds::resizeGL(int w, int h){
	mW = w;
	mH = h;
	glViewport( 0, 0, w, h);
	
	if (qState.size() > 0){
		//glViewport causes crash
		mD.resize(w,h);
	}
}

/*
 * Painting the GL thing
 */

void Leeds::paintGL() {

	double dt = sampleClock(1);

	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.9f, 0.9f, 0.9f, 1.0f)[0]);
	GLfloat depth = 1.0f;
	glClearBufferfv(GL_DEPTH, 0, &depth );
	
	mManager.update(); // Placed here as it has opengl texture calls
	if (qState.size() > 0)
		qState.back().draw();
	
	// Draw the list states
	for (state_iterator it = lState.begin(); it != lState.end(); ++it){
		it->draw();
	}
	mFPS = 1.0 / dt;	
}

/*
 * Actions for states - Toggling the view Cameras
 */
 
void Leeds::toggleShowCameras() {
	ListState<StateDisplayCameras> s(lState,pInfo);
	if ( hasState(lState, s.mID) == lState.end())
		s();
	else
		s.remove();
}

/*
 * Toggle the scanning state
 */

void Leeds::toggleScanning() {
	StackState<StateScan> s(qState,pInfo);
	if (!s.remove())
		s();
}

/*
 * Toggle Texturing State
 */
 
void Leeds::toggleTexturing(){
	
	StackState<StateTexturing> s(qState,pInfo);
	if (!s.remove())
		s();

}

/*
 * Fire up the calibrate world
 */
 
 void Leeds::calibrateWorld() {
	StackState<StateCalibrateWorld> s(qState,pInfo);
	if (!s.remove())
		s();
 }
 
 /*
  * Toggle the tool view
  */
 
void Leeds::toggleToolView() {
	ListState<StateToolView> s(lState,pInfo);
	if ( hasState(lState, s.mID) == lState.end())
		s();
	else
		s.remove();
}
 
 
 /*
  * Fire up the calibrate Cameras
  */
 
 void Leeds::calibrateCameras() {
	StackState<StateCalibrateCamera> s(qState,pInfo);
	if (!s.remove())
		s();
 }
 
 /*
  * Toggle the detected points
  */
  
 void Leeds::toggleDetected(){
	if (qState.back().mID == "StateScan"){
		pInfo->sr = !pInfo->sr;
	}
 }
 
 /*
  * Save Meshes to file
  */
  
void Leeds::save() {
	mM.generate(mManager.getCams());
	mM.saveToFile("./data/test.pcd");
	mM.saveMeshToFile("./data/test.stl");
}

void Leeds::load(std::string filename) {
	 mM.loadFile(filename);
}


void Leeds::wheelEvent(QWheelEvent *event) {
	int numDegrees = event->delta();
	pInfo->d.zoomCamera(numDegrees);
	event->accept();
 }


/*
 * Clear the Mesh
 */
 
void Leeds::clearMesh() {mM.clearMesh(); }

/*
 * Generate Mesh
 */

void Leeds::generateMesh() { mM.generate(pInfo->c.getCams()); }

/*
 * Toggle drawing the mesh filled in
 */
 
void Leeds::toggleDrawMesh() { 
ListState<StateDrawMesh> s(lState,pInfo);
	if ( hasState(lState, s.mID) == lState.end())
		s();
	else
		s.remove();
}
