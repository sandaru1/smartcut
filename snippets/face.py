import sys
from opencv import cv
from opencv import highgui
import opencv
 
def detect(image):
	image_size = cv.cvGetSize(image)
 
	# create grayscale version
	grayscale = cv.cvCreateImage(image_size, 8, 1)
	cv.cvCvtColor(image, grayscale, opencv.CV_BGR2GRAY)
 
	# create storage
	storage = cv.cvCreateMemStorage(0)
	cv.cvClearMemStorage(storage)
 
	# equalize histogram
	cv.cvEqualizeHist(grayscale, grayscale)
 
	# detect objects
	cascade = cv.cvLoadHaarClassifierCascade('haarcascade_frontalface_alt.xml', cv.cvSize(1,1))
	faces = cv.cvHaarDetectObjects(grayscale, cascade, storage, 1.2, 2, opencv.CV_HAAR_DO_CANNY_PRUNING, cv.cvSize(100, 100))
 
	if faces:
		for i in faces:
			r = image[int(i.y):int(i.y+i.height),int(i.x):int(i.x+i.width)]
			cv.cvSmooth(r,r,cv.CV_BLUR,51,51)
			#image[int(i.x):int(i.x+i.width),int(i.y):int(i.y+i.height)] = r
 
if __name__ == "__main__":
	highgui.cvStartWindowThread()
	highgui.cvNamedWindow('win', 1)
	capture = highgui.cvCreateCameraCapture(0)
	while (1) :
		im = highgui.cvQueryFrame(capture)
		detect(im)
		highgui.cvShowImage("win", im)
		key = highgui.cvWaitKey(10)
		if (key == 10) :
			sys.exit(0)

