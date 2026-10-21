"""Explicit filtered KPAD injection and native-compatible read export for SB4E01."""
import importlib.util
import hashlib
import json
import math
from bisect import bisect_right
from pathlib import Path

from dolphin_collect import Memory, ObserverError, REPO, SEAM, private_path

_spec = importlib.util.spec_from_file_location("headless_contract", REPO / "native/tools/headless.py")
contract = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(contract)
ENTRY = 0x8054B440
RETURN = 0x804CF884
BUTTONS = dict(LEFT=1, RIGHT=2, DOWN=4, UP=8, PLUS=16, TWO=256, ONE=512,
               B=1024, A=2048, MINUS=4096, Z=8192, C=16384, HOME=32768)


def load_script(path, frames):
    script = json.loads(Path(path).read_text())
    if set(script) != {"version", "frames", "segments"} or script["version"] != 1 or script["frames"] != frames:
        raise ObserverError("KPAD script version/frame count mismatch")
    end = 0
    for segment in script["segments"]:
        if set(segment) - {"start", "end", "buttons", "pointer", "stick", "acc"} or not {"start", "end"} <= set(segment):
            raise ObserverError("unknown/missing KPAD segment fields")
        start, stop = segment["start"], segment["end"]
        if type(start) is not int or type(stop) is not int or not end <= start < stop <= frames:
            raise ObserverError("KPAD segments must be ordered, disjoint and bounded")
        end = stop
        buttons = segment.get("buttons", [])
        if not isinstance(buttons, list) or len(set(buttons)) != len(buttons) or any(name not in BUTTONS for name in buttons):
            raise ObserverError("invalid KPAD buttons")
        for name in ("pointer", "stick"):
            value = segment.get(name)
            if value is not None and (not isinstance(value, list) or len(value) != 2 or any(
                    type(x) not in (float, int) or not math.isfinite(x) or not -1 <= x <= 1 for x in value)):
                raise ObserverError("pointer/stick must be normalized finite pairs")
        acc = segment.get("acc")
        if acc is not None and (not isinstance(acc, list) or len(acc) != 3 or any(
                type(x) not in (float, int) or not math.isfinite(x) or not -8 <= x <= 8 for x in acc)):
            raise ObserverError("acc must be a finite three-axis vector within +/-8g")
    return script


