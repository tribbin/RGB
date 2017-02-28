// Adafruit NeoPixel - Version: Latest 
#include <Adafruit_NeoPixel.h>
#define LEFTPIN 2 // Left button (select)
#define RIGHTPIN 3 // Right button (change)
#define NEOPIN 6
#define BARLENGTH 8

Adafruit_NeoPixel strip = Adafruit_NeoPixel(17, NEOPIN, NEO_GRB + NEO_KHZ800);

int brightness = 255;
int colorValueMax = 64;
int colorValueDiff = 8;
int animationSteps = 8;
int animationDelay = 20;
int animationWait = 200;

class Color {
  public:
    Color(int, int, int);
    int r;
    int g;
    int b;
};

int randomColorValue() {
  int value = random(0,colorValueMax/colorValueDiff)*colorValueDiff;
  return value;
}

Color::Color(int r=randomColorValue(), int g=randomColorValue(), int b=randomColorValue()) {
  this->r = r;
  this->g = g;
  this->b = b;
}

Color black = Color(0,0,0);
Color aim = Color(0,0,0);
Color targets[BARLENGTH];

void fadeLeftBarColor(Color &target) {

  uint32_t fc = strip.getPixelColor(7);
  
  byte fr = fc >> 16;
  byte fg = fc >> 8;
  byte fb = fc;
  int rs = (target.r-fr)/animationSteps;
  int gs = (target.g-fg)/animationSteps;
  int bs = (target.b-fb)/animationSteps;

  for(int i=1; i<animationSteps; i++) {
    for(int j=7; j<7+4; j++) {
      strip.setPixelColor(j,fr+rs*i,fg+gs*i,fb+bs*i);
    }
    strip.show();
    delay(animationDelay);
  }
  for(int j=7; j<7+4; j++) {
    strip.setPixelColor(j,target.r,target.g,target.b);
  }
  strip.show();
  delay(animationDelay);
}

void presentColor(Color target) {
  fadeLeftBarColor(black);
  delay(animationWait);
  fadeLeftBarColor(target);
  delay(animationWait);
}

void confirmColor() {

  uint32_t fc = strip.getPixelColor(7);
  
  byte fr = fc >> 16;
  byte fg = fc >> 8;
  byte fb = fc;

  for(int j=7; j<7+BARLENGTH; j++) {
      strip.setPixelColor(j,0,0,0);
  }
  strip.show();

  delay(animationDelay);

  for(int j=7; j<7+BARLENGTH; j++) {
      strip.setPixelColor(j,colorValueMax,colorValueMax,colorValueMax);
  }
  strip.show();

  delay(animationDelay);
  
  for(int j=7; j<7+BARLENGTH; j++) {
      strip.setPixelColor(j,fr,fg,fb);
  }
  strip.show();

  delay(animationDelay);
}

void flashPixel(char color) {
  int index;
  switch (color) {
    case 'r':
      index = 1;
      break;
    case 'g':
      index = 3;
      break;
    case 'b':
      index = 5;
      break;
  }
  
  uint32_t fc = strip.getPixelColor(index);
  
  byte fr = fc >> 16;
  byte fg = fc >> 8;
  byte fb = fc;  
  
  strip.setPixelColor(index,colorValueMax,colorValueMax,colorValueMax);
  strip.show();
  delay(animationDelay);
  strip.setPixelColor(index,fr,fg,fb);
  strip.show();
  delay(animationWait);
}

void setRightBarColor(int r, int g, int b) {
  for( int i = 7+4; i < 7+BARLENGTH; i++) {
    strip.setPixelColor(i,r,g,b);
  }
}

void fadeMiddleColor() {

  uint32_t fc = strip.getPixelColor(0);
  
  byte fr = fc >> 16;
  byte fg = fc >> 8;
  byte fb = fc;
  int rs = (aim.r-fr)/animationSteps;
  int gs = (aim.g-fg)/animationSteps;
  int bs = (aim.b-fb)/animationSteps;

  for(int i=1; i<animationSteps; i++) {
    strip.setPixelColor(0,fr+rs*i,fg+gs*i,fb+bs*i);
    strip.setPixelColor(1,fr+rs*i,0,0);
    strip.setPixelColor(2,fr+rs*i,fg+gs*i,0);
    strip.setPixelColor(3,0,fg+gs*i,0);
    strip.setPixelColor(4,0,fg+gs*i,fb+bs*i);
    strip.setPixelColor(5,0,0,fb+bs*i);
    strip.setPixelColor(6,fr+rs*i,0,fb+bs*i);

    setRightBarColor(fr+rs*i,fg+gs*i,fb+bs*i);

    strip.show();
    delay(animationDelay);
  }
  strip.setPixelColor(0,aim.r,aim.g,aim.b);
  strip.setPixelColor(1,aim.r,0,0);
  strip.setPixelColor(2,aim.r,aim.g,0);
  strip.setPixelColor(3,0,aim.g,0);
  strip.setPixelColor(4,0,aim.g,aim.b);
  strip.setPixelColor(5,0,0,aim.b);
  strip.setPixelColor(6,aim.r,0,aim.b);
  strip.show();

}

int compareColor(Color lhs, Color rhs) {
  return (lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b);
}

void matchColor(Color target) {

  while(!compareColor(aim, target)) {
    int left;
    int right;
    do {
      left = digitalRead(LEFTPIN);
      right = digitalRead(RIGHTPIN);
      delay(animationDelay);
    } while (left == LOW && right == LOW);
/*
    flashPixel('r');
    while(aim.r < target.r) {
      aim.r+=colorValueDiff;
      fadeMiddleColor();
    }
    while(aim.r > target.r) {
      aim.r-=colorValueDiff;
      fadeMiddleColor();
    }
    flashPixel('g');
    while(aim.g < target.g) {
      aim.g+=colorValueDiff;
      fadeMiddleColor();
    }
    while(aim.g > target.g) {
      aim.g-=colorValueDiff;
      fadeMiddleColor();
    }
    flashPixel('b');
    while(aim.b < target.b) {
      aim.b+=colorValueDiff;
      fadeMiddleColor();
    }
    while(aim.b > target.b) {
      aim.b-=colorValueDiff;
      fadeMiddleColor();
    }
*/
  }
}

void setup() {
  strip.setBrightness(brightness);
  strip.begin();
  strip.show();
  pinMode(LEFTPIN,INPUT);
  pinMode(RIGHTPIN,INPUT);
}

void loop() {
  for (int i=0; i<BARLENGTH; i++) {
    presentColor(targets[i]);
    matchColor(targets[i]);
    delay(animationWait);
    confirmColor();
  }
}
