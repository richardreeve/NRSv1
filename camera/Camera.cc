// $Id: Camera.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

/* Class that wraps around camera (using V4L or potentially V4L2) to
 * produce images is standard formats */

#include <iostream>
#include <string>

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <linux/videodev.h>
#include "pwc-ioctl.h"
#include "Camera.hh"

bool dummyCamera=false;

#ifdef __TESTIT

int main(int , char *)
{
   Camera *camera;
   
   camera = new Camera("/dev/video");
   camera->InitiateCapture();
   delete camera;
   return 0;
}

#endif //def __TESTIT

/* Initialise camera */
Camera::Camera(char *device):
  iDepth(0),
  iPlane(0),
  iBytes(0),
  current_frame(0),
  iFrames(0),
  bri_poss(false), 
  con_poss(false),
  hue_poss(false),
  col_poss(false),
  whi_poss(false)
{
  int i;
  bool valid;
  struct video_picture store;
  std::string name;

  cam_fd=open(device, O_RDWR);

  if (cam_fd < 0) 
    {
      perror("Camera::Camera - failed to open device");
      exit(0);
    }

  if (ioctl(cam_fd, VIDIOCGCAP, &vcap) < 0)
    {
      perror("Camera::Camera - error getting capabilities");
      exit(0);
    }
 
  max_size.width = vcap.maxwidth;
  max_size.height = vcap.maxheight;

  min_size.width = vcap.minwidth;
  min_size.width = vcap.minheight;

   std::cout << "Name: " << vcap.name << std::endl;

   if ((vcap.type & VID_TYPE_TUNER)!=0)
     std::cout << "VID_TYPE_TUNER" << std::endl;
   if ((vcap.type & VID_TYPE_TELETEXT)!=0)
     std::cout << "VID_TYPE_TELETEXT" << std::endl;
   if ((vcap.type & VID_TYPE_OVERLAY)!=0)
     std::cout << "VID_TYPE_OVERLAY" << std::endl;
   if ((vcap.type & VID_TYPE_CHROMAKEY)!=0)
     std::cout << "VID_TYPE_CHROMAKEY" << std::endl;
   if ((vcap.type & VID_TYPE_CLIPPING)!=0)
     std::cout << "VID_TYPE_CLIPPING" << std::endl;
   if ((vcap.type & VID_TYPE_FRAMERAM)!=0)
     std::cout << "VID_TYPE_FRAMERAM" << std::endl;
   if ((vcap.type & VID_TYPE_SCALES)!=0)
     std::cout << "VID_TYPE_SCALES" << std::endl;
   if ((vcap.type & VID_TYPE_MONOCHROME)!=0)
     std::cout << "VID_TYPE_MONOCHROME" << std::endl;
   if ((vcap.type & VID_TYPE_SUBCAPTURE)!=0)
     std::cout << "VID_TYPE_SUBCAPTURE" << std::endl;

   // Fatal if no capture to shared memory
   if ((vcap.type & VID_TYPE_CAPTURE)!=0)
     {
#ifdef DEBUG
       std::cout << "VID_TYPE_CAPTURE" << std::endl;
#endif //def DEBUG
     }
   else
     {
       std::cerr << "Camera::Camera - no capture to shared memory" << std::endl;
       exit(0);
     }

   // We expect 1 channel - if so, then ignore.
   if (vcap.channels>1)
     std::cout << "Channels: " << vcap.channels << std::endl;
#ifdef DEBUG
   std::cout << "Audios: " << vcap.audios << std::endl;
   std::cout << "maxwidth: " << vcap.maxwidth << std::endl;
   std::cout << "maxheight: " << vcap.maxheight << std::endl;
   std::cout << "minwidth: " << vcap.minwidth << std::endl;
   std::cout << "minheight: " << vcap.minheight << std::endl;
#endif //def DEBUG

   if (vcap.channels<1) // Must have at least 1 video channel!
     {
       std::cerr << "Camera::Camera - no video channels" << std::endl;
       exit(0);
     }
   
   valid=false;
   for (i=0; i<vcap.channels; i++)
     {
       vchan.channel=i;
#ifdef DEBUG
       std::cout << "VIDIOCGCHAN: get info on channel " << i <<  std::endl << std::endl;
#endif //def DEBUG
       if (ioctl(cam_fd, VIDIOCGCHAN, &vchan) < 0)
	 {
	   perror("Camera::Camera - error getting channel info");
	   exit(0);
	 }
       std::cout << vchan.name << " on channel " << vchan.channel << std::endl;
#ifdef DEBUG
       std::cout << "Tuners: " << vchan.tuners << std::endl;
       std::cout << "Norm:" << vchan.norm << std::endl;
#endif //def DEBUG
       if ((vchan.flags & VIDEO_VC_TUNER)!=0)
	 std::cout << "VIDEO_VC_TUNER" << std::endl;
       if ((vchan.flags & VIDEO_VC_AUDIO)!=0)
	 std::cout << "VIDEO_VC_AUDIO" << std::endl;
#ifdef VIDEO_VC_NORM
       if ((vchan.flags & VIDEO_VC_NORM)!=0)
	 std::cout << "VIDEO_VC_NORM" << std::endl;
#endif
       if ((vchan.type & VIDEO_TYPE_TV)!=0)
	 std::cout << "VIDEO_TYPE_TV" << std::endl;
       if ((vchan.type & VIDEO_TYPE_CAMERA)!=0)
	 {
	   // this is what we expect
	   //std::cout << "VIDEO_TYPE_CAMERA" << std::endl;
	   valid=true; // Must have at least one camera!
	 }
     }

#ifdef DEBUG
   std::cout << std::endl << "VIDIOCGWIN: get Window info" << std::endl << std::endl;
#endif //def DEBUG

   if (ioctl(cam_fd, VIDIOCGWIN, &vwin) < 0)
     {
       perror("Camera::Camera - error getting window info");
       exit(0);
     }
   
   iSize.width = vwin.width;
   iSize.height = vwin.height;
   current_fps = (vwin.flags & 0x3F);

#ifdef DEBUG
   std::cout << "Window x,y coords: " << vwin.x << ", " << vwin.y << std::endl;
   std::cout << "Window size: " << vwin.width << "x" << vwin.height << std::endl;
   std::cout << "Chroma Key: " << vwin.chromakey << std::endl;
   std::cout << "Flags: " << vwin.flags << std::endl;
   std::cout << "=> fps: " << ((vwin.flags>>16) & 0x3F) << std::endl;
#endif //def DEBUG

   // Go to lowest speed to allow change to max resolution
   for (i=0; (i <= 0x3F) &&  ((int)((vwin.flags>>16)&0x3F)!=i-1); i++)
     {
       vwin.flags&=~(0x3F<<16);
       vwin.flags+=i<<16;
       if (ioctl(cam_fd, VIDIOCSWIN, &vwin) < 0)
	 {
	   perror("Camera::Camera - error setting window info");
	   exit(0);
	 }
       if (ioctl(cam_fd, VIDIOCGWIN, &vwin) < 0)
	 {
	   perror("Camera::Camera - error getting window info");
	   exit(0);
	 }
     }

   valid=false;

   // get resolution from user
   while (!valid)
     {
       std::cout << "Possible resolutions are:" << std::endl << std::endl;
       std::cout << "- MAX - maximum available is: " << vcap.maxwidth << "x" 
	    << vcap.maxheight << std::endl;
       if (vcap.maxwidth>=640)
	 std::cout << "- VGA = 640x480" << std::endl;
       if (vcap.maxwidth>=352)
	 std::cout << "- CIF = 352x288" << std::endl;
       if (vcap.maxwidth>=320)
	 std::cout << "- SIF = 320x240" << std::endl;
       if (vcap.maxwidth>=176)
	 std::cout << "- QCIF = 176x144" << std::endl;
       if (vcap.maxwidth>=160)
	 std::cout << "- QSIF = 160x120" << std::endl;
       if (vcap.maxwidth>=128)
	 std::cout << "- sQCIF = 128x96" << std::endl;
       std::cout << std::endl << "Please type in the name of the resolution you desire:";
       std::cin >> name;
       if (name == "MAX")
	 {
	   std::cout << std::endl << "Using 640x480 - maximum speed should be 15fps"
		<< " (may claim 19)"
		<< std::endl;
	   // Go to max resolution
	   vwin.width=vcap.maxwidth;
	   vwin.height=vcap.maxheight;
	   valid=true;
	 }
       if (name == "VGA")
	 {
	   // should run at 15fps at this size...
	   std::cout << std::endl << "Using 640x480 - maximum speed should be 15fps"
		<< " (may claim 19)"
		<< std::endl;
	   vwin.width=640;
	   vwin.height=480;
	   valid=true;
	 }
       if (name == "CIF")
	 {
	   // should run at 30fps at this size...
	   std::cout << std::endl << "Using 352x288 - maximum speed should be 30fps"
		<< std::endl;
	   vwin.width=352;
	   vwin.height=288;
	   valid=true;
	 }
       if (name == "SIF")
	 {
	   std::cout << std::endl << "Using 320x240 - maximum speed should be 30fps"
		<< std::endl;
	   vwin.width=320;
	   vwin.height=240;
	   valid=true;
	 }
       if (name == "QCIF")
	 {
	   std::cout << std::endl << "Using 176x144 - maximum speed should be 30fps"
		<< std::endl;
	   vwin.width=176;
	   vwin.height=144;
	   valid=true;
	 }
       if (name == "QSIF")
	 {
	   std::cout << std::endl << "Using 160x120 - maximum speed should be 30fps"
		<< std::endl;
	   vwin.width=160;
	   vwin.height=120;
	   valid=true;
	 }
       if (name == "sQCIF")
	 {
	   std::cout << std::endl << "Using 128x96 - maximum speed should be 30fps"
		<< std::endl;
	   vwin.width=128;
	   vwin.height=96;
	   valid=true;
	 }
       if (!valid)
	 std::cout << "Sorry, that size is not available" << std::endl << std::endl;
     }

   if (ioctl(cam_fd, VIDIOCSWIN, &vwin) < 0)
     {
       perror("Camera::Camera - error setting window info");
       exit(0);
     }
   if (ioctl(cam_fd, VIDIOCGWIN, &vwin) < 0)
     {
       perror("Camera::Camera - error getting window info");
       exit(0);
     }
   if ((name == "MAX") &&
       ((vwin.width!=(unsigned)max_size.width) ||
	(vwin.height!=(unsigned)max_size.height)))
     {
       std::cout << "Mismatch in window sizes - max window size is: " << vwin.width 
	    << "x" << vwin.height << std::endl;
       max_size.width = vwin.width;
       max_size.height = vwin.height;
     }
   iSize.width=vwin.width;
   iSize.height=vwin.height;

   // Go to max speed at this resolution
#ifdef DEBUG
   std::cout << "Going to max speed at this resolution." << std::endl;
#endif //def DEBUG
   // technically 0x3F is max speed (=63fps), but really 30, so we'll try 31
   for (i=0x1F; (i >= 0) &&  ((int)((vwin.flags>>16)&0x3F)!=i+1); i--)
     {
       vwin.flags&=~(0x3F<<16);
       vwin.flags+=i<<16;
       if (ioctl(cam_fd, VIDIOCSWIN, &vwin) < 0)
	 {
	   perror("Camera::Camera - error setting window info");
	   exit(0);
	 }
       if (ioctl(cam_fd, VIDIOCGWIN, &vwin) < 0)
	 {
	   perror("Camera::Camera - error getting window info");
	   exit(0);
	 }
     }

   std::cout << "Max speed is " << ((vwin.flags>>16) & 0x3F) << "fps"
	<< std::endl << std::endl;
   max_fps = ((vwin.flags>>16) & 0x3F);
   current_fps=max_fps;

#ifdef DEBUG
   std::cout << std::endl << "VIDIOCGPICT: get Picture info" << std::endl << std::endl;
#endif //def DEBUG

   if (ioctl(cam_fd, VIDIOCGPICT, &vpic) < 0)
     {
       perror("Camera::Camera - error getting picture info");
       exit(0);
     }

#ifdef DEBUG
   std::cout << "Brightness: " << vpic.brightness << std::endl;
   std::cout << "Hue: " << vpic.hue << std::endl;
   std::cout << "Colour: " << vpic.colour << std::endl;
   std::cout << "Contrast: " << vpic.contrast << std::endl;
   std::cout << "Whiteness: " << vpic.whiteness << std::endl;
   std::cout << "Depth: " << vpic.depth << std::endl;
   

   if (vpic.palette == VIDEO_PALETTE_GREY)
     std::cout << "VIDEO_PALETTE_GREY" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_HI240)
     std::cout << "VIDEO_PALETTE_HI240" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_RGB565)
     std::cout << "VIDEO_PALETTE_RGB565" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_RGB555)
     std::cout << "VIDEO_PALETTE_RGB555" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_RGB24)
     std::cout << "VIDEO_PALETTE_RGB24" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_RGB32)
     std::cout << "VIDEO_PALETTE_RGB32" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_YUV422)
     std::cout << "VIDEO_PALETTE_YUV422" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_YUYV)
     std::cout << "VIDEO_PALETTE_YUYV" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_UYVY)
     std::cout << "VIDEO_PALETTE_UYVY" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_YUV420)
     std::cout << "VIDEO_PALETTE_YUV420" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_YUV411)
     std::cout << "VIDEO_PALETTE_YUV411" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_RAW)
     std::cout << "VIDEO_PALETTE_RAW" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_YUV422P)
     std::cout << "VIDEO_PALETTE_YUV422P" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_YUV411P)
     std::cout << "VIDEO_PALETTE_YUV411P" << std::endl;
