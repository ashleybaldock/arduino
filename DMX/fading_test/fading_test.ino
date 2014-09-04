#include <DmxSimple.h>
#include "RGBUtils.h"

int dmx_pin = 2;
int rts_pin = 3;
int loop_delay = 10;

void setup() {
  pinMode(rts_pin, OUTPUT);
  digitalWrite(rts_pin, HIGH);
  
  DmxSimple.usePin(dmx_pin);
  
  DmxSimple.write(1, 255);
  DmxSimple.write(2, 0);
  DmxSimple.write(3, 0);
}

void set_dmx_rgb(Colour colour) {
  DmxSimple.write(1, colour.r);
  DmxSimple.write(2, colour.g);
  DmxSimple.write(3, colour.b);
}

Colour start;
Colour current;
Colour target;

Fade current_fade;

void loop() {
  int brightness, colour;
  
  // Fading from/to RGB value in a time (loop_delay sets step interval)

  current_fade.next();
  set_dmx_rgb(current_fade.current);
  
  /*for (colour = 1; colour <= 3; colour++) {
    for (brightness = 0; brightness <= 255; brightness++) {
      DmxSimple.write(colour, brightness);
      delay(10); // Wait 10ms
    }
    for (brightness = 255; brightness >= 0; brightness--) {
      DmxSimple.write(colour, brightness);
      delay(10); // Wait 10ms
    }
  }*/
}
