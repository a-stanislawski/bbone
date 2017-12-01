#!/bin/bash

SCRIPT=$(readlink -f "$0")
# Absolute path this script
SPATH=$(dirname "$SCRIPT")

# paths for dictionary
#DICT="$SPATH/dict/9416.dic"
#LM="$SPATH/dict/9416.lm"

#start motor control
$SPATH/testbbb > $SPATH/logs/motor &
sleep 2

# remember process PID to control its state
MOTOR=$!
sleep 5

# reading moves
$SPATH/audio > $SPATH/logs/audio &
sleep 2

#export data to read voltage
echo cape-bone-iio > /sys/devices/bone_capemgr.9/slots
sleep 2

#update date and time
sleep 3
ntpdate -b -s -u pool.ntp.org


#echo "Connection lost" | festival --tts

sleep 3