#endif //def DEBUG

   store.brightness=vpic.brightness;
   store.hue=vpic.hue;
   store.colour=vpic.colour;
   store.contrast=vpic.contrast;
   store.whiteness=vpic.whiteness;
   store.depth=vpic.depth;
   store.palette=vpic.palette;
   
#ifdef DEBUG
   std::cout << std::endl << "VIDIOCSPICT: setting picture info" << std::endl << std::endl;
#endif //def DEBUG

   if (store.brightness>32767)
     vpic.brightness=65535;
   else
     vpic.brightness=0;

   if (store.hue>32767)
     vpic.hue=65535;
   else
     vpic.hue=0;
   
   if (store.colour>32767)
     vpic.colour=65535;
   else
     vpic.colour=0;
   
   if (store.contrast>32767)
     vpic.contrast=65535;
   else
     vpic.contrast=0;
   
   if (store.whiteness>32767)
     vpic.whiteness=65535;
   else
     vpic.whiteness=0;

   if (!setVPIC()) exit(0);

   if (vpic.brightness==store.brightness)
     std::cout << "Brightness fixed" << std::endl;
   else
     {
       std::cout << "Brightness controllable" << std::endl;
       bri_poss=true;
     }

   if(vpic.hue==store.hue)
     std::cout << "Hue fixed" << std::endl;
   else
     {
       std::cout << "Hue controllable" << std::endl;
       hue_poss=true;
     }

   if(vpic.colour==store.colour)
     std::cout << "Colour fixed" << std::endl;
   else
     {
       std::cout << "Colour controllable" << std::endl;
       col_poss=true;
     }

   if(vpic.contrast==store.contrast)
     std::cout << "Contrast fixed" << std::endl;
   else
     {
       std::cout << "Contrast controllable" << std::endl;
       con_poss=true;
     }

   if(vpic.whiteness==store.whiteness)
     std::cout << "Whiteness fixed" << std::endl;
   else
     {
       std::cout << "Whiteness controllable" << std::endl;
       whi_poss=true;
     }

   if (ioctl(cam_fd, VIDIOCSPICT, &store) < 0)
     {
       perror("CAMERA::CAMERA - error setting picture info");
       exit(0);
     }
   if (ioctl(cam_fd, VIDIOCGPICT, &vpic) < 0)
     {
       perror("Camera::Camera - error getting picture info");
       exit(0);
     }

   std::cout << std::endl << "Palettes available:" << std::endl;
   if (canPalette(VIDEO_PALETTE_GREY))
     std::cout << "GREY possible" << std::endl;
   if (canPalette(VIDEO_PALETTE_HI240))
     std::cout << "HI240 possible" << std::endl;
   if (canPalette(VIDEO_PALETTE_RGB565))
     std::cout << "RGB565 possible" << std::endl;
   if (canPalette(VIDEO_PALETTE_RGB555))
     std::cout << "RGB555 possible" << std::endl;
   if (canPalette(VIDEO_PALETTE_RGB24))
     {
       std::cout << "RGB24 possible" << std::endl;
       std::cout << "BGR24 possible" << std::endl;
     }
   if (canPalette(VIDEO_PALETTE_RGB32))
     {
       std::cout << "RGB32 possible" << std::endl;
       std::cout << "BGR32 possible" << std::endl;
     }
   if (canPalette(VIDEO_PALETTE_YUV422))
     std::cout << "YUV422 possible" << std::endl;
   if (canPalette(VIDEO_PALETTE_YUYV))
     std::cout << "YUYV possible" << std::endl;
   if (canPalette(VIDEO_PALETTE_UYVY))
     std::cout << "UYVY possible" << std::endl;
   if (canPalette(VIDEO_PALETTE_YUV420))
     std::cout << "YUV420 possible" << std::endl;
   if (canPalette(VIDEO_PALETTE_YUV411))
     std::cout << "YUV411 possible" << std::endl;
   if (canPalette(VIDEO_PALETTE_RAW))
     std::cout << "RAW possible" << std::endl;
   if (canPalette(VIDEO_PALETTE_YUV422P))
     std::cout << "YUV422P possible" << std::endl;
   if (canPalette(VIDEO_PALETTE_YUV411P))
     std::cout << "YUV411P possible" << std::endl;
  
   setPalette(store.palette);

   valid=false;
   while (!valid)
     {
       std::cout << "Please type the name of the palette you require:";
       std::cin >> name;
       if (name == "GREY")
	 if (setPalette(VIDEO_PALETTE_GREY))
	   {
	     std::cout << "Using VIDEO_PALETTE_GREY" << std::endl << std::endl;
	     std::cout << "I'm afraid I don't know iDepth and iPlane"
		       << std::endl;
	     valid=true;
	   }
       if (name == "HI240")
	 if (setPalette(VIDEO_PALETTE_HI240))
	   {
	     std::cout << "Using VIDEO_PALETTE_HI240" << std::endl
		       << std::endl;
	     std::cout << "I'm afraid I don't know iDepth and iPlane"
		       << std::endl;
	     valid=true;
	   }
       if (name == "RGB565")
	 if (setPalette(VIDEO_PALETTE_RGB565))
	   {
	     std::cout << "Using VIDEO_PALETTE_RGB565" << std::endl
		       << std::endl;
	     std::cout << "I'm afraid I don't know iDepth and iPlane"
		       << std::endl;
	     valid=true;
	   }
       if (name == "RGB555")
	 if (setPalette(VIDEO_PALETTE_RGB555))
	   {
	     std::cout << "Using VIDEO_PALETTE_RGB555" << std::endl
		       << std::endl;
	     std::cout << "I'm afraid I don't know iDepth and iPlane"
		       << std::endl;
	     valid=true;
	   }
       if (name == "RGB24")
	 if (setPalette(VIDEO_PALETTE_RGB24|0x80))
	   {
	     iDepth = 3;
	     iPlane = 0;
	     iBytes = 1;
	     std::cout << "Using VIDEO_PALETTE_RGB24" << std::endl
		       << std::endl;
	     valid=true;
	   }
       if (name == "RGB32")
	 if (setPalette(VIDEO_PALETTE_RGB32|0x80))
	   {
	     iDepth = 4;
	     iPlane = 0;
	     iBytes = 1;
	     std::cout << "Using VIDEO_PALETTE_RGB32" << std::endl
		       << std::endl;
	     valid=true;
	   }
       if (name == "BGR24")
	 if (setPalette(VIDEO_PALETTE_RGB24))
	   {
	     iDepth = 3;
	     iPlane = 2;
	     iBytes = 1;
	     std::cout << "Using VIDEO_PALETTE_BGR24" << std::endl
		       << std::endl;
	     valid=true;
	   }
       if (name == "BGR32")
	 if (setPalette(VIDEO_PALETTE_RGB32))
	   {
	     iDepth = 4;
	     iPlane = 2;
	     iBytes = 1;
	     std::cout << "Using VIDEO_PALETTE_BGR32" << std::endl
		       << std::endl;
	     valid=true;
	   }
       if (name == "YUV422")
	 if (setPalette(VIDEO_PALETTE_YUV422))
	   {
	     std::cout << "Using VIDEO_PALETTE_YUV422" << std::endl
		       << std::endl;
	     std::cout << "I'm afraid I don't know iDepth and iPlane"
		       << std::endl;
	     valid=true;
	   }
       if (name == "YUYV")
	 if (setPalette(VIDEO_PALETTE_YUYV))
	   {
	     std::cout << "Using VIDEO_PALETTE_YUYV" << std::endl << std::endl;
	     std::cout << "I'm afraid I don't know iDepth and iPlane"
		       << std::endl;
	     valid=true;
	   }
       if (name == "UYVY")
	 if (setPalette(VIDEO_PALETTE_UYVY))
	   {
	     std::cout << "Using VIDEO_PALETTE_UYVY" << std::endl << std::endl;
	     std::cout << "I'm afraid I don't know iDepth and iPlane"
		       << std::endl;
	     valid=true;
	   }
       if (name == "YUV420")
	 if (setPalette(VIDEO_PALETTE_YUV420))
	   {
	     iDepth = 6;
	     iPlane = 0;
	     iBytes = 4;
	     std::cout << "Using VIDEO_PALETTE_YUV420" << std::endl
		       << std::endl;
	     valid=true;
	   }
       if (name == "YUV411")
	 if (setPalette(VIDEO_PALETTE_YUV411))
	   {
	     std::cout << "Using VIDEO_PALETTE_YUV411" << std::endl
		       << std::endl;
	     std::cout << "I'm afraid I don't know iDepth and iPlane"
		       << std::endl;
	     valid=true;
	   }
       if (name == "RAW")
	 if (setPalette(VIDEO_PALETTE_RAW))
	   {
	     std::cout << "Using VIDEO_PALETTE_RAW" << std::endl << std::endl;
	     std::cout << "I'm afraid I don't know iDepth and iPlane"
		       << std::endl;
	     valid=true;
	   }
       if (name == "YUV422P")
	 if (setPalette(VIDEO_PALETTE_YUV422P))
	   {
	     std::cout << "Using VIDEO_PALETTE_YUV422P" << std::endl
		       << std::endl;
	     std::cout << "I'm afraid I don't know iDepth and iPlane"
		       << std::endl;
	     valid=true;
	   }
       if (name == "YUV411P")
	 if (setPalette(VIDEO_PALETTE_YUV411P))
	   {
	     std::cout << "Using VIDEO_PALETTE_YUV411P" << std::endl
		       << std::endl;
	     std::cout << "I'm afraid I don't know iDepth and iPlane"
		       << std::endl;
	     valid=true;
	   }
       if (name == "YUV420P")
	 if (setPalette(VIDEO_PALETTE_YUV420P))
	   {
	     std::cout << "Using VIDEO_PALETTE_YUV420P" << std::endl
		       << std::endl;
	     iDepth = 4;
	     iPlane = 0;
	     iBytes = 4;
	   }
       if (name == "YUV410P")
	 if (setPalette(VIDEO_PALETTE_YUV410P))
	   {
	     std::cout << "Using VIDEO_PALETTE_YUV410P" << std::endl
		       << std::endl;
	     std::cout << "I'm afraid I don't know iDepth and iPlane"
		       << std::endl;
	   }
       if (name == "PLANAR")
	 if (setPalette(VIDEO_PALETTE_PLANAR))
	   {
	     std::cout << "Using VIDEO_PALETTE_PLANAR" << std::endl
		       << std::endl;
	     std::cout << "I'm afraid I don't know iDepth and iPlane"
		       << std::endl;
	   }
       
       if (!valid)
	 std::cout << "Sorry, that palette is not available" << std::endl
		   << std::endl;
     }

