"""Run with python -m demo_console --help."""
import argparse
import csv
import io
import json
import sys
import threading
import time
import urllib.error
import urllib.request
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from .engine import Engine


def serve(host, port, seed):
    engine = Engine(seed)
    lock = threading.RLock()
    stop = threading.Event()

    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *_):
            pass

        def reply(self, value, status=200, mime="application/json"):
            body = json.dumps(value, allow_nan=False).encode() if mime == "application/json" else value
            self.send_response(status)
            self.send_header("Content-Type", mime)
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Cache-Control", "no-store")
            self.send_header("X-Content-Type-Options", "nosniff")
            self.send_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self'; connect-src 'self'; frame-ancestors 'none'")
            self.end_headers()
            self.wfile.write(body)

        def do_GET(self):
            with lock:
                if self.path in ("/api/state", "/api/export"):
                    self.reply(engine.snapshot())
                elif self.path == "/api/history.csv":
                    output = io.StringIO()
                    writer = csv.DictWriter(output, fieldnames=["time", "throughput", "primary_rx", "backup_rx"])
                    writer.writeheader()
                    writer.writerows(engine.history)
                    self.reply(output.getvalue().encode(), mime="text/csv")
                elif self.path in ("/", "/app.js", "/style.css"):
                    filename = "index.html" if self.path == "/" else self.path[1:]
                    mime = {"index.html": "text/html; charset=utf-8", "app.js": "text/javascript", "style.css": "text/css"}[filename]
                    self.reply((Path(__file__).parent / "static" / filename).read_bytes(), mime=mime)
                else:
                    self.reply({"error": "Not found"}, 404)

        def do_POST(self):
            if self.path != "/api/command":
                self.reply({"error": "Not found"}, 404)
                return
            origin = self.headers.get("Origin")
            if origin and origin != "http://" + self.headers.get("Host", ""):
                self.reply({"error": "Cross-origin mutation rejected"}, 403)
                return
            try:
                size = int(self.headers.get("Content-Length", "0"))
                if not 0 < size <= 16384:
                    raise ValueError("Expected a JSON body under 16 KiB")
                if self.headers.get_content_type() != "application/json":
                    raise ValueError("Content-Type must be application/json")
                data = json.loads(self.rfile.read(size))
                if not isinstance(data, dict):
                    raise ValueError("JSON object required")
                with lock:
                    engine.command(data)
                    self.reply({"ok": True})
            except (ValueError, TypeError) as error:
                self.reply({"error": str(error)}, 400)

    def tick():
        while not stop.wait(.1):
            with lock:
                engine.step(.1)

    server = ThreadingHTTPServer((host, port), Handler)
    thread = threading.Thread(target=tick, daemon=True)
    thread.start()
    print(f"PHOTON simulation: http://{host}:{server.server_port} — Ctrl+C to stop", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        stop.set()
        server.server_close()
        thread.join(2)


def request(base, command=None):
    req = urllib.request.Request(base.rstrip("/") + ("/api/command" if command is not None else "/api/state"),
                                 data=json.dumps(command).encode() if command is not None else None,
                                 headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=5) as response:
            return json.load(response)
    except urllib.error.HTTPError as error:
        raise ValueError(json.load(error).get("error", str(error))) from error
    except urllib.error.URLError as error:
        raise ValueError(f"Console unavailable at {base}. Start 'python -m demo_console serve'. {error.reason}") from error


def display(s):
    t = s["traffic"]
    print(f"SIMULATED | t={s['time']:.1f}s | {s['phase']} epoch={s['epoch']} | "
          f"A={s['boards']['A']['uplink']} B={s['boards']['B']['uplink']} | "
          f"{t['throughput_mbps']:.1f} Mbps loss={t['loss_pct']:.2f}% outage={t['outage_s']:.2f}s")
    for i, link in enumerate(s["links"]):
        a, b = s["sfps"][f"A{i}"], s["sfps"][f"B{i}"]
        print(f"  SFP{i}: {link['health']:<10} Rx A={a['rx_dbm']:.2f} B={b['rx_dbm']:.2f} dBm | {link['reason']}")


def main():
    # Windows redirected consoles often default to cp1252; protocol arrows are UTF-8.
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    parser = argparse.ArgumentParser(description="PHOTON shared CLI + GUI demo. All telemetry is synthetic.")
    parser.add_argument("--url", default="http://127.0.0.1:8765")
    sub = parser.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("serve"); p.add_argument("--host", default="127.0.0.1"); p.add_argument("--port", type=int, default=8765); p.add_argument("--seed", type=int, default=7)
    p = sub.add_parser("status"); p.add_argument("--json", action="store_true")
    sub.add_parser("watch")
    p = sub.add_parser("scenario"); p.add_argument("name", choices=["healthy", "fade", "cut", "flap", "backup_fault", "partition", "write_fault"])
    p = sub.add_parser("switch"); p.add_argument("target", type=int, choices=[0, 1])
    p = sub.add_parser("inject"); p.add_argument("field"); p.add_argument("value", help="JSON value: true, false, or attenuation number"); p.add_argument("--board", choices=["A", "B"], default="B"); p.add_argument("--link", type=int, choices=[0, 1], default=0)
    p = sub.add_parser("policy"); p.add_argument("values", help='JSON object, e.g. {"revertive":true}')
    p = sub.add_parser("traffic"); p.add_argument("--mbps", type=float, default=900); p.add_argument("--stop", action="store_true")
    p = sub.add_parser("events"); p.add_argument("--count", type=int, default=20)
    p = sub.add_parser("export"); p.add_argument("path")
    args = parser.parse_args()
    try:
        if args.cmd == "serve":
            serve(args.host, args.port, args.seed)
        elif args.cmd == "watch":
            while True:
                display(request(args.url)); time.sleep(1)
        elif args.cmd in ("status", "events", "export"):
            state = request(args.url)
            if args.cmd == "export":
                Path(args.path).write_text(json.dumps(state, indent=2), encoding="utf-8")
                print(f"Saved simulated diagnostic snapshot to {args.path}")
            elif args.cmd == "events":
                for e in state["events"][-args.count:]:
                    print(f"{e['time']:8.2f} {e['severity']:5} {e['source']:12} {e['message']}")
            elif args.json:
                print(json.dumps(state, indent=2))
            else:
                display(state)
        else:
            data = {"action": args.cmd}
            if args.cmd == "scenario": data["name"] = args.name
            elif args.cmd == "switch": data["target"] = args.target
            elif args.cmd == "inject": data.update(field=args.field, value=json.loads(args.value), board=args.board, link=args.link)
            elif args.cmd == "policy": data["values"] = json.loads(args.values)
            elif args.cmd == "traffic": data.update(mbps=args.mbps, running=not args.stop)
            print(json.dumps(request(args.url, data)))
    except (ValueError, OSError) as error:
        parser.exit(1, f"Error: {error}\n")
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
