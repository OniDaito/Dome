/**
* @brief Primitive Classes
* @file primitive.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 05/07/2012
*
*/


#ifndef PRIMITIVE_HPP
#define PRIMITIVE_HPP

#include "config.hpp"


/*
 * Struct for primitive objects with their own co-ordinates
 * \todo maybe pass in shaders here as well?
 */
 
class Primitive {
public:
	Primitive();

	virtual ~Primitive(); 
	
	void generate(std::string filename, glm::vec3 scale);
	void move(glm::vec3 p) {mPos += p; mLook+=p; compute(); };
	void rotate(glm::vec3 r) {mRot += r; compute(); };
	glm::mat4 getMatrix() { return mTransMatrix * mRotMatrix * mScaleMatrix; };
	
	
	void setLook(glm::vec3 v) {mLook = v; };
	void setPos(glm::vec3 v) {mPos = v; };
	
	void compute();
	int loadAsset(std::string filename);
	
	glm::vec3 getPos() { return mPos;};
	glm::vec3 getLook() { return mLook;}
	
	void bind() { mVBO.bind(); };
	void unbind() {mVBO.unbind(); };
	GLuint getNumElements() { return mVBO.mNumElements; };

protected:
	void recursiveCreate (const struct aiScene *sc, const struct aiNode* nd);
	
	VBOData mVBO;
	
	const struct aiScene* pScene;
	
	glm::vec3 mPos;
	glm::vec3 mUp;
	glm::vec3 mLook;
	glm::vec3 mRot;
	glm::vec3 mScale;
	
	glm::mat4 mTransMatrix;
	glm::mat4 mRotMatrix;
	glm::mat4 mScaleMatrix;

};

#endif
