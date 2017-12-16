/*
 *      test.c  --  USB Video Class test application
 *
 *      Copyright (C) 2005-2008
 *          Laurent Pinchart (laurent.pinchart@skynet.be)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 */

/*
 * WARNING: This is just a test application. Don't fill bug reports, flame me,
 * curse me on 7 generations :-).
 */
 
 ///http://stackoverflow.com/questions/8836872/mjpeg-to-raw-rgb24-with-video4linux
 ///http://stackoverflow.com/questions/5280756/libjpeg-ver-6b-jpeg-stdio-src-vs-jpeg-mem-src
 
#include "uvc_camera.hpp"

using namespace std;

namespace uvc {

/*
 * Open a device with UVC
 */

int VideoOpen(Device &device) {
	struct v4l2_capability cap;
	int ret;

	device.dev = open(device.dev_name.c_str(), O_RDWR);
	if (device.dev < 0) {
		printf("Error opening device %s: %d.\n", device.dev_name, errno);
		return device.dev;
	}

	memset(&cap, 0, sizeof cap);
	ret = ioctl(device.dev, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) {
		printf("Error opening device %s: unable to query device.\n", device.dev_name);
		close(device.dev);
		return ret;
	}

#if 0
	if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
		printf("Error opening device %s: video capture not supported.\n",
			devname);
		close(dev);
		return -EINVAL;
	}
#endif

	printf("Device %s opened: %s.\n", device.dev_name, cap.card);
	return device.dev;
}

int UVCSetControl(Device &device, unsigned int id, int value) {
	struct v4l2_control ctrl;
	int ret;

	ctrl.id = id;
	ctrl.value = value;

	ret = ioctl(device.dev, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0) {
		printf("unable to set control: %s (%d).\n", strerror(errno), errno);
	}
  return ret;
}

/*
 * Set a format for this camera. Read it from the device struct
 */

