#!/usr/bin/env python3
"""Serves a folder over HTTP, optionally slowly (for the updater test). usage: slow_http.py PORT DIR [DELAY]
DELAY = seconds to wait after every 16 KB."""
import functools, http.server, sys, time

class Handler(http.server.SimpleHTTPRequestHandler):
    delay = 0.0

    def copyfile(self, src, dst):
        while True:
            chunk = src.read(16384)
            if not chunk:
                break
            dst.write(chunk)
            if self.delay:
                time.sleep(self.delay)

    def log_message(self, fmt, *args):
        sys.stderr.write("[http] " + (fmt % args) + "\n")

if __name__ == "__main__":
    port, folder = int(sys.argv[1]), sys.argv[2]
    Handler.delay = float(sys.argv[3]) if len(sys.argv) > 3 else 0.0
    srv = http.server.ThreadingHTTPServer(("127.0.0.1", port), functools.partial(Handler, directory=folder))
    srv.serve_forever()