class ScriptedInput:
    def __init__(self, path, frames, output, live=False):
        self.script = load_script(path, frames)
        self.starts = [item['start'] for item in self.script['segments']]
        self.path = Path(path)
        self.live = live
        self.modified = self.path.stat().st_mtime_ns
        self.revisions = []
        self.output = Path(output)
        self.file = self.output.with_suffix(".jsonl.partial").open("w")
        self.previous = [0] * 4
        self.previous_pointer = None
        self.previous_acc = [0, -1, 0]
        self.ordinal = 0
        self.reads = 0
        self.pending = None
        self.breakpoints = set()
        self.deferred_stop = False
        self.resume_diversions = []
        self.raw_suffix = None

    def start_suffix(self, suffix, source_start):
        """Freeze a shared suffix after the completed sync loop; preserve past intent."""
        if self.pending is not None or self.ordinal != 0:
            raise ObserverError("input suffix must start between completed source loops")
        segments = []
        for item in self.script["segments"]:
            if item["start"] < source_start:
                segments.append(dict(item, end=min(item["end"], source_start)))
        for item in suffix["segments"]:
            segments.append(dict(item, start=item["start"] + source_start,
                                 end=item["end"] + source_start))
        self.script = dict(version=1, frames=source_start + suffix["frames"], segments=segments)
        self.starts = [item['start'] for item in segments]
        self.live = False
        self.revisions.append(dict(suffix_source_start=source_start))

    def start_record_suffix(self, suffix, source_start, remote):
        from kpad_observe import PROBE, PROBE_SHA256
        if self.pending is not None or self.ordinal != 0:
            raise ObserverError('raw suffix must start between completed loops')
        if hashlib.sha256(remote.read(PROBE, 0x8c)).hexdigest() != PROBE_SHA256:
            raise ObserverError('original WPADProbe complete function signature mismatch')
        suffix.start(source_start)
        suffix.receipt.update(connection_method='read-only WPADProbe-equivalent after each SDK read',
                              probe_sha256=PROBE_SHA256)
        self.raw_suffix = suffix
        self.live = False
        self.revisions.append(dict(raw_suffix_source_start=source_start, sha256=suffix.expected_sha256))

    def install(self, remote):
        for address, encoded in ((ENTRY, "38e000014800000c"), (RETURN - 4, "4807bbc1"), (RETURN, "3b9c0001907d0004")):
            if remote.read(address, len(bytes.fromhex(encoded))) != bytes.fromhex(encoded):
                raise ObserverError("original KPAD seam instruction mismatch")
        remote.ok(f"Z0,{ENTRY:x},4")
        self.breakpoints.add(ENTRY)

    def sample(self, frame, channel):
        if self.live and self.path.stat().st_mtime_ns != self.modified:
            updated = load_script(self.path, self.script["frames"])
            def intent(script, at):
                item = next((item for item in script["segments"] if item["start"] <= at < item["end"]), {})
                return {key: item[key] for key in ("buttons", "pointer", "stick", "acc") if key in item}
            if any(intent(updated, old) != intent(self.script, old) for old in range(frame + 1)):
                raise ObserverError("live edits must preserve all past/current frame inputs")
            self.script = updated
            self.starts = [item['start'] for item in updated['segments']]
            self.modified = self.path.stat().st_mtime_ns
            self.revisions.append(dict(first_future_frame=frame + 1,
                                       sha256=hashlib.sha256(self.path.read_bytes()).hexdigest()))
        sample = contract.idle_sample(channel == 0)
        index = bisect_right(self.starts, frame) - 1
        item = self.script['segments'][index] if index >= 0 else {}
        segment = item if channel == 0 and item and frame < item['end'] else {}
        hold = sum(BUTTONS[name] for name in segment.get("buttons", []))
        sample.update(hold=hold, trig=hold & ~self.previous[channel], release=self.previous[channel] & ~hold)
        self.previous[channel] = hold
        def vector(pair):
            return {axis: contract.floating(value) for axis, value in zip("xy", pair)}
        if channel == 0:
            sample["ex_status"]["fs"]["stick"] = vector(segment.get("stick", [0, 0]))
            acc = segment.get("acc", [0, -1, 0])
            sample.update(acc={axis: contract.floating(value) for axis, value in zip("xyz", acc)},
                          acc_value=contract.floating(math.sqrt(sum(value * value for value in acc))),
                          acc_speed=contract.floating(math.sqrt(sum((a - b) ** 2 for a, b in zip(acc, self.previous_acc)))))
            self.previous_acc = acc
            pointer = segment.get("pointer")
            if pointer is not None:
                old = self.previous_pointer or pointer
                delta = [pointer[i] - old[i] for i in range(2)]
                sample.update(pos=vector(pointer), vec=vector(delta), speed=contract.floating(math.hypot(*delta)),
                              dpd_valid_fg=2, dist=contract.floating(2))
            self.previous_pointer = pointer
        return sample

    @staticmethod
    def write(remote, address, data):
        Memory.checked_pointer(address, address)
        if len(data) not in (4, 240) or address + len(data) > (0x81800000 if address < 0x90000000 else 0x94000000):
            raise ObserverError("invalid KPAD write range")
        # Only the captured SDK output buffers and error word are passed here.
        remote.ok(f"M{address:x},{len(data):x}:{data.hex()}")

    def emit(self, frame, channel, capacity, sample):
        result, error = (1, 0) if channel == 0 else (0, -1)
        row = dict(frame=frame, read_ordinal=self.ordinal, channel=channel, requested_length=capacity,
                   returned_count=result, error=error,
                   connection=dict(probe_error=error, device_type=1 if channel == 0 else 253), samples=[sample])
        self.file.write(json.dumps(row, separators=(",", ":")) + "\n")
        self.file.flush()
        self.ordinal += 1
        self.reads += 1
        return row

    def handle(self, remote, pc, frame):
        if pc == ENTRY:
            if self.pending is not None or remote.reg(67) != RETURN:
                raise ObserverError("nested/unrecognized KPAD reader")
            channel, buffer, capacity, error = [remote.reg(i) for i in range(3, 7)]
            if channel >= 4 or capacity != 120:
                raise ObserverError("unexpected KPAD channel/capacity")
            Memory.checked_pointer(buffer, buffer)
            Memory.checked_pointer(error, error)
            self.pending = (channel, buffer, capacity, error)
            remote.ok(f"Z0,{RETURN:x},4")
            self.breakpoints.add(RETURN)
        elif pc == RETURN:
            if self.pending is None:
                raise ObserverError("KPAD return without entry")
            channel, buffer, capacity, error = self.pending
            if self.raw_suffix:
                from kpad_observe import read_connection
                row = self.raw_suffix.take(frame, self.ordinal, channel, capacity)
                row = self.raw_suffix.connection(row, read_connection(remote, channel))
                result, err = row['returned_count'], row['error']
                samples = row['samples']
            else:
                result, err = (1, 0) if channel == 0 else (0, -1)
                samples = [self.sample(frame, channel)]
            for index, sample in enumerate(samples):
                raw = bytes(contract.encode_sample(sample))
                self.write(remote, buffer + index * 240, raw)
                if remote.read(buffer + index * 240, 240) != raw:
                    raise ObserverError('KPAD sample injection readback mismatch')
            self.write(remote, error, (err & 0xffffffff).to_bytes(4, 'big'))
            remote.ok(f'P3={result & 0xffffffff:08x}')
            if (remote.reg(3) != (result & 0xffffffff) or
                    remote.read(error, 4) != (err & 0xffffffff).to_bytes(4, 'big')):
                raise ObserverError('KPAD injection readback mismatch')
            if self.raw_suffix:
                self.file.write(json.dumps(row, separators=(',', ':')) + '\n')
                self.file.flush()
                self.ordinal += 1
                self.reads += 1
            else:
                self.emit(frame, channel, capacity, samples[0])
            self.pending = None
        else:
            return False
        remote.ok(f"Z0,{pc + 4:x},4")
        self.breakpoints.add(pc + 4)
        remote.ok(f"z0,{pc:x},4")
        self.breakpoints.discard(pc)
        remote.stop("c")
        next_pc = remote.reg(64)
        self.deferred_stop = next_pc != pc + 4
        if self.deferred_stop:
            if next_pc == pc or next_pc not in self.breakpoints | {SEAM}:
                raise ObserverError(f"KPAD step breakpoint expected {pc + 4:#x}, got {next_pc:#x}")
            self.resume_diversions.append(dict(frame=frame, expected_pc=f"{pc + 4:08x}", stopped_pc=f"{next_pc:08x}"))
        remote.ok(f"z0,{pc + 4:x},4")
        self.breakpoints.discard(pc + 4)
        if pc == ENTRY:
            remote.ok(f"Z0,{ENTRY:x},4")
            self.breakpoints.add(ENTRY)
        return True

    def end_frame(self, source_frame=None):
        if self.pending is not None:
            raise ObserverError("game seam encountered inside KPAD read")
        if self.raw_suffix:
            self.raw_suffix.end_frame(source_frame)
        self.ordinal = 0

    def close(self, remote, complete=False):
        if remote:
            for address in self.breakpoints:
                try:
                    remote.ok(f"z0,{address:x},4")
                except Exception:
                    pass
        self.file.close()
        if self.raw_suffix:
            self.raw_suffix.close(complete)
        if complete:
            self.output.with_suffix(".jsonl.partial").rename(self.output)


def expand(path, output):
    """Expand the declared WPadHolder four-read contract; verify against live exports."""
    output = private_path(output)
    if output.exists() or output.with_suffix(".jsonl.partial").exists():
        raise ObserverError("input expansion output already exists")
    frames = json.loads(Path(path).read_text())["frames"]
    inputs = ScriptedInput(path, frames, output)
    try:
        for frame in range(frames):
            for channel in range(4):
                inputs.emit(frame, channel, 120, inputs.sample(frame, channel))
            inputs.end_frame()
    except BaseException:
        inputs.close(None)
        raise
    inputs.close(None, complete=True)


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="Expand the frozen controller script to native KPAD JSONL")
    parser.add_argument("script", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    expand(args.script, args.output)
