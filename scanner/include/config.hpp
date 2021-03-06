/**
* @brief Global Configuration parameters
* @file config.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 26/04/2012
*
*/

#ifndef CONFIG_HPP
#define CONFIG_HPP


#include "s9gear.hpp"

#include <vector>
#include <opencv/highgui.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include "vbo.hpp"

typedef struct {
	// OpenCV Camera and calibration pairs info
	cv::Size boardSize;
	cv::Size camSize;
	int fps;
	float squareSize;
	int startCam;
	int endCam;
	float interval;
	int maxImages;
	
	// Point Detection Parameters
	double_t pointThreshold;
	double_t scanInterval;
	
	// World Sizes
	float xs,ys,zs;
	float xe,ye,ze;
	
	cv::Point3i meshResolution;
	
	// PCL Values
	int poissonDepth;
	float poissonSamples;
	float poissonScale;
	
	float pclSearchRadius;
	float pclPolynomialOrder;
	float pclUpsamplingRadius;
	float pclUpsamplingStepSize;
	
	float pclFilterMeanK;
	float pclFilterThresh;


}GlobalConfig;


inline bool checkError(int line) {
	int Error;
	if((Error = glGetError()) != GL_NO_ERROR){
		std::string ErrorString;
		switch(Error)
		{
		case GL_INVALID_ENUM:
			ErrorString = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			ErrorString = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			ErrorString = "GL_INVALID_OPERATION";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			ErrorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			ErrorString = "GL_OUT_OF_MEMORY";
			break;
		default:
			ErrorString = "UNKNOWN";
			break;
		}
		std::cerr << "Leeds - OpenGL Error " << ErrorString << " at " << line << std::endl;
	}
	return Error == GL_NO_ERROR;
}


/*
 * OpenCV parameters per camera - these hold intrinsic values and extrinsic-to-world values
 */

class CameraParameters{
public:
	
	CameraParameters() { M = cv::Mat::eye(3, 3, CV_64F); mCalibrated = false;};
	cv::Mat M, D, R, T; 		// Camera Matrix, Distortion co-efficients, rotation and translation matrices
	std::vector<cv::Mat> Rs;	// Rotations for each view
	std::vector<cv::Mat> Ts;	// Translations for each view
	bool mCalibrated;			// Is this calibrated and distortion free?
};


/*
 * Camera UVC Controls for the Logitech C910s
 */

typedef enum {
		BRIGHTNESS = 0x00980900,
		CONTRAST = 0x00980901,
		SATURATION = 0x00980902,
		GAIN = 0x00980913,
		SHARPNESS = 0x0098091b,
		AUTO_EXPOSURE = 0x009a0901,
		EXPOSURE = 0x009a0902,
		FOCUS = 0x009a090a,
		AUTO_FOCUS = 0x009a090c
}CameraControl;


#endif
