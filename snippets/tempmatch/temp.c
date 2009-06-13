#include <cv.h> 
#include <highgui.h>
#include <stdio.h>

int main(int argc, char** argv) {
  double m,M;
  IplImage* image; IplImage* icon = NULL;
  CvCapture* capture = 0;
  CvPoint point1; CvPoint point2;

  CvRect r;
  r.x = 280;
  r.y = 150;
  r.width=100;
  r.height =150;    

  capture = cvCaptureFromAVI("/media/data/MVI_2488.AVI");
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
