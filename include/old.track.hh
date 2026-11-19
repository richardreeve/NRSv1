// $Id: old.track.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

/* This is the header file for tracking software written by Richard
 * Reeve at the Department of Psychology, University of Stirling. It
 * is loosely based on some code by Henrik Lund and Esther de Ves
 * Cuenca, adapted by Rens Kortmann at the Department of Artificial
 * Intelligence, University of Edinburgh. This software may be
 * redistributed without charge. This notice should be preserved,
 * though do add your name if you modify the code. */

#ifndef TRACKER_HH
#define TRACKER_HH

#include "base.h"
#include "Camera.hh"

#define DEPTH       3        /* default number of bytes per pixel */
#define RGBPLANE    0        /* default colour plane to measure intensity in:
				B=0, G=1, R=2. could be RGB ;) */
#define FRAME_MAX   2        /* number of frames in shared memory */
#define WIN_ROWS    10       /* rows in search window */
#define ROWS_MAX    29       /* WIN_ROWS * ROWS_MAX = ROWS */
#define SIZEXY      64       /* default size of tracking window */
#define INDEX       200      /* default number of recording slots */
#define THRESHOLD   10       /* default background light threshold */
#define LIGHT_THRESHOLD 500  /* led light threshold */
#define LIGHT_RADIUS 5
#define LIGHT_FIND_THRESHOLD 20000  /* led light threshold */
#define LIGHT_KEEP_THRESHOLD 80000  /* led light threshold */
#define LIGHTSIZE 12         /* size of area counted for each leds moment */
#define LEDDIST 20           /* min inter LED distance */

#define RECORDS_MAX 5000     /* maximum number of records */

#define STRETCH     2        /* Default size of pixel for calibration */

#define DEFAULT_POS_FILE "/home/richardr/data/data.res"
#define DEFAULT_THETA_FILE "/home/richardr/data/theta"
#define DEFAULT_GRIDPOINTS_FILE "/home/richardr/data/gridpoints.dat"

#define COUNTS 1000 // default track time

/* This structure stores all the information about the frame being
 * processed */

typedef struct
{
  unsigned long intensity;
  float posx;
  float posy;
  float  posx1;
  float  posy1;
  float  posx2;
  float  posy2;
  float mytime;
} Path;

/* Structure where the real positions in centimeters are stored */

typedef struct 
{
  float posx;
  float posy;
  float posx1;
  float posy1;
  float posx2;
  float posy2;
} CMPosition;


extern Camera *gCamera;
extern Size cameraSize;

/* Colour depth, and position of red - defaults in header file */
extern int Depth;
extern int RGBPlane;

/* default filename for storing positions */
extern char defaultName[NAME_LEN];

/* calibration values should be stored in these files */
extern char defaultData[NAME_LEN];

/* Pixel size for calibration mode */
extern int Stretch;

/* Function declarations */

bool quick_calibrate();

bool process_image(float *realx, float *realy, float *alpha,
		   bool allegro, bool slow_graphics, bool find_body);

int moments(unsigned char *frame, int rowini, int columini, 
	    int sizex, int sizey, unsigned long *intensity, 
	    float *pposx,float *pposy, float *pposx1,float *pposy1, 
	    float *pposx2, float *pposy2, float *palpha);

int ReadMatrixTheta(char *filename,double theta[6][2]);

void pixel_to_real( float pixel_x, float pixel_y, 
		    float *real_x, float *real_y);

#endif //ndef TRACKER_HH

