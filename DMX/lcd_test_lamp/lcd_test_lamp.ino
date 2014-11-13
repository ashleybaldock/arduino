#include <DmxSimple.h>
#include <LiquidCrystal.h>
#include "RGBUtils.h"
#include "LightMenu.h"

// Values used by DMX
#define DMX_PIN 2
#define RTS_PIN 3

int loop_delay = 10;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
#define backlight_pin 10
int last_button = btnNONE;

int read_LCD_buttons() {
  int adc_key_in = analogRead(0);
  if (adc_key_in > 1000) return btnNONE;
  if (adc_key_in < 50)   return btnRIGHT;  
  if (adc_key_in < 250)  return btnUP; 
  if (adc_key_in < 450)  return btnDOWN; 
  if (adc_key_in < 650)  return btnLEFT; 
  if (adc_key_in < 850)  return btnSELECT;  

  return btnNONE;  // when all others fail, return this...
}

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
Fade red_blue (red, blue, 60000);
Fade blue_green (blue, green, 60000);
Fade green_red (green, red, 60000);

Fade red_blue_quick (low_red, low_blue, 5000);
Fade blue_red_quick (low_blue, low_red, 5000);

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


void setup() {
  Serial.begin(9600);
  pinMode(RTS_PIN, OUTPUT);
  digitalWrite(RTS_PIN, HIGH);
  
  DmxSimple.usePin(DMX_PIN);
  
  DmxSimple.write(1, 255);
  DmxSimple.write(2, 0);
  DmxSimple.write(3, 0);

  // Init LCD
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  set_backlight_level(0.5);
  
  
  slow_colour_cycle.add(red_blue);
  slow_colour_cycle.add(blue_green);
  slow_colour_cycle.add(green_red);
  slow_colour_cycle.start(millis());
  
  
  segment0.add(red_blue_quick);
  segment0.add(blue_red_quick);
  segment0.set_delay(0);
  lamp_cycle.set_fade_sequence(0, &segment0);
  segment1.add(red_blue_quick);
  segment1.add(blue_red_quick);
  segment1.set_delay(2500);
  lamp_cycle.set_fade_sequence(1, &segment1);
  segment2.add(red_blue_quick);
  segment2.add(blue_red_quick);
  segment2.set_delay(5000);
  lamp_cycle.set_fade_sequence(2, &segment2);
  segment3.add(red_blue_quick);
  segment3.add(blue_red_quick);
  segment3.set_delay(7500);
  lamp_cycle.set_fade_sequence(3, &segment3);
  segment4.add(red_blue_quick);
  segment4.add(blue_red_quick);
  segment4.set_delay(10000);
  lamp_cycle.set_fade_sequence(4, &segment4);
  segment5.add(red_blue_quick);
  segment5.add(blue_red_quick);
  segment5.set_delay(12500);
  lamp_cycle.set_fade_sequence(5, &segment5);
  segment6.add(red_blue_quick);
  segment6.add(blue_red_quick);
  segment6.set_delay(15000);
  lamp_cycle.set_fade_sequence(6, &segment6);
  segment7.add(red_blue_quick);
  segment7.add(blue_red_quick);
  segment7.set_delay(17500);
  lamp_cycle.set_fade_sequence(7, &segment7);
  segment8.add(red_blue_quick);
  segment8.add(blue_red_quick);
  segment8.set_delay(20000);
  lamp_cycle.set_fade_sequence(8, &segment8);

  lamp_cycle.start(millis());  
}


void loop() {
  Colour current;
  float percent = 20;
  int ret = 0;
  
  // Light bar
  ret = slow_colour_cycle.get_current(millis(), current, percent);
  
  lcd.setCursor(12,0);
  lcd.print(ret);
  
  set_dmx_rgb(1, current);
  
  lcd.setCursor(12,1);
  lcd.print(percent);
  Serial.println(percent);
 
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
  int lcd_key = read_LCD_buttons();  // read the buttons

  switch (lcd_key) {
    case btnRIGHT: {
      if (last_button != btnRIGHT) {
        last_button = btnRIGHT;
        if (current_menu_item->right != NULL) {
          //current_menu_item->exit_right();  // Call function pointer
          current_menu_item = current_menu_item->right;
          //current_menu_item->enter();       // Call function pointer
        }
      }
      lcd.print("RIGHT ");
      break;
      }
    case btnLEFT: {
      if (last_button != btnLEFT) {
        last_button = btnLEFT;
      }
      lcd.print("LEFT   ");
      break;
    }
    case btnUP: {
      if (last_button != btnUP) {
        last_button = btnUP;
      }
      lcd.print("UP    ");
      break;
    }
    case btnDOWN: {
      if (last_button != btnDOWN) {
        last_button = btnDOWN;
      }
      lcd.print("DOWN  ");
      break;
    }
    case btnSELECT: {
      if (last_button != btnSELECT) {
        last_button = btnSELECT;
      }
      lcd.print("SELECT");
      break;
    }
    case btnNONE: {
      last_button = btnNONE;
      lcd.print("NONE  ");
      break;
    }
  }
}
