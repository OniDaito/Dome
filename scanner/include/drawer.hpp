/**
* @brief Drawing Class
* @file drawer.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 26/04/2012
*
*/


#ifndef _DRAWER_HPP_
#define _DRAWER_HPP_

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <queue>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>

#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <boost/shared_ptr.hpp>

#define GLM_SWIZZLE 
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/swizzle.hpp>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "vbo.hpp"
#include "fbo.hpp"
#include "camera.hpp"
#include "primitive.hpp"

#include "camera_manager.hpp"
#include "utils.hpp"
#include "config.hpp"
#include "shader.hpp"


/*
 * Drawer class - Performs all drawing functions using OpenCV and GLM for its functionality
 * Loaders for shaders and VBO code is custom
 * We assume a graphics context has already been given to this class.
 * 
 * \todo could have one drawer per state technically? Different cameras and views allowed?
 * 
 */

class Drawer {
public:
	Drawer() {};
	virtual ~Drawer();
	void setup(GlobalConfig &config);
	void resize(size_t w, size_t h);
	void drawCamerasFlat(std::vector<boost::shared_ptr<LeedsCam> > &cams);
	void drawMesh(VBOData &mesh, std::vector<boost::shared_ptr<LeedsCam> > &cams);
	void drawMeshPoints(VBOData &points, float r, float g, float b);
	void drawResultFlat(size_t cami);
	void drawCameraQuad();
	void drawCameraNormal(VBOData &line);
	void drawNormals(VBOData &lines);
	void drawReferenceQuad();
	void drawZoomed();
	void drawToolView(VBOData &mesh,std::vector<boost::shared_ptr<LeedsCam> > &cams);
	void drawGripper();
	void rotateCamera(int dx, int dy, double dt);
	void zoomCamera(float_t z);
	void moveCamera(int dx, int dy, double dt);
	bool picked(size_t u, size_t v, size_t s, glm::vec3 col);
	void moveGripper(float_t du, float_t dv, float_t dt);

protected:

	void recursiveCreate (const struct aiScene *sc, const struct aiNode* nd);
	
	struct SharedObj {
		SharedObj(GlobalConfig &config) : mConfig(config) {};
		GlobalConfig &mConfig;
		size_t mW,mH;
		
		//Shaders
		Shader mShaderFlatCam;
		Shader mShaderMeshPoints;
		Shader mShaderQuad;
		Shader mShaderLighting;
		Shader mShaderNormals;
		Shader mShaderGripper;
		Shader mShaderPicker;
				
		glm::mat4 mModelMatrix;
		
		OrbitCamera mCam;
		Camera mGripperCam;
		
		struct aiLogStream mStream;
		
		FBO	mFBOPick;
		FBO	mFBOTool;
		FBO mFBOZoom;
		
		VBOData	mVCam;
		VBOData	mVQuad;
		Primitive mGripper;
	
		
	};
	
	
	boost::shared_ptr<SharedObj> mObj;
};

#endif
