// $Id: calibrate.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

/* calibrate.linux - a program to calibrate the camera tracker
   uses allegro graphic/games libraries
   Richard Reeve
   Thanks to Tim Chapman for some example code
*/

#include <iostream>

#include <cstring>
#include <linux/videodev.h>
#include <allegro.h>

#include "base.h"
#include "old.track.hh"
#include "Camera.hh"
#include "base.h"
#include "gui.h"

void check_capabilities();

int main( int argc, char *argv[] ) 
{
  int palette=-1;
  int speed=-1;
  bool preset=false;

  cameraSize.height=-1;
  cameraSize.width=-1;

#ifdef __NO_CAMERA
  dummyCamera=true;
#endif

  while (argc>1)
    {
      if ((!strcmp(argv[1], "--dummy")) || (!strcmp(argv[1], "-d")))
	{
	  dummyCamera=true;
	  if (cameraSize.height==-1)
	    {
	      cameraSize.width=160;
	      cameraSize.height=120;
	    }
	  if (speed==-1)
	    speed=30;
	  argc--;
	  argv++;
	}
      else if ((!strcmp(argv[1], "--size")) || (!strcmp(argv[1], "-s")))
	{
	  argc--;
	  argv++;
	  preset=true;
	  if ((!strcmp(argv[1], "max")) || (!strcmp(argv[1], "MAX")))
	    { // over the top figures
	      argc--;
	      argv++;
	      cameraSize.width=1000;
	      cameraSize.height=1000;
	      if (speed==-1)
		speed=31;
	    }
	  else if ((!strcmp(argv[1], "vga")) || (!strcmp(argv[1], "VGA")))
	    {
	      argc--;
	      argv++;
	      cameraSize.width=640;
	      cameraSize.height=480;
	      if (speed==-1)
		speed=15;
	    }
	  else if ((!strcmp(argv[1], "cif")) || (!strcmp(argv[1], "CIF")))
	    {
	      argc--;
	      argv++;
	      cameraSize.width=352;
	      cameraSize.height=288;
	      if (speed==-1)
		speed=30;
	    }
	  else if ((!strcmp(argv[1], "sif")) || (!strcmp(argv[1], "SIF")))
	    {
	      argc--;
	      argv++;
	      cameraSize.width=320;
	      cameraSize.height=240;
	      if (speed==-1)
		speed=30;
	    }
	  else if ((!strcmp(argv[1], "qcif")) || (!strcmp(argv[1], "QCIF")))
	    {
	      argc--;
	      argv++;
	      cameraSize.width=176;
	      cameraSize.height=144;
	      if (speed==-1)
		speed=30;
	    }
	  else if ((!strcmp(argv[1], "qsif")) || (!strcmp(argv[1], "QSIF")))
	    {
	      argc--;
	      argv++;
	      cameraSize.width=160;
	      cameraSize.height=120;
	      if (speed==-1)
		speed=30;
	    }
	  else if ((!strcmp(argv[1], "sqcif")) || (!strcmp(argv[1], "sQCIF")))
	    {
	      argc--;
	      argv++;
	      cameraSize.width=128;
	      cameraSize.height=96;
	      if (speed==-1)
		speed=30;
	    }
	  else
	    {
	      std::cerr << "Size not recognized!" << std::endl
		   << "Choose one of max, vga, cif, sif, qcif, qsif, sqcif"
		   << std::endl;
	      exit(1);
	    }
	}
      else if ((!strcmp(argv[1], "--palette")) || (!strcmp(argv[1], "-p")))
	{
	  argc--;
	  argv++;
	  preset=true;
	  if ((!strcmp(argv[1], "RGB24")) || (!strcmp(argv[1], "rgb24")))
	    {
	      argc--;
	      argv++;
	      palette=VIDEO_PALETTE_RGB24|0x80;
	      Depth=3;
	      RGBPlane=0;
	    }
	  else if ((!strcmp(argv[1], "BGR24")) || (!strcmp(argv[1], "bgr24")))
	    {
	      argc--;
	      argv++;
	      palette=VIDEO_PALETTE_RGB24;
	      Depth=3;
	      RGBPlane=2;
	    }
	  else if ((!strcmp(argv[1], "RGB32")) || (!strcmp(argv[1], "rgb32")))
	    {
	      argc--;
	      argv++;
	      palette=VIDEO_PALETTE_RGB32|0x80;
	      Depth=4;
	      RGBPlane=0;
	    }
	  else if ((!strcmp(argv[1], "BGR32")) || (!strcmp(argv[1], "bgr32")))
	    {
	      argc--;
	      argv++;
	      palette=VIDEO_PALETTE_RGB32;
	      Depth=4;
	      RGBPlane=2;
	    }
	  else
	    {
	      std::cerr << "Palette not recognized!" << std::endl
		   << "Choose one of rgb24, rgb32, bgr24, bgr32"
		   << std::endl;
	      exit(1);
	    }
	}
      else if ((!strcmp(argv[1], "--fps")) || (!strcmp(argv[1], "-f")))
	{
	  argc--;
	  argv++;
	  preset=true;
	  speed=atoi(argv[1]);
	  argc--;
	  argv++;
	}
      else if ((!strcmp(argv[1], "--stretch")) || (!strcmp(argv[1], "-S")))
	{
	  argc--;
	  argv++;
	  preset=true;
	  Stretch=atoi(argv[1]);
	  argc--;
	  argv++;
	}
      else if ((!strcmp(argv[1], "--help")) || (!strcmp(argv[1], "-h")))
	{
	  std::cout << "calibrate.linux: a camera calibration program using allegro"
	       << std::endl << std::endl << "-d (--dummy) to use a dummy camera"
	       << std::endl << "-s (--size) <size> to use image size at most <size>"
	       << std::endl << "-p (--palette) <p> to use palette <p>"
	       << std::endl << "-f (--fps) <speed> to run at <speed> fps at most"
	       << std::endl << "-S (--stretch) <n> to have pixel of size <n>"
	       << std::endl;
	  exit(0);
	}
      else
	{
	  std::cerr << "Argument not recognised: " << argv[1] << std::endl;
	  exit(1);
	}
    }
	  
  // Initialise camera
  if (!dummyCamera)
    {
      std::cout << "Initialising camera" << std::endl;
      if (!preset)
	gCamera = new Camera("/dev/video");
      else
	{
	  if (cameraSize.height==-1)
	    {
	      std::cerr << "image size must be set for preset mode" << std::endl;
	      exit(1);
	    }
	  else if (speed==-1)
	    {
	      std::cerr << "max speed must be set for preset mode" << std::endl;
	      exit(1);
	    }
	  else if (palette==-1)
	    {
	      std::cerr << "palette must be set for preset mode" << std::endl;
	      exit(1);
	    }
	  else
	    gCamera = new Camera("/dev/video", cameraSize, speed, palette);
	}
      gCamera->InitiateCapture();
      cameraSize=gCamera->getSize();
    }

  // Initialise allegro
  allegro_init();
  install_keyboard(); // install keyboard handler
  install_mouse();    // and mouse handler
  install_timer();    // timer needed for mouse
  std::cout << "Desktop colour depth = " << desktop_color_depth() << std::endl;

  set_color_depth(32);

  // set the screen to be twice the size of the camera
  // plus a bit at the bottom for comments
  //set_gfx_mode(GFX_AUTODETECT, cameraSize.width*2, cameraSize.height*2+24, 
  //	       0, 0); 
  set_gfx_mode(GFX_XWINDOWS, cameraSize.width*Stretch, 
	       cameraSize.height*Stretch+24, 
	       0, 0);
  clear(screen);
  
#ifdef DEBUG
  check_capabilities();
#endif //def DEBUG

  quick_calibrate();
  
  delete gCamera;
  allegro_exit(); // byebye allegro
  return 0;
}

