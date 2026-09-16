"""
Tiny HTTP server that receives image POSTs from the browser and saves them.
Run this from a command prompt, then trigger the browser JS to POST images to it.
"""
import json
import base64
from pathlib import Path
from http.server import BaseHTTPRequestHandler, HTTPServer

SAVE_DIR = Path(__file__).parent

class Handler(BaseHTTPRequestHandler):
    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        # Required for fetch() from an https page (e.g. chatgpt.com) to localhost
        self.send_header('Access-Control-Allow-Private-Network', 'true')
        self.end_headers()

    def do_POST(self):
        length = int(self.headers.get('Content-Length', 0))
        body = json.loads(self.rfile.read(length))
        name = body['name']
        data = body['data']  # data:image/png;base64,...
        b64 = data.split(',', 1)[1]
        out = SAVE_DIR / name
        out.write_bytes(base64.b64decode(b64))
        print(f'  Saved: {out}')
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps({'ok': True, 'path': str(out)}).encode())

    def log_message(self, fmt, *args):
        pass  # suppress default logging

print(f'Saving to: {SAVE_DIR}')
print('Listening on http://localhost:8765 — waiting for browser to POST images...')
HTTPServer(('localhost', 8765), Handler).serve_forever()
