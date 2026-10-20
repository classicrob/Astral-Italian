#!/usr/bin/env python3
"""Run one heavy command with a wall deadline after the caller acquires its lock."""
import os
import signal
import subprocess
import sys


def run(command, seconds=540):
    child = subprocess.Popen(command, start_new_session=True)
    def stop():
        try:
            os.killpg(child.pid, signal.SIGTERM)
        except ProcessLookupError:
            return
        try:
            child.wait(timeout=5)
        except subprocess.TimeoutExpired:
            pass
        try:
            os.killpg(child.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
        child.wait()
    def interrupted(signum, _frame):
        stop()
        raise SystemExit(128 + signum)
    previous = {sig: signal.signal(sig, interrupted) for sig in (signal.SIGINT, signal.SIGTERM)}
    try:
        try:
            return child.wait(timeout=seconds)
        except subprocess.TimeoutExpired:
            print("heavy command exceeded its wall deadline", file=sys.stderr, flush=True)
            stop()
            return 124
    finally:
        for sig, handler in previous.items():
            signal.signal(sig, handler)


def main(argv=None):
    args = sys.argv if argv is None else ["bounded_run.py", *argv]
    if len(args) < 3 or not args[1].isdigit() or not 1 <= int(args[1]) <= 3600:
        raise SystemExit("usage: bounded_run.py <seconds:1..3600> <command> [args...]")
    return run(args[2:], int(args[1]))


if __name__ == "__main__":
    raise SystemExit(main())
