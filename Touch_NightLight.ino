/*
 * Name: Touch_NightLight
 * Created: June 2016
 * Authors: John B Damask, liberally borrowing code from several Adafruit tutorials
 * Purpose: I have a NeoPixel strand, a Feather and three sensors for a total of 7 possible states
 * Notes: As of June 26, 2016 this works nicely but isn't perfect. Issues:
 *        1. Capacitive sensors are funky about grounding so I have a very low threshold for triggering the event.
 *           It works in my house when the device is plugged into the wall (with no ground) but I don't know if
 *           it would behave the same way on battery.
 *        2. To run long animations and listen for new events without an interrupt, I use the controller loop 
 *            instead of nested FOR loops. The animations aren't smooth yet but they are decent approximations.
 *        3. theaterChaseRainbow method needs to be re-written as it currently has a blocking FOR loop.
 *        4.  Clean up
 * 
 * Please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 */

#include <Adafruit_NeoPixel.h>
#include <CapacitiveSensor.h>

#define PIN       6
#define NUMPIXELS 23

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(NUMPIXELS, PIN);


class TouchLight{
  CapacitiveSensor cs;  // Pin to monitor. 1M resistor between send and receive pin
  long total = 0;           // Sensor value
  int threshold = 100;        // Touch threshold at which we are ON 
  int id;
  
  public:
  TouchLight(uint8_t sendPin, uint8_t receivePin, int mID) : cs(CapacitiveSensor(sendPin, receivePin)), id(mID){}

  boolean isTouched(){
    total = cs.capacitiveSensor(30);
    if(total > threshold){
      Serial.print(id);
      Serial.print("\tCS Value: ");
      Serial.println(total);
      return true;
    } else {
      return false;
    }
  }
};

uint8_t currentState = 0; // Keeps track of current state 
uint8_t prevState = 0;    // Keeps track of previous state
long rSeed = 0;        // Holds random seed
uint8_t rRed = 0;         // Random red
uint8_t rGreen = 0;       // Random green
uint8_t rBlue = 0;        // Random blue
long previousMillis = 0;        // will store last time LED was updated
long duration = 1000;           // Duration for timer
//uint8_t prevPixel = 0; // Keeps track of pixels for rainbow effect
//uint8_t prevPixelColor = 0;
uint16_t pixelCount = 0;
uint16_t colorCount = 0; 
uint16_t pixelOffset = 0;
uint16_t loopCounter = 0;
boolean onOff = false;
TouchLight tRed = TouchLight(22,19,1);
TouchLight tGreen = TouchLight(22,20,2);
TouchLight tBlue = TouchLight(22,21,3);

void setup() {
  // turn off neopixel
  pixel.begin(); // This initializes the NeoPixel library.
  for(uint8_t i=0; i<NUMPIXELS; i++) {
    pixel.setPixelColor(i, pixel.Color(0,0,0)); // off
  }
  pixel.show();

  // Set a random seed so our random colors are different on each run
  // Per Arduino idiom, use an unconnected pin to generate the seed
  rSeed = analogRead(18);
}

void loop() {

  prevState = currentState;   // Keep track of previous state
  long now = millis();
  if(now - previousMillis > duration){
    previousMillis = now;    
  } else {
    stateCondition();
    return;
  }
  // Touch
  if (tRed.isTouched() && tGreen.isTouched() && tBlue.isTouched()){
    currentState = 7;
    ////delay(20);
    Serial.println("All touched!");
  } else if (tRed.isTouched() && tGreen.isTouched()){
    currentState = 6;
    //delay(20);
  } else if (tRed.isTouched() && tBlue.isTouched()) {
    currentState = 5;
    //delay(20);
  } else if (tGreen.isTouched() && tBlue.isTouched()) {      
    rRed = random(1,255);
    rGreen = random(1,255);
    rBlue = random(1,255);    
    currentState = 4;
    //delay(20);
    } else if (tRed.isTouched()) {
    // If red sensor touched twice in succession, turn strip off
    if(prevState != 1 ){
      currentState = 1;
    } else {
      currentState = 0;
    }
  } else if (tGreen.isTouched()) {    
    currentState = 2;
  } else if (tBlue.isTouched()) {    
    currentState = 3;
  }
  stateCondition();
}

void stateCondition(){

  switch(currentState){    
    case 1:   // RED
      colorWipe(pixel.Color(255,0,0));
      Serial.println("Red");
      break;
    case 2:  // GREEN
      colorWipe(pixel.Color(0,255,0));
      Serial.println("Green");
      break;  
    case 3:  // BLUE
      colorWipe(pixel.Color(0,0,255));
      Serial.println("Blue");
      break;  
    case 4:   // RANDOM COLOR
      Serial.println("Random");
      colorWipe(pixel.Color(rRed,rGreen,rBlue));
      break;
    case 5:   // RAINBOW
      Serial.println("Rainbow");
      rainbowCycle();
      break;
    case 6:   // THEATER CHASE
      Serial.println("Theater Chase");
      theaterChase(10, 50);
      break;
    case 7:   // THEATER CHASE RAINBOW
      Serial.println("Theater Chase Rainbow");
      theaterChaseRainbow(10);
      break;
    default:
      colorWipe(pixel.Color(0,0,0));
      break;            
  }
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c) {
  for(uint16_t i=0; i<pixel.numPixels(); i++) {
    pixel.setPixelColor(i, c);
    pixel.setBrightness(30);
    pixel.show();
    delay(5);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
// JBD - I changed the original rainbow() code from strandtest so we can receive inputs w/out interrupts
// It's not as smooth as the original
void rainbowCycle() {  
    pixelCount =  (pixelCount > pixel.numPixels()) ? 0 : pixelCount + 1;
    colorCount = (colorCount > 255*5) ? 0 : colorCount + 1;
    pixel.setPixelColor(pixelCount, Wheel(((pixelCount * 256 / pixel.numPixels()) + colorCount) & 255));    
    pixel.setBrightness(100);
    pixel.show();    
    delay(10);  
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
    Serial.print("PixelCount = ");
    Serial.println(pixelCount);
    Serial.print("PixelOffset = ");
    Serial.println(pixelOffset);
    pixelCount =  (pixelCount > pixel.numPixels() ) ? 0 : pixelCount+1;
    if(pixelCount == 0){
      if(pixelOffset == 3){
        pixelOffset = 0;
      }else{
        pixelOffset++;        
      }
      onOff = !onOff;
    }
       if(onOff){
          pixel.setPixelColor(pixelCount+pixelOffset, c);    //turn every third pixel on
          pixel.setBrightness(100);
          pixel.show();
          delay(10);
       }else{
          pixel.setPixelColor(pixelCount+pixelOffset, 0);    //turn every third pixel on 
          pixel.show();
          delay(10);
       }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < pixel.numPixels(); i=i+3) {
        pixel.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      pixel.show();      
      delay(50);

      for (uint16_t i=0; i < pixel.numPixels(); i=i+3) {
        pixel.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
