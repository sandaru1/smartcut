void change_color(IplImage* src,int x,int y,int width,int height,int hue,gint8* data) {
  int x1 = x-width/2;
  int y1 = y-height/2;
  int x2 = x+width/2;
  int y2 = y+height/2;
  int i,k;

  IplImage* img = cvCreateImage(cvSize(src->width,src->height),IPL_DEPTH_8U,3);
  cvCvtColor(src,img,CV_BGR2HLS);

  for(i=x1;i<x2;i++)
    for(k=y1;k<y2;k++) {
      CvScalar s;
      s=cvGetAt(img,k,i);
      s.val[0]=(s.val[0]+hue);
//      s.val[1]=MAX(s.val[1]-10,0);
//      s.val[2]=MIN(s.val[2]+50,100);
      cvSetAt(img,s,k,i);
    }
  cvCvtColor(img,img,CV_HLS2BGR);
  for(i=x1;i<x2;i++)
    for(k=y1;k<y2;k++) {
      int pos = (k*src->width +i) * 3;
      data[pos] = img->imageData[pos+2];
      data[pos+1] = img->imageData[pos+1];
      data[pos+2] = img->imageData[pos];
    }
}
