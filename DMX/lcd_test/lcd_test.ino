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

void set_dmx_rgb(const Colour& colour) {
  DmxSimple.write(1, colour.r);
  DmxSimple.write(2, colour.g);
  DmxSimple.write(3, colour.b);
}

MenuItem* current_menu_item;

Colour red (255, 0, 0);
Colour blue (0, 0, 255);
Colour green (0, 255, 0);
Colour yellow (255, 255, 0);
Colour magenta (255, 0, 255);
Colour cyan (0, 255, 255);
Fade red_blue ( red, blue, 60000);
Fade blue_green ( blue, green, 60000);
Fade green_red ( green, red, 60000);


Fade* current_fade = &red_blue;

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
  lcd.print("Push the buttons");
  set_backlight_level(0.5);
  
  red_blue.next = &blue_green;
  blue_green.next =  &green_red;
  green_red.next = &red_blue;

  current_fade->start(millis());
}


void loop() {
  
  // Update current Fade (if any)
  Colour current;
  float percent;
  int ret = current_fade->current(millis(), current, percent);
  
  set_dmx_rgb(current);
  
  lcd.setCursor(12,1);            // move cursor to second line "1" and 9 spaces over
  lcd.print(percent);
  Serial.println(percent);

  if (ret == RGBUtils_Fade_Completed) {
    current_fade = current_fade->next;
    current_fade->start(millis());
  }
  
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
