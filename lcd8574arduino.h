/*============================================================================

  lcd8574.h

  Functions to control an HD44780 LCD module via an PCF8574 I2C-to-parallel
  controller. The LCD controller is operated in 4-bit mode, so all the
  necessary pins can be connected to one PCF8574 device, which has only
  eight digitial outputs.

  There are many ways to connect the PCF8574 to the HD8840. Please see
  the definitions at the top of lcd8574arduino.c, to see typical connections
  (or edit the file if your connections are different).

  Although both the PCF8574 and the HD44780 have data-read
  operations, this code makes no use of them. If the module's R/W pin
  in connected, it is set permanently low, for write mode.

  Copyright (c)1990-2020 Kevin Boone. Distributed under the terms of the
  GNU Public Licence, v3.0

  ==========================================================================*/

#pragma once

#include <inttypes.h>
#include <Wire.h>

// Constant for pixel size, used by the constructor. In practive, 
//  only 5x8 seems to be used, and I think the 5x10 is only available
//  in single-line displays
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

class LCD8574Arduino  
{
public:
  /** LCD8574Arduino constructor -- specify the I2C address and the
      size of the panel. */
  LCD8574Arduino (uint8_t lcdi2c_addr,uint8_t lcdcols,uint8_t lcdrows,
    uint8_t charsize = LCD_5x8DOTS);

  /** Initialze the display hardware. Call this after the constructor. 
   *  In principle this method can fail, but we
   *  have no real way to find out if it does.
   */
  void init();

  /** Clear the screen, and implicitly home the cursor. */
  void clear();

  /** Send the cursor to the home position. */
  void home();
  
  /** Turn the display on. */
  void display_on (void);

  /** Turn the display off. */
  void display_off (void);

  /** Set the cursor to blink. */
  void blink_on (void);

  /** Set the cursor not to blink. */
  void blink_off (void);

  /** Show the cursor. */
  void cursor_on (void);

  /** Hide the cursor. */
  void cursor_off (void);

  /** Use the LCD panel's logic to scroll the whole display left. */
  void scroll_left();

  /** Use the LCD panel's logic to scroll the whole display right. */
  void scroll_right();

  /** Enable left-to-right text layout (default). */ 
  void left_to_right (void);

  /** Enable right-to-left text layout (default). */ 
  void right_to_left (void);

  /** Turn the LCD backlight on. */ 
  void backlight_on (void);

  /** Turn the LCD backlight off. */ 
  void backlight_off (void);
  
  /** Enable the LCD display's built-in text scrolling. */
  void autoscroll_on (void);

  /** Disable the LCD display's built-in text scrolling. */
  void autoscroll_off (void); 


  /** Set the cursor position (zero-based). */
  void set_cursor (uint8_t row, uint8_t col); 

  /** Write a character at the specific location. Handle line wrapping,
   *  etc, in this class, not in the LCD display. */
  void write_char_at (uint8_t row, uint8_t col, char c);

  /** Write the character at the current entry point. What happens if
      the text overwrites the line or the panel is entirely under the control
      of the display hardware, not controlled by this class. */
  void write_char_raw (char c);

  /** Write a character at the current position, handling line wrapping, etc.
   *  in this software, not in the display. */
  void write_char (char c);

  /** Print the specified string, handling line wrapping, etc., in this
   *  class, not in the LCD hardware. */
  void print (const char *c);

  /** Print the specified string, allowing the LCD hardware to decide what
   *   to do about line wrapping, etc. */
  void print_raw (const char *c);

private:
  /* Note that private methods are documented in the .cpp source file */
  void send_byte (uint8_t, uint8_t);
  void write4bits (uint8_t);
  void write_i2c_byte (uint8_t);
  void command (uint8_t);
  void do_clock(uint8_t);
  void write_normal_char (uint8_t c);
  void write_form_feed (void);
  void write_line_feed (void);
  void write_cr (void);

  uint8_t charsize; // As set in the constructor
  uint8_t i2c_addr; // As set in the constructor
  uint8_t hardware_mode; // Lines, pixel size, etc
  uint8_t display_mode; // Power, cursor, blink, etc
  uint8_t text_handling_mode; // Direction, scrolling, etc
  uint8_t cols;
  uint8_t rows;

  // A value computed from the pin that is connected to the backlight
  //   LED on the panel
  uint8_t backlight_flag;

  // Current cursor row 
  uint8_t current_row;
  // Current cursor column
  uint8_t current_col;

};

