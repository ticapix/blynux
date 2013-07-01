#!/usr/bin/env python

import os
import sys
import subprocess
import time

blynux_bin = 'blynux' + os.path.splitext(sys.executable)[1]
blynux_path = os.path.join(os.path.dirname(__file__), '..', 'build', blynux_bin)

def setColor(c):
    cmd = [blynux_path, '--device', '0', '--color', c]
    subprocess.call(cmd)

def flip(c1, c2):
    """last 8 secs in total"""
    for i in range(0, 10):  # 10*0.2 = 4secs
        setColor(c1)
        time.sleep(0.2)
        setColor(c2)
        time.sleep(0.2)
    for i in range(0, 20): # 20*0.1 = 4secs
        setColor(c1)
        time.sleep(0.1)
        setColor(c2)
        time.sleep(0.1)
    setColor(c2)

if len(sys.argv) <= 1:
    print "usage %s <secs as float>" % sys.argv[0]
    sys.exit(1)
timeout = float(eval(sys.argv[1]))
timeout = max(timeout, 3*8)
print "timer for %f secs" % timeout

#for i in range(0, 3):
#    for c in ['white', 'cyan', 'magenta', 'blue', 'yellow', 'green', 'red']:
#        setColor(c)
#        time.sleep(0.2)

setColor('green')
time.sleep((timeout - 3*8) * 3./ 5.)
flip('green', 'yellow')
time.sleep((timeout - 3*8) * 1./ 5.)
flip('yellow', 'red')
time.sleep((timeout - 3*8) * 1./ 5.)
flip('red', 'off')
