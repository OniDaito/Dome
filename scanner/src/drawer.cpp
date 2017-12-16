/**
* @brief Drawer class that deals with drawing EVERYTHING
* @file drawer.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 28/05/2012
*
*/

#include "drawer.hpp"

using namespace std;
using namespace boost;
using namespace cv;
using namespace boost::assign; 


/*
 * Basic setup function for shared objects
 */

 void Drawer::setup(GlobalConfig &config) {
	mObj.reset(new SharedObj(config));
	// Load Shaders
	mObj->mShaderFlatCam.load("./data/flatcam.vert", "./data/flatcam.frag");
	mObj->mShaderMeshPoints.load("./data/meshpoint.vert", "./data/meshpoint.frag");
	mObj->mShaderQuad.load("./data/quad.vert", "./data/quad.frag");
	mObj->mShaderLighting.load("./data/lighting.vert", "./data/lighting.frag");
	mObj->mShaderNormals.load("./data/meshnormal.vert", "./data/meshnormal.frag");
	mObj->mShaderGripper.load("./data/gripper.vert", "./data/gripper.frag");
	mObj->mShaderPicker.load("./data/picker.vert", "./data/picker.frag");
	
	mObj->mFBOPick.setup(640,480);
	mObj->mFBOTool.setup(640,480);
	mObj->mFBOZoom.setup(640,480);
	
	// Setup VBO objects
	// Quad for Camera drawing
	mObj->mVCam.mIndices += 0,3,1,3,2,1;
	mObj->mVCam.mVertices += 0.0f,0.0f,0.0f,
		(float_t)config.camSize.width, 0.0f,0.0f, 
		(float_t)config.camSize.width, (float_t)config.camSize.height,0.0f,
		0.0f, (float_t)config.camSize.height,0.0f;
	
	mObj->mVCam.mTexCoords += 0.0, (float_t)config.camSize.height,
		(float_t)config.camSize.width, (float_t)config.camSize.height,
		(float_t)config.camSize.width, 0.0,
		0.0,0.0;
		
	mObj->mVCam.compile(VBO_VERT | VBO_IDCE | VBO_TEXC );
	
	//Quad for reference. Square that is 1 x 1 and can then be scaled
	mObj->mVQuad.mIndices += 0,3,1,1,2,3;
	mObj->mVQuad.mVertices += 0.0f,0.0f,0.0f,
		(float_t)1.0f, 0.0f,0.0f, 
		(float_t)1.0f, (float_t)1.0f,0.0f,
		0.0f, (float_t)1.0f,0.0f;
	
	mObj->mVQuad.mColours += 0.0f,0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,1.0f,
		0.0f,1.0f,0.0f,1.0f,
		1.0f,0.0f,0.0f,1.0f;
		
	mObj->mVQuad.compile(VBO_VERT | VBO_IDCE | VBO_COLR );
	
	// Setup ASSIMP to load models
	
	mObj->mStream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT,NULL);
	aiAttachLogStream(&mObj->mStream);
	
	mObj->mGripper.generate("./data/gripper.stl", glm::vec3(0.2,0.2,0.2) );
	mObj->mGripper.setPos(glm::vec3(0,0,0));
	mObj->mGripper.setLook(glm::vec3(0,-1,0));
	mObj->mGripper.move(glm::vec3(10.0,10.0,10.0));

	// Model Transform for now - Scales to the screen
	
	mObj->mCam.move( glm::vec3(0, 0.0, 200.0) );

}

void Drawer::resize(size_t w, size_t h){
	mObj->mW = w;
	mObj->mH = h;
	
	mObj->mFBOPick.resize(w,h);
	mObj->mFBOTool.resize(w,h);
	mObj->mFBOZoom.resize(w,h);
	
	mObj->mCam.setRatio( static_cast<float_t>(mObj->mW) / mObj->mH);
	
	float_t sf = mObj->mW / (mObj->mConfig.xe - mObj->mConfig.xs);
	mObj->mModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(sf,sf,sf));
}

/*
 * Drawer Destructor
 */
 
