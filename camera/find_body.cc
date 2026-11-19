// $Id: find_body.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

/* This is tracking software written by Richard Reeve and adapted by
 * Tim Chapman at the Department of Psychology, University of
 * Stirling. This software may be redistributed without charge. This
 * notice should be preserved, though do add your name if you modify
 * the code. */

#ifdef ALLEGRO_PRESENT
#include <allegro.h>
#include "gui.h"
#endif //def ALLEGRO_PRESENT
#include <iostream>
#include <valarray>

#include "camera.hh"
#include "track.hh"
/* Definition of global variables */

/* Camera */
Camera *gCamera;
CameraSize cameraSize;

int normalColour;
int errorColour;
int highlightColour;

/* Available modes */
int calibrationMode=FALSE;
int waitFlag=FALSE; // wait for key before commencing tracking behaviour
int rawFlag=FALSE; // raw mode does not bother to do perspective correction transform

/* Possible input parameters - defaults in header file */
int windowSize=SIZEXY;
int bgThreshold=THRESHOLD;

/* Colour depth, and position of red - defaults in header file */
int Depth=DEPTH;
int RGBPlane=RGBPLANE;

int Stretch=STRETCH;

/* default filename for storing positions */
char defaultName[NAME_LEN]=DEFAULT_POS_FILE;

/* calibration values should be stored in these files */
char defaultData[NAME_LEN]=DEFAULT_THETA_FILE;

/* Data stored */
Path pathWindow[RECORDS_MAX];
int caliIndex=0;







/* This function processes the image returning true 
   if we have found the robot */
