"""
Description: This script is a simple python web server which accepts a PUT request and writes the supplied data to a file.
There is a known issue with the signal module when trying to run this on Windows 7.  It has worked in various *nix environments.

This script is a combination of the following links:
https://gist.githubusercontent.com/codification/1393204/raw/3fd4a48d072ec8f7f453d146814cb6e7fc6a129f/server.py
http://www.acmesystems.it/python_httpserver
"""
#!/usr/bin/python
from __future__ import print_function

import sys
import os
import time
import json
import mimetypes
import socket
import platform
from os import curdir
from threading import Thread

# Python 2/3 compatible HTTPServer imports
if sys.version_info[0] < 3:
    from BaseHTTPServer import HTTPServer, BaseHTTPRequestHandler
else:
    from http.server import BaseHTTPRequestHandler, HTTPServer


class SimpleHandler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    # Match previous log format for request status lines
    def log_message(self, fmt, *args):
        try:
            client = "%s:%s" % (self.client_address[0], self.client_address[1])
        except Exception:
            client = "?"
        ts = time.strftime('%Y-%m-%d %H:%M:%S')
        print("[%s] %s - " % (ts, client) + (fmt % args))

    # Minimal helper to send a response and close the connection robustly
    def _send(self, status, body_bytes=b"", content_type=None):
        self.send_response(status)
        if content_type:
            self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body_bytes)))
        self.send_header("Connection", "close")
        self.end_headers()
        if body_bytes:
            try:
                self.wfile.write(body_bytes)
                try:
                    self.wfile.flush()
                except Exception:
                    pass
            except Exception as e:
                print('write failed:', e)
        try:
            self.connection.shutdown(socket.SHUT_WR)
        except Exception:
            pass
        self.close_connection = True

    def do_PUT(self):
        print('\n----------- REQUEST METHOD: ','PUT ------------')
        print('Client:', self.client_address)
        print('Request-Line:', getattr(self, 'requestline', ''))
        print(self.headers)
        start = time.time()
        # Expect: 100-continue
        if self.headers.get('Expect', '').lower() == '100-continue':
            if hasattr(self, 'send_response_only'):
                self.send_response_only(100)
            else:
                self.send_response(100)
            self.end_headers()
        # Content-Length
        length_hdr = self.headers.get('Content-Length')
        print('PUT: Content-Length header =', length_hdr)
        if not length_hdr:
            self.send_error(411, "Length Required")
            return
        try:
            length = int(length_hdr)
        except ValueError:
            self.send_error(400, "Bad Content-Length")
            return
        # Read body
        body = self.rfile.read(length) if length > 0 else b""
        print('PUT: bytes read =', len(body))
        # Resolve path and save
        rel = os.path.normpath(self.path.lstrip('/\\'))
        print('PUT: safe_rel_path =', rel)
        dst_path = os.path.join(curdir, rel)
        print('PUT: dest_path =', dst_path)
        d = os.path.dirname(dst_path)
        if d and not os.path.exists(d):
            try:
                os.makedirs(d)
            except OSError:
                if not os.path.isdir(d):
                    raise
        with open(dst_path, 'wb') as f:
            f.write(body)
        print('PUT: wrote file bytes =', len(body))
        # Configurable response
        try:
            status = int(os.environ.get('PUT_SUCCESS_STATUS', '200'))
        except Exception:
            status = 200
        mode = os.environ.get('PUT_RESPONSE_BODY', 'json').lower()
        if mode == 'empty':
            resp = b''
            ctype = 'text/plain; charset=utf-8'
        elif mode == 'text':
            resp = b'OK\n'
            ctype = 'text/plain; charset=utf-8'
        else:
            payload = { 'status': 'ok', 'saved_bytes': len(body), 'path': rel }
            resp = json.dumps(payload).encode('utf-8')
            ctype = 'application/json; charset=utf-8'
        # Print in a way consistent with earlier logs
        if len(resp) == 0:
            if status == 204:
                print('PUT: sending no-content (204) response')
            else:
                print('PUT: sending empty response with status =', status)
        else:
            print('PUT: sending response bytes =', len(resp))
        self._send(status, resp, ctype)
        dur = (time.time() - start) * 1000.0
        print('-------------   PUT SUCCESS  -------------- (%.2f ms)\n' % dur)

    def do_POST(self):
        print('\n--------- REQUEST METHOD: ','POST----------')
        print('Client:', self.client_address)
        print('Request-Line:', getattr(self, 'requestline', ''))
        print(self.headers)
        start = time.time()
        try:
            length = int(self.headers.get('Content-Length', '0'))
        except ValueError:
            self.send_error(400, "Bad Content-Length")
            return
        data = self.rfile.read(length) if length > 0 else b""
        print('POST: bytes read =', len(data))
        print('Data Content: ', data, "\n")
        resp = json.dumps({ 'status': 'ok', 'received_bytes': len(data) }).encode('utf-8')
        print('POST: sending response bytes =', len(resp))
        self._send(200, resp, 'application/json; charset=utf-8')
        dur = (time.time() - start) * 1000.0
        print('-------------  POST SUCCESS  -------------- (%.2f ms)\n' % dur)

    def do_GET(self):
        print('\n----------- REQUEST METHOD: ','GET -------------\n')
        print('Client:', self.client_address)
        print('Request-Line:', getattr(self, 'requestline', ''))
        print(self.headers)
        start = time.time()
        raw_path = self.path.split('?', 1)[0].split('#', 1)[0]
        rel = os.path.normpath(raw_path.lstrip('/\\'))
        # Root banner
        if rel in ('', '.', os.path.sep):
            banner = b"WiseConnect HTTP server OK\n"
            print('GET: root path requested; sending OK banner bytes =', len(banner))
            self._send(200, banner, 'text/plain; charset=utf-8')
            dur = (time.time() - start) * 1000.0
            print('--------------   GET COMPLETE  -------------- (%.2f ms)\n' % dur)
            return
        full_path = os.path.join(curdir, rel)
        if os.path.isdir(full_path):
            msg = ('Directory access not allowed: %s' % raw_path).encode('utf-8')
            print('GET: directory requested; sending 404 bytes =', len(msg), 'for', raw_path)
            self._send(404, msg, 'text/plain; charset=utf-8')
            dur = (time.time() - start) * 1000.0
            print('--------------   GET COMPLETE  -------------- (%.2f ms)\n' % dur)
            return
        if os.path.isfile(full_path):
            try:
                with open(full_path, 'rb') as f:
                    data = f.read()
                ctype, _ = mimetypes.guess_type(full_path)
                if not ctype:
                    ctype = 'application/octet-stream'
                print('--------------   GET SUCCESS  --------------\n')
                # Send via helper to keep consistent headers and close
                print('GET: sending bytes =', len(data), 'from', full_path)
                self._send(200, data, ctype)
                dur = (time.time() - start) * 1000.0
                print('--------------   GET COMPLETE  -------------- (%.2f ms)\n' % dur)
            except Exception as e:
                err = ('GET failed: %s' % str(e)).encode('utf-8')
                print('GET: sending error bytes =', len(err))
                self._send(500, err, 'text/plain; charset=utf-8')
        else:
            msg = ('File Not Found: %s' % raw_path).encode('utf-8')
            print('GET: sending 404 bytes =', len(msg), 'for', raw_path)
            self._send(404, msg, 'text/plain; charset=utf-8')

    def do_HEAD(self):
        self._send(200, b"", 'text/html')

    def do_OPTIONS(self):
        # CORS/preflight minimal
        self.send_response(204)
        self.send_header("Allow", "GET, POST, PUT, HEAD, OPTIONS")
        self.send_header("Content-Length", "0")
        self.send_header("Connection", "close")
        self.end_headers()
        try:
            self.connection.shutdown(socket.SHUT_WR)
        except Exception:
            pass
        self.close_connection = True