Drawer::~Drawer() {
	aiDetachAllLogStreams();
}


void Primitive::recursiveCreate (const struct aiScene *sc, const struct aiNode* nd) {
	unsigned int i;
	unsigned int n = 0, t;

	// draw all meshes assigned to this node
	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = pScene->mMeshes[nd->mMeshes[n]];

		for (t = 0; t < mesh->mNumFaces; ++t) {
			const struct aiFace* face = &mesh->mFaces[t];
			
			for(i = 0; i < face->mNumIndices; i++) {
				int index = face->mIndices[i];
				
			//	if(mesh->mColors[0] != NULL)
			//		glColor4fv((GLfloat*)&mesh->mColors[0][index]);
				
				if(mesh->mNormals != NULL) {
					mVBO.mNormals.push_back(mesh->mNormals[index].x);
					mVBO.mNormals.push_back(mesh->mNormals[index].y);
					mVBO.mNormals.push_back(mesh->mNormals[index].z);
				}
						
				mVBO.mVertices.push_back(mesh->mVertices[index].x);
				mVBO.mVertices.push_back(mesh->mVertices[index].y);
				mVBO.mVertices.push_back(mesh->mVertices[index].z);
			}
		
		}
		
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		recursiveCreate(sc, nd->mChildren[n]);
	}

}


/*
 * Draw Camera Flat - Set matrices to ortho and bind basic texture shader
 */
		 
void Drawer::drawCamerasFlat(std::vector<boost::shared_ptr<LeedsCam> > &cams) {
		
	// Assume nothing. Set the matrices
	size_t cami = 0;
	size_t camx = 0;
	float_t space = 20.0f;
	float_t x = space;
	float_t y = space;
	float_t sf = 0.5;

	glEnable(GL_TEXTURE_RECTANGLE);
	//glDisable(GL_CULL_FACE);

	mObj->mVCam.bind();
	BOOST_FOREACH (boost::shared_ptr<LeedsCam> cam, cams) {
	
		mObj->mShaderFlatCam.begin();
		glm::mat4 Projection = glm::ortho(0.0f,(float_t)mObj->mW,(float_t)mObj->mH,0.0f);
		glm::mat4 Model = glm::scale(glm::mat4(1.0f),glm::vec3(sf,sf,1.0)) * glm::translate(glm::mat4(1.0f), glm::vec3(x,y,0.0));
		glm::mat4 MVP = Projection * Model;
		
		GLint LocationMVP = glGetUniformLocation(mObj->mShaderFlatCam.getProgram(), "mMVPMatrix");
		glUniformMatrix4fv(	LocationMVP, 1, GL_FALSE, glm::value_ptr(MVP)); 
		
		cam->bindRectified();
		GLint LocationTex = glGetUniformLocation(mObj->mShaderFlatCam.getProgram(), "mBaseTex");
		glUniform1i(LocationTex,0);
		
		glDrawElements(GL_TRIANGLES, mObj->mVCam.mNumIndices, GL_UNSIGNED_INT, 0);
	
		mObj->mShaderFlatCam.end();
		
		// go by row and column via idx
		if ( x + space + (mObj->mConfig.camSize.width * sf)  > mObj->mW){
			camx = 0;
			x = space;
			y += mObj->mConfig.camSize.height + space;
		}
		else {
			x += space + mObj->mConfig.camSize.width;
		}
		
		cami++;
		camx++;
	}
	
	mObj->mVCam.unbind();
	
	glDisable(GL_TEXTURE_RECTANGLE);
	
}


/*
 * Draw One Camera Flat on a grid
 */
 
