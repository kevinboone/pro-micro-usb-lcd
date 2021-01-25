/*==========================================================================
  
    lcd8574_arduino.c

    Implementation of the class that is specified in 
    lcd8574_arduino.h. This file contains methods for initializing the LCD 
    module and writing text to it.

    Datasheet for the HD44780:
    https://www.sparkfun.com/datasheets/LCD/HD44780.pdf

    Datasheet for the PCF8574:
    https://www.ti.com/lit/ds/symlink/pcf8574.pdf

    Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/
#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>

#include "lcd8574arduino.h"

// These first few defines map the wiring of the D(n) pins on the
// 8547 i2c-to-parallel IC to the lines of the LCD display.
// They will need to be changed if you've not used the same wiring as
//   I have.
// For the record, my wiring is:
// Register select (cmd/data) -- pin D0 
// R/W (not used here) -- pin D1
// Clock (enable) -- pin D2
// LED backlight -- pin D3
// Pins D4-D7 map to the four data lines that are used on the LCD
//   display in 4-bit mode

// Cmd/data (register select) flag -- pin 0 = B1
#define LCD_CMDDATA_FLAG 1

// R/W bit (for completeness) -- pin 1 = B10. Not used at present
#define LCD_RW_FLAG B00000010  

// Flag for enable (clock) line -- pin 2 = B100 
#define LCD_ENABLE_FLAG B00000100  

// Flags for backlight control -- pin 4 = B1000
#define LCD_BACKLIGHT_FLAG B1000 

// HD44780 LCD module command set (see datasheet)
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// HD44780 flags for text layout mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// HD44780 flags for display mode (power, cursor, etc) 
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// HD44780 flags for cursor and text scrolling 
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// HD44780 flags for hardware mode 
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00


/**
 * LCD8574Arduino constructor
 */
LCD8574Arduino::LCD8574Arduino (uint8_t lcdi2c_addr, 
    uint8_t _cols, uint8_t _rows, uint8_t charsize)
  {
  i2c_addr = lcdi2c_addr;
  cols = _cols;
  rows = _rows;
  charsize = charsize;
  // Turn backlight one by default -- display is useless without it
  backlight_flag = LCD_BACKLIGHT_FLAG;
  }

/**
 * init
 */
void LCD8574Arduino::init()
  {
  Wire.begin();

  hardware_mode = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  if (rows > 1) 
    {
    hardware_mode |= LCD_2LINE;
    }

  // for some 1 line displays you can select a 10 pixel high font
  if ((charsize != 0) && (rows == 1)) 
    {
    hardware_mode |= LCD_5x10DOTS;
    }

  delay(50); 
  
  // Now we pull both RS and R/W low to begin commands
  write_i2c_byte (backlight_flag);	
  delay(1000);

  // Set into 4-bit mode
  //  // Now... this is all a bit nasty...
  // We need to set 4-bit mode, but the LCD module powers up in
  //  eight bit mode. We can't be sure this is the first program
  //  to use the LCD since power-up, so we don't know what
  //  mode it's in. And we need to issue a command to set 4-bit
  //  mode -- without knowing what mode we're in. So first we have
  //  to enable 8-bit mode and then, knowing we're in 8-bit mode,
  //  we must set 4-bit mode. Setting 8-bit mode without knowing the
  //  current mode can be accomplished by sending the mode-setting
  //  command as three identical 4-bit commands. If we start in
  //  8-bit mode, some of these commands are gibberish 8-bit
  //  commands with four of their bits set wrongly. But there's still
  //  enough coherence for the module to get the message with thi
  //  command sequence. This method of setting the mode is widely
  //  used, even though it isn't documented, and it seems to work OK.

  write4bits (0x03 << 4);
  delayMicroseconds(4500); 
   
  write4bits(0x03 << 4);
  delayMicroseconds(4500); 
   
  write4bits(0x03 << 4); 
  delayMicroseconds(150);
   
  write4bits(0x02 << 4); 

  command (LCD_FUNCTIONSET | hardware_mode);  
	
  display_mode = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  display_on();
	
  // Clear display
  clear();
	
  // Initialize text handling settings 
  text_handling_mode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  command (LCD_ENTRYMODESET | text_handling_mode);
	
  // Home the cursor (not visible at this point)
  home();
  }