bool process_image(float *realxp, float *realyp, float *anglep,
		   bool allegro, bool slow_graphics)
{
  /*initialization of static variables*/
  static bool first_time=true;
  static int init_row=0;
  static int init_col=0;
  static int num_rows=WIN_ROWS;
  static int num_cols=0; // overridden on first run through
  bool found_lights=false;
  
  unsigned char *frame;
  long tot_r=0;
  long tot_g=0;
  long tot_b=0;
  static unsigned char av_r, av_g, av_b;
  unsigned long intensity;
  float  posx, posy;


#ifdef ALLEGRO_PRESENT
  int i,j, current_pos;
  unsigned char r, g, b;
  static BITMAP *camera_image;
#endif //def ALLEGRO_PRESENT

  int val;
  float posx1, posy1, posx2, posy2;

  /* Read the file with the matrix obtained from the camera calibration. */
  if (first_time)
    {
#ifdef ALLEGRO_PRESENT
      if (allegro)
	{
	  camera_image=create_bitmap(cameraSize.width, cameraSize.height);
	  normalColour=makecol(0x40, 0x80, 0xff);
	  errorColour=makecol(0xff, 0x0, 0x0);
	  highlightColour=makecol(0x20, 0xb0, 0x20);
	}
#endif // def ALLEGRO_PRESENT
      num_cols=cameraSize.width;
    }
  
  //frame = gCamera->GetPointer(frame_num); 
  frame = gCamera->GetPointer( gCamera->ReadNextImage() ); 
  
//    std::valarray<unsigned char> basic(frame, 
//  				cameraSize.width*cameraSize.height*Depth);
//    std::slice_array<unsigned char> desire = basic[slice(plane, 
//  						  cameraSize.width * 
//  						  cameraSize.height,
//  						  depth)];
  
  std::valarray<int> plane(cameraSize.width*cameraSize.height);
  for (i=0; i<cameraSize.width*cameraSize.height; i++)
    {
      plane[i]=frame[i*Depth+plane_no];
    }



  if (first_time)
    {
      for (i=0; i<cameraSize.height*cameraSize.width; i++)
	{
	  tot_r+=frame[i*Depth+RGBPlane];
	  tot_g+=frame[i*Depth+1];
	  tot_b+=frame[i*Depth+2-RGBPlane];
	}
      av_r=static_cast<unsigned char>(tot_r/
				      (cameraSize.height*cameraSize.width));
      av_g=static_cast<unsigned char>(tot_g/
				      (cameraSize.height*cameraSize.width));
      av_b=static_cast<unsigned char>(tot_b/
				      (cameraSize.height*cameraSize.width));
      std::cout << "[R G B] = [" << static_cast<int>(av_r) << " "
	   << static_cast<int>(av_g) << " " 
	   << static_cast<int>(av_b) << "]" << std::endl;
    }

  /* Next function processes an image window whose initial coordinates
   * are (init_row,init_col) and whose size is num_rows*num_cols
   * (rows*cols). The function calculates three different moments in
   * the window. One of them is the global moment in the window
   * (posx,posy). This global moment is used to detect the robot in
   * the image. When the robot is detected, the global intensity,
   * which is stored in the 'intensity' variable, is higher than
   * LIGHT_THRESHOLD. In this case, the function calculates the
   * positions of two lights in the robot. */

      for (i=0; i<cameraSize.height*cameraSize.width; i++)
	{
	  frame[i*Depth+RGBPlane] = (frame[i*Depth+RGBPlane] > av_r ?
				     frame[i*Depth+RGBPlane] - av_r : 0);
	}
  val=moments(frame, init_row, init_col, num_rows, num_cols, &intensity,
	      &posx, &posy, &posx1, &posy1, &posx2, &posy2, anglep);

  if (!found_lights && (intensity > LIGHT_FIND_THRESHOLD))
    {
      int ir = init_row;
      int ic = init_col;
      int nr = num_rows;
      int nc = num_cols;
      num_rows=windowSize;
      num_cols=windowSize;
      // These instructions calculate the next position of the
      // window to process. The centre of this window is the last
      // moment calculated (posx,posy). 
      if((init_row=(int)(posx-(num_rows/2)))<0)
	init_row=0;
      if((posx+(num_rows/2)>cameraSize.height))
	init_row=cameraSize.height-num_rows;
      if((init_col=(int)(posy-(num_cols/2)))<0)
	init_col=0;
      if((posy+(num_cols/2)>cameraSize.width))
	init_col=cameraSize.width-num_cols;
      val=moments(frame, init_row, init_col, num_rows, num_cols, &intensity,
		  &posx, &posy, &posx1, &posy1, &posx2, &posy2, anglep);
      if (intensity < LIGHT_KEEP_THRESHOLD)
	{
	  init_row = ir;
	  init_col = ic;
	  num_rows = nr;
	  num_cols = nc;
	}
      else
	found_lights=true;
    }
  
  if (intensity < LIGHT_KEEP_THRESHOLD)
    found_lights = false;
 
  //  printf("fn=%d si=%d\n",frame_num, structureIndex);
  
#ifdef ALLEGRO_PRESENT
  // set allegro to true to plot image to an allegro bitmap
  if (allegro)
    {
      current_pos = 0;
      
      for (j=0; j<cameraSize.height; j++) 
	{
	  for (i=0; i<cameraSize.width; i++)
	    {	
//  	      r = (frame[current_pos+RGBPlane] > av_r ?
//  		   frame[current_pos+RGBPlane] - av_r : 0);
	      r = frame[current_pos+RGBPlane];
	      g = (frame[current_pos+1] > av_g ? 
		   frame[current_pos+1] - av_g : 0);
	      b = (frame[current_pos+2-RGBPlane] > av_b ?
		   frame[current_pos+2-RGBPlane] - av_b : 0);
	      putpixel(camera_image, i, j, makecol(r, g, b));
	      current_pos+=Depth;
	    }
	}
      if (slow_graphics || first_time)
	{
	  blit(camera_image, screen, 
	       0, 0, 0, 0, cameraSize.width, cameraSize.height);
	  first_time=false;
	}
      else
	blit(camera_image, screen, init_col, init_row, init_col, init_row,
	     num_cols, num_rows);
      rect(screen, init_col, init_row, init_col+num_cols, init_row+num_rows,
	   normalColour);
    }
#endif //def ALLEGRO_PRESENT

//    printf("( %d , %d ) %d x %d - %lu\n", init_row, init_col, 
//  	 num_rows, num_cols, intensity);
  
//    if (intensity>LIGHT_THRESHOLD)
//      found_lights = true;
//    else
//      found_lights = false;
  
  if (found_lights)
    {
      // calculate raw->real coords using calibrated theta table in datafile
      pixel_to_real(posx, posy, realxp, realyp);

#ifdef ALLEGRO_PRESENT
      // various outputs of the tracking data
      //      printf("ptr:[%f, %f] (%f, %f @ %f) - %lu\n",
      //             realxp, realyp, posx, posy, anglep, intensity);
      if (allegro)
	{
	  circle(screen, (int) posy, (int) posx, 
		 LIGHT_RADIUS, highlightColour);
	  line(screen, (int) posy1, (int) posx1, (int) posy2, (int) posx2, 
	       errorColour);
	  //putpixel( screen, (int)posy, (int)posx, 0xffffff );
	  //putpixel( screen, (int)posy1, (int)posx1, makecol( 255,0,0) );
	  //putpixel( screen, (int)posy2, (int)posx2, makecol(0,255,0) );
	  textprintf(screen, font, 0, cameraSize.height, normalColour,
		     "%f %f ", *realxp, *realyp);
	}
#endif //def ALLEGRO_PRESENT

      // We will be here when the lights have been detected, so
      // tracking begins 
      num_rows=windowSize;
      num_cols=windowSize;
      // These instructions calculate the next position of the
      // window to process. The centre of this window is the last
      // moment calculated (posx,posy). 
      if((init_row=(int)(posx-(num_rows/2)))<0)
	init_row=0;
      if((posx+(num_rows/2)>cameraSize.height))
	init_row=cameraSize.height-num_rows;
      if((init_col=(int)(posy-(num_cols/2)))<0)
	init_col=0;
      if((posy+(num_cols/2)>cameraSize.width))
	init_col=cameraSize.width-num_cols;
    } 
  else
    {
      //      printf("Not found - %lu\n",intensity);
      // Returning to scan. In the case that lights get lost, the
      // frame is scanned to find the lights again. This means
      // that the process window must be changed: window size will
      // be WIN_ROWS*cameraSize.width pixels, and the initial row will be
      // the closest multiple of WIN_ROWS to the last position. 
      init_row=WIN_ROWS*((1+(init_row/WIN_ROWS))%ROWS_MAX);
      init_col=0;
      num_rows=WIN_ROWS;
      num_cols=cameraSize.width;
    }

  return found_lights;
}