class VerboseHTTPServer(HTTPServer):
    def server_bind(self):
        HTTPServer.server_bind(self)
        host, port = self.server_address[:2]
        print('Server bind complete on %s:%s' % (host if host else '0.0.0.0', port))

    def get_request(self):
        request, client_address = HTTPServer.get_request(self)
        print('ACCEPT connection from %s:%s' % (client_address[0], client_address[1]))
        return request, client_address

    def close_request(self, request):
        try:
            peer = request.getpeername()
            print('CLOSE connection to %s:%s' % (peer[0], peer[1]))
        except Exception:
            pass
        return HTTPServer.close_request(self, request)


def run_on(port):
    print("\nLaunching HTTP server on port %i ..." % port)
    server_address = ('', port)
    httpd = VerboseHTTPServer(server_address, SimpleHandler)
    print("Server successfully acquired the socket with port:", port)
    print("Listening on: http://0.0.0.0:%d (Python %s, %s)" % (port, sys.version.split()[0], platform.system()))
    print("Press Ctrl+C to shut down the server and exit.")
    print("\nAwaiting New connection\n")
    httpd.serve_forever()


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: put-server.py <port #>")
        sys.exit()
    ports = [int(arg) for arg in sys.argv[1:]]
    for port_number in ports:
        server = Thread(target=run_on, args=(port_number,))
        server.daemon = True
        server.start()
    print('Server threads started for ports:', ports)
    print('Press q to quit...')
    while True:
        try:
            time.sleep(1.0)
        except KeyboardInterrupt:
            break
