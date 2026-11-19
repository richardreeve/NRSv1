// $Id: Camera.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

/* Class that wraps around camera (using V4L or potentially V4L2) to
 * produce images is standard formats */

#ifndef CAMERA_HH
#define CAMERA_HH

#include <linux/videodev.h>

struct Size
{ 
  Size() : width( 0 ), height( 0 ) {}
  Size( int w, int h ) : width( w ), height( h ) {}

  int width;
  int height;
};

extern bool dummyCamera;

class Camera
{
public:

  /* device is the device name of the camera */
  Camera(char *device); 
  /* size is screen size, speed in fps, colour eg.  VIDEO_PALETTE_RGB24 */
  Camera(char *device, Size size, int speed, int colour); 
  ~Camera();

  void InitiateCapture( int frames=0);/* frames is number of available
				       * frames to use (0=all) */
  
  Size getSize()
  {
    return iSize;
  }

  bool setSize(Size size);

  int getDepth()
  {
    return iDepth;
  }

  int getPlane()
  {
    return iPlane;
  }

  int getBytes()
  {
    return iBytes;
  }

  int getFramerate();
  bool setFramerate(int fps);

  int getFrames();
  bool setFrames(int frames);

  int getBrightness();
  bool canBrightness();
  bool setBrightness(int val);

  int getContrast();
  bool canContrast();
  bool setContrast(int val);

  int getHue();
  bool canHue();
  bool setHue(int val);

  int getColour();
  bool canColour();
  bool setColour(int val);

  int getWhiteness();
  bool canWhiteness();
  bool setWhiteness(int val);

  bool setPalette(int pal);
  bool canPalette(int pal);
  int getPalette();

  int ReadNextImage(); /* returns frame number */
  
  unsigned char *getPointer(int frame);

private:

  /// Stride (bytes per repeating chunk of data)
  int iDepth;
  /// Offset of relevant data inside chunk
  int iPlane;
  /// Number of bytes inside chunk
  int iBytes;

  int cam_fd; // file descriptor for camera

  struct video_capability vcap; /* capabilities of v4l device - name,
				 * type, channels, audios,
				 * (max/min)(width/height) */
  struct video_channel vchan;   // channel information
  struct video_window vwin;     /* details of camera image - width,
				 * height, flags (fps) */
  struct video_picture vpic;    /* tunable parameters of camera image
				 * - brightness, hue, colour,
				 * contrast, whiteness, depth, palette
				 * */

  struct video_mbuf vmbuf;      /* shared buffer info - size, frames,
                                 * offsets[] */
  struct video_mmap vmmap;      /* set next frame to capture - frame,
                                 * height, width, format (palette) */
  

  Size max_size, min_size, iSize; // max, min and current size of image

  int max_fps, current_fps;     // maximum and current speed of camera

  unsigned char *shared_buffer;         // shared image buffer
  int current_frame;            // current frame number in image buffer
  int iFrames;                  // Number of frames to use

  bool setVPIC();

  bool bri_poss, con_poss, hue_poss, col_poss, whi_poss;
};

#endif //ndef CAMERA_HH
