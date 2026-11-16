"""Deterministic behavioral model. No hardware I/O or measured throughput.

All thresholds, VLAN IDs and timing defaults are proposed demo policy.
The protocol deliberately permits an outage, never two user uplinks per board.
"""
import math
import random
from collections import deque
from dataclasses import asdict, dataclass


@dataclass
class Policy:
    trip_dbm: float = -16.0
    recover_dbm: float = -13.0
    persistence_s: float = 1.5
    recovery_s: float = 5.0
    hard_fault_s: float = 0.3
    hold_down_s: float = 8.0
    heartbeat_timeout_s: float = 1.5
    transaction_timeout_s: float = 2.0
    auto: bool = True
    revertive: bool = False


class Engine:
    def __init__(self, seed=7):
        self.seed = seed
        self.rng = random.Random(seed)
        self.now = 0.0
        self.policy = Policy()
        self.boards = {b: {"online": True, "uplink": 0, "epoch": 0,
                           "write_fault": False, "last_heartbeat": 0.0,
                           "prepared": None} for b in ("A", "B")}
        self.links = [{"attenuation": {"A": 0., "B": 0.}, "cut": False,
                       "present": {"A": True, "B": True},
                       "tx_fault": {"A": False, "B": False},
                       "tx_disable": {"A": False, "B": False},
                       "power": {"A": True, "B": True},
                       "stale": {"A": False, "B": False},
                       "health": "HEALTHY", "bad_since": None,
                       "good_since": None, "reason": "Nominal"} for _ in range(2)]
        self.telemetry = {}
        self.events = deque(maxlen=600)
        self.history = deque(maxlen=600)
        self.messages = []
        self.partition = False
        self.drop_messages = False
        self.latency_s = .08
        self.transaction = None
        self.epoch = 0
        self.active = 0
        self.last_switch = -100.
        self.last_attempt = -100.
        self.last_heartbeat_sent = -1.
        self.phase = "IDLE"
        self.scenario = "healthy"
        self.scenario_start = 0.
        self.offered_mbps = 900.
        self.traffic_running = True
        self.bytes_delivered = 0.
        self.bytes_lost = 0.
        self.outage_s = 0.
        self.failovers = 0
        self.traffic = {}
        self.event("INFO", "SYSTEM", "Simulation started; A coordinator, B agent. No hardware connected.")
        self.step(.0)

    def event(self, severity, source, text):
        self.events.append({"id": (self.events[-1]["id"] + 1) if self.events else 1,
                            "time": round(self.now, 3), "severity": severity,
                            "source": source, "message": text})

    def management_up(self):
        # Independent, CPU-terminated management VLAN on each physical link.
        return (not self.partition and all(b["online"] for b in self.boards.values())
                and any(self.physical_good(i) for i in range(2)))

    def physical_good(self, link):
        return all(self.telemetry.get(f"{b}{link}", {}).get("link_up", False) for b in ("A", "B"))

    def sample(self):
        for i, link in enumerate(self.links):
            for b, peer in (("A", "B"), ("B", "A")):
                powered = link["power"][b] and link["present"][b] and self.boards[b]["online"]
                remote_tx = (link["power"][peer] and link["present"][peer]
                             and self.boards[peer]["online"] and not link["tx_fault"][peer]
                             and not link["tx_disable"][peer])
                rx = -7.0 - link["attenuation"][b] + self.rng.uniform(-.045, .045)
                if link["cut"] or not powered or not remote_tx:
                    rx = -40.
                los = rx < -23.
                # Illustrative receiver/link curve; not a transceiver qualification model.
                up = powered and remote_tx and not link["cut"] and rx > -19.
                old = self.telemetry.get(f"{b}{i}", {})
                fresh = not link["stale"][b] and powered
                self.telemetry[f"{b}{i}"] = {
                    "board": b, "sfp": i, "rx_dbm": round(rx, 3) if fresh else old.get("rx_dbm", rx),
                    "tx_dbm": -5.5 if powered and not link["tx_disable"][b] and not link["tx_fault"][b] else -40.,
                    "temperature_c": round(39 + i * 2 + (b == "B") + math.sin(self.now / 20), 2),
                    "voltage_v": round(3.3 + .006 * math.sin(self.now), 3),
                    "bias_ma": 6.2 if powered else 0., "present": link["present"][b],
                    "power_enabled": link["power"][b], "tx_disabled": link["tx_disable"][b],
                    "tx_fault": link["tx_fault"][b], "los": los, "link_up": up,
                    "dom_age_s": 0. if fresh else round(old.get("dom_age_s", 0.) + self._dt, 3),
                    "attenuation_db": round(link["attenuation"][b], 2), "synthetic": True}

    def detect(self):
        p = self.policy
        for i, link in enumerate(self.links):
            samples = [self.telemetry[f"{b}{i}"] for b in ("A", "B")]
            hard = any(not s["link_up"] or s["tx_fault"] or s["los"] for s in samples)
            stale = any(s["dom_age_s"] >= 2. for s in samples)
            low = min(s["rx_dbm"] for s in samples) <= p.trip_dbm
            bad = hard or stale or low
            clear = not bad and min(s["rx_dbm"] for s in samples) >= p.recover_dbm
            old = link["health"]
            if bad:
                link["good_since"] = None
                if link["bad_since"] is None:
                    link["bad_since"] = self.now
                link["reason"] = "Link/LOS/TX fault" if hard else "Stale DOM" if stale else "Low receive power"
                if self.now - link["bad_since"] + 1e-9 >= (p.hard_fault_s if hard else p.persistence_s):
                    link["health"] = "FAILED"
                elif old != "FAILED":
                    link["health"] = "SUSPECT"
            else:
                link["bad_since"] = None
                if old == "FAILED" or old == "RECOVERING":
                    if clear:
                        if link["good_since"] is None:
                            link["good_since"] = self.now
                        link["health"] = "RECOVERING"
                        if self.now - link["good_since"] + 1e-9 >= p.recovery_s:
                            link["health"] = "HEALTHY"
                    else:
                        link["good_since"] = None
                        link["health"] = "FAILED"
                else:
                    link["health"] = "HEALTHY" if clear else "SUSPECT"
                link["reason"] = "Nominal" if link["health"] == "HEALTHY" else "Hysteresis / recovery hold"
            if old != link["health"]:
                self.event("WARN" if link["health"] in ("FAILED", "SUSPECT") else "INFO",
                           f"LINK{i}", f"{old} → {link['health']}: {link['reason']}")

    def send(self, kind, dest, epoch, target):
        self.messages.append({"due": self.now + self.latency_s, "kind": kind,
                              "dest": dest, "epoch": epoch, "target": target})
        self.event("DEBUG", "PROTOCOL", f"TX {kind} → {dest}, epoch {epoch}, SFP{target}")

    def begin_switch(self, target, reason):
        if target not in (0, 1):
            raise ValueError("target must be 0 or 1")
        if self.transaction:
            raise ValueError("A transaction is already in progress")
        if self.links[target]["health"] != "HEALTHY" or not self.physical_good(target):
            raise ValueError("Target must be healthy, recovered and physically up")
        if not self.management_up():
            raise ValueError("No bidirectional peer management path")
        if all(b["uplink"] == target for b in self.boards.values()):
            raise ValueError("Both boards already use that uplink")
        self.epoch += 1
        self.last_attempt = self.now
        self.transaction = {"epoch": self.epoch, "target": target, "stage": "PREPARE",
                            "acks": [], "started": self.now, "sent": self.now,
                            "reason": reason}
        self.phase = "PREPARE"
        self.event("INFO", "COORDINATOR", f"Begin epoch {self.epoch}: SFP{target}; {reason}")
        for b in self.boards:
            self.send("PREPARE", b, self.epoch, target)

    def protocol(self):
        if self.now - self.last_heartbeat_sent >= .5:
            self.last_heartbeat_sent = self.now
            if self.management_up() and not self.drop_messages:
                for board in self.boards.values():
                    board["last_heartbeat"] = self.now
        # Independent local watchdog closes user forwarding on peer loss.
        for name, board in self.boards.items():
            if not board["online"] or self.now - board["last_heartbeat"] > self.policy.heartbeat_timeout_s:
                if board["uplink"] is not None:
                    board["uplink"] = None
                    self.event("ERROR", name, "Peer watchdog / offline: user forwarding blocked")
        due = [m for m in self.messages if m["due"] <= self.now]
        self.messages = [m for m in self.messages if m["due"] > self.now]
        for msg in due:
            t = self.transaction
            if not t or msg["epoch"] != t["epoch"]:
                continue  # stale epochs can never commit
            if not self.management_up() or self.drop_messages:
                self.event("WARN", "PROTOCOL", f"Dropped {msg['kind']} for {msg['dest']}")
                continue
            kind, name = msg["kind"], msg["dest"]
            if kind.endswith("_ACK"):
                if kind == t["stage"] + "_ACK" and name not in t["acks"]:
                    t["acks"].append(name)
                continue
            if kind != t["stage"]:
                continue
            board = self.boards[name]
            if board["write_fault"]:
                self.event("ERROR", name, f"Switch readback failed during {kind}; no ACK")
                continue
            if kind == "PREPARE":
                if not self.physical_good(t["target"]):
                    continue
                board["prepared"] = t["epoch"]
            elif kind == "QUIESCE":
                board["uplink"] = None
            elif kind == "COMMIT":
                if board["prepared"] != t["epoch"] or not self.physical_good(t["target"]):
                    continue
                board["uplink"] = t["target"]
                board["epoch"] = t["epoch"]
            self.event("DEBUG", name, f"Applied {kind}; readback OK")
            self.send(kind + "_ACK", name, t["epoch"], t["target"])
        t = self.transaction
        if not t:
            return
        if self.now - t["started"] > self.policy.transaction_timeout_s:
            # Local transaction watchdogs are modeled on both boards, not a remote magic write.
            if t["stage"] != "PREPARE":
                for board in self.boards.values():
                    board["uplink"] = None
            self.event("ERROR", "PROTOCOL", f"Epoch {t['epoch']} timed out in {t['stage']}; retry after hold-down")
            self.transaction = None
            self.phase = "TIMEOUT"
            return
        if len(t["acks"]) == 2:
            if t["stage"] == "COMMIT":
                self.active = t["target"]
                self.last_switch = self.now
                self.failovers += 1
                self.event("INFO", "COORDINATOR", f"Epoch {t['epoch']} verified on A+B; SFP{self.active} forwarding")
                self.transaction = None
                self.phase = "IDLE"
                return
            t["stage"] = "QUIESCE" if t["stage"] == "PREPARE" else "COMMIT"
            self.phase = t["stage"]
            t["acks"] = []
            t["sent"] = self.now
            for b in self.boards:
                self.send(t["stage"], b, t["epoch"], t["target"])
        elif self.now - t["sent"] >= .4:
            t["sent"] = self.now
            for b in self.boards:
                if b not in t["acks"]:
                    self.send(t["stage"], b, t["epoch"], t["target"])

    def automatic(self):
        if not self.policy.auto or self.transaction or not self.management_up():
            return
        if self.now - max(self.last_switch, self.last_attempt) < self.policy.hold_down_s:
            return
        same = all(b["uplink"] == self.active for b in self.boards.values())
        target = None
        if not same:
            target = self.active if self.links[self.active]["health"] == "HEALTHY" else 1 - self.active
        elif self.links[self.active]["health"] == "FAILED":
            target = 1 - self.active
        elif self.policy.revertive and self.active == 1 and self.links[0]["health"] == "HEALTHY":
            target = 0
        if target is not None and self.links[target]["health"] == "HEALTHY":
            self.begin_switch(target, "automatic health policy / forwarding reconciliation")

    def step(self, dt=.1):
        self._dt = dt
        self.now += dt
        elapsed = self.now - self.scenario_start
        if self.scenario == "fade":
            self.links[0]["attenuation"]["B"] = min(15., elapsed * 1.2)
        elif self.scenario == "flap":
            self.links[0]["cut"] = elapsed % 2 < .15
        self.sample()
        self.detect()
        self.protocol()
        self.automatic()
        a, b = self.boards["A"]["uplink"], self.boards["B"]["uplink"]
        connected = a is not None and a == b and self.physical_good(a)
        rx = min(self.telemetry[f"{side}{a}"]["rx_dbm"] for side in ("A", "B")) if connected else -40
        loss = min(.95, max(0., (-rx - 14.) / 8.)) if connected else 1.
        offered = self.offered_mbps if self.traffic_running else 0.
        throughput = min(940., offered) * (1 - loss)
        self.bytes_delivered += throughput * 1e6 / 8 * dt
        self.bytes_lost += max(0., offered - throughput) * 1e6 / 8 * dt
        if not connected and self.traffic_running:
            self.outage_s += dt
        self.traffic = {"synthetic": True, "running": self.traffic_running,
                        "offered_mbps": offered, "throughput_mbps": round(throughput, 2),
                        "loss_pct": round(100 * (1 - throughput / offered), 3) if offered else 0.,
                        "rtt_ms": round(.35 + loss * 8, 3) if connected else None,
                        "jitter_ms": round(.02 + loss * 2, 3) if connected else None,
                        "delivered_bytes": int(self.bytes_delivered), "lost_bytes": int(self.bytes_lost),
                        "outage_s": round(self.outage_s, 3), "connected": connected}
        if not self.history or self.now - self.history[-1]["time"] >= .49:
            self.history.append({"time": round(self.now, 2), "throughput": throughput,
                                 "primary_rx": min(self.telemetry[f"{b}0"]["rx_dbm"] for b in ("A", "B")),
                                 "backup_rx": min(self.telemetry[f"{b}1"]["rx_dbm"] for b in ("A", "B"))})

    def configure(self, values):
        if not isinstance(values, dict) or set(values) - set(asdict(self.policy)):
            raise ValueError("Unknown policy field")
        merged = asdict(self.policy) | values
        for key, value in merged.items():
            if key in ("auto", "revertive"):
                if type(value) is not bool:
                    raise ValueError(f"{key} must be boolean")
            elif type(value) not in (float, int) or not math.isfinite(value):
                raise ValueError(f"{key} must be a finite number")
        if not -35 <= merged["trip_dbm"] < merged["recover_dbm"] <= 0:
            raise ValueError("Require -35 ≤ trip < recover ≤ 0 dBm")
        for key in merged:
            if key.endswith("_s") and not .1 <= merged[key] <= 120:
                raise ValueError(f"{key} must be between 0.1 and 120 seconds")
        self.policy = Policy(**merged)
        self.event("INFO", "POLICY", f"Updated {values}")

    def command(self, data):
        action = data.get("action")
        if action == "policy":
            self.configure(data.get("values", {}))
        elif action == "switch":
            self.begin_switch(data.get("target"), "operator request")
        elif action == "scenario":
            name = data.get("name")
            if name not in ("healthy", "fade", "cut", "flap", "backup_fault", "partition", "write_fault"):
                raise ValueError("Unknown scenario")
            for link in self.links:
                link["cut"] = False
                link["attenuation"] = {"A": 0., "B": 0.}
                for field in ("tx_fault", "tx_disable", "stale"):
                    link[field] = {"A": False, "B": False}
                for field in ("present", "power"):
                    link[field] = {"A": True, "B": True}
            for board in self.boards.values():
                board["online"] = True
                board["write_fault"] = False
            self.partition = name == "partition"
            self.drop_messages = False
            self.links[0]["cut"] = name == "cut"
            self.links[1]["cut"] = name == "backup_fault"
            self.boards["B"]["write_fault"] = name == "write_fault"
            self.scenario, self.scenario_start = name, self.now
            self.event("INFO", "INJECTION", f"Scenario {name}; previous physical injections cleared, policy/state retained")
        elif action == "inject":
            field, value = data.get("field"), data.get("value")
            board, link = data.get("board", "B"), data.get("link", 0)
            if board not in self.boards or type(link) is not int or link not in (0, 1):
                raise ValueError("board A/B and link 0/1 required")
            if field == "attenuation":
                if type(value) not in (int, float) or not math.isfinite(value) or not 0 <= value <= 40:
                    raise ValueError("attenuation must be 0..40 dB")
                self.links[link][field][board] = value
                self.scenario = "custom"
            else:
                if type(value) is not bool:
                    raise ValueError("Fault value must be boolean")
                if field in ("partition", "drop_messages"):
                    setattr(self, field, value)
                elif field in ("online", "write_fault"):
                    self.boards[board][field] = value
                elif field == "cut":
                    self.links[link][field] = value
                    self.scenario = "custom"
                elif field in ("present", "tx_fault", "tx_disable", "power", "stale"):
                    self.links[link][field][board] = value
                else:
                    raise ValueError("Unknown injection field")
            self.event("WARN", "INJECTION", f"{board}/SFP{link} {field}={value}")
        elif action == "traffic":
            rate = data.get("mbps", self.offered_mbps)
            running = data.get("running", self.traffic_running)
            if type(rate) not in (int, float) or not math.isfinite(rate) or not 0 <= rate <= 1000 or type(running) is not bool:
                raise ValueError("traffic: mbps 0..1000, running boolean")
            self.offered_mbps, self.traffic_running = rate, running
            self.event("INFO", "TRAFFIC", f"Synthetic stream {'started' if running else 'stopped'} at {rate} Mbps")
        else:
            raise ValueError("Unknown action")

    def snapshot(self):
        boards = {}
        for name, board in self.boards.items():
            boards[name] = dict(board, role="coordinator" if name == "A" else "agent",
                                heartbeat_age_s=round(self.now - board["last_heartbeat"], 2),
                                vlans=[{"vid": 100, "purpose": "user", "ports": [0] + ([5 + board["uplink"]] if board["uplink"] is not None else [])},
                                       {"vid": 4090, "purpose": "primary management", "ports": [5, 7]},
                                       {"vid": 4091, "purpose": "backup management", "ports": [6, 7]}])
        return {"mode": "SIMULATED", "time": round(self.now, 2), "seed": self.seed,
                "policy": asdict(self.policy), "boards": boards, "links": self.links,
                "sfps": self.telemetry, "traffic": self.traffic, "phase": self.phase,
                "transaction": self.transaction, "active": self.active, "epoch": self.epoch,
                "failovers": self.failovers, "management_up": self.management_up(),
                "partition": self.partition, "drop_messages": self.drop_messages,
                "scenario": self.scenario, "events": list(self.events), "history": list(self.history),
                "limitations": ["Synthetic DOM and traffic; no iperf process or board connection",
                                "Proposed VLAN/handshake policy, not LAN96455 register-level firmware",
                                "Optical quality curve illustrative; no measured BER or SI model"]}
