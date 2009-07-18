#include <cv.h>
#include <highgui.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "fire.h"

IplImage *image = 0, *grey = 0, *prev_grey = 0, *pyramid = 0, *prev_pyramid = 0, *swap_temp;

int win_size = 10;
const int MAX_COUNT = 500;
CvPoint2D32f* points[2] = {0,0}, *swap_points;
char* status = 0;
int count = 0;
int need_to_init = 1;
int night_mode = 0;
int flags = 0;
int add_remove_pt = 0;
CvPoint pt;


int main(int argc, char** argv)
{
    CvCapture* capture = 0;

  CvRect newr;


          CvRect r;
/*  r.x = 280;
  r.y = 150;
  r.width=100;
  r.height =140;
  capture = cvCaptureFromAVI("/media/data/smartcut-videos/MVI_2488.AVI");*/

/*          r.x = 380;
          r.y = 130;
          r.width=40;
          r.height=80;
    
    capture = cvCaptureFromAVI("/media/data/smartcut-videos/o1.avi");*/

/*          r.x = 180;
          r.y = 10;
          r.width=220;
          r.height=180;
    
    capture = cvCaptureFromAVI("/media/data/smartcut-videos/car.avi");*/



          r.x = 315;
          r.y = 120;
          r.width=30;
          r.height=70;
    
    capture = cvCaptureFromAVI("/media/data/smartcut-videos/bus.avi");

/*          r.x = 610;
          r.y = 275;
          r.width=90;
          r.height =80;
    
    capture = cvCaptureFromAVI("/media/data/orangeball.avi");*/



    if( !capture )
    {
        fprintf(stderr,"Could not initialize capturing...\n");
        return -1;
    }

    cvNamedWindow( "output", 1 );
//    init_fire();
//    memset(fire,0,sizeof(fire));

/*    image = cvQueryFrame( capture );
    cvRectangle( image, cvPoint(r.x,r.y),cvPoint(r.x+r.width,r.y+r.height), CV_RGB(255,0,0) );
    cvShowImage( "output", image );
    cvWaitKey(0);*/

    for(;;)
    {
        IplImage* frame = 0;
        int i, k, c;

        frame = cvQueryFrame( capture );
        if( !frame )
            break;

        // TODO: dynamic allocation of fire array
        //if ( fire == NULL ) {
          //fire = (int***)malloc(sizeof(int)*frame->width*frame->height*2);
          //memset(fire,0,sizeof(int)*frame->width*frame->height*2);
          //init_fire();
        //}

        if( !image )
        {
            /* allocate all the buffers */
            image = cvCreateImage( cvGetSize(frame), 8, 3 );
            image->origin = frame->origin;
            grey = cvCreateImage( cvGetSize(frame), 8, 1 );
            prev_grey = cvCreateImage( cvGetSize(frame), 8, 1 );
            pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
            prev_pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
            points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
            points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
            status = (char*)cvAlloc(MAX_COUNT);
            flags = 0;
        }

        cvCopy( frame, image, 0 );
        cvCvtColor( image, grey, CV_BGR2GRAY );

        if( need_to_init )
        {
          IplImage*  g = cvCreateImage( cvSize(r.width,r.height), 8, 1 );

          int i, j, R, G, B, pos;
          for( i = 0 ; i < r.height ; i++ ) {
            for( j = 0 ; j < r.width ; j++ ) {
              pos = ((i+r.y)*r.width + j+r.x) * 3;
              R = image->imageData[pos];
              G = image->imageData[pos + 1];
              B = image->imageData[pos + 2];
              g->imageData[i*r.width+j] = (R + G + B)/3;
            }
          }


            /* automatic initialization */
            IplImage* eig = cvCreateImage( cvGetSize(g), 32, 1 );
            IplImage* temp = cvCreateImage( cvGetSize(g), 32, 1 );
            double quality = 0.1;
            double min_distance = 10;

            count = MAX_COUNT;
            cvGoodFeaturesToTrack( g, eig, temp, points[1], &count,
                                   quality, min_distance, 0, 3, 0, 0.04 );
            cvReleaseImage( &eig );
            cvReleaseImage( &temp );

          for(i=0;i<count;i++) {
            points[1][i].x += r.x;
            points[1][i].y += r.y;
          }
        }
        else if( count > 0 )
        {
            cvCalcOpticalFlowPyrLK( prev_grey, grey, prev_pyramid, pyramid,
                points[0], points[1], count, cvSize(win_size,win_size), 3, status, 0,
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), flags );
            flags |= CV_LKFLOW_PYR_A_READY;
            for( i = k = 0; i < count; i++ )
            {                
                if( !status[i] )
                    continue;
                
                points[1][k++] = points[1][i];
                //set_fire(points[1][i].x,points[1][i].y,image->width,image->height);
                cvCircle( image, cvPointFrom32f(points[1][i]), 3, CV_RGB(0,255,0), -1, 8,0);
            }
            count = k;
        }

        float X=0,Y=0;
          for(i=0;i<count;i++) {
            X += points[1][i].x;
            Y += points[1][i].y;
          }
        X /= (double)count;
        Y /= (double)count;
        cvRectangle( image, cvPoint(X-r.width/2,Y-r.height/2),cvPoint(X+r.width/2,Y+r.height/2), CV_RGB(255,0,0) );

        CV_SWAP( prev_grey, grey, swap_temp );
        CV_SWAP( prev_pyramid, pyramid, swap_temp );
//        CV_SWAP( points[0], points[1], swap_points );
        int nc = 0;
        for(i=0;i<count;i++) {
          float dx = abs(points[1][i].x-X);
          float dy = abs(points[1][i].y-Y);
          if (dx<=(r.width/2) && dy<=(r.height/2)) {
            points[0][nc++] = points[1][i];
          }
        }
        count = nc;
        need_to_init = 0;


       // reduce increase size of the object
        CvPoint* cx = (CvPoint*)cvAlloc(nc*sizeof(CvPoint));
        for(i=0;i<nc;i++) {
          cx[i].x = (int)points[0][i].x;
          cx[i].y = (int)points[0][i].y;
        }
        CvMat pointMat = cvMat( 1, nc, CV_32SC2, cx );
        newr = cvBoundingRect( &pointMat, 0 );

        
        if (abs(newr.width-r.width)>r.width/4) {
          r.width = r.width + (newr.width-r.width)/4;
        }

        if (abs(newr.height-r.height)>r.height/4) {
          r.height = r.height + (newr.height-r.height)/4;
        }


//      expand_fire(image->width,image->height);
//      draw_fire(image->width,image->height,image->imageData);

        cvShowImage( "output", image );
        c = cvWaitKey(10);
    }

    cvReleaseCapture( &capture );
    cvDestroyWindow("output");

    return 0;
}
