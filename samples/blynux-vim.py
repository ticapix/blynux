#!/usr/bin/env python
# vim: tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 smarttab expandtab
import psutil
import os
import sys
import subprocess
import time

VIM_EXTENSIONS = ( 'c', 'cpp', 'sh', 'cs', 'py', 'pl', 'rb', 'js' )

def setColor(c):
    blynux_bin = 'blynux'
    cmd = [blynux_bin, '--device', '0', '--color', c]
    subprocess.call(cmd)

def is_vim_coding(proc):
    if proc.name != 'vim':
        return False
    fname = proc.cmdline[1]
    i = fname.rfind('.')
    if i < 0:
        try:
            ffname = proc.getcwd() + os.sep + fname
            f = open(ffname, 'r')
            if f.read(2) == '#!':
                return True
        except:
            pass
    elif fname[i+1:] in VIM_EXTENSIONS:
        return True
    return False

timeout = 15
timer = 0
busy = False
setColor('green')
try:
    while True:
        vim_found = False
        for proc in psutil.process_iter():
            if is_vim_coding(proc):
                vim_found = True
                break

        if vim_found:
            if not busy:
                setColor('red')
                busy = True
                timer = timeout

        elif busy and timer > 0:
            timer = timer - 1
            if timer == 0:
                busy = False
                setColor('green')

        time.sleep(1)
except KeyboardInterrupt:
    pass