void Drawer::drawResultFlat(size_t cami = 0) {
	
	float_t space = 20.0f;
	float_t x = space;
	float_t y = space;
	float_t sf = 0.5;
	
	int cols = mObj->mW / (space + mObj->mConfig.camSize.width * sf);  
	
	int c = cami / cols;
	int r = cami % cols;
	
	x = space + c * (space + mObj->mConfig.camSize.width);
	y = space + r * (space + mObj->mConfig.camSize.height);
	
	glEnable(GL_TEXTURE_RECTANGLE);
	glDisable(GL_CULL_FACE);

	
	mObj->mVCam.bind();
	mObj->mShaderFlatCam.begin();
	glm::mat4 Projection = glm::ortho(0.0f,(float_t)mObj->mW,(float_t)mObj->mH,0.0f);
	glm::mat4 Model = glm::scale(glm::mat4(1.0f),glm::vec3(sf,sf,1.0)) * glm::translate(glm::mat4(1.0f), glm::vec3(x,y,0.0));
	glm::mat4 MVP = Projection * Model;
	
	GLint LocationMVP = glGetUniformLocation(mObj->mShaderFlatCam.getProgram(), "mMVPMatrix");
	glUniformMatrix4fv(	LocationMVP, 1, GL_FALSE, glm::value_ptr(MVP)); 
	
	GLint LocationTex = glGetUniformLocation(mObj->mShaderFlatCam.getProgram(), "mBaseTex");
	glUniform1i(LocationTex,0);
	
	glDrawElements(GL_TRIANGLES, mObj->mVCam.mNumIndices, GL_UNSIGNED_INT, 0);

	mObj->mShaderFlatCam.end();
				
	mObj->mVCam.unbind();
	
	glDisable(GL_TEXTURE_RECTANGLE);
	
}

/*
 * Test if the cursor picked a colour -  test against our FBO
 */
 
bool Drawer::picked(size_t u, size_t v, size_t s, glm::vec3 col){
	mObj->mFBOPick.bind();
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	checkError(__LINE__);
	int ds = s*s * 4;
	GLfloat *data = (GLfloat*) new GLfloat[ds];
	glReadPixels(u - s/2, v - s/2, s, s, GL_RGBA, GL_FLOAT, data);
	checkError(__LINE__);
	bool b = false;
	for (int i = 0; i < ds; i+=4){
		glm::vec3 c( data[i], data[i+1], data[i+2]);
		if (c == col){
			b = true;
			break;
		}
	}
	
	checkError(__LINE__);
	mObj->mFBOPick.unbind();
	delete data;
	return b;

}


void Drawer::moveGripper(float_t du, float_t dv, float_t dt) {
	
	glm::vec3 dir = mObj->mCam.getPos() - mObj->mCam.getLook();
	dir = glm::normalize(dir);
	glm::vec3 shiftx = glm::cross(dir,mObj->mCam.getUp());
	shiftx *= (du * dt * 20000.0f);
	glm::vec3 shifty = mObj->mCam.getUp() * (dv * dt * 20000.0f);

	glm::vec3 dpos = shiftx + shifty;
	mObj->mGripper.move(dpos);
		
	
}


/*
 * Draw a gripper tool
 */
 
