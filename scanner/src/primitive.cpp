/**
* @brief Primitive Classes
* @file primitive.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 05/07/2012
*
*/

#include "primitive.hpp"

using namespace std;
using namespace boost;
using namespace boost::assign; 


Primitive::Primitive() {
	mPos = glm::vec3(0,0,0);
	mLook = glm::vec3(0,0,1);
	mUp = glm::vec3(0,1,0);
}

/*
 * Primitive Generation
 */
 
 
void Primitive::generate(std::string filename, glm::vec3 scale = glm::vec3(1.0,1.0,1.0) ) {
	
	loadAsset(filename);
	cerr << "Leeds - Loaded " << filename << " with " << mVBO.mNumElements / 3 <<  " faces." <<endl;
	mScale = scale;
}

/*
 * Recompute the matrices on the primitive
 */
 
void Primitive::compute() {
	glm::quat q_rotate;
	
	q_rotate = glm::rotate( q_rotate, mRot.x, glm::vec3( 1, 0, 0 ) );
	q_rotate = glm::rotate( q_rotate, mRot.y, glm::vec3( 0, 1, 0 ) );
	q_rotate = glm::rotate( q_rotate, mRot.z, glm::vec3( 0, 0, 1 ) );

	mLook = q_rotate * mLook;
	mUp = q_rotate * mUp;
	
	mRotMatrix = glm::mat4_cast(q_rotate);
	
	mTransMatrix = glm::translate(glm::mat4(1.0f), mPos);
	
	mScaleMatrix = glm::scale(glm::mat4(1.0f), mScale);
}


int Primitive::loadAsset (std::string filename){

	pScene = aiImportFile(filename.c_str(),aiProcessPreset_TargetRealtime_MaxQuality);
	if (pScene) {
	/*	get_bounding_box(&scene_min,&scene_max);
		scene_center.x = (scene_min.x + scene_max.x) / 2.0f;
		scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
		scene_center.z = (scene_min.z + scene_max.z) / 2.0f;*/
		recursiveCreate(pScene, pScene->mRootNode);
		
		mVBO.mNumElements = mVBO.mVertices.size() / 3;
		mVBO.compile(VBO_VERT | VBO_NORM);
		checkError(__LINE__);
		
		return 0;
	
	}
	return 1;
}

Primitive::~Primitive() {
	aiReleaseImport(pScene);
}