#ifdef DEBUG
   std::cout << std::endl << "VIDIOCGMBUF: get shared buffer info" << std::endl
	     << std::endl;
#endif //def DEBUG

   if (ioctl(cam_fd, VIDIOCGMBUF, &vmbuf) < 0)
     {
       perror("Camera::Camera - error getting shared buffer info");
       exit(0);
     }

#ifdef DEBUG
   std::cout << "Size of shared buffer: " << vmbuf.size << std::endl;
#endif //def DEBUG

   std::cout << "Number of frames in shared buffer: " << vmbuf.frames
	     << std::endl;
   if ((iFrames==0) || (iFrames>=vmbuf.frames))
     {
       std::cout << "Using all of the frames" << std::endl;
       iFrames=vmbuf.frames;
     }
   else
     std::cout << "Using " << iFrames << " frames" << std::endl;

#ifdef DEBUG
   for (i=0; i<iFrames; i++)
     std::cout << "Offset of frame " << i << " is " << vmbuf.offsets[i]
	       << std::endl;
#endif //def DEBUG

   shared_buffer=(unsigned char*) mmap(0, vmbuf.size, PROT_READ|PROT_WRITE, 
			       MAP_SHARED, cam_fd, 0);
   if (shared_buffer<0)
     {
       perror("Camera::Camera - mmap failed");
       exit(0);
     }
}