void Drawer::drawGripper() {
	
	glm::mat4 MVP = mObj->mCam.getProjMatrix() * mObj->mCam.getViewMatrix() * mObj->mModelMatrix * mObj->mGripper.getMatrix();
	glm::mat4 MV = mObj->mCam.getProjMatrix() * mObj->mCam.getProjMatrix() * mObj->mGripper.getMatrix();
	glm::mat4 MN = glm::inverseTranspose(mObj->mCam.getViewMatrix());
	glm::mat4 MI = glm::inverse(mObj->mCam.getViewMatrix());

	// render to the picker
	
	mObj->mFBOPick.bind();
	mObj->mGripper.bind();
	mObj->mShaderPicker.begin();
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);
	GLfloat depth = 1.0f;
	glClearBufferfv(GL_DEPTH, 0, &depth );
	GLint location = glGetUniformLocation(mObj->mShaderPicker.getProgram(), "uMVPMatrix");
	glUniformMatrix4fv(	location, 1, GL_FALSE, glm::value_ptr(MVP));
	location = glGetUniformLocation(mObj->mShaderPicker.getProgram(), "uColour");
	glUniform4f( location, 1.0f,0.0f,0.0f,1.0f); 
	checkError(__LINE__);
	glDrawArrays(GL_TRIANGLES, 0, mObj->mGripper.getNumElements());
	checkError(__LINE__);
	
	mObj->mShaderPicker.end();
	mObj->mGripper.unbind();
	mObj->mFBOPick.unbind();

	// Render to framebuffer as normal

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	mObj->mGripper.bind();
	mObj->mShaderGripper.begin();
	checkError(__LINE__);

	location = glGetUniformLocation(mObj->mShaderGripper.getProgram(), "uMVPMatrix");
	glUniformMatrix4fv(	location, 1, GL_FALSE, glm::value_ptr(MVP)); 
	
	location = glGetUniformLocation(mObj->mShaderGripper.getProgram(), "uMVMatrix");
	glUniformMatrix4fv(	location, 1, GL_FALSE, glm::value_ptr(MV)); 
	
	location = glGetUniformLocation(mObj->mShaderGripper.getProgram(), "uNormalMatrix");
	glUniformMatrix4fv(	location, 1, GL_FALSE, glm::value_ptr(MN)); 
	
	location = glGetUniformLocation(mObj->mShaderGripper.getProgram(), "uMInverseMatrix");
	glUniformMatrix4fv(	location, 1, GL_FALSE, glm::value_ptr(MI)); 

	location = glGetUniformLocation(mObj->mShaderGripper.getProgram(), "uLight0");
	glUniform3f( location, 10.0f,5.0f,20.0f); 
	
	location = glGetUniformLocation(mObj->mShaderGripper.getProgram(), "uTangent");
	glUniform3f( location, 0.0f,0.0f,1.0f); 

	location = glGetUniformLocation(mObj->mShaderGripper.getProgram(), "uCamPos");
	glm::vec3 p = mObj->mCam.getPos();
	glUniform3f( location, p.x,p.y,p.z );
	checkError(__LINE__);

	// We ARE NOT USING DRAW ELEMENTS as we need to do per face operations so we duplicate vertices
	glDrawArrays(GL_TRIANGLES, 0, mObj->mGripper.getNumElements());

	mObj->mShaderGripper.end();
	mObj->mGripper.unbind();
	checkError(__LINE__);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

}


/*
 * Update the internal camera from mouse
 */

static float_t sSense = 0.0000000001;
 
void Drawer::rotateCamera(int dx, int dy, double dt) {

	if (dt <= 0)
		return;

	mObj->mCam.yaw( static_cast<float_t>(dx) * (sSense/static_cast<float_t>(dt)));
	mObj->mCam.pitch(static_cast<float_t>(dy) * (sSense/static_cast<float_t>(dt)));
	
}

/*
 * Zoom the camera
 */
 
void Drawer::zoomCamera(float_t z) {
	mObj->mCam.zoom(z);
}


/*
 * Shift the camera
 */

void Drawer::moveCamera(int dx, int dy, double dt){
	mObj->mCam.shift(static_cast<float_t>(dx), static_cast<float_t>(dy));

}

