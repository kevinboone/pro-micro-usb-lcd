#!/bin/bash

# A simple script that uses the usb-lcd firmware to display the
#  time and date. Note that the serial device might be /dev/ttyUSBxx
#  on some systems. Note that updating once a minute means, of course,
#  that the time could be wrong by up to one minute ;)

DEVICE=/dev/ttyACM0

while true ; do
  DATE=`date "+%b %d %Y"`
  TIME=`date "+%I:%M %p"`
  printf "\f$DATE\r\n$TIME" > $DEVICE 
  sleep 60
done

