# Pro Micro USB LCD

Firmware and circuit design for the SparkFun Pro Micro, for displaying data 
sent to the USB port on a small HD44780-style LCD display. This provides
an "auxilliary display" for showing status information, etc.

The program responds to the following control characters, as well as the
usual ASCII set:

Line feed (10) -- move to the next row
Carriage return (13) -- move to start of line
Form feed (12) -- clear the display

The HD44780 supports 8-bit characters, but not in any standard encoding
-- you'll need to look at the datasheet to see the character table for
non-ASCII characters.

See the scripts `clock_sample.sh` and `status_sample.sh` to see how this 
program might be used in Linux. There's nothing Linux-specific about the
firmware, but I'm not sure how to interact with COM ports in Windows at
the script level (if it's even possible).

Note -- if you're seeing "ATxxx" commands being displayed when using this
design with a Linux host, this is probably because the Linux modem manager is
trying to control the USB port as if it were a serial model. This isn't
something that can be fixed in this software (except by parsing and handling AT
commands). Normal USB data sent by an application will clear 
these bogus AT commands from the display.

For full details of the design, see

http://kevinboone.me/pro-micro-usb-lcd.html

Note that I don't do IDEs or, for the most part, Windows. This firmware
is intended to be built using command-line tools on Linux. For more
information on building Arduino programs this way, see:

http://kevinboone.me/pro-micro-blink.html

