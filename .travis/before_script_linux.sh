#!/bin/bash
set -ev

export DISPLAY=:99.0
sh -e /etc/init.d/xvfb start
sleep 3
sudo modprobe snd-dummy
sudo usermod -a -G audio $USER
