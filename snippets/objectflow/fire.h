int fire[640][480][2];
int palette[256][3];

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

void init_fire() {
  for(int i=0;i<256;i++) {
    HSL2BGR(i/3,255,MIN(255, i * 2),palette[i]);
  }
}

void set_fire(int x,int y,int w,int h) {
  fire[x][y][0] = abs(32768 + rand()) % 256;
  fire[(x - 1 + w) % w][(y + 1) % h][0] = abs(32768 + rand()) % 256;
  fire[(x) % w][(y + 1) % h][0] = abs(32768 + rand()) % 256;
  fire[(x + 1) % w][(y + 1) % h][0] = abs(32768 + rand()) % 256;
  fire[(x) % w][(y + 2) % h][0] = abs(32768 + rand()) % 256;
  fire[x][y][1] = 129;
}

void expand_fire(int w,int h) {
  for(int y = 0; y < h - 1; y++)
    for(int x = 0; x < w; x++)
    {
      fire[x][y][0] = 
        ((fire[(x - 1 + w) % w][(y + 1) % h][0]
        + fire[(x) % w][(y + 1) % h][0]
        + fire[(x + 1) % w][(y + 1) % h][0]
        + fire[(x) % w][(y + 2) % h][0])
        * 32) / 129;
      int temp = fire[(x + 1) % w][(y) % h][1];
      if (temp>1)
        fire[x][y][1] = temp - 3;
      fire[x][y][1] = MAX(fire[x][y][1]-3,0);
    }
}

void draw_fire(int w,int h,char* data) {
  for(int x = 0; x < w; x++)
    for(int y = 0; y < h; y++)
    {
      int pos = (x+y*w)*3;
      data[pos] = lerp(fire[x][y][1]/129.0,data[pos],palette[fire[x][y][0]][0]);
      data[pos+1] = lerp(fire[x][y][1]/129.0,data[pos+1],palette[fire[x][y][0]][1]);
      data[pos+2] = lerp(fire[x][y][1]/129.0,data[pos+2],palette[fire[x][y][0]][2]);
    }
}
