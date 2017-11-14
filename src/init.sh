#!/bin/bash


SCRIPT=$(readlink -f "$0")
# Absolute path this script is in, thus /home/user/bin
SPATH=$(dirname "$SCRIPT")


DICT="$SPATH/dict/2930.dic"
LM="$SPATH/dict/2930.lm"

sudo ~/pocketsphinx-5prealpha/src/programs/pocketsphinx_continuous -lm $LM -dict $DICT -inmic yes >> $SPATH/com2 &

$SPATH/testbbb > $SPATH/logs/motor &


# ~/BTled/audio > ~/logaudio &



#ok
$SPATH/audio2 > $SPATH/logs/audio2 &

#$SPATH/audio > $SPATH/logs/audio &

#~/BTled/
#~/BTled/audio3 > ~/logaudio3 &


date >> logs/script
echo -e " completed\n" >> logs/script