Camera::Camera(char *device, Size size, int speed, int palette):
  iDepth(0),
  iPlane(0),
  iBytes(0),
  current_frame(0),
  iFrames(0),
  bri_poss(false), 
  con_poss(false),
  hue_poss(false),
  col_poss(false),
  whi_poss(false)
{
  int i;
  bool valid;
  struct video_picture store;
  std::string name;

  cam_fd=open(device, O_RDWR);

  if (cam_fd < 0) 
    {
      perror("Camera::Camera - failed to open device");
      exit(0);
    }

  if (ioctl(cam_fd, VIDIOCGCAP, &vcap) < 0)
    {
      perror("Camera::Camera - error getting capabilities");
      exit(0);
    }
 
  max_size.width = vcap.maxwidth;
  max_size.height = vcap.maxheight;

  min_size.width = vcap.minwidth;
  min_size.width = vcap.minheight;

   std::cout << "Name: " << vcap.name << std::endl;

   if ((vcap.type & VID_TYPE_TUNER)!=0)
     std::cout << "VID_TYPE_TUNER" << std::endl;
   if ((vcap.type & VID_TYPE_TELETEXT)!=0)
     std::cout << "VID_TYPE_TELETEXT" << std::endl;
   if ((vcap.type & VID_TYPE_OVERLAY)!=0)
     std::cout << "VID_TYPE_OVERLAY" << std::endl;
   if ((vcap.type & VID_TYPE_CHROMAKEY)!=0)
     std::cout << "VID_TYPE_CHROMAKEY" << std::endl;
   if ((vcap.type & VID_TYPE_CLIPPING)!=0)
     std::cout << "VID_TYPE_CLIPPING" << std::endl;
   if ((vcap.type & VID_TYPE_FRAMERAM)!=0)
     std::cout << "VID_TYPE_FRAMERAM" << std::endl;
   if ((vcap.type & VID_TYPE_SCALES)!=0)
     std::cout << "VID_TYPE_SCALES" << std::endl;
   if ((vcap.type & VID_TYPE_MONOCHROME)!=0)
     std::cout << "VID_TYPE_MONOCHROME" << std::endl;
   if ((vcap.type & VID_TYPE_SUBCAPTURE)!=0)
     std::cout << "VID_TYPE_SUBCAPTURE" << std::endl;

   // Fatal if no capture to shared memory
   if ((vcap.type & VID_TYPE_CAPTURE)!=0)
     {
#ifdef DEBUG
       std::cout << "VID_TYPE_CAPTURE" << std::endl;
#endif //def DEBUG
     }
   else
     {
       std::cerr << "Camera::Camera - no capture to shared memory" << std::endl;
       exit(0);
     }

   // We expect 1 channel - if so, then ignore.
   if (vcap.channels>1)
     std::cout << "Channels: " << vcap.channels << std::endl;
#ifdef DEBUG
   std::cout << "Audios: " << vcap.audios << std::endl;
   std::cout << "maxwidth: " << vcap.maxwidth << std::endl;
   std::cout << "maxheight: " << vcap.maxheight << std::endl;
   std::cout << "minwidth: " << vcap.minwidth << std::endl;
   std::cout << "minheight: " << vcap.minheight << std::endl;
#endif //def DEBUG

   if (vcap.channels<1) // Must have at least 1 video channel!
     {
       std::cerr << "Camera::Camera - no video channels" << std::endl;
       exit(0);
     }
   
   valid=false;
   for (i=0; i<vcap.channels; i++)
     {
       vchan.channel=i;
#ifdef DEBUG
       std::cout << "VIDIOCGCHAN: get info on channel " << i <<  std::endl << std::endl;
#endif //def DEBUG
       if (ioctl(cam_fd, VIDIOCGCHAN, &vchan) < 0)
	 {
	   perror("Camera::Camera - error getting channel info");
	   exit(0);
	 }
       std::cout << vchan.name << " on channel " << vchan.channel << std::endl;
#ifdef DEBUG
       std::cout << "Tuners: " << vchan.tuners << std::endl;
       std::cout << "Norm:" << vchan.norm << std::endl;
#endif //def DEBUG
       if ((vchan.flags & VIDEO_VC_TUNER)!=0)
	 std::cout << "VIDEO_VC_TUNER" << std::endl;
       if ((vchan.flags & VIDEO_VC_AUDIO)!=0)
	 std::cout << "VIDEO_VC_AUDIO" << std::endl;
#ifdef VIDEO_VC_NORM
       if ((vchan.flags & VIDEO_VC_NORM)!=0)
	 std::cout << "VIDEO_VC_NORM" << std::endl;
#endif
       if ((vchan.type & VIDEO_TYPE_TV)!=0)
	 std::cout << "VIDEO_TYPE_TV" << std::endl;
       if ((vchan.type & VIDEO_TYPE_CAMERA)!=0)
	 {
	   // this is what we expect
	   //std::cout << "VIDEO_TYPE_CAMERA" << std::endl;
	   valid=true; // Must have at least one camera!
	 }
     }

#ifdef DEBUG
   std::cout << std::endl << "VIDIOCGWIN: get Window info" << std::endl << std::endl;
#endif //def DEBUG

   if (ioctl(cam_fd, VIDIOCGWIN, &vwin) < 0)
     {
       perror("Camera::Camera - error getting window info");
       exit(0);
     }
   
   iSize.width = vwin.width;
   iSize.height = vwin.height;
   current_fps = ((vwin.flags>>16) & 0x3F);

#ifdef DEBUG
   std::cout << "Window x,y coords: " << vwin.x << ", " << vwin.y << std::endl;
   std::cout << "Window size: " << vwin.width << "x" << vwin.height << std::endl;
   std::cout << "Chroma Key: " << vwin.chromakey << std::endl;
   std::cout << "Flags: " << vwin.flags << std::endl;
   std::cout << "=> fps: " << ((vwin.flags>>16) & 0x3F) << std::endl;
#endif //def DEBUG

   // Go to lowest speed to allow change to max resolution
   for (i=0; (i <= 0x3F) &&  ((int)((vwin.flags>>16)&0x3F)!=i-1); i++)
     {
       vwin.flags&=~(0x3F<<16);
       vwin.flags+=i<<16;
       if (ioctl(cam_fd, VIDIOCSWIN, &vwin) < 0)
	 {
	   perror("Camera::Camera - error setting window info");
	   exit(0);
	 }
       if (ioctl(cam_fd, VIDIOCGWIN, &vwin) < 0)
	 {
	   perror("Camera::Camera - error getting window info");
	   exit(0);
	 }
     }

   // Use preset resolution
   if (vcap.maxwidth<size.width)
     {
       if ((speed!=30) || (size.width!=1000)) // you chose max size...
	 {
	   std::cerr << "Size chosen too big, shrinking to " 
		<< vcap.maxwidth << " x " << vcap.maxheight << std::endl;
	 }
       size.width=vcap.maxwidth;
       size.height=vcap.maxheight;
     }
   vwin.width=size.width;
   vwin.height=size.height;
   
   if (ioctl(cam_fd, VIDIOCSWIN, &vwin) < 0)
     {
       perror("Camera::Camera - error setting window info");
       exit(0);
     }
   if (ioctl(cam_fd, VIDIOCGWIN, &vwin) < 0)
     {
       perror("Camera::Camera - error getting window info");
       exit(0);
     }
   if ((vwin.width!=(unsigned int)size.width) 
       || (vwin.height!=(unsigned int)size.height))
     {
       std::cout << "Mismatch in window sizes - window size set as: " << vwin.width 
	    << "x" << vwin.height << std::endl;
     }
   iSize.width=vwin.width;
   iSize.height=vwin.height;

   for (i=speed; (i >= 0) &&  ((int)((vwin.flags>>16)&0x3F)!=i+1); i--)
     {
       vwin.flags&=~(0x3F<<16);
       vwin.flags+=i<<16;
       if (ioctl(cam_fd, VIDIOCSWIN, &vwin) < 0)
	 {
	   perror("Camera::Camera - error setting window info");
	   exit(0);
	 }
       if (ioctl(cam_fd, VIDIOCGWIN, &vwin) < 0)
	 {
	   perror("Camera::Camera - error getting window info");
	   exit(0);
	 }
     }

   if (((int)((vwin.flags>>16)&0x3F)!=speed) && (speed!=31)) // not for max
     {
       std::cout << "Mismatch in speeds - speed set to: " 
	    << ((vwin.flags>>16)&0x3F)
	    << "fps" << std::endl << std::endl;
     }
   
   max_fps = ((vwin.flags>>16) & 0x3F);
   current_fps=max_fps;
   
#ifdef DEBUG
   std::cout << std::endl << "VIDIOCGPICT: get Picture info" << std::endl << std::endl;
#endif //def DEBUG

   if (ioctl(cam_fd, VIDIOCGPICT, &vpic) < 0)
     {
       perror("Camera::Camera - error getting picture info");
       exit(0);
     }

#ifdef DEBUG
   std::cout << "Brightness: " << vpic.brightness << std::endl;
   std::cout << "Hue: " << vpic.hue << std::endl;
   std::cout << "Colour: " << vpic.colour << std::endl;
   std::cout << "Contrast: " << vpic.contrast << std::endl;
   std::cout << "Whiteness: " << vpic.whiteness << std::endl;
   std::cout << "Depth: " << vpic.depth << std::endl;
   

   if (vpic.palette == VIDEO_PALETTE_GREY)
     std::cout << "VIDEO_PALETTE_GREY" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_HI240)
     std::cout << "VIDEO_PALETTE_HI240" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_RGB565)
     std::cout << "VIDEO_PALETTE_RGB565" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_RGB24)
     std::cout << "VIDEO_PALETTE_RGB24" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_RGB32)
     std::cout << "VIDEO_PALETTE_RGB32" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_RGB555)
     std::cout << "VIDEO_PALETTE_RGB555" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_YUV422)
     std::cout << "VIDEO_PALETTE_YUV422" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_YUYV)
     std::cout << "VIDEO_PALETTE_YUYV" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_UYVY)
     std::cout << "VIDEO_PALETTE_UYVY" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_YUV420)
     std::cout << "VIDEO_PALETTE_YUV420" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_YUV411)
     std::cout << "VIDEO_PALETTE_YUV411" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_RAW)
     std::cout << "VIDEO_PALETTE_RAW" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_YUV422P)
     std::cout << "VIDEO_PALETTE_YUV422P" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_YUV411P)
     std::cout << "VIDEO_PALETTE_YUV411P" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_YUV420P)
     std::cout << "VIDEO_PALETTE_YUV420P" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_YUV410P)
     std::cout << "VIDEO_PALETTE_YUV410P" << std::endl;
   if (vpic.palette == VIDEO_PALETTE_PLANAR)
     std::cout << "VIDEO_PALETTE_PLANAR" << std::endl;
