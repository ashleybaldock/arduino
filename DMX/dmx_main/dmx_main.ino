#include <DmxSimple.h>
#include <LiquidCrystal.h>
#include "RGBUtils.h"
#include "LightMenu.h"

// Values used by DMX
#define DMX_PIN 2
#define RTS_PIN 3

#define NUM_FIXTURES 2

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

Colour red (255, 0, 0);
Colour low_red (128, 0, 0);
Colour blue (0, 0, 255);
Colour low_blue (0, 0, 128);
Colour green (0, 255, 0);
Colour low_green (0, 128, 0);
Colour yellow (255, 255, 0);
Colour magenta (255, 0, 255);
Colour cyan (0, 255, 255);


Fadeable* fixtures[NUM_FIXTURES];
int fixture_ids[NUM_FIXTURES] = {1, 4};


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
  
  fixtures[0] = slow_colour_cycle;
  
  MultiFade* lamp_cycle = new MultiFade();
  FadeSequence* segment0 = new FadeSequence();
  FadeSequence* segment1 = new FadeSequence();
  FadeSequence* segment2 = new FadeSequence();
  FadeSequence* segment3 = new FadeSequence();
  FadeSequence* segment4 = new FadeSequence();
  FadeSequence* segment5 = new FadeSequence();
  FadeSequence* segment6 = new FadeSequence();
  FadeSequence* segment7 = new FadeSequence();
  FadeSequence* segment8 = new FadeSequence();
  
  // Default sequence for lamp
  segment0->add(red, 5000);
  segment0->add(blue, 5000);
  segment0->set_delay(0);
  lamp_cycle->set_fade_sequence(0, segment0);
  segment1->add(red, 5000);
  segment1->add(blue, 5000);
  segment1->set_delay(2500);
  lamp_cycle->set_fade_sequence(1, segment1);
  segment2->add(red, 5000);
  segment2->add(blue, 5000);
  segment2->set_delay(5000);
  lamp_cycle->set_fade_sequence(2, segment2);
  segment3->add(red, 5000);
  segment3->add(blue, 5000);
  segment3->set_delay(7500);
  lamp_cycle->set_fade_sequence(3, segment3);
  segment4->add(red, 5000);
  segment4->add(blue, 5000);
  segment4->set_delay(10000);
  lamp_cycle->set_fade_sequence(4, segment4);
  segment5->add(red, 5000);
  segment5->add(blue, 5000);
  segment5->set_delay(12500);
  lamp_cycle->set_fade_sequence(5, segment5);
  segment6->add(red, 5000);
  segment6->add(blue, 5000);
  segment6->set_delay(15000);
  lamp_cycle->set_fade_sequence(6, segment6);
  segment7->add(red, 5000);
  segment7->add(blue, 5000);
  segment7->set_delay(17500);
  lamp_cycle->set_fade_sequence(7, segment7);
  segment8->add(red, 5000);
  segment8->add(blue, 5000);
  segment8->set_delay(20000);
  lamp_cycle->set_fade_sequence(8, segment8);
  lamp_cycle->start(millis());
  
  fixtures[1] = lamp_cycle;
}



void loop() {
  FadeState current;
  float current_percent = 0;
  int ret = 0;
  Colour current_colour;

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
        ret = fixtures[fixture_id]->get_current(millis(), current);
        current.get_channel(0, current_colour, current_percent);
        
        FadeSequence* new_fadesequence = new FadeSequence();
        new_fadesequence->add(colour, 1000); // Single item in sequence with same from/to, fixed colour
        new_fadesequence->set_lead_in(current_colour, 1000);

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
        ret = fixtures[fixture_id]->get_current(millis(), current);
        current.get_channel(0, current_colour, current_percent);
        new_fadesequence->set_lead_in(current_colour, 1000);
        
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
  for (int i = 0; i < NUM_FIXTURES; i++) {
    if (fixtures[i] == 0) {
      continue;
    }
    ret = fixtures[i]->get_current(millis(), current);
    
    for (int j = 0; j < current.get_num_channels(); j++) {
      current.get_channel(j, current_colour, current_percent);
      set_dmx_rgb(fixture_ids[i] + j * 3, current_colour);
    }
  }
  
  lcd.setCursor(12,0);
  lcd.print(ret);
  
  
  lcd.setCursor(12,1);
  lcd.print(current_percent);

  lcd.setCursor(7,1);            // move cursor to second line "1" and 9 spaces over
  lcd.print(millis()/1000);      // display seconds elapsed since power-up

  lcd.setCursor(0,1);            // move to the begining of the second line

}
