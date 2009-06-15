#include <cv.h>
#include <highgui.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

IplImage *image = 0;

void HSL2BGR(int H,int SL,int L,int* bgr)
{
  double h = H/255.0, sl = SL/255.0, l = L/255.0;
  double v;
  double r,g,b;
  r = l;   // default to gray
  g = l;
  b = l;
  v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
  if (v > 0)
  {
    double m;
    double sv;
    int sextant;
    double fract, vsf, mid1, mid2;
    m = l + l - v;
    sv = (v - m ) / v;
    h *= 6.0;
    sextant = (int)h;
    fract = h - sextant;
    vsf = v * sv * fract;
    mid1 = m + vsf;
    mid2 = v - vsf;
    switch (sextant)
    {
      case 0:
        r = v;
        g = mid1;
        b = m;
        break;
      case 1:
        r = mid2;
        g = v;
        b = m;
        break;
      case 2:
        r = m;
        g = v;
        b = mid1;
        break;
      case 3:
        r = m;
        g = mid2;
        b = v;
        break;
      case 4:
        r = mid1;
        g = m;
        b = v;
        break;
      case 5:
        r = v;
        g = m;
        b = mid2;
        break;
    }
  }
  bgr[0] = (int)(b * 255.0f);
  bgr[1] = (int)(g * 255.0f);
  bgr[2] = (int)(r * 255.0f);
}

int lerp(float t,int A, int B) {
    float a = A/255.0, b = B/255.0;
    return (int)((a + t * (b - a))*255);
}

int main(int argc, char** argv)
{
  int fire[640][480][2];
  int palette[256][3];
  int i;

  memset(fire,0,sizeof(fire));

  for(i=0;i<256;i++) {
    HSL2BGR(i/3,255,MIN(255, i * 2),palette[i]);
  }

  CvCapture* capture = cvCaptureFromAVI( "/home/sandaru1/Desktop/smartcut/server/media/videos/53.avi" ); 

  cvNamedWindow("win", 1);

  image = cvCreateImage( cvSize(640,480), 8, 3 );

  for(;;)
  {
		IplImage* frame = cvQueryFrame( capture );
		if (!frame) break;
		cvCopy( frame, image, 0 );

    int h = image->height;
    int w = image->width;

    for(int x = 0; x < image->width; x++)  {
      fire[x][image->height - 1][0] = abs(32768 + rand()) % 256;
      fire[x][image->height - 1][1] = 129;
    }

    for(int y = 0; y < h - 1; y++)
    for(int x = 0; x < w; x++)
    {
      fire[x][y][0] =
        ((fire[(x - 1 + w) % w][(y + 1) % h][0]
        + fire[(x) % w][(y + 1) % h][0]
        + fire[(x + 1) % w][(y + 1) % h][0]
        + fire[(x) % w][(y + 2) % h][0])
        * 32) / 129;
      int temp = fire[(x) % w][(y + 1) % h][1];
      if (temp>1)
        fire[x][y][1] = temp - 1;
    }

    for(int x = 0; x < w; x++)
    for(int y = h-129; y < h; y++)
    {
      int pos = (x+y*w)*3;
      image->imageData[pos] = lerp(fire[x][y][1]/129.0,image->imageData[pos],palette[fire[x][y][0]][0]);
      image->imageData[pos+1] = lerp(fire[x][y][1]/129.0,image->imageData[pos+1],palette[fire[x][y][0]][1]);
      image->imageData[pos+2] = lerp(fire[x][y][1]/129.0,image->imageData[pos+2],palette[fire[x][y][0]][2]);
    }

	
    cvWaitKey(10);
    cvShowImage( "win", image );
	}

  cvDestroyWindow("win");

  return 0;
}