#endif //def DEBUG

   store.brightness=vpic.brightness;
   store.hue=vpic.hue;
   store.colour=vpic.colour;
   store.contrast=vpic.contrast;
   store.whiteness=vpic.whiteness;
   store.depth=vpic.depth;
   store.palette=vpic.palette;
   
#ifdef DEBUG
   std::cout << std::endl << "VIDIOCSPICT: setting picture info" << std::endl << std::endl;
#endif //def DEBUG
   
   if (store.brightness>32767)
     vpic.brightness=65535;
   else
     vpic.brightness=0;

   if (store.hue>32767)
     vpic.hue=65535;
   else
     vpic.hue=0;
   
   if (store.colour>32767)
     vpic.colour=65535;
   else
     vpic.colour=0;
   
   if (store.contrast>32767)
     vpic.contrast=65535;
   else
     vpic.contrast=0;
   
   if (store.whiteness>32767)
     vpic.whiteness=65535;
   else
     vpic.whiteness=0;

   if (!setVPIC()) exit(0);

   if (vpic.brightness==store.brightness)
     std::cout << "Brightness fixed" << std::endl;
   else
     {
       std::cout << "Brightness controllable" << std::endl;
       bri_poss=true;
     }

   if(vpic.hue==store.hue)
     std::cout << "Hue fixed" << std::endl;
   else
     {
       std::cout << "Hue controllable" << std::endl;
       hue_poss=true;
     }

   if(vpic.colour==store.colour)
     std::cout << "Colour fixed" << std::endl;
   else
     {
       std::cout << "Colour controllable" << std::endl;
       col_poss=true;
     }

   if(vpic.contrast==store.contrast)
     std::cout << "Contrast fixed" << std::endl;
   else
     {
       std::cout << "Contrast controllable" << std::endl;
       con_poss=true;
     }

   if(vpic.whiteness==store.whiteness)
     std::cout << "Whiteness fixed" << std::endl;
   else
     {
       std::cout << "Whiteness controllable" << std::endl;
       whi_poss=true;
     }

   if (ioctl(cam_fd, VIDIOCSPICT, &store) < 0)
     {
       perror("CAMERA::CAMERA - error setting picture info");
       exit(0);
     }
   if (ioctl(cam_fd, VIDIOCGPICT, &vpic) < 0)
     {
       perror("Camera::Camera - error getting picture info");
       exit(0);
     }

   struct pwc_whitebalance aWB;
   
   aWB.mode=PWC_WB_INDOOR;

   ioctl( cam_fd, VIDIOCPWCSAWB, &aWB );

   std::cout << "red balance = " << aWB.read_red << std::endl;
   std::cout << "blue balance = " << aWB.read_blue << std::endl;

   std::cout << "Trying to restore stored palette " << store.palette 
	     << std::endl;

   setPalette(store.palette);

   // Using preset palette
   if (!setPalette(palette))
     {
       std::cout << "Failed to use requested palette" << std::endl;
       exit(1);
     }

   if (vpic.palette == VIDEO_PALETTE_GREY)
     {
       std::cout << "Using VIDEO_PALETTE_GREY" << std::endl << std::endl;
       std::cout << "I'm afraid I don't know iDepth and iPlane" << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_HI240)
     {
       std::cout << "Using VIDEO_PALETTE_HI240" << std::endl << std::endl;
       std::cout << "I'm afraid I don't know iDepth and iPlane" << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_RGB565)
     {
       std::cout << "Using VIDEO_PALETTE_RGB565" << std::endl << std::endl;
       std::cout << "I'm afraid I don't know iDepth and iPlane" << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_RGB555)
     {
       std::cout << "Using VIDEO_PALETTE_RGB555" << std::endl << std::endl;
       std::cout << "I'm afraid I don't know iDepth and iPlane" << std::endl;
     }
   else if (vpic.palette == (VIDEO_PALETTE_RGB24|0x80))
     {
       iDepth = 3;
       iPlane = 0;
       iBytes = 1;
       std::cout << "Using VIDEO_PALETTE_RGB24" << std::endl << std::endl;
     }
   else if (vpic.palette == (VIDEO_PALETTE_RGB32|0x80))
     {
       iDepth = 4;
       iPlane = 0;
       iBytes = 1;
       std::cout << "Using VIDEO_PALETTE_RGB32" << std::endl << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_RGB24)
     {
       iDepth = 3;
       iPlane = 2;
       iBytes = 1;
       std::cout << "Using VIDEO_PALETTE_BGR24" << std::endl << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_RGB32)
     {
       iDepth = 4;
       iPlane = 2;
       iBytes = 1;
       std::cout << "Using VIDEO_PALETTE_BGR32" << std::endl << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_YUV422)
     {
       std::cout << "Using VIDEO_PALETTE_YUV422" << std::endl << std::endl;
       std::cout << "I'm afraid I don't know iDepth and iPlane" << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_YUYV)
     {
       std::cout << "Using VIDEO_PALETTE_YUYV" << std::endl << std::endl;
       std::cout << "I'm afraid I don't know iDepth and iPlane" << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_UYVY)
     {
       std::cout << "Using VIDEO_PALETTE_UYVY" << std::endl << std::endl;
       std::cout << "I'm afraid I don't know iDepth and iPlane" << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_YUV420)
     {
       iDepth = 6;
       iPlane = 0;
       iBytes = 4;
       std::cout << "Using VIDEO_PALETTE_YUV420" << std::endl << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_YUV411)
     {
       std::cout << "Using VIDEO_PALETTE_YUV411" << std::endl << std::endl;
       std::cout << "I'm afraid I don't know iDepth and iPlane" << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_RAW)
     {
       std::cout << "Using VIDEO_PALETTE_RAW" << std::endl << std::endl;
       std::cout << "I'm afraid I don't know iDepth and iPlane" << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_YUV422P)
     {
       std::cout << "Using VIDEO_PALETTE_YUV422P" << std::endl << std::endl;
       std::cout << "I'm afraid I don't know iDepth and iPlane" << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_YUV411P)
     {
       std::cout << "Using VIDEO_PALETTE_YUV411P" << std::endl << std::endl;
       std::cout << "I'm afraid I don't know iDepth and iPlane" << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_YUV420P)
     {
       std::cout << "Using VIDEO_PALETTE_YUV420P" << std::endl << std::endl;
       iDepth = 4;
       iPlane = 0;
       iBytes = 4;
     }
   else if (vpic.palette == VIDEO_PALETTE_YUV410P)
     {
       std::cout << "Using VIDEO_PALETTE_YUV410P" << std::endl << std::endl;
       std::cout << "I'm afraid I don't know iDepth and iPlane" << std::endl;
     }
   else if (vpic.palette == VIDEO_PALETTE_PLANAR)
     {
       std::cout << "Using VIDEO_PALETTE_PLANAR" << std::endl << std::endl;
       std::cout << "I'm afraid I don't know iDepth and iPlane" << std::endl;
     }
   else
     {
       std::cout << "I don't know the palette or the iDepth and iPlane"
		 << std::endl;
     }

