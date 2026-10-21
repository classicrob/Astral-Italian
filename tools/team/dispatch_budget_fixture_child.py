"""Disposable diagnostic child ONLY; not a production process launcher.

Source-only pending allocation. Every role self-expires after ten seconds.
"""
import argparse
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import time


def publish(path, value):
    temporary = path.with_name(path.name + '.tmp')
    fd = os.open(temporary, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
    try:
        data = json.dumps(value, sort_keys=True).encode()
        if os.write(fd, data) != len(data):
            raise RuntimeError('incomplete fixture identity write')
    finally:
        os.close(fd)
    os.rename(temporary, path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('role', choices=['ignore-term', 'leader-worker', 'worker'])
    parser.add_argument('root', type=Path)
    args = parser.parse_args()
    # Independent self-expiry even if ownership validation prevents all signals.
    signal.signal(signal.SIGALRM, signal.SIG_DFL)
    signal.alarm(10)
    if args.role in ('ignore-term', 'worker'):
        signal.signal(signal.SIGTERM, signal.SIG_IGN)
    else:
        signal.signal(signal.SIGTERM, signal.SIG_DFL)
    identity = dict(pid=os.getpid(), pgid=os.getpgrp(), sid=os.getsid(0), uid=os.getuid())
    if args.role == 'worker':
        identity['ppid'] = os.getppid()
        publish(args.root / 'worker-identity.json', identity)
    else:
        publish(args.root / 'child-identity.json', identity)
        if args.role == 'leader-worker':
            # Inherit the owned leader's existing PGID/SID; do NOT create a session.
            worker = subprocess.Popen(
                [sys.executable, str(Path(__file__).resolve()), 'worker', str(args.root)],
                close_fds=True, start_new_session=False)
            publish(args.root / 'lineage.json', dict(leader_pid=os.getpid(), worker_pid=worker.pid))
            until = time.monotonic() + 3
            while not (args.root / 'worker-verified.json').exists():
                if time.monotonic() >= until:
                    raise RuntimeError('parent did not verify worker; no budget trigger')
                time.sleep(0.01)
        # Only after TERM policy/identity (and worker acknowledgement) are ready.
        publish(args.root / 'trigger.json', dict(ready=True))
    while True:
        time.sleep(0.1)


if __name__ == '__main__':
    main()
