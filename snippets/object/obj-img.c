#include <cv.h>
#include <highgui.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

IplImage *image = 0, *hsv = 0, *hue = 0, *mask = 0, *backproject = 0, *histimg = 0;
CvHistogram *hist = 0;
int hdims = 16;
CvRect track_window;
float hranges_arr[] = {0,180};
float* hranges = hranges_arr;
CvBox2D track_box;
CvConnectedComp track_comp;
int vmin = 10, vmax = 256, smin = 30;

CvScalar hsv2rgb( float hue )
{
    int rgb[3], p, sector;
    static const int sector_data[][3]=
        {{0,2,1}, {1,2,0}, {1,0,2}, {2,0,1}, {2,1,0}, {0,1,2}};
    hue *= 0.033333333333333333333333333333333f;
    sector = cvFloor(hue);
    p = cvRound(255*(hue - sector));
    p ^= sector & 1 ? 255 : 0;

    rgb[sector_data[sector][0]] = 255;
    rgb[sector_data[sector][1]] = 0;
    rgb[sector_data[sector][2]] = p;

    return cvScalar(rgb[2], rgb[1], rgb[0],0);
}


int main(int argc, char** argv)
{
	int i, bin_w, c;
	int _vmin = vmin, _vmax = vmax;

    CvMemStorage* storage = cvCreateMemStorage(0);
	CvRect selection;

	selection.x = 50;
	selection.y = 200;
	selection.width = 100;
	selection.height = 200;

    cvNamedWindow("win", 1);

    image = cvLoadImage( "a.png" );

    hsv = cvCreateImage( cvGetSize(image), 8, 3 );
    hue = cvCreateImage( cvGetSize(image), 8, 1 );
    mask = cvCreateImage( cvGetSize(image), 8, 1 );
    backproject = cvCreateImage( cvGetSize(image), 8, 1 );
    hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
    histimg = cvCreateImage( cvSize(320,200), 8, 3 );
    cvZero( histimg );

	cvCvtColor( image, hsv, CV_BGR2HSV );
    cvCvtColor( image, hsv, CV_BGR2HSV );

	cvInRangeS( hsv, cvScalar(0,smin,MIN(_vmin,_vmax),0),
                        cvScalar(180,256,MAX(_vmin,_vmax),0), mask );
	cvSplit( hsv, hue, 0, 0, 0 );

	float max_val = 0.f;
    cvSetImageROI( hue, selection );
    cvSetImageROI( mask, selection );
    cvCalcHist( &hue, hist, 0, mask );
    cvGetMinMaxHistValue( hist, 0, &max_val, 0, 0 );
	cvConvertScale( hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0 );
	cvResetImageROI( hue );
    cvResetImageROI( mask );

    cvZero( histimg );
    bin_w = histimg->width / hdims;
    for( i = 0; i < hdims; i++ )
    {
        int val = cvRound( cvGetReal1D(hist->bins,i)*histimg->height/255 );
        CvScalar color = hsv2rgb(i*180.f/hdims);
        cvRectangle( histimg, cvPoint(i*bin_w,histimg->height),
                        cvPoint((i+1)*bin_w,histimg->height - val),
                        color, -1, 8, 0 );
    }

    image = cvLoadImage( "b.png" );
	track_window.x = 230;
	track_window.y = 50;
	track_window.width = 300;
	track_window.height = 300;
    cvRectangle( image, cvPoint(track_window.x,track_window.y),cvPoint(track_window.x+track_window.width,track_window.y+track_window.height), CV_RGB(255,0,0) );
	cvCvtColor( image, hsv, CV_BGR2HSV );
    cvInRangeS( hsv, cvScalar(0,smin,MIN(_vmin,_vmax),0),
                        cvScalar(180,256,MAX(_vmin,_vmax),0), mask );
    cvSplit( hsv, hue, 0, 0, 0 );

    cvCalcBackProject( &hue, backproject, hist );
    cvAnd( backproject, mask, backproject, 0 );
    cvCamShift( backproject, track_window,
                cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
                &track_comp, &track_box );
    track_window = track_comp.rect;
    
	cvEllipseBox( image, track_box, CV_RGB(255,0,0), 3, CV_AA, 0 );

    cvShowImage( "win", image );

    cvWaitKey(0);

    cvDestroyWindow("win");

    return 0;
}
