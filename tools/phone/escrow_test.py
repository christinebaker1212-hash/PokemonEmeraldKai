#!/usr/bin/env python3
"""The co-op server's mail-trade escrow (mp_net.h ESC_*; docs/PHONE_LINK.md "Trades by mail"), straight over a socket:
neither deposit can be taken before both are in; either side may call it off until then and takes back its own;
locks keep a side to whoever first used it; every op can be repeated; deposits survive a server restart.

    python3 tools/phone/escrow_test.py SERVER_BINARY      (exit 0 = passed)
"""
import os
import socket
import struct
import subprocess
import sys
import tempfile
import time

C_MAIL, S_MAIL = 12, 32
PUT, TAKE, CANCEL, LOOK, DENIED = 7, 8, 9, 10, 6
H0, H1, T0, T1, CLOSED = 1, 2, 4, 8, 16


def fail(msg):
    print('ESCROW TEST FAILED: ' + msg)
    sys.exit(1)


class Conn:
    def __init__(self, port):
        self.s = socket.create_connection(('127.0.0.1', port), timeout=5)

    def ask(self, payload):
        body = bytes([C_MAIL]) + payload
        self.s.sendall(struct.pack('<H', len(body)) + body)
        while True:
            hdr = self._read(3)
            n, t = struct.unpack('<H', hdr[:2])[0], hdr[2]
            data = self._read(n - 1)
            if t == S_MAIL:
                op, key, state = data[0], data[1:17], data[17]
                ln = struct.unpack('<H', data[18:20])[0] if len(data) >= 20 else 0
                return op, state, data[20:20 + ln]

    def _read(self, n):
        b = b''
        while len(b) < n:
            c = self.s.recv(n - len(b))
            if not c:
                fail('the server hung up')
            b += c
        return b


def op(c, o, key, side=None, secret=None, data=None):
    p = bytes([o]) + key
    if side is not None:
        p += bytes([side]) + secret
    if data is not None:
        p += struct.pack('<H', len(data)) + data
    return c.ask(p)


def start(binary, port, work):
    p = subprocess.Popen([binary, '--port', str(port), '--lan'], cwd=work, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    for _ in range(50):
        try:
            socket.create_connection(('127.0.0.1', port), timeout=1).close()
            return p
        except OSError:
            time.sleep(0.1)
    fail('no server')


def main():
    binary = os.path.abspath(sys.argv[1])
    work = tempfile.mkdtemp(prefix='escrow')
    port = 48900 + os.getpid() % 90
    srv = start(binary, port, work)
    try:
        a, b = Conn(port), Conn(port)
        sa, sb, x = b'A' * 16, b'B' * 16, b'X' * 16
        k1 = os.urandom(16)
        # a trade: A deposits, B can't take yet, B deposits, both take the other's, again gives the same
        if op(a, PUT, k1, 0, sa, b'monA')[1] != H0:
            fail('first deposit')
        if op(a, PUT, k1, 0, sa, b'monA')[1] != H0:
            fail('a repeated deposit')
        r = op(b, TAKE, k1, 1, sb)
        if r[2] or r[1] != H0:
            fail('taken before both were in: %s' % (r,))
        if op(b, PUT, k1, 0, sb, b'evil')[0] != DENIED:
            fail('B used A\'s side')
        if op(b, PUT, k1, 1, sb, b'monB')[1] != H0 | H1:
            fail('second deposit')
        if op(a, CANCEL, k1, 0, sa)[1] & CLOSED:
            fail('called off with both in')
        if op(Conn(port), TAKE, k1, 1, x)[0] != DENIED:
            fail('a stranger took B\'s')
        r = op(a, TAKE, k1, 0, sa)
        if r[2] != b'monB':
            fail('A got %s' % (r,))
        r = op(b, TAKE, k1, 1, sb)
        if r[2] != b'monA' or r[1] != H0 | H1 | T0 | T1:
            fail('B got %s' % (r,))
        if op(b, TAKE, k1, 1, sb)[2] != b'monA':
            fail('a repeated take')
        # called off: B accepted (deposited), A never came; B cancels and takes back its own; A's late deposit is refused
        k2 = os.urandom(16)
        op(b, PUT, k2, 1, sb, b'monB2')
        r = op(b, CANCEL, k2, 1, sb)
        if not r[1] & CLOSED or r[2] != b'monB2':
            fail('call off: %s' % (r,))
        r = op(a, PUT, k2, 0, sa, b'monA2')
        if r[1] & H0 or not r[1] & CLOSED:
            fail('a deposit after it was called off: %s' % (r,))
        if op(a, TAKE, k2, 0, sa)[2]:
            fail('took from a called-off trade')
        # turned down before anything: a tombstone; a later deposit is refused
        k3 = os.urandom(16)
        if not op(b, CANCEL, k3, 1, sb)[1] & CLOSED:
            fail('a turned-down offer')
        if op(b, PUT, k3, 1, sb, b'late')[1] != CLOSED:
            fail('a deposit on a turned-down offer')
        # a server restart keeps deposits
        k4 = os.urandom(16)
        op(a, PUT, k4, 0, sa, b'kept')
        srv.terminate()
        srv.wait()
        srv = start(binary, port, work)
        b = Conn(port)
        op(b, PUT, k4, 1, sb, b'other')
        if op(b, TAKE, k4, 1, sb)[2] != b'kept':
            fail('a deposit lost in a restart')
        if op(b, LOOK, k1)[1] != H0 | H1 | T0 | T1:
            fail('LOOK')
        print('ESCROW TEST PASSED: deposits, takes, calling off, locks, repeats, a restart')
    finally:
        srv.terminate()


if __name__ == '__main__':
    main()
