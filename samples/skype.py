#!/usr/bin/env python
import Skype4Py  # http://skype4py.sourceforge.net/doc/html/
import os
import sys
import subprocess
import time

blynux_bin = 'blynux' + os.path.splitext(sys.executable)[1]


def setColor(c):
    cmd = [blynux_bin, '--device', '0', '--color', c]
    subprocess.call(cmd)


def blink(c):
    setColor(c)
    time.sleep(0.2)
    setColor('off')
    time.sleep(0.2)


class MySkypeEvents(object):
    def AttachmentStatus(self, status):
        if status == Skype4Py.apiAttachAvailable:
            self.Attach()
        if status == Skype4Py.apiAttachSuccess:
            print("Connected to skype client")

    def UserStatus(self, status):
        print(('status: ' + status))

    def Notify(self, notification):
        print(('notification: ' + notification))

    def MessageStatus(self, message, status):
        print('status', status)
        if status == 'RECEIVED':
            blink('yellow')
        elif status == 'READ':
            blink('green')
        elif status == 'SENDING':
            blink('cyan')
        elif status == 'SENT':
            blink('blue')
        else:
            blink('red')

if __name__ == "__main__":
    skype = Skype4Py.Skype(Events=MySkypeEvents())
    skype.Attach()
    print('Your full name:', skype.CurrentUser.FullName)

    cmd = ''
    while not cmd == 'exit':
        cmd = raw_input('')
