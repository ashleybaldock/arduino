#include <DmxSimple.h>
#include <LiquidCrystal.h>
#include "RGBUtils.h"
#include "LightMenu.h"
#include "MemoryFree.h"

// Values used by DMX
#define DMX_PIN 2
#define RTS_PIN 3

#define NUM_FIXTURES 2

int loop_delay = 10;

int lamp_pin_set = 0;
unsigned long lamp_pin_last_set_time = 0;
unsigned long lamp_pin_delay = 100;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons
#define backlight_pin 10

#define lamp_pin 11

#define commandFixedColour   1
#define commandFadeSequence  2
#define commandMultiFade     3
#define commandLampTouch     4

// Set backlight level using percentage
void set_backlight_level(float percentage) {
  unsigned char level = percentage * 255;
  analogWrite(backlight_pin, level);
}

void set_dmx_rgb(int channel_base, const Colour& colour) {
  DmxSimple.write(channel_base, colour.r);
  DmxSimple.write(channel_base + 1, colour.g);
  DmxSimple.write(channel_base + 2, colour.b);
}

Colour red (255, 0, 0);
Colour blue (0, 0, 255);
Colour green (0, 255, 0);
Colour yellow (255, 255, 0);
Colour magenta (255, 0, 255);
Colour cyan (0, 255, 255);

// Create and configure fixtures
FadeSequence fixture1;
MultiFade fixture2(9);

Fixture fixtures[NUM_FIXTURES] = {Fixture(1, &fixture1, Fixture_Single),
                                  Fixture(4, &fixture2, Fixture_Multi)};

void setup() {
  digitalWrite(lamp_pin, LOW);
  pinMode(lamp_pin, INPUT);
  
  Serial.begin(9600);
  pinMode(RTS_PIN, OUTPUT);
  digitalWrite(RTS_PIN, HIGH);
  Serial.println("Startup");
  
  DmxSimple.usePin(DMX_PIN);
  DmxSimple.write(1, 255);
  DmxSimple.write(2, 0);
  DmxSimple.write(3, 0);

  // Init LCD
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  set_backlight_level(0.2);
  
  // Default fade sequence for light bar (fixture 1)
  fixture1.set_step_count(3);
  fixture1.set_step(0, red, 6000);
  fixture1.set_step(1, blue, 6000);
  fixture1.set_step(2, green, 6000);
  fixture1.start(millis());
  //fixtures[0] = &fixture1;
  
  // Default sequence for lamp (fixture 2)
  for (int i = 0; i < 9; i++) {
    fixture2.set_step_count(i, 2);
  }
  
  fixture2.set_step(0, 0, red, 5000);
  fixture2.set_step(0, 1, blue, 5000);
  fixture2.set_delay(0, 0);
  fixture2.set_step(1, 0, red, 5000);
  fixture2.set_step(1, 1, blue, 5000);
  fixture2.set_delay(1, 2500);
  fixture2.set_step(2, 0, red, 5000);
  fixture2.set_step(2, 1, blue, 5000);
  fixture2.set_delay(2, 5000);
  fixture2.set_step(3, 0, red, 5000);
  fixture2.set_step(3, 1, blue, 5000);
  fixture2.set_delay(3, 7500);
  fixture2.set_step(4, 0, red, 5000);
  fixture2.set_step(4, 1, blue, 5000);
  fixture2.set_delay(4, 10000);
  fixture2.set_step(5, 0, red, 5000);
  fixture2.set_step(5, 1, blue, 5000);
  fixture2.set_delay(5, 12500);
  fixture2.set_step(6, 0, red, 5000);
  fixture2.set_step(6, 1, blue, 5000);
  fixture2.set_delay(6, 15000);
  fixture2.set_step(7, 0, red, 5000);
  fixture2.set_step(7, 1, blue, 5000);
  fixture2.set_delay(7, 17500);
  fixture2.set_step(8, 0, red, 5000);
  fixture2.set_step(8, 1, blue, 5000);
  fixture2.set_delay(8, 20000);
  fixture2.start(millis());
  Serial.println("Startup done");
}



