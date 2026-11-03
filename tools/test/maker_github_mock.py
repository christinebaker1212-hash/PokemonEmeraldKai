#!/usr/bin/env python3
"""A stand-in for the part of GitHub's REST API Moemon Maker's Publish uses (src/maker/maker_publish.c): blobs, trees,
commits, the branch's ref, contents listings / raw files and the commits that touched a path. Files are kept per commit
as {path: bytes}. tools/test/maker_publish.sh runs it; it prints what it was asked and checks the token.

usage: maker_github_mock.py PORT TOKEN
"""
import base64
import hashlib
import json
import sys
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlparse, parse_qs

PORT, TOKEN = int(sys.argv[1]), sys.argv[2]
REPO = 'owner/game'
blobs = {}      # sha -> bytes
trees = {}      # sha -> {path: blob sha}  (whole tree)
subtrees = {}   # sha -> {relative path: blob sha} (MoemonMaker/)
commits = {}    # sha -> {tree, parents, message, date}
lock = threading.Lock()
n = [0]


def gitsha(kind, data):
    return hashlib.sha1(b'%s %d\0' % (kind.encode(), len(data)) + data).hexdigest()


def tree_id(files):
    t = hashlib.sha1(json.dumps(sorted(files.items())).encode()).hexdigest()
    trees[t] = dict(files)
    sub = {p[len('MoemonMaker/'):]: s for p, s in files.items() if p.startswith('MoemonMaker/')}
    st = hashlib.sha1(json.dumps(sorted(sub.items())).encode()).hexdigest()
    subtrees[st] = sub
    return t, st


def commit(tree, parents, message):
    n[0] += 1
    c = hashlib.sha1(('%s %s %s %d' % (tree, parents, message, n[0])).encode()).hexdigest()
    commits[c] = {'tree': tree, 'parents': parents, 'message': message, 'date': '2026-11-03T12:%02d:00Z' % n[0]}
    return c


readme = b'the game\n'
blobs[gitsha('blob', readme)] = readme
t0, _ = tree_id({'README.md': gitsha('blob', readme)})
head = [commit(t0, [], 'start')]


class H(BaseHTTPRequestHandler):
    def log_message(self, *a):
        pass

    def reply(self, code, obj=None, raw=None):
        body = raw if raw is not None else json.dumps(obj if obj is not None else {}).encode()
        self.send_response(code)
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def body(self):
        n = int(self.headers.get('Content-Length') or 0)
        return json.loads(self.rfile.read(n) or b'{}')

    def handle_any(self, method):
        u = urlparse(self.path)
        q = parse_qs(u.query)
        p = u.path
        print('mock: %s %s' % (method, self.path), flush=True)
        if self.headers.get('Authorization') != 'Bearer ' + TOKEN:
            return self.reply(401, {'message': 'Bad credentials'})
        base = '/repos/' + REPO
        with lock:
            if method == 'GET' and p == base:
                return self.reply(200, {'default_branch': 'main'})
            if method == 'GET' and p == base + '/git/ref/heads/main':
                return self.reply(200, {'object': {'sha': head[0], 'type': 'commit'}})
            if method == 'GET' and p.startswith(base + '/git/commits/'):
                c = commits.get(p.rsplit('/', 1)[1])
                return self.reply(200, {'tree': {'sha': c['tree']}}) if c else self.reply(404)
            if method == 'GET' and p.startswith(base + '/git/trees/'):
                t = trees.get(p.rsplit('/', 1)[1])
                if t is None:
                    return self.reply(404)
                _, st = tree_id(t)
                entries = [{'path': 'README.md', 'type': 'blob', 'sha': t.get('README.md', '')}]
                if any(k.startswith('MoemonMaker/') for k in t):
                    entries.append({'path': 'MoemonMaker', 'mode': '040000', 'type': 'tree', 'sha': st})
                return self.reply(200, {'tree': entries})
            if method == 'GET' and p.startswith(base + '/contents/'):
                path = p[len(base + '/contents/'):]
                ref = q.get('ref', ['main'])[0]
                c = commits.get(head[0] if ref == 'main' else ref)
                files = trees[c['tree']] if c else {}
                if path in files:
                    return self.reply(200, raw=blobs[files[path]])
                items = [{'name': k[len(path) + 1:], 'sha': s, 'type': 'file'} for k, s in sorted(files.items())
                         if k.startswith(path + '/') and '/' not in k[len(path) + 1:]]
                return self.reply(200, items) if items else self.reply(404, {'message': 'Not Found'})
            if method == 'POST' and p == base + '/git/blobs':
                b = self.body()
                data = base64.b64decode(b['content'])
                s = gitsha('blob', data)
                blobs[s] = data
                return self.reply(201, {'sha': s})
            if method == 'POST' and p == base + '/git/trees':
                b = self.body()
                files = dict(trees[b['base_tree']])
                for e in b['tree']:
                    if e['type'] == 'tree' and e['path'] == 'MoemonMaker':
                        files = {k: v for k, v in files.items() if not k.startswith('MoemonMaker/')}
                        files.update({'MoemonMaker/' + k: v for k, v in subtrees[e['sha']].items()})
                    elif e['sha'] is None:
                        files.pop(e['path'], None)
                    else:
                        files[e['path']] = e['sha']
                t, _ = tree_id(files)
                return self.reply(201, {'sha': t})
            if method == 'POST' and p == base + '/git/commits':
                b = self.body()
                return self.reply(201, {'sha': commit(b['tree'], b['parents'], b['message'])})
            if method == 'PATCH' and p == base + '/git/refs/heads/main':
                b = self.body()
                if commits[b['sha']]['parents'] != [head[0]]:
                    return self.reply(422, {'message': 'not a fast forward'})
                head[0] = b['sha']
                print('mock: main is now %s: %s' % (b['sha'][:7], commits[b['sha']]['message'].splitlines()[0]), flush=True)
                return self.reply(200, {'object': {'sha': b['sha']}})
            if method == 'GET' and p == base + '/commits':
                out, c = [], head[0]
                while c:
                    cc = commits[c]
                    par = cc['parents'][0] if cc['parents'] else None
                    mine = {k: v for k, v in trees[cc['tree']].items() if k.startswith('MoemonMaker/')}
                    prev = {k: v for k, v in trees[commits[par]['tree']].items() if k.startswith('MoemonMaker/')} if par else {}
                    if mine != prev:
                        out.append({'sha': c, 'commit': {'author': {'name': 'u', 'date': cc['date']}, 'message': cc['message']},
                                    'parents': [{'sha': par}]})
                    c = par
                return self.reply(200, out)
        return self.reply(404, {'message': 'Not Found'})

    def do_GET(self):
        self.handle_any('GET')

    def do_POST(self):
        self.handle_any('POST')

    def do_PATCH(self):
        self.handle_any('PATCH')


ThreadingHTTPServer(('127.0.0.1', PORT), H).serve_forever()
