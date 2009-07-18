#include <cv.h>
#include <highgui.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
  IplImage* src = cvLoadImage( "img.jpg", 1 );
  int i,k;

  cvNamedWindow("win", 1);

  IplImage* img = cvCreateImage(cvSize(src->width,src->height),IPL_DEPTH_8U,3);
//  cvCopy(src,img,0);
  cvCvtColor(src,img,CV_BGR2HLS);

  // BGR
  // Yellow = 00FFFF
  // target = Yellow
  // result = Red

  for(i=310;i<350;i++)
    for(k=130;k<200;k++) {
      CvScalar s;
      s=cvGetAt(img,k,i);
      s.val[0]=(s.val[0]+180);
//      s.val[1]=MAX(s.val[1]-10,0);
//      s.val[2]=MIN(s.val[2]+50,100);
      cvSetAt(img,s,k,i);
    }
  cvCvtColor(img,img,CV_HLS2BGR);

  cvShowImage( "win", img );

  cvWaitKey(0);

  cvDestroyWindow("win");

  return 0;
}
