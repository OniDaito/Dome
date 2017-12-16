/**
* @brief Writing out a bmp file
* @file bmp.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 08/01/2016
*
*/

#include <fstream>
#include <iostream>
#include <math.h>
#include "bmp.hpp"

using namespace std;

namespace s9 {
namespace image { 


// Set the RGB value - note we save in B G R because that way, we can
// display straight to X11

void SetRGB(Bitmap &bitmap, unsigned int x, unsigned int y, float r, float g, float b) {
  int p = (y * bitmap.width + x) * 4;
  // TODO - add max and min limits here
  bitmap.data[p] = static_cast<char>(floor(b * 255.0));
  bitmap.data[p+1] = static_cast<char>(floor(g * 255.0));
  bitmap.data[p+2] = static_cast<char>(floor(r * 255.0));
  bitmap.data[p+3] = static_cast<char>(255);
}

void SetRGB(Bitmap &bitmap, unsigned int x, unsigned int y, char r, char g, char b) {
  int p = (y * bitmap.width + x) * 4;
  bitmap.data[p] = b;
  bitmap.data[p+1] = g;
  bitmap.data[p+2] = r;
  bitmap.data[p+3] = static_cast<char>(255);
}



// Get an RGB value from our x y pixel co-ordinate.
BitmapRGB GetRGB(Bitmap &bitmap, int x, int y) { 
  BitmapRGB colour;
  int p = (y * bitmap.width + x) * 4;
  colour.b = bitmap.data[p];
  colour.g = bitmap.data[p+1];
  colour.r = bitmap.data[p+2];
  return colour;
} 
  
// Write the bitmap out as an actual file
// TODO - replace options with width and height so this is more generic - same with the bitmap type
void WriteBitmap (Bitmap &bitmap, std::string output_path) {

  // TODO - Proper C++ style casts please!

  // Bitmap header
  struct __attribute__ ((packed)) bmp24_file_header
  {
    char        magic1;       // 'B'
    char        magic2;       // 'M'
    int         size;         // 0
    short int   reserved1;    // 0
    short int   reserved2;    // 0
    int         offbytes;      // 14 + 40
  };

  // Bitmap info header
  struct __attribute__ ((packed)) bmp24_info_header
  {
    int        size;                // 40
    int        width;               // pic.width
    int        height;              // pic.height
    short int   planes;             // 1
    short int   bit_count;          // 24
    int        compression;         // 0
    int        size_image;          // (pic.width * 3 + extra_bytes) * pic.height
    int        x_pels_per_meter;    // 2952
    int        y_pels_per_meter;    // 2952
    int        clr_used;            // 0
    int        clr_important;       // 0
  };


  ofstream myfile(output_path,  ios::out | std::ios::binary);

  //std::cout << sizeof(bmp24_file_header) << "," << sizeof(bmp24_info_header) << std::endl;

  bmp24_file_header header;
  bmp24_info_header info;

  int extra_bytes = (4 - (bitmap.width * 3) % 4) % 4;

  // Write the header to the bmp
  header.magic1 = 'B';
  header.magic2 = 'M';
  header.size = 0; // (options.width * 3 + extra_bytes) * options.height + 14 + 40;
  header.reserved1 = 0;
  header.reserved2 = 0;
  header.offbytes =  sizeof(bmp24_file_header) + sizeof(bmp24_info_header);

  // Write the header

  info.size = sizeof(bmp24_info_header);
  info.width = static_cast<long>(bitmap.width);
  info.height = static_cast<long>(bitmap.height);
  info.planes = 1;
  info.bit_count = 24;
  info.compression = 0;
  info.size_image = (bitmap.width * 3 + extra_bytes) * bitmap.height;
  info.x_pels_per_meter = 2952;
  info.y_pels_per_meter = 2952;
  info.clr_used = 0;
  info.clr_important = 0;

  myfile.write( reinterpret_cast<char *>(&header), sizeof(header));
  myfile.write( reinterpret_cast<char *>(&info), sizeof(info));

  for (int y = bitmap.height - 1; y >= 0; --y ){

    for (int x = 0; x < bitmap.width; ++x){
      BitmapRGB colour = GetRGB(bitmap, x, y);
      char r = colour.r;
      char g = colour.g;
      char b = colour.b;
      myfile << b << g << r;
    }
    for (int b = 0; b < extra_bytes; ++b){
      myfile << 'a';
    }
  }
  
  myfile.close();
}


}} // end namespaces