int VideoSetFormat(Device &device) {
	struct v4l2_format fmt;
	int ret;

	memset(&fmt, 0, sizeof fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = device.width;
	fmt.fmt.pix.height = device.height;
	fmt.fmt.pix.pixelformat = device.pixelformat;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;

	ret = ioctl(device.dev, VIDIOC_S_FMT, &fmt);
	if (ret < 0) {
		printf("Unable to set format: %d.\n", errno);
		return ret;
	}

	printf("Video format set: width: %u height: %u buffer size: %u\n",
		fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.sizeimage);
	return 0;
}

/*
 * Attempt to set the framerate
 */

int VideoSetFramerate(Device &device) {
	struct v4l2_streamparm parm;
	int ret;

	memset(&parm, 0, sizeof parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(device.dev, VIDIOC_G_PARM, &parm);
	if (ret < 0) {
		printf("Unable to get frame rate: %d.\n", errno);
		return ret;
	}

	printf("Current frame rate: %u/%u\n",
		parm.parm.capture.timeperframe.numerator,
		parm.parm.capture.timeperframe.denominator);

	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = device.fps;

	ret = ioctl(device.dev, VIDIOC_S_PARM, &parm);
	if (ret < 0) {
		printf("Unable to set frame rate: %d.\n", errno);
		return ret;
	}

	ret = ioctl(device.dev, VIDIOC_G_PARM, &parm);
	if (ret < 0) {
		printf("Unable to get frame rate: %d.\n", errno);
		return ret;
	}

	printf("Frame rate set: %u/%u\n",
		parm.parm.capture.timeperframe.numerator,
		parm.parm.capture.timeperframe.denominator);
	return 0;
}

/*
 * Request buffers for running
 */

int VideoReqbufs(Device &device) {
	struct v4l2_requestbuffers rb;
	int ret;

	memset(&rb, 0, sizeof rb);
	rb.count = device.nbufs;
	rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rb.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(device.dev, VIDIOC_REQBUFS, &rb);
	if (ret < 0) {
		printf("Unable to allocate buffers: %d.\n", errno);
		return ret;
	}

	printf("%u buffers allocated.\n", rb.count);
	return rb.count;
}

/*
 * Actually enable the device
 */

int VideoEnable(Device &device, int enable) {
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret;

	ret = ioctl(device.dev, enable ? VIDIOC_STREAMON : VIDIOC_STREAMOFF, &type);
	if (ret < 0) {
		printf("Unable to %s capture: %d.\n",
			enable ? "start" : "stop", errno);
		return ret;
	}

	return 0;
}

/*
 * Query the menu
 */

void VideoQueryMenu(Device &device, unsigned int id) {
	struct v4l2_querymenu menu;
	int ret;

	menu.index = 0;
	while (1) {
		menu.id = id;
		ret = ioctl(device.dev, VIDIOC_QUERYMENU, &menu);
		if (ret < 0)
			break;

		printf("  %u: %.32s\n", menu.index, menu.name);
		menu.index++;
	};
}

/*
 * Print the list of controls on this camera
 */

void VideoListControls(Device &device) {
	struct v4l2_queryctrl query;
	struct v4l2_control ctrl;
	char value[12];
	int ret;

#ifndef V4L2_CTRL_FLAG_NEXT_CTRL
	unsigned int i;

	for (i = V4L2_CID_BASE; i <= V4L2_CID_LASTP1; ++i) {
		query.id = i;
#else
	query.id = 0;
	while (1) {
		query.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
#endif
		ret = ioctl(device.dev, VIDIOC_QUERYCTRL, &query);
		if (ret < 0)
			break;

		if (query.flags & V4L2_CTRL_FLAG_DISABLED)
			continue;

		ctrl.id = query.id;
		ret = ioctl(device.dev, VIDIOC_G_CTRL, &ctrl);
		if (ret < 0)
			strcpy(value, "n/a");
		else
			sprintf(value, "%d", ctrl.value);

		printf("control 0x%08x %s min %d max %d step %d default %d current %s.\n",
			query.id, query.name, query.minimum, query.maximum,
			query.step, query.default_value, value);

		if (query.type == V4L2_CTRL_TYPE_MENU) {
			VideoQueryMenu(device, (unsigned int)query.id);
    }

	}
}

/*
 * Enumerate the inputs
 */

void VideoEnumInputs(Device &device) {
	struct v4l2_input input;
	unsigned int i;
	int ret;

	for (i = 0; ; ++i) {
		memset(&input, 0, sizeof input);
		input.index = i;
		ret = ioctl(device.dev, VIDIOC_ENUMINPUT, &input);
		if (ret < 0)
			break;

		if (i != input.index)
			printf("Warning: driver returned wrong input index "
				"%u.\n", input.index);

		printf("Input %u: %s.\n", i, input.name);
	}
}

/*
 * Get a particular input
 */

int VideoGetInput(Device &device) {
	__u32 input;
	int ret;

	ret = ioctl(device.dev, VIDIOC_G_INPUT, &input);
	if (ret < 0) {
		printf("Unable to get current input: %s.\n", strerror(errno));
		return ret;
	}

	return input;
}

/*
 * Set a particular Input
 */

int VideoSetInput(Device &device, unsigned int input) {
	__u32 _input = input;
	int ret;

	ret = ioctl(device.dev, VIDIOC_S_INPUT, &_input);
	if (ret < 0){
		printf("Unable to select input %u: %s.\n", input, strerror(errno));
  }

	return ret;
}

/*
 * Set everything up and launch a thread to start capture
 */

bool StartCapture(Device &device) {
	/* Video buffers */

	/* Open the video device. */

  VideoOpen(device);
	if (device.dev < 0){
		return false;
  }
		
	int ret = VideoGetInput(device);
	cout << "Input " << ret << " selected" << endl;
	
	VideoListControls(device);
	
	/* Set the video format. */
	if (VideoSetFormat(device) < 0) {
		Close(device);
		return false;
	}

	/* Set the frame rate. */
	if (VideoSetFramerate(device) < 0) {
		Close(device);
		return false;
	}

	/* Allocate buffers. */
	if (VideoReqbufs(device) < 0) {
		Close(device);
		return 1;
	}

	/* Map the buffers. */
	for (int i = 0; i < device.nbufs; ++i) {
		memset(&device.buf, 0, sizeof device.buf);
		device.buf.index = i;
		device.buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		device.buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(device.dev, VIDIOC_QUERYBUF, &device.buf);
		if (ret < 0) {
			printf("Unable to query buffer %u (%d).\n", i, errno);
			Close(device);
			return false;
		}
		printf("length: %u offset: %u\n", device.buf.length, device.buf.m.offset);

		device.mem[i] = mmap(0, device.buf.length, PROT_READ, MAP_SHARED, device.dev, device.buf.m.offset);
		if (device.mem[i] == MAP_FAILED) {
			printf("Unable to map buffer %u (%d)\n", i, errno);
			Close(device);
			return false;
		}
		printf("Buffer %u mapped at address %p.\n", i, device.mem[i]);
	}
	
	// RGB8 Buffer
	device.buffer_size = 0;
		
	int size = device.width * device.height * 3;
	device.jbuffer = new unsigned char[size];
	memset(device.jbuffer, 0, size);
	
	/* Queue the buffers. */
	for (int i = 0; i < device.nbufs; ++i) {
		
		memset(&device.buf, 0, sizeof device.buf);
		device.buf.index = i;
		device.buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		device.buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(device.dev, VIDIOC_QBUF, &device.buf);
		if (ret < 0) {
			printf("Unable to queue buffer (%d).\n", errno);
			Close(device);
			return 1;
		}
	}
		
	/* Start streaming. */
	VideoEnable(device, 1);
}


/*
 * close device
 */

void Close(Device &device){
	/* Stop streaming. */
	VideoEnable(device, 0);
	Close(device);
}


/*
 * Threaded function that queries buffers and performs yuyv to RGB conversion
 */

void Capture(Device &device) {

  // try and grab at the requested framerate
  /* Dequeue a buffer. */
  memset(&device.buf, 0, sizeof device.buf);
  device.buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  device.buf.memory = V4L2_MEMORY_MMAP;
  
  int ret = ioctl(device.dev, VIDIOC_DQBUF, &device.buf);
  
  if (ret < 0) {
    printf("Unable to dequeue buffer (%d).\n", errno);
    Close(device);
    return;
  }
  
  try{
    if (device.buf.bytesused > 0) {
      //jpeg_decode((BYTE**)&jbuffer, (BYTE*)mem[buf.index], mWidth, mHeight);
      yuyv2rgb((BYTE*)device.mem[device.buf.index], (BYTE*)device.jbuffer, device.width, device.height);
      //memcpy((void*)jbuffer,(void*) mem[buf.index], buf.bytesused);
    }
  }
  catch (...){
    
  }
    
  ret = ioctl(device.dev, VIDIOC_QBUF, &device.buf);
  if (ret < 0) {
    printf("Unable to requeue buffer (%d).\n", errno);
    Close(device);
    return;
  }
  
  // Pause for the length of time to match the fps
  usleep( 1.0 / device.fps * 1000000.0);  
}

}
