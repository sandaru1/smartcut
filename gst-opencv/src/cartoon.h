guint8 flat[256];

void flatten_color(guint8 *c)
{
  int i;
  for(i=0;i<3;i++)
   c[i] = flat[c[i]];
//   c[i] = (guint8)40*(c[i]/40.0);
}

long max_contrast(guint8 *C1,guint8 *C2) {
  double c1[3],c2[3];
  int i;
  long out = 0;

  for(i=0;i<3;i++) {
    c1[i] = flat[C1[i]];
    c2[i] = flat[C2[i]];
//    c1[i] = 40*(C1[i]/40.0);
//    c2[i] = 40*(C2[i]/40.0);
  }
  for(i=0;i<3;i++)
   out += (c1[i]-c2[i])*(c1[i]-c2[i]); 
  return out;
}

guint8* getColorAt(guint8 *base,int x,int y,int width) {
  return (guint8*)(base+(y*width+x)*3);
}


long get_max_contrast(guint8 *image,int x,int y,int diff_space,int width)
{
   guint8 *c1,*c2;
   long max=0;
   
   /*
    *  Calculate first error (horizontal)
    */
   c1 = getColorAt(image,x-diff_space,y,width);
   c2 = getColorAt(image,x+diff_space,y,width);
   max = MAX(max,max_contrast(c1,c2));

   /*
    *  Calculate second error (vertical)
    */
   c1 = getColorAt(image,x,y-diff_space,width);
   c2 = getColorAt(image,x,y+diff_space,width);
   max = MAX(max,max_contrast(c1,c2));
   
   /*
    *  Calculate third error (upperleft-lowerright)
    */
   c1 = getColorAt(image,x-diff_space,y-diff_space,width);
   c2 = getColorAt(image,x+diff_space,y+diff_space,width);
   max = MAX(max,max_contrast(c1,c2));

   /*
    *  Calculate third error (lowerleft-upperright)
    */
   c1 = getColorAt(image,x+diff_space,y-diff_space,width);
   c2 = getColorAt(image,x-diff_space,y+diff_space,width);
   max = MAX(max,max_contrast(c1,c2));

   return max;
}

void make_color(guint8 *c,int r,int g,int b) {
  c[0] = r;
  c[1] = g;
  c[2] = b;
}

void setColor(guint8 *image,guint8 *c,int x,int y,int width) {
  int i;
  guint8 *pos = getColorAt(image,x,y,width);
  for(i=0;i<3;i++)
    pos[i] = c[i];
}

void copyColor(guint8* c1,guint8* c2) {
  int i;
  for(i=0;i<3;i++)
    c2[i] = c1[i];
}


/*
 *  Cartoonify picture, do a form of edge detect 
 */
void make_cartoon(guint8 *source,guint8 *target,int X,int Y,int WIDTH,int HEIGHT,int width,int height,int diff_space,int trip_level)
{
    int x,y;
    guint8 black[3],c[3];

  if (flat[255]==0) {
    for(x=0;x<255;x++) {
     flat[x] = (guint8)40*(x/40.0);
    }
  }

   
    make_color(black,0,0,0);
   
   for (x=X+diff_space;x<(X+WIDTH)-(1+diff_space);x++)
   {
      for (y=Y+diff_space;y<(Y+HEIGHT)-(1+diff_space);y++)
      {
         if (get_max_contrast(source,x,y,diff_space,width)>trip_level)
      	 {
    	    /* 
    	     *  Make a border pixel
    	     */
          //printf("set at %d %d\n",x,y);
          setColor(target,black,x,y,width);
      	 }
      	 else
      	 {
    	    /*
    	     *   Copy original color
    	     */
          //printf("set at %d %d\n",x,y);
          copyColor(getColorAt(source,x,y,width),c);
          flatten_color(c);
          setColor(target,c,x,y,width);
      	 }
      }
   }
}