END_OF_MAIN();

void check_capabilities()
{
  std::cout << "Graphics capabilities" << std::endl;
  if (gfx_capabilities&GFX_CAN_SCROLL)
    std::cout << "GFX_CAN_SCROLL" << std::endl;
  if (gfx_capabilities&GFX_CAN_TRIPLE_BUFFER)
    std::cout << "GFX_CAN_TRIPLE_BUFFER" << std::endl;
  if (gfx_capabilities&GFX_HW_CURSOR)
    std::cout << "GFX_HW_CURSOR" << std::endl;
  if (gfx_capabilities&GFX_HW_HLINE)
    std::cout << "GFX_HW_HLINE" << std::endl;
  if (gfx_capabilities&GFX_HW_HLINE_XOR)
    std::cout << "GFX_HW_HLINE_XOR" << std::endl;
  if (gfx_capabilities&GFX_HW_HLINE_SOLID_PATTERN)
    std::cout << "GFX_HW_HLINE_SOLID_PATTERN" << std::endl;
  if (gfx_capabilities&GFX_HW_HLINE_COPY_PATTERN)
    std::cout << "GFX_HW_HLINE_COPY_PATTERN" << std::endl;
  if (gfx_capabilities&GFX_HW_FILL)
    std::cout << "GFX_HW_FILL" << std::endl;
  if (gfx_capabilities&GFX_HW_FILL_XOR)
    std::cout << "GFX_HW_FILL_XOR" << std::endl;
  if (gfx_capabilities&GFX_HW_FILL_SOLID_PATTERN)
    std::cout << "GFX_HW_FILL_SOLID_PATTERN" << std::endl;
  if (gfx_capabilities&GFX_HW_FILL_COPY_PATTERN)
    std::cout << "GFX_HW_FILL_COPY_PATTERN" << std::endl;
  if (gfx_capabilities&GFX_HW_LINE)
    std::cout << "GFX_HW_LINE" << std::endl;
  if (gfx_capabilities&GFX_HW_LINE_XOR)
    std::cout << "GFX_HW_LINE_XOR" << std::endl;
  if (gfx_capabilities&GFX_HW_TRIANGLE)
    std::cout << "GFX_HW_TRIANGLE" << std::endl;
  if (gfx_capabilities&GFX_HW_TRIANGLE_XOR)
    std::cout << "GFX_HW_TRIANGLE_XOR" << std::endl;
  if (gfx_capabilities&GFX_HW_GLYPH)
    std::cout << "GFX_HW_GLYPH" << std::endl;
  if (gfx_capabilities&GFX_HW_VRAM_BLIT)
    std::cout << "GFX_HW_VRAM_BLIT" << std::endl;
  if (gfx_capabilities&GFX_HW_VRAM_BLIT_MASKED)
    std::cout << "GFX_HW_VRAM_BLIT_MASKED" << std::endl;
  if (gfx_capabilities&GFX_HW_MEM_BLIT)
    std::cout << "GFX_HW_MEM_BLIT" << std::endl;
  if (gfx_capabilities&GFX_HW_MEM_BLIT_MASKED)
    std::cout << "GFX_HW_MEM_BLIT_MASKED" << std::endl;
  if (gfx_capabilities&GFX_HW_SYS_TO_VRAM_BLIT)
    std::cout << "GFX_HW_SYS_TO_VRAM_BLIT" << std::endl;
  if (gfx_capabilities&GFX_HW_SYS_TO_VRAM_BLIT_MASKED)
    std::cout << "GFX_HW_SYS_TO_VRAM_BLIT_MASKED" << std::endl;
}