/*
 * Draw Mesh Filled in
 */
 
 void Drawer::drawMesh(VBOData &mesh, std::vector<boost::shared_ptr<LeedsCam> > &cams) {
	glm::mat4 MVP = mObj->mCam.getProjMatrix() * mObj->mCam.getViewMatrix()  * mObj->mModelMatrix;
	glm::mat4 MV = mObj->mCam.getViewMatrix() * mObj->mModelMatrix;
	glm::mat4 MN = glm::inverseTranspose(mObj->mCam.getViewMatrix());
	
	glEnable(GL_TEXTURE_RECTANGLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glEnable(GL_DEPTH_TEST);
	
	// Enable Textures
	for (int i=0; i < cams.size(); i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_RECTANGLE, cams[i]->getTexture());
	}
	checkError(__LINE__);

	// Draw the Points - maybe add a slight blur or alpha sprite
	mesh.bind();
	

	mObj->mShaderLighting.begin();
		
	GLint location = glGetUniformLocation(mObj->mShaderLighting.getProgram(), "mMVPMatrix");
	glUniformMatrix4fv(	location, 1, GL_FALSE, glm::value_ptr(MVP)); 
	
	location = glGetUniformLocation(mObj->mShaderLighting.getProgram(), "mMVMatrix");
	glUniformMatrix4fv(	location, 1, GL_FALSE, glm::value_ptr(MV)); 
	
	location = glGetUniformLocation(mObj->mShaderLighting.getProgram(), "mNormalMatrix");
	glUniformMatrix4fv(	location, 1, GL_FALSE, glm::value_ptr(MN)); 
	
	location = glGetUniformLocation(mObj->mShaderLighting.getProgram(), "mBaseTex");
	glUniform1i( location, 0); 
	
	location = glGetUniformLocation(mObj->mShaderLighting.getProgram(), "uShininess");
	glUniform1f( location, 20.0f); 
	
	location = glGetUniformLocation(mObj->mShaderLighting.getProgram(), "mLight0");
	glUniform3f( location, 1.0f,1.0f,1.0f); 
	
	// We ARE NOT USING DRAW ELEMENTS as we need to do per face operations so we duplicate vertices

	glDrawArrays(GL_TRIANGLES, 0, mesh.mNumElements);
	
	
	mObj->mShaderLighting.end();
	mesh.unbind();
	checkError(__LINE__);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	for (int i=0; i < cams.size(); i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glDisable(GL_TEXTURE_RECTANGLE);
 
 }
 
 /*
  * Draw a camera, taking up the maximum size of the screen - Assume texture is already bound
  */
  
void Drawer::drawCameraQuad() {

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	mObj->mVCam.bind();
	mObj->mShaderFlatCam.begin();
	glm::mat4 Projection = glm::ortho(0.0f,(float_t)mObj->mW,(float_t)mObj->mH,0.0f);
	
	float_t sf = mObj->mW > mObj->mH ? mObj->mH / mObj->mConfig.camSize.height : mObj->mW / mObj->mConfig.camSize.width;
	float_t x =  mObj->mW > mObj->mH ?( mObj->mW - (sf * mObj->mConfig.camSize.width) ) / 2.0: 0;
	float_t y =  mObj->mW > mObj->mH ? 0: ( mObj->mH - (sf * mObj->mConfig.camSize.height) ) / 2.0;
	
	glm::mat4 Model = glm::scale(glm::mat4(1.0f),glm::vec3(sf,sf,1.0)) * glm::translate(glm::mat4(1.0f), glm::vec3(x,y,0.0));
	glm::mat4 MVP = Projection * Model;
		
	GLint LocationMVP = glGetUniformLocation(mObj->mShaderFlatCam.getProgram(), "mMVPMatrix");
	glUniformMatrix4fv(	LocationMVP, 1, GL_FALSE, glm::value_ptr(MVP)); 
	
	GLint LocationTex = glGetUniformLocation(mObj->mShaderFlatCam.getProgram(), "mBaseTex");
	glUniform1i(LocationTex,0);
	
	glDrawElements(GL_TRIANGLES, mObj->mVCam.mNumIndices, GL_UNSIGNED_INT, 0);

	mObj->mShaderFlatCam.end();
	
	mObj->mVCam.unbind();
	checkError(__LINE__);
}


/*
 * Draw Mesh Points - We assume no indicies here
 */

void Drawer::drawMeshPoints(VBOData &points, float r, float g, float b){
	
	glm::mat4 MVP = mObj->mCam.getProjMatrix() * mObj->mCam.getViewMatrix()  * mObj->mModelMatrix;

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);

	// Draw the Points - maybe add a slight blur or alpha sprite
	points.bind();
	mObj->mShaderMeshPoints.begin();
	glPointSize(2.0f);
		
	GLint location = glGetUniformLocation(mObj->mShaderMeshPoints.getProgram(), "mMVPMatrix");
	glUniformMatrix4fv(	location, 1, GL_FALSE, glm::value_ptr(MVP)); 
	
	location = glGetUniformLocation(mObj->mShaderMeshPoints.getProgram(), "mColour");
	glUniform4f( location, r,g,b,0.5f); 
	
	// No indicies here. Not needed
	glDrawArrays(GL_POINTS, 0, points.mNumElements);
	
	mObj->mShaderMeshPoints.end();
	points.unbind();
	checkError(__LINE__);
	glDisable(GL_BLEND);
}

