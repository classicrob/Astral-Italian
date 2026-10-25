"""Read-only external startup observer, identical for baseline/candidate.

First-valid is bounded by the previous native 60-row flush and the flush that
contains its row. No interpolation from game/render intervals or guessed clock.
"""
import csv
import io
import json
from pathlib import Path
import re
import threading
import time


def clocks():
    return {'monotonic_ns': time.monotonic_ns(), 'wall_ns': time.time_ns()}


class Observer:
    def __init__(self, folder, stage=None):
        self.folder, self.stage = Path(folder), stage
        self.start = clocks()
        self.stop_event = threading.Event()
        self.thread = threading.Thread(target=self.run, daemon=True)
        self.result = {'method': 'external-read-only-route-flush-and-stderr', 'stage': stage,
                       'popen_call_before': self.start, 'poll_seconds': .002, 'route_flush_rows': 60}
        self.thread.start()

    def popen_before(self):
        self.start = clocks()
        self.result['popen_call_before'] = self.start

    def spawned(self, pid):
        self.result.update(pid=pid, popen_return_after=clocks())

    def run(self):
        previous = self.start
        log_bytes, route_bytes = b'', b''
        route_boundaries = {}
        handles = {}
        try:
            while not self.stop_event.is_set():
                before = previous
                for name in ('app.log', 'route.csv'):
                    if name not in handles:
                        try: handles[name] = (self.folder / name).open('rb')
                        except FileNotFoundError: continue
                    chunk = handles[name].read()
                    if name == 'app.log': log_bytes += chunk
                    else: route_bytes += chunk
                after = clocks()
                text = log_bytes.decode(errors='replace')
                if 'play_ready_observed' not in self.result:
                    match = re.search(r'\[play ready\] frame (\d+) wall_ms=(\d+)', text)
                    if match: self.result['play_ready_observed'] = {'seam': int(match[1]), 'native_wall_ms': int(match[2]), 'appearance_upper': after}
                if self.stage:
                    complete = route_bytes[:route_bytes.rfind(b'\n') + 1]
                    # Parse only when a native flush added data, not on every poll.
                    if len(complete) != self.result.get('parsed_route_bytes', 0):
                        self.result['parsed_route_bytes'] = len(complete)
                        rows = list(csv.DictReader(io.StringIO(complete.decode()))) if complete else []
                        count = len(rows)
                        for boundary in range(60, count + 1, 60):
                            route_boundaries.setdefault(boundary, {'before': before, 'after': after})
                        valid = next((r for r in rows if r['stage'] == self.stage and r['valid'] == '1'), None)
                        if valid and 'first_valid' not in self.result:
                            row = int(valid['frame'])
                            prior_count = row // 60 * 60
                            lower = route_boundaries[prior_count]['before'] if prior_count else self.start
                            self.result['first_valid'] = {'route_row': row, 'observed_rows': count,
                                'prior_flush_row_count': prior_count, 'lower': lower, 'upper': after,
                                'elapsed_ms_bounds': [(lower['monotonic_ns'] - self.start['monotonic_ns']) / 1e6,
                                                      (after['monotonic_ns'] - self.start['monotonic_ns']) / 1e6]}
                    native = re.search(r'\[native first valid\] stage=([^ ]+) route_row=(\d+) monotonic_ns=(\d+) wall_ns=(\d+)', text)
                    if native and 'native_first_valid' not in self.result:
                        self.result['native_first_valid'] = {'stage': native[1], 'route_row': int(native[2]),
                                                            'monotonic_ns': int(native[3]), 'wall_ns': int(native[4])}
                    if 'first_valid' in self.result and 'play_ready_observed' in self.result: break
                else:
                    match = re.search(r'^.*event=title_first_draw[^\n]*\n', text, re.M)
                    if match:
                        self.result['title_first_draw'] = {'raw': match[0].strip(), 'appearance_lower': before,
                            'appearance_upper': after, 'elapsed_ms_bounds': [(before['monotonic_ns']-self.start['monotonic_ns'])/1e6,
                                                                          (after['monotonic_ns']-self.start['monotonic_ns'])/1e6]}
                        break
                previous = after
                self.stop_event.wait(.002)
        except Exception as exc:
            self.result['observer_error'] = repr(exc)
        finally:
            for handle in handles.values(): handle.close()

    def finish(self):
        self.stop_event.set(); self.thread.join(timeout=5)
        self.result['observer_joined'] = not self.thread.is_alive()
        first, native = self.result.get('first_valid'), self.result.get('native_first_valid')
        if first and native:
            self.result['native_first_valid_inside_external_bounds'] = (native['stage'] == self.stage and
                native['route_row'] == first['route_row'] and first['lower']['monotonic_ns'] <= native['monotonic_ns'] <= first['upper']['monotonic_ns'])
        (self.folder/'startup-observer.json').write_text(json.dumps(self.result, indent=2)+'\n')
        return self.result