/* This function calculates moments in a window. There are three
 * different moments : two of them are the moments for each light
 * situated on the robot, the other one is the global moment in the
 * window. Parameters: char *frame: address of the frame to process.
 * int init_row, int init_col: initial row and column of the window to
 * process. int num_rows, num_cols: row number and column number , in
 * this order. unsigned long *intensity: The sum of intensity in the
 * window; float *pposx, float *pposy: Pointers where the x position
 * and y position of the moment are stored. float *pposx1,float
 * *pposx2,float *pposy1, float *pposy2: The positions of the moments
 * for each detected light in the robot. Return: (1) if successful. *
 * (-1) otherwise. */
int moments(unsigned char *frame, int init_row, int init_col, 
	    int num_rows, int num_cols, unsigned long *pintensity,
	    float *pposx, float *pposy, float *pposx1, float *pposy1,
	    float *pposx2, float *pposy2, float *palpha)
{
  int i, j,k,m;
  unsigned long &intensity = *pintensity;
  unsigned char  value;
  long x1=0;
  long y1=0;
  long x2=0;
  long y2=0;
  long xi=0;
  long yi=0;
  double xii=0;
  double yii=0;
  double xy=0;
  double bxx,byy,bxy,ramda,mean_x,mean_y;
  int lsize=LIGHTSIZE;
  int dist=LEDDIST;
  unsigned long windowI[2];
  int flag=FALSE,flag2=FALSE;
  int centerx1 = 0,centerx2,centery1 = 0,centery2;
  unsigned int highest=0;
  intensity=0;
  int current_pos=0;

  for(i=0;i<2;i++)
      windowI[i]=0;

  if(frame==NULL)
    {
      std::cout << "Invalid buffer" << std::endl;
      return(-1);
    }

  /* look for lights */
  for( i = init_row, 
	 current_pos = // Get to end of previous row - j will move to start
	 ((i - 1) * cameraSize.width + init_col + num_cols) * Depth +RGBPlane;
       i < (init_row + num_rows); i++ )
    {
      for( j = init_col, current_pos += (cameraSize.width - num_cols) * Depth;
	   j < (init_col + num_cols); j++, current_pos += Depth )
	{
	  value = frame[current_pos];
	  if (value >= bgThreshold)
	    {
	      //printf("%d, %d - %lu\n", (int)value, bgThreshold, intensity);
	      if (value > highest) {
		highest=value; 
		//printf("high=%d \n", value);
	      }
	      intensity+=value;
	      xi+=value*i;
	      yi+=value*j;
	    }
	}
    }

  /* calculate second moments */
  mean_x=(int)(((float)xi)/intensity); 
  mean_y=(int)(((float)yi)/intensity);
  
  for(i=init_row, current_pos=(i*cameraSize.width-cameraSize.width+init_col+num_cols)*Depth;
      i<(init_row+num_rows);i++)
    {
      for(j=init_col, current_pos+=(cameraSize.width-num_cols)*Depth;
	  j<(init_col+num_cols);j++, current_pos+=Depth)
	{
	  value=frame[current_pos + RGBPlane];
	  if(value>=bgThreshold)
	    {
	      xii +=value*(i-mean_x)*(i-mean_x);
	      yii +=value*(j-mean_y)*(j-mean_y);
	      xy += value*(i-mean_x)*(j-mean_y);
	    }
	}
    }

  if(intensity)
    {
      (*pposx)=((float)xi)/intensity;
      (*pposy)=((float)yi)/intensity;
      bxx = xii /intensity;
      byy = yii /intensity;
      bxy = xy /intensity;
      ramda=0.5*(-sqrt((bxx+byy)*(bxx+byy)-4*(bxx*byy-bxy*bxy)) +bxx+byy);
      *palpha=-atan((ramda - bxx)/(bxy));
    }
  
  /* Looking for the highest intensities (about 200 ) Find and
   * distinguish the two LEDs */

  if((intensity>LIGHT_KEEP_THRESHOLD) &&
     (num_cols<cameraSize.width)) /* for testing */
    {  
      for(i=init_row, current_pos=((i-1)*cameraSize.width
				   +init_col+num_cols)*Depth;
	  i<(init_row+num_rows);i++)
	for(j=init_col, current_pos+=(cameraSize.width-num_cols)*Depth;
	    j<(init_col+num_cols);j++, current_pos+=Depth)
	  {
	    if( (!flag2)&&(!flag)&&( (value=frame[current_pos+RGBPlane])==highest)) 
	      {
		
		centerx1=i;
		centery1=j;
		
		for(k=(centerx1-(lsize/2));k<(centerx1+(lsize/2));k++)
		  for(m=(centery1-(lsize/2));m<(centery1+(lsize/2));m++)
		    {
		      value=frame[((k*cameraSize.width)+m)*Depth+RGBPlane];
		      if(value>=bgThreshold) {
			windowI[0]+=value;
			x1+=value*k;
			y1+=value*m;
		      }
		    }
		(*pposx1)=((float)x1)/windowI[0];
		(*pposy1)=((float)y1)/windowI[0]; 
		flag=TRUE;
		highest=0;
		for(k=init_row;k<(init_row+num_rows);k++)
		  {
		    for(m=init_col;m<(init_col+num_cols);m++)
		      {
			value=frame[((k*cameraSize.width)+m)*Depth+RGBPlane];
			if( ((k<(centerx1-dist)) || (k>(centerx1+dist))) 
			    || ((m<(centery1-dist)) || (m>(centery1+dist))) )
			  if (value>highest)
			    highest=value;
		      }
		  }
	      }
	  }
      
      for(i=init_row, current_pos=(i*cameraSize.width-cameraSize.width+init_col+num_cols)*Depth;
	  i<(init_row+num_rows);i++)
	for(j=init_col, current_pos+=(cameraSize.width-num_cols)*Depth;
	    j<(init_col+num_cols);j++, current_pos+=Depth)
	  {
	    if( (!flag2)&&(flag)&&((value=frame[current_pos+RGBPlane])==highest) )
	      {
		
		if( ((i<(centerx1-dist)) || (i>(centerx1+dist))) 
		    || ((j<(centery1-dist)) || (j>(centery1+dist))) )
		  {
		    
		    centerx2=i;
		    centery2=j;
		    for(k=(centerx2-(lsize/2));k<(centerx2+(lsize/2));k++)
		      for(m=(centery2-(lsize/2));m<(centery2+(lsize/2));m++)
			{
			  value=frame[((k*cameraSize.width)+m)*Depth+RGBPlane];
			  if(value>=bgThreshold) {
			    windowI[1]+=value;
			    x2+=value*k;
			    y2+=value*m;
			  }
			}
		    (*pposx2)=((float)x2)/windowI[1];
		    (*pposy2)=((float)y2)/windowI[1]; 
		    flag2=TRUE;
		  }
		
		
	      } /* if*/
	    
	  }/* for*/  
      
    }/*first if*/
  
  return(1);
}


