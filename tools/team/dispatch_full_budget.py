"""DISABLED full-context integration of accepted disposable lifecycle component.

No generic compiler ownership gate is implemented/qualified here. run() refuses
before spawning. This file is a review candidate, not execution authorization.
"""
import os
from pathlib import Path
import shutil
import time

import dispatch_budget as accepted

SCAN_ENTRIES = 200000
SCAN_SECONDS = 2.0
GENERIC_OWNERSHIP_STATUS = 'UNIMPLEMENTED_UNQUALIFIED'


def bounded_allocated(root, *, entries=SCAN_ENTRIES, seconds=SCAN_SECONDS):
    """Finite traversal, no symlink following. Syscall latency remains sampled."""
    start = time.monotonic()
    count = 0
    total = 0
    todo = [Path(root)]
    if Path(root).is_symlink():
        raise accepted.DiskBudgetExceeded('allocation root symlink')
    while todo:
        if time.monotonic()-start >= seconds:
            raise accepted.DiskBudgetExceeded('allocation scan time bound')
        directory = todo.pop()
        try:
            with os.scandir(directory) as items:
                for item in items:
                    count += 1
                    if count > entries or time.monotonic()-start >= seconds:
                        raise accepted.DiskBudgetExceeded('allocation scan entry/time bound')
                    try:
                        stat = item.stat(follow_symlinks=False)
                        if item.is_symlink():
                            continue
                        total += stat.st_blocks*512
                        if item.is_dir(follow_symlinks=False):
                            todo.append(Path(item.path))
                    except FileNotFoundError:
                        continue
        except FileNotFoundError:
            continue
    return total


class FullBudgetJob(accepted.LiveBudgetJob):
    def __enter__(self):
        if self.baseline is None:
            self.baseline = bounded_allocated(self.root)
        return super().__enter__()

    def check(self):
        self.remaining()
        self.observe_fixture()
        start = time.monotonic()
        free = shutil.disk_usage(self.root).free
        try:
            size = bounded_allocated(self.root)
        except accepted.DiskBudgetExceeded as error:
            self.failure = str(error)
            raise
        self.remaining()
        self.max_scan_seconds = max(self.max_scan_seconds, time.monotonic()-start)
        growth = max(0, size-self.baseline)
        self.samples += 1
        self.peak_growth = max(self.peak_growth, growth)
        self.lowest_free = free if self.lowest_free is None else min(self.lowest_free, free)
        if free < self.min_free:
            self.failure = 'free-space floor crossed'
            raise accepted.DiskBudgetExceeded(self.failure)
        if growth > self.max_growth:
            self.failure = 'owned allocated-byte growth crossed'
            raise accepted.DiskBudgetExceeded(self.failure)

    def run(self, *args, **kwargs):
        # A fixture PID file or Popen requested setsid cannot qualify arbitrary
        # tools. Deliberate hard block until the exact generic gate is reviewed.
        raise RuntimeError('generic compiler launch ownership remains UNQUALIFIED')
