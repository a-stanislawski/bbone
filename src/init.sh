#!/bin/bash


SCRIPT=$(readlink -f "$0")
# Absolute path this script is in
SPATH=$(dirname "$SCRIPT")


DICT="$SPATH/dict/9416.dic"
LM="$SPATH/dict/9416.lm"

$SPATH/testbbb > $SPATH/logs/motor &
sleep 2
MOTOR=$!
sleep 5
sudo /root/pocketsphinx-5prealpha/src/programs/pocketsphinx_continuous -lm $LM -dict $DICT -inmic yes >> $SPATH/com &

sleep 6

$SPATH/audio > $SPATH/logs/audio &
sleep 2


#ok
$SPATH/audio2 > $SPATH/logs/audio2 &

#$SPATH/audio > $SPATH/logs/audio &

#~/BTled/
#~/BTled/audio3 > ~/logaudio3 &


date >> logs/script
echo -e " completed\n" >> logs/script

while(ps -e | grep $MOTOR) do
	sleep 2
done

echo "Connection lost" | festival --tts

sleep 3

