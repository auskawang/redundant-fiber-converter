# Laptop tools

[`demo_console/`](demo_console/README.md) contains the PHOTON virtual GUI and CLI. It uses Python 3.10+ and the standard library only.

From this directory:

```sh
python -m demo_console serve
```

Open http://127.0.0.1:8765. Run `python -m unittest demo_console.test_engine -v` for behavioral tests. See the [operator guide](demo_console/README.md) for CLI commands and simulation limits.