/*
 * Draw Normals - pairs of points
 */
 
void Drawer::drawNormals(VBOData &lines) {
	
	glEnable(GL_DEPTH_TEST);
	
	glm::mat4 MVP = mObj->mCam.getProjMatrix() * mObj->mCam.getViewMatrix()  * mObj->mModelMatrix;

	// Draw the Points - maybe add a slight blur or alpha sprite
	lines.bind();
	mObj->mShaderNormals.begin();
		
	GLint location = glGetUniformLocation(mObj->mShaderNormals.getProgram(), "mMVPMatrix");
	glUniformMatrix4fv(	location, 1, GL_FALSE, glm::value_ptr(MVP)); 
	
	// No indicies here. Not needed
	glDrawArrays(GL_LINES, 0, lines.mNumElements);
	
	mObj->mShaderNormals.end();
	lines.unbind();
	checkError(__LINE__);
	glDisable(GL_DEPTH_TEST);
} 

void Drawer::drawCameraNormal(VBOData &line){
	glEnable(GL_DEPTH_TEST);
	
	glm::mat4 shift = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0,5.0,5.0));
	glm::mat4 MVP =  mObj->mCam.getProjMatrix() * mObj->mCam.getViewMatrix() * mObj->mModelMatrix * shift;

	// Draw the Points - maybe add a slight blur or alpha sprite
	line.bind();
	mObj->mShaderNormals.begin();
		
	GLint location = glGetUniformLocation(mObj->mShaderNormals.getProgram(), "mMVPMatrix");
	glUniformMatrix4fv(	location, 1, GL_FALSE, glm::value_ptr(MVP)); 
	
	// No indicies here. Not needed
	glDrawArrays(GL_LINES, 0, line.mNumElements);
	
	mObj->mShaderNormals.end();
	line.unbind();
	checkError(__LINE__);
	glDisable(GL_DEPTH_TEST);
	
}

/*
 * Draw Reference Quad
 */
void Drawer::drawReferenceQuad(){
	
	glm::mat4 MVP =  mObj->mCam.getProjMatrix() * mObj->mCam.getViewMatrix() * mObj->mModelMatrix;

	mObj->mVQuad.bind();
	mObj->mShaderQuad.begin();
	
	GLint LocationMVP = glGetUniformLocation(mObj->mShaderQuad.getProgram(), "mMVPMatrix");

	glUniformMatrix4fv(	LocationMVP, 1, GL_FALSE, glm::value_ptr(MVP)); 
	glDrawElements(GL_TRIANGLES, mObj->mVQuad.mNumIndices, GL_UNSIGNED_INT, 0);

	mObj->mShaderQuad.end();
	mObj->mVQuad.unbind();
	checkError(__LINE__);
}

/*
 * Draw Tool View
 */
 
