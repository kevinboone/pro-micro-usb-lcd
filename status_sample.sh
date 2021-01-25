#!/bin/bash

# A simple script that uses the usb-lcd firmware to display the
#  time, load average, and largest CPU user, at intervals of 5 sec

DEVICE=/dev/ttyACM0

while true ; do
  TIME=`date "+%I:%M%p"`
  LA=`cut -f 1 -d ' ' < /proc/loadavg`
  TOP=`top -b -n 1 -w 100 | head -8 | tail -1 | cut -b 70-86`
  printf "\f$TIME $LA\r\n$TOP" > $DEVICE 
  sleep 5 
done

