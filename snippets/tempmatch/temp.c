#include <cv.h> 
#include <highgui.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265
#define TWO_PI 3.14159265*2.0

void getHSV(int RI,int GI,int BI,double *H,double *S) {
  double R = RI/255.0;
  double G = GI/255.0;
  double B = BI/255.0;

  double max = MAX(R,MAX(G,B));
  double min = MIN(R,MIN(G,B));

  double l = (max+min)/2.0;

  if (max==min) {
    *H = 0;
    *S = 0;
    return;
  }
  if (max==R) {
    *H = (60 * ((G-B)/(double)(max-min)));
  } else if (max==G) {
    *H = (60 * (B-R)/(double)(max-min) + 120);
  } else if (max==B) {
    *H = (60 * (R-G)/(double)(max-min) + 240);
  }
  
/*  if (l<=0.5) {
    *S = (max - min)/(2 * l);
  } else {
    *S = (max - min)/(2-2*l);
  }*/
  *S = (max-min)/max;
}

float lerp(float t, float a, float b) {
		return a + t * (b - a);
}


float clamp(float x, float a, float b) {
		return (x < a) ? a : (x > b) ? b : x;
}

int main(int argc, char** argv) {
  double m,M;
  IplImage* image; IplImage* icon = NULL;
  CvCapture* capture = 0;
  CvPoint point1; CvPoint point2;

  CvRect r;
//  r.x = 280;
//  r.y = 150;
//  r.width=100;
//  r.height =140;

          r.x = 380;
          r.y = 130;
          r.width = 40;
          r.height = 80;

  // sparkle
  int rays = 100;
  int radius = r.width/2;
  int amount = 10;

	float rayLengths[100];
	for (int i = 0; i < rays; i++) {
			rayLengths[i] = radius + radius * rand()/(float)RAND_MAX;
  }
  // end - sparkle


  capture = cvCaptureFromAVI("/media/data/smartcut-videos/o1.avi");
  cvNamedWindow( "output", 1 ); 


  for(;;)
  {
    image = cvQueryFrame( capture );
//    cvRectangle( image, cvPoint(r.x,r.y),cvPoint( r.x + r.width, r.y + r.height ), cvScalar( 0, 0, 255, 0 ), 1, 0, 0 ); 
//    cvShowImage( "output", image ); 
//    cvWaitKey(0); 

    if( !image )
      break;

    if (icon == NULL) {
      icon = cvCreateImage( cvSize(r.width,r.height), 8, 3 );
      cvSetImageROI( image, r);
      cvCopy( image, icon, 0 );
      cvResetImageROI(image);
    }

    IplImage* copyOfSource = cvCloneImage( image );
    IplImage* copyOfTarget = cvCloneImage( icon );

  CvSize sourceSize = cvGetSize( image );
  CvSize targetSize = cvGetSize( icon );
  
  int depth = image->depth;
  int numChannels = image->nChannels;


  for( int ii = 0; ii < 2; ii++ )
  {
    // start with the source image
    sourceSize.width /= 2;
    sourceSize.height /= 2;
    
    IplImage* smallSource = NULL;
    smallSource = cvCreateImage( sourceSize, depth, numChannels );
    cvPyrDown( copyOfSource, smallSource, CV_GAUSSIAN_5x5 );
    
    // prepare for next loop, if any
    cvReleaseImage( &copyOfSource );
    copyOfSource = cvCloneImage( smallSource );
    cvReleaseImage( &smallSource );
    
    // next, do the target    
    targetSize.width /= 2;
    targetSize.height /= 2;
    
    IplImage* smallTarget = NULL;
    smallTarget = cvCreateImage( targetSize, depth, numChannels );
    cvPyrDown( copyOfTarget, smallTarget, CV_GAUSSIAN_5x5 );
    
    // prepare for next loop, if any
    cvReleaseImage( &copyOfTarget );
    copyOfTarget = cvCloneImage( smallTarget );
    cvReleaseImage( &smallTarget );
  }


  // perform the match on the shrunken images
  CvSize smallTargetSize = cvGetSize( copyOfTarget );
  CvSize smallSourceSize = cvGetSize( copyOfSource );
  
  CvSize resultSize;
  resultSize.width = smallSourceSize.width - smallTargetSize.width + 1;
  resultSize.height = smallSourceSize.height - smallTargetSize.height + 1;
  
  IplImage* result = cvCreateImage( resultSize, IPL_DEPTH_32F, 1 );
  
  cvMatchTemplate( copyOfSource, copyOfTarget, result, CV_TM_CCORR_NORMED );

  // release memory we don't need anymore
  cvReleaseImage( &copyOfSource );
  cvReleaseImage( &copyOfTarget );

  cvMinMaxLoc(result, &m, &M, &point1, &point2, NULL); 

  point2.x *= 4;
  point2.y *= 4;

/*
// sparkle

  int i,j;
  for(i=0;i<image->width;i++)
    for(j=0;j<image->height;j++) {
      int pos = i + j*image->width;
      pos = pos * 3;
  float dx = i-(point2.x+icon->width/2);
	float dy = j-(point2.y+icon->height/2);
	float distance = dx*dx+dy*dy;
	float angle = (float)atan2(dy, dx);
	float d = (angle+PI) / (TWO_PI) * rays;
	int ray_pos = (int)d;
	float f = d - ray_pos;

  float length = lerp(f, rayLengths[ray_pos % rays], rayLengths[(ray_pos+1) % rays]);
	float g = length*length / (distance+0.0001f);
//	g = (float)pow(g, (100-amount) / 50.0);
	f -= 0.5f;
	f = 1 - f*f;
	f *= g;
	f = clamp(f, 0, 0.5);
//printf("%f\n",f);

	image->imageData[pos+2] = lerp(f, image->imageData[pos+2], 255);
	image->imageData[pos+1] = lerp(f, image->imageData[pos+1], 255);
	image->imageData[pos] = lerp(f, image->imageData[pos], 255);


    }
*/
  

/*
  // enlarge
  CvRect selection;
  selection.x = point2.x;
  selection.y = point2.y;
  selection.width = r.width;
  selection.height = r.height;

IplImage* selected = cvCreateImage( cvSize(selection.width,selection.height), 8, 3 );
cvSetImageROI( image, selection);
cvCopy( image, selected, 0 );
cvResetImageROI(image);

IplImage* selectedLarge = cvCreateImage( cvSize(selection.width*2,selection.height*2), 8, 3 );

cvResize(selected, selectedLarge, CV_INTER_LINEAR );

selection.x = MAX(0,point2.x - r.width/2);
selection.y = MAX(0,point2.y - r.height/2);
selection.width = r.width*2;
selection.height = r.height*2;

cvSetImageROI(image, selection);
cvCopy( selectedLarge, image, 0 );
cvResetImageROI(image);*/

/*
  // color convert
  double fromS,fromH;
  getHSV(255,0,0,&fromH,&fromS);
  double toR = 1.0;
  double toG = 1.0;
  double toB = 0;

  int i,j;
  for(i=0;i<icon->width;i++)
    for(j=0;j<icon->height;j++) {
      int pos = (point2.x + i) + (point2.y + j)*image->width;
      pos = pos * 3;
      double h,s;
      getHSV(image->imageData[pos+2],image->imageData[pos+1],image->imageData[pos],&h,&s);
      double hueShift = abs(h-fromH)/60.0;
      if (hueShift < 1 && s > 0 ) {
        double alpha = 1 - (1 - hueShift) * s;

        double RD = image->imageData[pos+2]/255.0;
        double GD = image->imageData[pos+1]/255.0;
        double BD = image->imageData[pos]/255.0;

        image->imageData[pos+2] = (toR * alpha)*255;
        image->imageData[pos+1] = (toG * alpha)*255;
        image->imageData[pos] = (toB * alpha)*255;
      }
    }
*/
  cvRectangle( image, point2,cvPoint( point2.x + icon->width, point2.y + icon->height ), cvScalar( 0, 0, 255, 0 ), 1, 0, 0 ); 
  cvShowImage( "output", image );


/*    int resultW = image->width - icon->width + 1; 
    int resultH = image->height - icon->height +1; 
    IplImage* result = cvCreateImage(cvSize(resultW, resultH), IPL_DEPTH_32F, 1); 
    cvMatchTemplate(image, icon, result, CV_TM_CCORR_NORMED);
    cvMinMaxLoc(result, &m, &M, &point1, &point2, NULL); 
    cvRectangle( image, point2,cvPoint( point2.x + icon->width, point2.y + icon->height ), cvScalar( 0, 0, 255, 0 ), 1, 0, 0 ); 
    cvShowImage( "output", image );*/
    cvWaitKey(10);
  }
  return 0; 
} 
