#!/bin/sh

if [ $# -eq 0 ] ; then
    echo 'Usage: $0 [ip address]'
    exit 1
fi

rsync -rlptzv --progress --exclude=.git --exclude=combined.log --exclude=node_modules "pi@$1:/home/pi/router/" ./