void Drawer::drawToolView(VBOData &mesh,std::vector<boost::shared_ptr<LeedsCam> > &cams) {
	

	glm::vec3 t0 = mObj->mGripper.getPos();
	glm::vec3 t1 = glm::vec3( 0.0,0.0,-1.0);
	
	glm::mat4 mViewMatrix = glm::lookAt(t0, t0 + t1, glm::vec3(0,1,0) );
	glm::mat4 iv = glm::inverse(mViewMatrix);
	mViewMatrix = mViewMatrix * iv;
	
	float_t r = (float_t)mObj->mFBOTool.getWidth() / (float_t)mObj->mFBOTool.getHeight();
	glm::mat4 mProjectionMatrix = glm::perspective(55.0f, r, 1.0f, 3000.0f);
	
	
	glm::mat4 MVP =  mProjectionMatrix * mViewMatrix * mObj->mModelMatrix;
	
	mObj->mFBOTool.bind();
	
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.8f, 0.8f, 0.8f, 1.0f)[0]);
	GLfloat depth = 1.0f;
	glClearBufferfv(GL_DEPTH, 0, &depth );
		
	glm::mat4 MV = mViewMatrix * mObj->mModelMatrix;
	glm::mat4 MN = glm::inverseTranspose(mViewMatrix);
	
	glEnable(GL_TEXTURE_RECTANGLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glEnable(GL_DEPTH_TEST);
	
	// Enable Textures
	for (int i=0; i < cams.size(); i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_RECTANGLE, cams[i]->getTexture());
	}
	checkError(__LINE__);

	// Draw the Points - maybe add a slight blur or alpha sprite
	mesh.bind();
	

	mObj->mShaderLighting.begin();
		
	GLint location = glGetUniformLocation(mObj->mShaderLighting.getProgram(), "mMVPMatrix");
	glUniformMatrix4fv(	location, 1, GL_FALSE, glm::value_ptr(MVP)); 
	
	location = glGetUniformLocation(mObj->mShaderLighting.getProgram(), "mMVMatrix");
	glUniformMatrix4fv(	location, 1, GL_FALSE, glm::value_ptr(MV)); 
	
	location = glGetUniformLocation(mObj->mShaderLighting.getProgram(), "mNormalMatrix");
	glUniformMatrix4fv(	location, 1, GL_FALSE, glm::value_ptr(MN)); 
	
	location = glGetUniformLocation(mObj->mShaderLighting.getProgram(), "mBaseTex");
	glUniform1i( location, 0); 
	
	location = glGetUniformLocation(mObj->mShaderLighting.getProgram(), "uShininess");
	glUniform1f( location, 20.0f); 
	
	location = glGetUniformLocation(mObj->mShaderLighting.getProgram(), "mLight0");
	glUniform3f( location, 1.0f,1.0f,1.0f); 
	
	// We ARE NOT USING DRAW ELEMENTS as we need to do per face operations so we duplicate vertices

	glDrawArrays(GL_TRIANGLES, 0, mesh.mNumElements);
	checkError(__LINE__);
	mObj->mShaderLighting.end();
	mesh.unbind();
	checkError(__LINE__);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	for (int i=0; i < cams.size(); i++){
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}


	mObj->mFBOTool.unbind();
	checkError(__LINE__);
	// Now draw the Quad in the Corner
	
	MVP = mObj->mCam.getProjMatrix() * mObj->mCam.getViewMatrix()  * mObj->mModelMatrix;
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glActiveTexture(GL_TEXTURE0);
	mObj->mVCam.bind();
	
	mObj->mFBOTool.bindColour();
	
	mObj->mShaderFlatCam.begin();
	glm::mat4 Projection = glm::ortho(0.0f,(float_t)mObj->mW,(float_t)mObj->mH,0.0f);
	checkError(__LINE__);
	float_t sf = 0.33;
	float_t x = 0.0;
	float_t y = 0.0;  
	checkError(__LINE__);
	glm::mat4 Model = glm::scale(glm::mat4(1.0f),glm::vec3(sf,sf,1.0)) * glm::translate(glm::mat4(1.0f), glm::vec3(x,y,0.0));
	MVP = Projection * Model;
		
	GLint LocationMVP = glGetUniformLocation(mObj->mShaderFlatCam.getProgram(), "mMVPMatrix");
	glUniformMatrix4fv(	LocationMVP, 1, GL_FALSE, glm::value_ptr(MVP)); 

	GLint LocationTex = glGetUniformLocation(mObj->mShaderFlatCam.getProgram(), "mBaseTex");
	glUniform1i(LocationTex,0);
	
	glDrawElements(GL_TRIANGLES, mObj->mVCam.mNumIndices, GL_UNSIGNED_INT, 0);

	mObj->mShaderFlatCam.end();
	mObj->mFBOTool.unbindColour();
	mObj->mVCam.unbind();
	checkError(__LINE__);
	
	glDisable(GL_TEXTURE_RECTANGLE);

	
}


/*
 * Draw Zoomed View
 */
 
 void Drawer::drawZoomed() {
	
	glm::mat4 MVP =  mObj->mGripperCam.getProjMatrix() * mObj->mGripperCam.getViewMatrix() * mObj->mModelMatrix;
	
}