void loop() {
  FadeState current;
  float current_percent = 0;
  int ret = 0;
  Colour current_colour;

  lcd.setCursor(0,0);            // move to the begining of the first line
  lcd.print(freeMemory());

  // Read serial line
  // Decode command
  // Switch based on command (maybe to set up new fade)
  
  // Fixture ID,Command
  //            Fixed Colour (1),R,G,B
  //            Sequence     (2),# of steps (1-255),Step N
  //                                                R,G,B,Duration
  //            MultiFade    (3),# of sections (9),Section N
  //                                               Delay,# of steps (1-255),Step N
  //                                                                        R,G,B,Duration
  //            LampTouch    (4)
  // e.g. 1,1,255,255,0\n
  
  int fixture_id = 0, command = 0;
  int red = 0, green = 0, blue = 0, steps = 0;
  unsigned long duration = 0;
  Colour colour;
  if (Serial.available() > 0) {
    fixture_id = Serial.parseInt() - 1; // Maybe 0, commands sent to the arduino itself
    Serial.print("Command for fixture ID: ");
    Serial.print(fixture_id);
    Serial.println("");
    command = Serial.parseInt();
    Serial.print("Command ID: ");
    Serial.print(command);
    switch (command) {
      case commandFixedColour: {
        Serial.println(" - Fixed colour command");
        // Create fade sequence that fades from current colour to specified colour
        ret = fixtures[fixture_id].sequence->get_current(millis(), current);
        current.get_channel(0, current_colour, current_percent);

        red = Serial.parseInt();
        Serial.print("RGB: (");
        Serial.print(red);
        green = Serial.parseInt();
        Serial.print(", ");
        Serial.print(green);
        blue = Serial.parseInt();
        Serial.print(", ");
        Serial.print(blue);
        Serial.println(")");
        colour = Colour(red, green, blue);
        
        FadeSequence* seq = static_cast<FadeSequence*>(fixtures[fixture_id].sequence);
        seq->reset();
        seq->set_step_count(1);
        seq->set_step(0, colour, 1000);
        seq->set_lead_in(current_colour, 1000);

        fixtures[fixture_id].sequence->start(millis());
        break;
      }
      case commandFadeSequence: {
        Serial.println(" - Fade sequence command");
        // Create fade sequence that fades from current colour to specified colour
        ret = fixtures[fixture_id].sequence->get_current(millis(), current);
        current.get_channel(0, current_colour, current_percent);

        steps = Serial.parseInt();
        FadeSequence* seq = static_cast<FadeSequence*>(fixtures[fixture_id].sequence);
        seq->reset();
        seq->set_step_count(steps);

        for (int i = 0; i < steps; i++) {
          Serial.print("Step: ");
          Serial.print(i);
          red = Serial.parseInt();
          Serial.print("RGB: (");
          Serial.print(red);
          green = Serial.parseInt();
          Serial.print(", ");
          Serial.print(green);
          blue = Serial.parseInt();
          Serial.print(", ");
          Serial.print(blue);
          colour = Colour(red, green, blue);
          duration = Serial.parseInt();
          Serial.print(") Duration: ");
          Serial.print(duration);
          Serial.println("ms");

          seq->set_step(0, colour, duration);
        }
        
        // Set lead-in fade
        seq->set_lead_in(current_colour, 1000);

        seq->start(millis());
        break;
      }
      case commandLampTouch: {
        Serial.println(" - Lamp Touch command");
        // Set lamp control pin to grounded output for 500ms
        pinMode(lamp_pin, OUTPUT);
        lamp_pin_set = 1;
        lamp_pin_last_set_time = millis();
        break;
      }
    }
    if (Serial.read() == '\n') {
      // End of line, end of command sequence
      Serial.println("Got newline - end of command");
    }
  }
  
  // Check if lamp control pin should revert to "off" state
  if (lamp_pin_set == 1) {
    if (lamp_pin_last_set_time + lamp_pin_delay < millis()) {
      pinMode(lamp_pin, INPUT);
      lamp_pin_set = 0;
    }
  }
  
  // Update all fixtures
  // Fixtures may either have a FadeSequence or MultiFade associated with them
  for (int i = 0; i < NUM_FIXTURES; i++) {
    ret = fixtures[i].sequence->get_current(millis(), current);
    
    for (int j = 0; j < current.get_num_channels(); j++) {
      Serial.print("loop: i: ");
      Serial.print(i);
      Serial.print(", j:");
      Serial.print(j);
      current.get_channel(j, current_colour, current_percent);
      set_dmx_rgb(fixtures[i].address + j * 3, current_colour);
    }
  }

  lcd.setCursor(7,1);            // move cursor to second line "1" and 9 spaces over
  lcd.print(millis()/1000);      // display seconds elapsed since power-up
}
