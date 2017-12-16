#ifndef __bmp_hpp__
#define __bmp_hpp__

#include <vector>

namespace s9 {

  namespace image {

    /**
     * Basic Bitmap struct
     */

    struct Bitmap {

      unsigned int width;
      unsigned int height;
      std::vector<char> data;

      // Basic constructor that deals with allocating our memory and little else
      // Potentially we could just leave a nice pointer so we don't need to do memory
      // copies but whatever.
      Bitmap (unsigned int w, unsigned int h) : width(w), height(h) {
        for (int i =0; i < width * height * 4; i+=4){
          data.push_back(0); 
          data.push_back(0);
          data.push_back(0);
          data.push_back(255);
        }  
      }

    };

    // Cheap return type
    struct BitmapRGB {
      char r;
      char g;
      char b;
    };

    void WriteBitmap (Bitmap &bitmap, std::string output_path);
    BitmapRGB GetRGB(Bitmap &bitmap, unsigned int x, unsigned int y);
    void SetRGB(Bitmap &bitmap, unsigned int x, unsigned int y, float r, float g, float b);
    void SetRGB(Bitmap &bitmap, unsigned int x, unsigned int y, char r, char g, char b);
  }
}
#endif

 
