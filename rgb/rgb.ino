// Adafruit NeoPixel - Version: Latest 
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#define RANDOM 0 // Unconnected pin for randomSeed().
#define LEFTPIN 1 // Connector: Left button (select).
#define RIGHTPIN 2 // Connector: Right button (change).
#define NEOPIN 3 // Connector: NeoPixels.
#define BUZZER 4 // For future functions.

Adafruit_NeoPixel strip = Adafruit_NeoPixel(17, NEOPIN, NEO_GRB + NEO_KHZ800);

byte colorValueMax = 64; // Color value limit to prevent blindness.
byte colorValueDiff = 8; // The increments for color-values.
byte animationSteps = 8; // Intermediate color-updates for fluid animations.
byte animationDelay = 20; // Delay between color-updates in milliseconds. (20ms = ~50 fps)
byte animationWait = 100; // Delay to slow things down a bit between actions.
byte buttonWait = 50; // Variable for tuning button-interaction.
                      // 1x buttonWait: time after short-press during which no interaction is registered.
                      // 2x buttonWait: time after long-press during which no interaction is registered.
                      // 1x buttonWait: time for the press to be considered for long-press.
                      // 6x buttonWait: time for the press to be registered as long-press.

// The order in which the RGB LEDs are connected to NEOPIN.
byte ring[7] = {0,1,2,3,4,5,6};
byte bar[8] = {7,8,9,10,11,12,13,14};

// The color-component locations on the ring.
byte rgb = ring[0];
byte r = ring[1];;
byte rg = ring[2];;
byte g = ring[3];;
byte gb = ring[4];;
byte b = ring[5];;
byte br = ring[6];;

byte leftBar[4] = {7,8,9,10};
byte rightBar[4] = {11,12,13,14};

// Our own Color class.
class Color {
  public:
    Color(byte, byte, byte);
    byte r;
    byte g;
    byte b;
};

// A single random byte value, adjusted to variables.
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
void setColor(byte pin[], byte pins, Color color) {
  for(int i = 0; i < pins; i++) {
    strip.setPixelColor(pin[i],color.r,color.g,color.b);
  }
}

// Change color of one LED.
void setColor(byte pin, byte r, byte g, byte b){
  strip.setPixelColor(pin,r,g,b);
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
  Color original = getColor(rgb);

  setColor(bar,4,&black);
  strip.show();
  delay(animationDelay);

  setColor(bar,4,&white);
  strip.show();
  delay(animationDelay);

  setColor(bar,4,&original);
  strip.show();
  delay(animationDelay);

  delete &original;

}

void setRingColor(Color color) {
  setColor(rgb,color.r,color.g,color.b);
  setColor(r,color.r,0,0);
  setColor(rg,color.r,color.g,0);
  setColor(g,0,color.g,0);
  setColor(gb,0,color.g,color.b);
  setColor(b,0,0,color.b);
  setColor(br,color.r,0,color.b);
  setColor(rightBar,4,color);
}

void fadeRingColor(Color to) {
  Color from = getColor(rgb);
  for (byte i = 0; i < animationSteps; i++) {
    Color intermediate = new Color(
        (from.r-to.r)/animationSteps*i,
        (from.g-to.g)/animationSteps*i,
        (from.b-to.b)/animationSteps*i
    );
    setRingColor(
      intermediate
    );
    delete &intermediate;
    delay(animationDelay);
  }
}

int compareColor(Color lhs, Color rhs) {
  return (lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b);
}

void matchColor(Color target) {
  
  byte components[] = {r,g,b};
  byte *pointers[] = {&aim.r,&aim.g,&aim.b};
  byte selected = 0;
  
  boolean left;
  boolean right;

  while(!compareColor(aim, target)) {
    
    delay(buttonWait);
    
    do {
      delay(buttonWait/2);
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
        delay(buttonWait);
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
      fadeRingColor(aim);
      strip.show();
      if(hold) {
        delay(buttonWait*4);
      }
    }
  }
}

void setup() {
  randomSeed(analogRead(RANDOM)); // For generating random colors.
  strip.setBrightness(255); // Set LEDs to maximum brightness.
  strip.begin(); // Initiate NeoPixels.
  strip.show(); // Update LEDs' value to black.
  pinMode(LEFTPIN,INPUT);
  pinMode(RIGHTPIN,INPUT);
}

void loop() {
    Color target = new Color(); // Create a random color.
    presentColor(target); // Present the new color to the player.
    matchColor(target); // Let the player match the color.
    confirmColor(); // Do a little dance.
    delete &target; // Delete the random color from memory.
}
