/**
* @brief Camera Classes
* @file camera.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 05/07/2012
*
*/

#include "camera.hpp"

using namespace std;
using namespace boost;
using namespace boost::assign; 


/*
 * Basic Camera - This one keeps the up vector always parallel to Y axis
 */

Camera::Camera(){
	mUp = glm::vec3(0,1,0);
	mPos = glm::vec3(0,0,1.0);
	mLook = glm::vec3(0,0,-1.0);
	mNear = 1.0f;
	mField = 55.0f;
	mFar = 100.0f;
}
	
void Camera::yaw(float_t a){
	glm::quat q_rotate;
	q_rotate = glm::rotate( q_rotate, a, glm::vec3( 0, 1, 0 ) );
	mLook = q_rotate * mLook;
	mUp = q_rotate * mUp;
	
	compute();
	
}

void Camera::pitch(float_t a){
	glm::quat q_rotate;
	q_rotate = glm::rotate( q_rotate, a, glm::vec3( 1, 0, 0 ) );
	mLook = q_rotate * mLook;
	mUp = q_rotate * mUp;
	
	compute();
}

void Camera::roll(float_t a){
	glm::quat q_rotate;
	q_rotate = glm::rotate( q_rotate, a, glm::vec3( 0, 0, 1 ) );
	mLook = q_rotate * mLook;
	mUp = q_rotate * mUp;
	
	compute();
}

void Camera::setLook(glm::vec3 l) {
	mLook = glm::normalize(l);
	compute(); 
}
	
void Camera::move(glm::vec3 m){
	mPos += m;
	compute();
}

void Camera::setRatio(float_t r) {
	mR = r;
	compute();
}
	
void Camera::compute() {
	mViewMatrix = glm::lookAt(mPos, mPos + mLook, mUp);
	mProjectionMatrix = glm::perspective(mField,mR,mNear, mFar);
}

/*
 * Orbital Camera as oppose to a truck camera
 */

OrbitCamera::OrbitCamera() : Camera() {
	mPos = glm::vec3(0,0,1.0);
	mLook = glm::vec3(0,0,0);
}

void OrbitCamera::compute() {
	mViewMatrix = glm::lookAt(mPos, mLook, mUp);
	mProjectionMatrix = glm::perspective(mField,mR,mNear, mFar);
}

void OrbitCamera::zoom(float_t z) {
	glm::vec3 dir = mPos - mLook;
	dir = glm::normalize(dir);
	dir *= z;
	mPos += dir;
	compute();
}

void OrbitCamera::shift(float_t du, float_t dv) {
	glm::vec3 dir = mPos - mLook;
	dir = glm::normalize(dir);
	glm::vec3 shiftx = glm::cross(dir,mUp);
	shiftx *= du;
	glm::vec3 shifty = mUp * dv;

	mPos += shiftx + shifty;
	mLook += shiftx + shifty;
	
	compute();
}


/*
 * Real Camera is a little different
 */
 
 
 
void RealCamera::yaw(float_t a){
	glm::quat q_rotate;
	q_rotate = glm::rotate( q_rotate, a, mUp );
	mLook = q_rotate * mLook;
	mUp = q_rotate * mUp;
	
	compute();
	
}

void RealCamera::pitch(float_t a){
	glm::quat q_rotate;
	
	glm::vec3 right = glm::cross(mUp,mLook);
	
	q_rotate = glm::rotate( q_rotate, a, right );
	mLook = q_rotate * mLook;
	mUp = q_rotate * mUp;
	
	compute();
}

void RealCamera::roll(float_t a){
	glm::quat q_rotate;
	q_rotate = glm::rotate( q_rotate, a, mLook);
	mLook = q_rotate * mLook;
	mUp = q_rotate * mUp;
	
	compute();
}