/* This function calculates the product between two matrices
 * X=I*THETA,in order to know the real position in centimeters. THETA
 * is the matrix which is found in the camera calibration. Parameters:
 * char *filename: File name to read. float theta[10][2]: Matrix to
 * store the data from the calibration. Return:(1) if it is
 * successful. (-1) if it is not successful. */

int ReadMatrixTheta(char *filename,double theta[6][2])
{
  FILE *in;
  
  std::cout << "Loading theta file: " << filename << std::endl;
  if( (in=fopen(filename,"rw"))==NULL)
    {
      std::cout << "Error opening data file " << in << std::endl;
      return(-1);
    }
  if( fscanf(in,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
	     &(theta[0][0]),&(theta[0][1]),&(theta[1][0]),&(theta[1][1]),
	     &(theta[2][0]),&(theta[2][1]),&(theta[3][0]),&(theta[3][1]),
	     &(theta[4][0]),&(theta[4][1]),&(theta[5][0]),&(theta[5][1]) )!=12)
    {
      std::cout << "Error reading file" << std::endl;
      return(-1);
    }
  
  fclose(in);
  return(1);
}

void pixel_to_real( float pixel_x, float pixel_y, 
		    float *real_x, float *real_y) 
{
  static double theta[6][2];
  static bool first_time=true;

  /* Read the file with the matrix obtained from the camera calibration. */
  if (first_time)
    {
      if (!ReadMatrixTheta(defaultData,theta))
        {
          std::cout << "Error reading theta file" << std::endl;
          exit(1);
        }
      first_time=false;
    }
  // convert between pixel coords and real (calibrated) coords. tpc.
      *real_x= theta[0][0]*1+
	theta[1][0]*pixel_y+
	theta[2][0]*pixel_x+
	theta[3][0]*pixel_x*pixel_y+
	theta[4][0]*pixel_y*pixel_y+ 
	theta[5][0]*pixel_x*pixel_x;

      *real_y= theta[0][1]*1+
	theta[1][1]*pixel_y+
	theta[2][1]*pixel_x+
	theta[3][1]*pixel_x*pixel_y+
	theta[4][1]*pixel_y*pixel_y+ 
	theta[5][1]*pixel_x*pixel_x;

}
