// Adafruit NeoPixel - Version: Latest 
#include <Adafruit_NeoPixel.h>
#define LEFTPIN 3 // Connector: Left button (select)
#define RIGHTPIN 4 // Connector: Right button (change)
#define NEOPIN 0 // Connector: NeoPixels

Adafruit_NeoPixel strip = Adafruit_NeoPixel(17, NEOPIN, NEO_GRB + NEO_KHZ800);

byte colorValueMax = 64; // Color value limit to prevent blindness.
byte colorValueDiff = 8; // The increments for color-values.
byte animationSteps = 8; // Intermediate color-updates for fluid animations.
byte animationDelay = 20; // Delay between color-updates in milliseconds. (20ms = ~50 fps)
byte animationWait = 100; // Delay to slow things down a bit between actions.
byte buttonWait = 100; // Button-press duration to be considered for long-press.

// The order in which the RGB LEDs are connected to NEOPIN.
byte ring[7] = {0,1,2,3,4,5,6};
byte leftBar[4] = {7,8,9,10};
byte rightBar[4] = {11,12,13,14};

// The color-component representation of the ring.
byte rgb = ring[0];
byte r = ring[1];;
byte rg = ring[2];;
byte g = ring[3];;
byte gb = ring[4];;
byte b = ring[5];;
byte br = ring[6];;

// Our own Color class.
class Color {
  public:
    Color(byte, byte, byte);
    byte r;
    byte g;
    byte b;
};

// A single random integer value, adjusted to our own needs.
byte randomColorValue() {
  return (random(0,colorValueMax/colorValueDiff)*colorValueDiff);
}

Color::Color(
    byte r=randomColorValue(),
    byte g=randomColorValue(),
    byte b=randomColorValue()
  ) {
  this->r = r;
  this->g = g;
  this->b = b;
}

Color black = Color(0,0,0);
Color white = Color(255,255,255);
Color aim = Color(0,0,0);

Color getColor(byte pin) {

  uint32_t fc = strip.getPixelColor(0);
  
  uint8_t fr = fc >> 16;
  uint8_t fg = fc >> 8;
  uint8_t fb = fc;
  
  return Color(fr,fg,fb);
}


// Change color of LEDs.
// Length of array 'final' must be either 1 or equal to the 'pin' array length.
void setColor(byte pin[], byte pins, Color color) {
  for(int i = 0; i < pins; i++) {
    strip.setPixelColor(pin[i],color.r,color.g,color.b);
  }
}

// Fade color of LEDs.
// Length of array 'final' must be either 1 or equal to the 'pin' array length.
void fadeColor(byte pin[], byte pins, Color final) {

  Color original = getColor(pin[0]);
  
  for(int i=1; i<=animationSteps; i++) {
    for(int j=0; j<pins; j++) {
      strip.setPixelColor(pin[j],
        original.r+(final.r-original.r)/animationSteps*i,
        original.g+(final.g-original.g)/animationSteps*i,
        original.b+(final.b-original.b)/animationSteps*i
      );
    }
    strip.show();
    delay(animationDelay);
  }
}

void flashPixel(byte pin[], byte pins) {

  Color original = getColor(pin[0]);
  
  setColor(pin,pins,white);
  strip.show();
  delay(animationDelay);

  setColor(pin,pins,original);
  strip.show();
}

void presentColor(Color target) {
  flashPixel(leftBar,4);
  delay(animationWait);
  fadeColor(leftBar,4,target);
}

void confirmColor() {

  uint32_t fc = strip.getPixelColor(7);
  
  uint8_t fr = fc >> 16;
  uint8_t fg = fc >> 8;
  uint8_t fb = fc;
  
  Color original = Color(fr,fg,fb);

  setColor(leftBar,4,&black);
  setColor(rightBar,4,&black);
  strip.show();
  delay(animationDelay);

  setColor(leftBar,4,&white);
  setColor(rightBar,4,&white);
  strip.show();
  delay(animationDelay);

  setColor(leftBar,4,&original);
  setColor(rightBar,4,&original);
  strip.show();
  delay(animationDelay);

  delete &original;

}

void setRingColor(Color color) {

  uint32_t fc = strip.getPixelColor(0);
  
  uint8_t fr = fc >> 16;
  uint8_t fg = fc >> 8;
  uint8_t fb = fc;
  uint8_t rs = (color.r-fr)/animationSteps;
  uint8_t gs = (color.g-fg)/animationSteps;
  uint8_t bs = (color.b-fb)/animationSteps;

  for(int i=1; i<animationSteps; i++) {
    strip.setPixelColor(rgb,fr+rs*i,fg+gs*i,fb+bs*i);
    strip.setPixelColor(r,fr+rs*i,0,0);
    strip.setPixelColor(rg,fr+rs*i,fg+gs*i,0);
    strip.setPixelColor(g,0,fg+gs*i,0);
    strip.setPixelColor(gb,0,fg+gs*i,fb+bs*i);
    strip.setPixelColor(b,0,0,fb+bs*i);
    strip.setPixelColor(br,fr+rs*i,0,fb+bs*i);

    setColor(rightBar,4,Color(fr+rs*i,fg+gs*i,fb+bs*i));

    strip.show();
    delay(animationDelay);
  }

  strip.setPixelColor(rgb,color.r,color.g,color.b);
  strip.setPixelColor(r,color.r,0,0);
  strip.setPixelColor(rg,color.r,color.g,0);
  strip.setPixelColor(g,0,color.g,0);
  strip.setPixelColor(gb,0,color.g,color.b);
  strip.setPixelColor(b,0,0,color.b);
  strip.setPixelColor(br,color.r,0,color.b);

  setColor(rightBar,4,color);

  strip.show();

}

int compareColor(Color lhs, Color rhs) {
  return (lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b);
}

void matchColor(Color target) {
  
  uint8_t components[] = {r,g,b};
  uint8_t *pointers[] = {&aim.r,&aim.g,&aim.b};
  uint8_t selected = 0;
  
  boolean left;
  boolean right;

  while(!compareColor(aim, target)) {
    
    delay(buttonWait);
    
    do {
      delay(buttonWait/4);
      left = digitalRead(LEFTPIN);
      right = digitalRead(RIGHTPIN);
    } while (!left && !right);

    if(left) {
      selected++;
      selected%=3;
      flashPixel(&components[selected],1);
    } else {
      boolean hold = true;
      for (byte i = 0; i < 6; i++) {
        delay(buttonWait/2);
        if(!digitalRead(RIGHTPIN)) {
          hold = false;
          break;
        }
      }
      if(hold) {
        *pointers[selected] -= colorValueDiff;
      } else {
        *pointers[selected] += colorValueDiff;
      }
      *pointers[selected] %= colorValueMax+colorValueDiff;
      setRingColor(aim);
      if(hold) {
        delay(buttonWait*2);
      }
    }
  }
}

void setup() {
  strip.setBrightness(64); // Set LEDs to maximum brightness.
  strip.begin(); // Initiate LEDs.
  strip.show(); // Update LEDs' value to black.
  pinMode(LEFTPIN,INPUT);
  pinMode(RIGHTPIN,INPUT);
}

void loop() {
    Color target = new Color();
    presentColor(target);
    matchColor(target);
    confirmColor();
    delete &target;
}
