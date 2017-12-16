/**
* @brief Shader Classes
* @file shader.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 05/07/2012
*
*/


#ifndef SHADER_HPP
#define SHADER_HPP


/*
 * Basic Shader class - loads and binds
 */

class Shader {
public:
	void load(std::string vert, std::string frag);
	GLuint getProgram() { return mProgram; };
	void begin() { glUseProgram(mProgram);};
	void end() {glUseProgram(0);};
	
	~Shader() { glDetachShader(mProgram, mVS); glDetachShader(mProgram, mFS);  } 
	
protected:
   
    GLuint mVS, mFS;
    GLuint mProgram;

};


#endif