#ifdef DEBUG
   std::cout << std::endl << "VIDIOCGMBUF: get shared buffer info" << std::endl << std::endl;
#endif //def DEBUG
   
   if (ioctl(cam_fd, VIDIOCGMBUF, &vmbuf) < 0)
     {
       perror("Camera::Camera - error getting shared buffer info");
       exit(0);
     }

#ifdef DEBUG
   std::cout << "Size of shared buffer: " << vmbuf.size << std::endl;
#endif //def DEBUG

   std::cout << "Number of frames in shared buffer: " << vmbuf.frames << std::endl;
   if ((iFrames==0) || (iFrames>=vmbuf.frames))
     {
       std::cout << "Using all of the frames" << std::endl;
       iFrames=vmbuf.frames;
     }
   else
     std::cout << "Using " << iFrames << " frames" << std::endl;

#ifdef DEBUG
   for (i=0; i<iFrames; i++)
     std::cout << "Offset of frame " << i << " is " << vmbuf.offsets[i] << std::endl;
#endif //def DEBUG

   shared_buffer=(unsigned char*) mmap(0, vmbuf.size, PROT_READ|PROT_WRITE, 
			       MAP_SHARED, cam_fd, 0);
   if (shared_buffer<0)
     {
       perror("Camera::Camera - mmap failed");
       exit(0);
     }
}