/**
 * clear
 * Note that the hardware implicitly resets the cursor, so we must reflect
 * that in the saved position
 */
void LCD8574Arduino::clear()
  {
  command (LCD_CLEARDISPLAY);
  delayMicroseconds (2000);  
  current_row = 0;
  current_col = 0;
  }

/**
 * home
 */
void LCD8574Arduino::home() 
  {
  command (LCD_RETURNHOME);  // set cursor position to origin
  delayMicroseconds(2000);   // allow time -- this operation is slow 
  current_row = 0;
  current_col = 0;
  }

/** 
 * set_cursor 
 */
void LCD8574Arduino::set_cursor (uint8_t row, uint8_t col)
  {
  static int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if (row < rows) 
    {
    command (LCD_SETDDRAMADDR | (col + row_offsets[row]));
    current_row = row;
    current_col = col;
    }
  }

/** 
 * display_off
 */
void LCD8574Arduino::display_off (void) 
  {
  display_mode &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | display_mode);
  }

/** 
 * display_on 
 */
void LCD8574Arduino::display_on (void) 
  {
  display_mode |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | display_mode);
  }

/** 
 * cursor_off
 */
void LCD8574Arduino::cursor_off (void) 
  {
  display_mode &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | display_mode);
  }

/** 
 * cursor_on 
 */
void LCD8574Arduino::cursor_on (void) 
  {
  display_mode |= LCD_CURSORON;
  command (LCD_DISPLAYCONTROL | display_mode);
  }

/** 
 * blink off 
 */
void LCD8574Arduino::blink_off (void) 
  {
  display_mode &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | display_mode);
  } 

/** 
 * blink on 
 */
void LCD8574Arduino::blink_on (void) 
  {
  display_mode |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | display_mode);
  }

/** 
 * scroll_left
 */
void LCD8574Arduino::scroll_left (void) 
  {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
  }

/** 
 * scroll_right
 */
void LCD8574Arduino::scroll_right (void) 
  {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
  }

/** 
 * left_to_right
 */
void LCD8574Arduino::left_to_right (void) 
  {
  text_handling_mode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | text_handling_mode);
  }

/** 
 * right_to_left
 */
void LCD8574Arduino::right_to_left (void) 
  {
  text_handling_mode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | text_handling_mode);
  }

/** 
 * autoscroll_on 
 */
void LCD8574Arduino::autoscroll_on (void) 
  {
  text_handling_mode |= LCD_ENTRYSHIFTINCREMENT;
  command (LCD_ENTRYMODESET | text_handling_mode);
  }

/** 
 * autoscroll_off
 */
void LCD8574Arduino::autoscroll_off (void) 
  {
  text_handling_mode &= ~LCD_ENTRYSHIFTINCREMENT;
  command (LCD_ENTRYMODESET | text_handling_mode);
  }

/**
 * backlight_off
 */
void LCD8574Arduino::backlight_off (void) 
  {
  // Write a dummy (NOP) command, just to set the 
  //  backlight pin 
  backlight_flag = 0;
   write_i2c_byte (0);
  }

/**
 * backlight_on
 */
void LCD8574Arduino::backlight_on (void) 
  {
  backlight_flag = LCD_BACKLIGHT_FLAG;
  // Write a dummy (NOP) command, just to set the 
  //  backlight pin 
  write_i2c_byte (0);
  }

/** 
 * write_char_at
 */
void LCD8574Arduino::write_char_at (uint8_t row, uint8_t col, char c)
  {
  if (row < rows && col < cols)
    {
    set_cursor (row, col);
    send_byte (c, 1);
    }
  }

/** 
 * write_char_raw 
 */
