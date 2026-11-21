#!/usr/bin/env python3
"""Moemon VMU manager: a pretend VM2 on a pseudo-terminal, answering docs/VM2_USB_PROTOCOL.md (tests only).

usage: fake_vm2.py SD_FOLDER INTERNAL.bin PORT_FILE
  SD_FOLDER: a folder standing in for the SD card; INTERNAL.bin: the internal memory (128 KB, made if missing).
  Writes the pty's name to PORT_FILE, then serves until killed.
"""
import os
import sys
import tty

sd, internal, port_file = sys.argv[1], sys.argv[2], sys.argv[3]
if not os.path.exists(internal):
    open(internal, 'wb').write(bytes(131072))
master, slave = os.openpty()
tty.setraw(slave)
open(port_file, 'w').write(os.ttyname(slave))


def rd(n):
    out = b''
    while len(out) < n:
        out += os.read(master, n - len(out))
    return out


def line():
    out = b''
    while not out.endswith(b'\r'):
        out += rd(1)
    return out[:-1].decode('ascii')


def send(b):
    os.write(master, b)


def say(s):
    send(s.encode('ascii') + b'\r\n')


def local(path):
    p = path.strip('\\').replace('\\', '/')
    return os.path.join(sd, p) if p else sd


while True:
    cmd = rd(6).decode('ascii', 'replace')
    if cmd == '@RMEM1':
        blk = rd(1)[0]
        send(open(internal, 'rb').read()[blk * 512:(blk + 1) * 512])
        continue
    if cmd == '@WMEM1':
        blk = rd(1)[0]
        data = rd(512)
        x = rd(1)[0]
        chk = 0
        for b in data:
            chk ^= b
        if chk != x:
            say('NACK')
            continue
        img = bytearray(open(internal, 'rb').read())
        img[blk * 512:(blk + 1) * 512] = data
        open(internal, 'wb').write(img)
        say('ACK')
        continue
    cmd += rd(1).decode('ascii', 'replace')
    if cmd == '@HAND00':
        say('ACK')
    elif cmd == '@INFO00':
        say('VM2 HW1.2 1.5.5 0123456789AB')
    elif cmd == '@WRTC00':
        rd(10)
        say('ACK')
    elif cmd == '@SDEXPL':
        say('ACK')
        folder = line()
        say('>START_DIR')
        d = local('' if folder == '\\' else folder)
        for name in sorted(os.listdir(d)):
            p = os.path.join(d, name)
            if os.path.isdir(p):
                t = os.path.join(p, 'title.txt')
                title = open(t, 'rb').read().split(b'\0')[0].decode() if os.path.exists(t) else ''
                say('%s|0|2026-11-21|1|%s|No' % (name, title))
            elif name.lower() != 'title.txt':
                say('%s|%d|2026-11-21|0||%s' % (name, os.path.getsize(p), 'Yes' if os.path.getsize(p) == 98370 else 'No'))
        say('>END_DIR')
    elif cmd == '@SDFLRD':
        say('ACK')
        p = local(line())
        data = open(p, 'rb').read().ljust(131072, b'\0')
        for blk in range(256):
            send(data[blk * 512:(blk + 1) * 512])
            if rd(1) != b'O':
                break
        say('ACK')
    elif cmd == '@SDFLWR':
        say('ACK')
        p = local(line())
        say('>START')
        data = b''
        for blk in range(256):
            data += rd(512)
            say('>NXTBLK')
        open(p, 'wb').write(data)
        say('ACK')
    elif cmd in ('@SDFCPY', '@SDRENM'):
        say('ACK')
        a, b = local(line()), local(line())
        try:
            if cmd == '@SDFCPY':
                open(b, 'wb').write(open(a, 'rb').read())
            else:
                os.rename(a, b)
            say('ACK')
        except OSError:
            say('NACK')
    elif cmd in ('@SDFDEL', '@SDDCRT', '@SDDDEL'):
        say('ACK')
        p = local(line())
        try:
            {'@SDFDEL': os.remove, '@SDDCRT': os.mkdir, '@SDDDEL': os.rmdir}[cmd](p)
            say('ACK')
        except OSError:
            say('NACK')
    elif cmd == '@SDFTTL':
        say('ACK')
        p = local(line())
        say('>START')
        open(p, 'wb').write(rd(128))
        say('ACK')
    elif cmd == '@REST00':
        pass
    else:
        say('NACK')
