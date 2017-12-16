/**
* @brief UVC Camera controller for Linux
* @file uvc_camera.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 03/05/2012
*
*/

#ifndef __UVC_VIDEO__
#define __UVC_VIDEO__


#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>

// videodev2 under ubuntu apparently
#include <linux/videodev2.h>


// Include GUVCView decoding
// http://sourceforge.net/p/guvcview/code/795/tree/trunk/src/

 extern "C" {
   #include "jpeg.h"
   #include "colorspaces.h"
 }
 

#define V4L_BUFFERS_DEFAULT	8
#define V4L_BUFFERS_MAX		32

#define OPT_ENUM_INPUTS		256
#define OPT_SKIP_FRAMES		257


/************************************************************************
 * values for the "interlace" parameter [y4m_*_interlace()]
 ************************************************************************/
#define Y4M_ILACE_NONE          0   /* non-interlaced, progressive frame */
#define Y4M_ILACE_TOP_FIRST     1   /* interlaced, top-field first       */
#define Y4M_ILACE_BOTTOM_FIRST  2   /* interlaced, bottom-field first    */
#define Y4M_ILACE_MIXED         3   /* mixed, "refer to frame header"    */

/************************************************************************
 * values for the "chroma" parameter [y4m_*_chroma()]
 ************************************************************************/
#define Y4M_CHROMA_420JPEG     0  /* 4:2:0, H/V centered, for JPEG/MPEG-1 */
#define Y4M_CHROMA_420MPEG2    1  /* 4:2:0, H cosited, for MPEG-2         */
#define Y4M_CHROMA_420PALDV    2  /* 4:2:0, alternating Cb/Cr, for PAL-DV */
#define Y4M_CHROMA_444         3  /* 4:4:4, no subsampling, phew.         */
#define Y4M_CHROMA_422         4  /* 4:2:2, H cosited                     */
#define Y4M_CHROMA_411         5  /* 4:1:1, H cosited                     */
#define Y4M_CHROMA_MONO        6  /* luma plane only                      */
#define Y4M_CHROMA_444ALPHA    7  /* 4:4:4 with an alpha channel          */


/*
 * Class to deal directly with the video. Does not contain OpenCV methods - this is decoupled into the camera manager
 */

namespace uvc {

  // A basic struct that holds the state of our device
  struct Device {
	  int                 width, height;
	  int                 fps;
    std::string         dev_name;
    
    // Stuff we dont need to set
    void                *mem[V4L_BUFFERS_MAX];
	  int                 dev;
	  unsigned char       *jbuffer;
	  unsigned int        buffer_size;
	  struct v4l2_buffer  buf;

    unsigned int pixelformat = V4L2_PIX_FMT_YUYV; //V4L2_PIX_FMT_MJPEG;
	  unsigned int nbufs = 2; // V4L_BUFFERS_DEFAULT;
	  unsigned int input = 0;
	  unsigned int skip = 0;

    Device (std::string d, int w, int h, int f) : dev_name(d), width(w), height(h), fps(f) {}

  };


  int VideoOpen(Device &device);
  int UVCSetControl(Device &device, unsigned int id, int value);
  int VideoSetFormat(Device &device);
  int VideoSetFramerate(Device &device);
  int VideoReqbufs(Device &device);
  int VideoEnable(Device &device, int enable);
  void VideoQueryMenu(Device &device, unsigned int id);
  void VideoListControls(Device &device);
  void VideoEnumInputs(Device &device);
  int VideoGetInput(Device &device);
  int VideoSetInput(Device &device, unsigned int input);
  bool StartCapture(Device &device);
  void Close(Device &device);
  void Capture(Device &device);

};

#endif
