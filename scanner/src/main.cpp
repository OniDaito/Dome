/**
* @brief Scanner program for pulling images off the cameras
* @file main.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 22/07/2017
*
*/

#include <getopt.h>
#include "string_utils.hpp"
#include "uvc_camera.hpp"
#include "bmp.hpp"

using namespace std;

struct Options {
  std::string device_path;
  std::string profile_path;
  std::string output_path;
  unsigned int width;
  unsigned int height;
  unsigned int fps;
  unsigned int focus;
};


void ParseCommandLine(Options &ops, int argc, char *argv[]) {
  int c;
    int digit_optind = 0;
    static struct option long_options[] = {
        {"width", 1, 0, 0},
        {"height", 0, 0, 0},
        {NULL, 0, NULL, 0}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, (char **)argv, "w:h:o:d:p:f:?", long_options, &option_index)) != -1) {
      int this_option_optind = optind ? optind : 1;
      switch (c) {
        case 'd' :
          ops.device_path = std::string(optarg);
          break;
        case 'w':
          ops.width = s9::FromString<unsigned int>(optarg);
          break;
        case 'h':
          ops.height = s9::FromString<unsigned int>(optarg);
          break;
        case 'o':
          ops.output_path = std::string(optarg);
          break;
        case 'p':
          ops.profile_path = std::string(optarg);
          break;
        case 'f':
          ops.focus = s9::FromString<unsigned int>(optarg);
          break;

        case '?' :
          cout << "Usage: scanner -d <device name> -w <width> -h <height> -p <profile> -o <output path> -f <focus>" << endl;
          break;
     }
  }

  if (optind < argc) {
      std::cout << "non-option ARGV-elements: " << std::endl;
      while (optind < argc)
          std::cout << argv[optind++];
      std::cout << std::endl;
  }

}

/**
 * Read one of the GUVCView profiles and set the controls
 */

void ApplyProfile(Options &ops, uvc::Device &device) {

  if (ops.profile_path.length() > 0){
    string profile = s9::TextFileRead(ops.profile_path);
    if (s9::StringBeginsWith(profile, "#V4L2/CTRL")){
      cout << "Reading profile from " << ops.profile_path << endl;

      // We really want focus so make sure its set to manual first
      int ret = UVCSetControl(device, 0x009a090c, 0);
      if(ret < 0){
          cout << "Error setting autofocus to 0" << endl;
      }

      vector<string> lines = s9::SplitStringNewline(profile);
      for (string line : lines){
        if (s9::StringContains(line, "ID{")){

          vector<string> tokens = s9::SplitStringChars(line,"{");
          // Should be 4 tokens
          if (tokens.size() == 4) {
            unsigned int id = s9::HexStringToUnsigned(s9::StringRemove(tokens[1], "};CHK"));
            int value = s9::FromString<int>(s9::StringRemove(tokens[3],"}"));
            
            int ret = UVCSetControl(device, id, value);
            if(ret < 0){
              cout << "Error with " << line << endl;
            }
          }
        }
      }
    }   
  }

}

int main(int argc, char *argv[]) {

  // Set default options and check for command line switches
  Options ops;
  
  ops.width = 640;
  ops.height = 480;
  ops.fps = 2;
  ops.focus = 102;
  ops.device_path = "/dev/video0";
  ops.output_path = "test.bmp";
  ops.profile_path = "";

  ParseCommandLine(ops, argc, argv);

  s9::image::Bitmap bmp (ops.width, ops.height);

  uvc::Device device (ops.device_path, ops.width, ops.height, ops.fps );

  StartCapture(device);

  //VideoListControls(device);
  
  Capture(device); 

  // For the focus we really need to take some snaps first
  // then keep setting and snapping. Quite annoying  
  
  // We actually need to wiggle the focus to get it to work :/

  UVCSetControl(device, 0x009a090c, 0);
  UVCSetControl(device, 0x009a090a, 85);
  UVCSetControl(device, 0x0098090c, 0);
  UVCSetControl(device, 0x0098091a, 6500);
  UVCSetControl(device, 0x00980900, 128);
  UVCSetControl(device, 0x00980918, 1);
  UVCSetControl(device, 0x0098091b, 100);
  UVCSetControl(device, 0x0098091c, 1);
  UVCSetControl(device, 0x009a0903, 0);

  ApplyProfile(ops, device);
  
  for (int i =0; i < 5; i++){
    UVCSetControl(device, 0x009a090a, ops.focus);
    Capture(device); 
  }
 
 
  // Slow, but we have an alpha channel so whatever
  for (int i = 0; i < ops.width * ops.height * 3; i+=3 ) {
    unsigned int x,y;
    x = (i/3) % ops.width;
    y = (i/3) / ops.width;

    s9::image::SetRGB(bmp, x, y, 
        static_cast<char>(device.jbuffer[i]), 
        static_cast<char>(device.jbuffer[i+1]), 
        static_cast<char>(device.jbuffer[i+2]));
  }
 

  s9::image::WriteBitmap(bmp, ops.output_path);
}