void LCD8574Arduino::write_char_raw (char c)
  {
  send_byte (c, 1);
  }

/** 
 * write_char 
 */
void LCD8574Arduino::write_char (char c)
  {
  switch (c)
    {
    case 10:
      write_line_feed ();
      break;
    case 12:
      write_form_feed ();
      break;
    case 13:
      write_cr ();
      break;
    default:
      write_normal_char (c);
    }
  }

/** 
 * print
 */
void LCD8574Arduino::print (const char *s)
  {
  while (*s)
    {
    write_char (*s);
    s++;
    }
  }

/** 
 * print_raw
 */
void LCD8574Arduino::print_raw (const char *s)
  {
  while (*s)
    {
    write_char_raw (*s);
    s++;
    }
  }

/* =========================================================================
       private functions below this point
=========================================================================*/

/** 
 * command
 * Send a command to the display. That is, sent a byte, with the
 * cmd/data pin set low 
 */
inline void LCD8574Arduino::command (uint8_t value) 
  {
  send_byte (value, 0);
  }

/** 
 * send_byte
 * Send a byte as two four-bit blocks, with the cmd/data mode pin
 * set as specific. This should be zero for commands, and one for data
 */
void LCD8574Arduino::send_byte (uint8_t value, uint8_t data_mode) 
  {
  uint8_t high = value & 0xf0;
  uint8_t low = (value<<4) & 0xf0;
  uint8_t flag;
  if (data_mode)
    flag = LCD_CMDDATA_FLAG;
  else
     flag = 0;
  write4bits (high | flag);
  write4bits (low | flag); 
  }

/**
 * write4bits
 * Write a 4-bit block. We actually send 8 bits, because that is how
 * the i2c-to-parallel conversion works. The 4 non-data bits reflect
 * cmd/data selection, backlight, etc.
 */
void LCD8574Arduino::write4bits (uint8_t value) 
  {
  write_i2c_byte (value);
  do_clock(value);
  }

/**
 * Write a single byte onto the I2C channel, using the Wire library.
 * Note that one of the outputs of the 8547 might be connected to the
 * backlight. We need to keep this output at the present value, whatever
 * other data bits are set.
 */
void LCD8574Arduino::write_i2c_byte (uint8_t data)
  {                                        
  Wire.beginTransmission (i2c_addr);
  Wire.write ((int)(data) | backlight_flag);
  Wire.endTransmission();   
  }

/** do_clock
 * Take the clock (enable) high for one microsecond, then 
 * low for 50 microseconds, while keeping the other outputs
 * of the 8547 (as specified in data)
 * the same. This has the effect of strobing only
 * the clock line. We use this function to clock in commands and
 * data, four bits at a time. 
 */
void LCD8574Arduino::do_clock(uint8_t data)
  {
  write_i2c_byte (data | LCD_ENABLE_FLAG);	
  delayMicroseconds(1);		
  write_i2c_byte (data & ~LCD_ENABLE_FLAG);	
  // Allow at least 37 usec to settle
  delayMicroseconds(50);
  } 

/**
 * write_normal_char
 * Writes an "ordinary" character -- not a control character.
 */
void LCD8574Arduino::write_normal_char (uint8_t c)
  {
  if (current_row < rows)
    {
    set_cursor (current_row, current_col);
    send_byte (c, 1);
    current_col++;
    if (current_col >= cols)
      {
      current_row++;
      current_col = 0;
      }
    }
  else
    {
    // TODO -- overflow, scroll, etc
    }
  }

/**
 * write_form_feed
 * In practice, just clears the screen 
 */
void LCD8574Arduino::write_form_feed (void)
  {
  clear();
  }

/**
 * write_line_feed
 */
void LCD8574Arduino::write_line_feed (void)
  {
  if (current_row < rows - 1)
    {
    current_row++;
    set_cursor (current_row, current_col);
    }
  }

/**
 * write_cr
 */
void LCD8574Arduino::write_cr (void)
  {
  current_col = 0;
  set_cursor (current_row, current_col);
  }


