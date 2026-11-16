"""HTTP/CLI service smoke check: python -m demo_console.smoke_test [base_url]."""
import json
import sys
import urllib.error
import urllib.request


def main():
    base = sys.argv[1] if len(sys.argv) > 1 else 'http://127.0.0.1:8765'
    for path in ('/', '/app.js', '/style.css', '/api/state', '/api/history.csv', '/api/export'):
        with urllib.request.urlopen(base + path, timeout=5) as response:
            assert response.status == 200 and response.read(), path
            print('PASS', path)
    for body, origin, status in (({'action': 'policy', 'values': {'trip_dbm': 0}}, None, 400),
                                 ({'action': 'traffic'}, 'http://untrusted.example', 403)):
        headers = {'Content-Type': 'application/json'}
        if origin:
            headers['Origin'] = origin
        req = urllib.request.Request(base + '/api/command', data=json.dumps(body).encode(), headers=headers)
        try:
            urllib.request.urlopen(req, timeout=5)
            raise AssertionError('Invalid request accepted')
        except urllib.error.HTTPError as error:
            assert error.code == status, error.code
            print('PASS rejected request', status)


if __name__ == '__main__': main()
