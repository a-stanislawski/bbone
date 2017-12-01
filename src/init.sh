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

# start voice recognition
#sudo /root/pocketsphinx-5prealpha/src/programs/pocketsphinx_continuous -lm $LM -dict $DICT -inmic yes >> $SPATH/com &

# reading moves
$SPATH/audio > $SPATH/logs/audio &
sleep 2

#$SPATH/audio2 > $SPATH/logs/audio2 &


#export data to read voltage
echo cape-bone-iio > /sys/devices/bone_capemgr.9/slots

sleep 2


# check if main program is running
#while ! ifconfig wlan0 > /dev/null 2> /dev/null ; do
	#/sbin/ifdown wlan0
#	sleep 2
#	echo 111
#done

#echo 222
#/sbin/ifup wlan0
sleep 3
ntpdate -b -s -u pool.ntp.org

#echo "Connection lost" | festival --tts

sleep 3