Camera::~Camera()
{
  if (shared_buffer)
    munmap(shared_buffer, vmbuf.size);
  if (cam_fd)
    close(cam_fd);
}

bool Camera::setVPIC()
{
  if (ioctl(cam_fd, VIDIOCSPICT, &vpic) < 0)
    {
      perror("Camera::setVPIC - error setting picture info");
      return(false);
    }
  if (ioctl(cam_fd, VIDIOCGPICT, &vpic) < 0)
    {
      perror("Camera::setVPIC - error getting picture info");
      return(false);
    }
  return(true);
}

void Camera::InitiateCapture(int frames)
{
  int i;

  if (shared_buffer)
    {
#ifdef DEBUG
      std::cout << std::endl << "VIDIOCSYNC: Synchronise buffers" << std::endl << std::endl;
#endif //def DEBUG
      for (i=0; i<iFrames; i++)
	{
	  if (ioctl(cam_fd, VIDIOCSYNC, &i) < 0)
	    {
	      perror("Camera::InitiateCapture - error syncing buffer");
	    }
	}
      munmap(shared_buffer, vmbuf.size);
    }

#ifdef DEBUG
  std::cout << std::endl << "VIDIOCGMBUF: get shared buffer info" << std::endl << std::endl;
#endif //def DEBUG
  if (ioctl(cam_fd, VIDIOCGMBUF, &vmbuf) < 0)
    {
      perror("Camera::Camera - error getting shared buffer info");
      exit(0);
    }
  
#ifdef DEBUG
  std::cout << "Size of shared buffer: " << vmbuf.size << std::endl;
  std::cout << "Number of frames in shared buffer: " << vmbuf.frames << std::endl;
#endif //def DEBUG
  if ((iFrames==0) || (iFrames>=vmbuf.frames))
    {
#ifdef DEBUG
      std::cout << "Using all " << vmbuf.frames << " of the frames" << std::endl;
#endif //def DEBUG
      iFrames=vmbuf.frames;
    }
  else
    {
#ifdef DEBUG
      std::cout << "Using " << iFrames << " frames" << std::endl;
#endif //def DEBUG
    }

#ifdef DEBUG
  for (i=0; i<iFrames; i++)
    std::cout << "Offset of frame " << i << " is " << vmbuf.offsets[i] << std::endl;
#endif //def DEBUG
  
  shared_buffer=(unsigned char *) mmap(0, vmbuf.size, PROT_READ|PROT_WRITE, 
			       MAP_SHARED, cam_fd, 0);
  if (shared_buffer<0)
    {
      perror("Camera::InitiateCapture - mmap failed");
      exit(0);
    }

#ifdef DEBUG
  std::cout << std::endl << "VIDIOCMCAPTURE: Initiate capture" << std::endl << std::endl;
#endif //def DEBUG

  for (i=0; i<iFrames; i++)
    {
      vmmap.frame=i;
      vmmap.height=iSize.height;
      vmmap.width=iSize.width;
      vmmap.format=vpic.palette;
      if (ioctl(cam_fd, VIDIOCMCAPTURE, &vmmap) < 0)
	{
	  perror("Camera::InitiateCapture - error initiating capture");
	}
    }
  current_frame=-1;
}

