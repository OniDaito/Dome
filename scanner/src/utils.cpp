/**
* @briefMisc Functions that appear in various places
* @file utils.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 03/05/2012
*
*/

#include "utils.hpp"

using namespace std;
using namespace cv;

/*
 * Find Chessboards in the Matrix
 */

bool findChessboard(Mat &cam0, vector<Point2f> &corners, Mat &board, cv::Size &size ) {

	if ( findChessboardCorners(cam0, size, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK) ) {
	
		// Update the view
		
		cam0.copyTo(board);
		drawChessboardCorners(board, size, corners, true);
		
		Mat grey = cam0;

		cvtColor( cam0, grey, CV_RGB2GRAY);		
		cornerSubPix(grey, corners, Size(11,11), Size(-1,-1), TermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 30, 0.01));

		return true;
	}

	cam0.copyTo(board);	
	return false;
}


/*
 * Load Intrinsic Parameters from disk
 */


bool loadCameraParameters(string filename, CameraParameters &ip) {
	try {
		FileStorage fs(filename.c_str(), CV_STORAGE_READ);
		if(!fs.isOpened()) {
			cout << "Leeds - Failed to open intrinsic file "  << filename << endl;
			return false;
		}

		fs["M"] >> ip.M;
		fs["D"] >> ip.D;

		try {
			fs["R"] >> ip.R;
			fs["T"] >> ip.T;
		}
		catch (...) {
			cout << "Leeds - failed to load world transforms in intrinsics." << endl;
		}
		
		cout << "Leeds - Loaded camera Parameters " << filename << endl;
		ip.mCalibrated = true;
		fs.release();
		return true;
		
	} catch(...) {
		cout << "Leeds - failed to load intrinsic variables. You will need to calibrate." << endl;
	}

	return false;
}


/*
 * Save the Intrinsic Parameters to a set of files
 */

bool saveCameraParameters(string filename, CameraParameters &in) {
	

    FileStorage fs(filename, CV_STORAGE_WRITE);
    if( fs.isOpened() ) {
        fs << "M" << in.M << "D" << in.D << "R" << in.R << "T" << in.T;
        fs.release();
        return true;
    }
    else
        cout << "Error: can not save the intrinsic parameters\n";
    
    return false;
}





