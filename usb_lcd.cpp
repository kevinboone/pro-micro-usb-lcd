/**

A simple program for the SparkFun Pro Micro, or similar Arduino board,
that uses the LCD8574Arduino to display information received on the
USB port.

The program responds to the following control characters:

Line feed (10) -- move to the next row
Carriage return (13) -- move to start of line
Form feed (12) -- clear the display

See the file "clock_sample.sh" to see how this program might be
used in Linux.

Note -- if you're seeing "ATxxx" commands being displayed, this is probably
because the modem manager is trying to control the USB port as if it were
a serial model. This isn't something that can be fixed in this software 
(except by parsing and handling AT commands).

Kevin Boone, January 2021

*/

#include <Arduino.h>
#include <HardwareSerial.h>
#include "lcd8574arduino.h" 

#define BANNER "usb-lcd\r\nReady"

// Create LCD panel instance, specifying size
LCD8574Arduino lcd (0x27, 16, 2);

// Clear banner will be set after the initial banner is cleared,
// after receiving the first character from USB
bool cleared_banner = false;

/** 
 * setup
 * Initialize the USB port and the LCD panel
 */
void setup()
  {
  Serial.begin (57600); 

  lcd.init();
  lcd.backlight_on();
  //lcd.cursor_on(); // Some application might need the cursor on
  lcd.print (BANNER);
  }


/** 
 * loop 
 * Repeat every time a character is received 
 */
void loop()
  {
  Serial.flush();

  // Wait until a character has been received
  while (!Serial.available());

  // Clear the banner if necessary
  if (!cleared_banner)
    {
    lcd.clear();
    cleared_banner = true;
    }

  // Read and display the character
  uint8_t c = Serial.read();
  lcd.write_char (c);
  }

