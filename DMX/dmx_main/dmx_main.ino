#include <DmxSimple.h>
#include <LiquidCrystal.h>
#include "RGBUtils.h"
#include "LightMenu.h"

// Values used by DMX
#define DMX_PIN 2
#define RTS_PIN 3

#define NUM_FIXTURES 10

int loop_delay = 10;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons
#define backlight_pin 10

#define commandFixedColour   1
#define commandFadeSequence  2

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

MenuItem* current_menu_item;


Colour red (255, 0, 0);
Colour low_red (128, 0, 0);
Colour blue (0, 0, 255);
Colour low_blue (0, 0, 128);
Colour green (0, 255, 0);
Colour low_green (0, 128, 0);
Colour yellow (255, 255, 0);
Colour magenta (255, 0, 255);
Colour cyan (0, 255, 255);

FadeSequence slow_colour_cycle;

FadeSequence segment_change;
FadeSequence segment0;
FadeSequence segment1;
FadeSequence segment2;
FadeSequence segment3;
FadeSequence segment4;
FadeSequence segment5;
FadeSequence segment6;
FadeSequence segment7;
FadeSequence segment8;

MultiFade lamp_cycle;

// Fixtures (implement Fadeable interface)
// Assumed to be dynamically allocated objects since delete will be used on them
Fadeable* fixtures[NUM_FIXTURES];


void setup() {
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
  FadeSequence* slow_colour_cycle = new FadeSequence();
  
  slow_colour_cycle->add(red, 60000);
  slow_colour_cycle->add(blue, 60000);
  slow_colour_cycle->add(green, 60000);
  slow_colour_cycle->start(millis());
  
  fixtures[1] = slow_colour_cycle;
  
  
  // Default sequence for lamp
  segment0.add(red, 5000);
  segment0.add(blue, 5000);
  segment0.set_delay(0);
  lamp_cycle.set_fade_sequence(0, &segment0);
  segment1.add(red, 5000);
  segment1.add(blue, 5000);
  segment1.set_delay(2500);
  lamp_cycle.set_fade_sequence(1, &segment1);
  segment2.add(red, 5000);
  segment2.add(blue, 5000);
  segment2.set_delay(5000);
  lamp_cycle.set_fade_sequence(2, &segment2);
  segment3.add(red, 5000);
  segment3.add(blue, 5000);
  segment3.set_delay(7500);
  lamp_cycle.set_fade_sequence(3, &segment3);
  segment4.add(red, 5000);
  segment4.add(blue, 5000);
  segment4.set_delay(10000);
  lamp_cycle.set_fade_sequence(4, &segment4);
  segment5.add(red, 5000);
  segment5.add(blue, 5000);
  segment5.set_delay(12500);
  lamp_cycle.set_fade_sequence(5, &segment5);
  segment6.add(red, 5000);
  segment6.add(blue, 5000);
  segment6.set_delay(15000);
  lamp_cycle.set_fade_sequence(6, &segment6);
  segment7.add(red, 5000);
  segment7.add(blue, 5000);
  segment7.set_delay(17500);
  lamp_cycle.set_fade_sequence(7, &segment7);
  segment8.add(red, 5000);
  segment8.add(blue, 5000);
  segment8.set_delay(20000);
  lamp_cycle.set_fade_sequence(8, &segment8);

  lamp_cycle.start(millis());  
}



void loop() {
  float percent = 0;
  int ret = 0;
  Colour current;

  // Read serial line
  // Decode command
  // Switch based on command (maybe to set up new fade)
  
  // Fixture ID,Command
  //            Fixed Colour (1),R,G,B
  //            Sequence     (2),# of steps (1-255),Step N
  //                                                R,G,B,Duration
  // e.g. 1,1,255,255,0\n
  
  int fixture_id = 0, command = 0;
  int red = 0, green = 0, blue = 0, steps = 0, duration = 0;
  Colour colour;
  if (Serial.available() > 0) {
    fixture_id = Serial.parseInt(); // Maybe 0, commands sent to the arduino itself
    Serial.print("Command for fixture ID: ");
    Serial.print(fixture_id);
    Serial.println("");
    command = Serial.parseInt();
    Serial.print("Command ID: ");
    Serial.print(command);
    switch (command) {
      case commandFixedColour: {
        Serial.println(" - Fixed colour command");
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
        
        // Create fade sequence that fades from current colour to specified colour
        ret = fixtures[fixture_id]->get_current(millis(), current, percent);
        
        FadeSequence* new_fadesequence = new FadeSequence();
        new_fadesequence->add(colour, 1000); // Single item in sequence with same from/to, fixed colour
                new_fadesequence->set_lead_in(current, 1000);

        // Free previous fixture_1 Fadeable object
        delete fixtures[fixture_id];
        fixtures[fixture_id] = new_fadesequence;
        fixtures[fixture_id]->start(millis());
        break;
      }
      case commandFadeSequence: {
        Serial.println(" - Fade sequence command");
        FadeSequence* new_fadesequence = new FadeSequence();
        steps = Serial.parseInt();
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

          new_fadesequence->add(colour, duration);
        }
        
        // Set lead-in fade
        ret = fixtures[fixture_id]->get_current(millis(), current, percent);
        new_fadesequence->set_lead_in(current, 1000);
        
        // Free previous fixture_1 Fadeable object
        delete fixtures[fixture_id];
        fixtures[fixture_id] = new_fadesequence;
        fixtures[fixture_id]->start(millis());
        break;
      }
    }
    if (Serial.read() == '\n') {
      // End of line, end of command sequence
      Serial.println("Got newline - end of command");
    }
  }
  
  // Update all fixtures
  
  // Light bar (fixture 1)
  ret = fixtures[1]->get_current(millis(), current, percent);

  lcd.setCursor(12,0);
  lcd.print(ret);
  
  set_dmx_rgb(1, current);
  
  lcd.setCursor(12,1);
  lcd.print(percent);
 
  // Lamp
  ret = lamp_cycle.get_current(0, millis(), current, percent);
  set_dmx_rgb(4, current);
  ret = lamp_cycle.get_current(1, millis(), current, percent);
  set_dmx_rgb(7, current);
  ret = lamp_cycle.get_current(2, millis(), current, percent);
  set_dmx_rgb(10, current);
  ret = lamp_cycle.get_current(3, millis(), current, percent);
  set_dmx_rgb(13, current);
  ret = lamp_cycle.get_current(4, millis(), current, percent);
  set_dmx_rgb(16, current);
  ret = lamp_cycle.get_current(5, millis(), current, percent);
  set_dmx_rgb(19, current);
  ret = lamp_cycle.get_current(6, millis(), current, percent);
  set_dmx_rgb(22, current);
  ret = lamp_cycle.get_current(7, millis(), current, percent);
  set_dmx_rgb(25, current);
  ret = lamp_cycle.get_current(8, millis(), current, percent);
  set_dmx_rgb(28, current);

  
  // Get current status of buttons
  // Update current button - only execute function if status changes
  // (to None or another button) to avoid multiple presses registering
  
  lcd.setCursor(7,1);            // move cursor to second line "1" and 9 spaces over
  lcd.print(millis()/1000);      // display seconds elapsed since power-up

  lcd.setCursor(0,1);            // move to the begining of the second line


}
