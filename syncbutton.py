#!/usr/bin/env python
# coding=utf-8
import RPi.GPIO as GPIO
import time
import os,sys
import signal

GPIO.setmode(GPIO.BCM)
pin_btn = 18

GPIO.setup(pin_btn, GPIO.IN, pull_up_down = GPIO.PUD_UP)

def onPress(channel):
    print('pressed')
    rc = os.system("killall baidu_nd_sync >/dev/null 2>&1")
    if rc != 0:
        print('start sync')
        os.system("/home/pi/oss/baidu_nd_sync &")
    else:
        print('stop sync')

GPIO.add_event_detect(pin_btn, GPIO.FALLING, callback = onPress, bouncetime = 500)

try:
    while True:
        time.sleep(10)
except KeyboardInterrupt:
    print('User press Ctrl+c, exit;')
finally:
    GPIO.cleanup()
