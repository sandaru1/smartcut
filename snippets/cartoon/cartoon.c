#include <cv.h>
#include <highgui.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

IplImage *image = 0;

int diff_space = 1;
int trip_level = 5000;

void flatten_color(CvScalar *c)
{
  for(int i=0;i<3;i++)
   c->val[i] = 40*(c->val[i]/40);
}

long max_contrast(CvScalar c1,CvScalar c2) {
   flatten_color(&c1);
   flatten_color(&c2);
   return (c1.val[0]-c2.val[0])*(c1.val[0]-c2.val[0]) +
           (c1.val[1]-c2.val[1])*(c1.val[1]-c2.val[1]) +
           (c1.val[2]-c2.val[2])*(c1.val[2]-c2.val[2]);
}


long get_max_contrast(IplImage *image,int x,int y)
{
   CvScalar c1,c2;
   long max=0;
   
   /*
    *  Calculate first error (horizontal)
    */
   c1 = cvGetAt(image,x-diff_space,y);
   c2 = cvGetAt(image,x+diff_space,y);
   max = MAX(max,max_contrast(c1,c2));

   /*
    *  Calculate second error (vertical)
    */
   c1 = cvGetAt(image,x,y-diff_space);
   c2 = cvGetAt(image,x,y+diff_space);
   max = MAX(max,max_contrast(c1,c2));
   
   /*
    *  Calculate third error (upperleft-lowerright)
    */
   c1 = cvGetAt(image,x-diff_space,y-diff_space);
   c2 = cvGetAt(image,x+diff_space,y+diff_space);
   max = MAX(max,max_contrast(c1,c2));

   /*
    *  Calculate third error (lowerleft-upperright)
    */
   c1 = cvGetAt(image,x+diff_space,y-diff_space);
   c2 = cvGetAt(image,x-diff_space,y+diff_space);
   max = MAX(max,max_contrast(c1,c2));
   return max;
}

void make_color(CvScalar* c,int r,int g,int b) {
  c->val[0] = r;
  c->val[1] = g;
  c->val[2] = b;
}


/*
 *  Cartoonify picture, do a form of edge detect 
 */
void make_cartoon(IplImage *source,IplImage *target)
{
    int x,y,r,g,b;
    CvScalar white,black,c;
   
    make_color(&black,0,0,0);
    make_color(&white,255,255,255); 
   
   for (x=diff_space;x<source->height-(1+diff_space);x++)
   {
      for (y=diff_space;y<source->width-(1+diff_space);y++)
      {
         if (get_max_contrast(source,x,y)>trip_level)
      	 {
    	    /* 
    	     *  Make a border pixel
    	     */
          //printf("set at %d %d\n",x,y);
          cvSetAt(target,black,x,y);
      	 }
      	 else
      	 {
    	    /*
    	     *   Copy original color
    	     */
          //printf("set at %d %d\n",x,y);
          c = cvGetAt(source,x,y);
          flatten_color(&c);
          cvSetAt(target,c,x,y);
      	 }
      }
   }
}


int main(int argc, char** argv)
{

  CvCapture* capture = cvCaptureFromAVI( argv[1] ); 

  cvNamedWindow("win", 1);


  for(;;)
  {
		IplImage* frame = cvQueryFrame( capture );
    if (!image) {
      image = cvCreateImage( cvSize(frame->width,frame->height), 8, 3 );
    }
		if (!frame) break;
		cvCopy( frame, image, 0 );
	
    make_cartoon(frame,image);    

    cvWaitKey(10);
    cvShowImage( "win", image );
	}

  cvDestroyWindow("win");

  return 0;
}