bool Camera::setSize(Size size)
{
  if ((size.width==iSize.width) && (size.height==iSize.height))
    return true;
  else
    {
      //  vwin.width=size.width;
      //  vwin.height=size.height;
      return false;
    }
}



//  int Camera::getDepth()
//  {
//    return vpic.depth;
//  }

int Camera::getFramerate()
{
  return current_fps;
}

bool Camera::setFramerate(int fps)
{
  return false;
}

int Camera::getFrames()
{
  return iFrames;
}

bool Camera::setFrames(int frames)
{
  return false;
}

int Camera::getBrightness()
{
  return vpic.brightness;
}

bool Camera::canBrightness()
{
  return bri_poss;
}

bool Camera::setBrightness(int val)
{
  if (bri_poss)
    {
      vpic.brightness=val;
      if (!setVPIC()) return false;
      return true;
    }
  else
    return false;
}

int Camera::getContrast()
{
  return vpic.contrast;
}

bool Camera::canContrast()
{
  return con_poss;
}
bool Camera::setContrast(int val)
{
  if (con_poss)
    {
      vpic.contrast=val;
      if (!setVPIC()) return false;
      return true;
    }
  else
    return false;
}

int Camera::getHue()
{
  return vpic.hue;
}

bool Camera::canHue()
{
  return hue_poss;
}
bool Camera::setHue(int val)
{
  if (hue_poss)
    {
      vpic.hue=val;
      if (!setVPIC()) return false;
      return true;
    }
  else
    return false;
}

int Camera::getColour()
{
  return vpic.colour;
}

bool Camera::canColour()
{
  return col_poss;
}
bool Camera::setColour(int val)
{
  if (col_poss)
    {
      vpic.colour=val;
      if (!setVPIC()) return false;
      return true;
    }
  else
    return false;
}

int Camera::getWhiteness()
{
  return vpic.whiteness;
}

bool Camera::canWhiteness()
{
  return whi_poss;
}
bool Camera::setWhiteness(int val)
{
  if (whi_poss)
    {
      vpic.whiteness=val;
      if (!setVPIC()) return false;
      return true;
    }
  else
    return false;
}

int Camera::getPalette()
{
  return vpic.palette;
}

bool Camera::canPalette(int pal)
{
  int curr = getPalette();
  bool can = setPalette( pal );
  setPalette( curr );
  return can;
}

bool Camera::setPalette(int pal)
{
  vpic.palette = pal;
  if (!setVPIC()) return false;
  if (vpic.palette == pal)
    return true;
  else
    return false;
  return false;
}

int Camera::ReadNextImage()
{
  int next_frame=current_frame+1;
  if (next_frame>=iFrames)
    next_frame=0;

  if (!shared_buffer)
    {
      perror("Camera::ReadNextImage - shared buffer not yet set up");
      exit(0);
    }

  if (current_frame>=0)
    {
      vmmap.frame=current_frame;
      vmmap.height=iSize.height;
      vmmap.width=iSize.width;
      vmmap.format=vpic.palette;

      if (ioctl(cam_fd, VIDIOCMCAPTURE, &vmmap) < 0)
	{
	  perror("Camera::ReadNextImage - error freeing buffer");
	}
    }

  if (ioctl(cam_fd, VIDIOCSYNC, &next_frame) < 0)
    {
      perror("Camera::ReadNextImage - error syncing buffer");
    }

  current_frame=next_frame;
  
  return(current_frame);
}

unsigned char *Camera::getPointer(int frame)
{
  return shared_buffer + vmbuf.offsets[frame];
}
