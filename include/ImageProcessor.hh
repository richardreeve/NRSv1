/* Class that tracks the robot in the camera image */
// $Id: ImageProcessor.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef IMAGEPROCESSOR_HH
#define IMAGEPROCESSOR_HH

#ifdef ALLEGRO_PRESENT
#include <allegro.h>
#endif //def ALLEGRO_PRESENT
#include <cmath>

#include "Camera.hh"

#define WIN_ROWS     10       // rows in search window
#define SIZEXY       64       // default size of tracking window
#define IGNORE_LEVEL 10       // level above bg which will be ignored
#define LIGHT_RADIUS 5
#define FIND_LOCK    10000
#define KEEP_LOCK    40000
#define MAX_SEARCHES 30
#define CLOSE        0.00025

struct Position
{
  Position() : bAngle(false), bPos(false) {}
  Position( double x, double y ) : iX(x), iY(y), bAngle(false), bPos(true) {}
  Position( double x, double y, double theta ) : iX(x), iY(y), iTheta(theta),
						 bAngle(true), bPos(true) {}

  bool hasPosition()
  {
    return bPos;
  }

  bool hasAngle()
  {
    return bAngle;
  }

  void setPos( double anX, double anY )
  {
    iX = anX;
    iY = anY;
    bPos = true;
    bAngle = false;
  }

  void setPos( double anX, double anY, double aTheta )
  {
    iX = anX;
    iY = anY;
    iTheta = aTheta;
    bPos = true;
    bAngle = true;
  }

  double getDistance( Position& aPos )
  {
    return sqrt( (aPos.iX - iX) * (aPos.iX - iX) +
		 (aPos.iY - iY) * (aPos.iY - iY) );
  }

  void losePos()
  {
    bPos = false;
    bAngle = false;
  }

  double iX;
  double iY;
  double iTheta;
  bool bAngle;
  bool bPos;
};

class ImageProcessor
{
public:


  ImageProcessor( Camera *aCamera, char *aFilename
#ifdef ALLEGRO_PRESENT
		  , bool isAllegro, bool isHighGraphics
#endif //def ALLEGRO_PRESENT
		  );
  
  ~ImageProcessor();

  void doTrack( bool newImage = true );

  Camera *getCamera()
  {
    return iCamera;
  }

  Position getPos()
  {
    return iPos;
  }

  bool hasRobot()
  {
    return getPos().hasPosition();
  }

  /// Set position of robot for next timeframe
  void setPos( int x, int y );

#ifdef ALLEGRO_PRESENT
  void draw( bool full = false );
#endif // def ALLEGRO_PRESENT

  /// calculate real position from camera position
  Position calcReal( double x, double y );

private:

  /// The camera images are coming from
  Camera *iCamera;

  /// current Position of target
  Position iPos;

  /// corresponding pixel
  Size iLock;
 
  /// current window top left
  Size iTopLeft;

  /// window size
  Size iWindowSize;

  /// background offset
  int iOffset;

  /// pointer to current image
  unsigned char *iFrame;

  /// coordinate transform
  double iTheta[6][2];

#ifdef ALLEGRO_PRESENT
  /// Standard RGB palette
  bool bRGB;

  BITMAP *iCameraImage;
  bool bAllegro;
  bool bHighGraphics;
  int iNormalColour;
  int iErrorColour;
  int iHighlightColour;
#endif //def ALLEGRO_PRESENT

};

#endif //ndef IMAGEPROCESSOR_HH
