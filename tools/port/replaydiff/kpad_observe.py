"""Read-only SDK KPAD samples and WPADProbe results for native input tapes."""
import hashlib
import json
import math
from pathlib import Path
import struct

from dolphin_collect import Memory, ObserverError, SEAM
from kpad_script import ENTRY, contract

PROBE = 0x805EB410
PROBE_SHA256 = "974525354eae0f68bdd24c37d67cd750508544f9bd24c4717a173e72097a67a6"
CONTROL_BLOCKS = 0x80791C10


def signed(value):
    return value - 0x100000000 if value & 0x80000000 else value


def decode_sample(raw):
    if len(raw) != 240:
        raise ObserverError('KPAD status must contain 240 bytes')
    def real(offset):
        value = struct.unpack_from('>f', raw, offset)[0]
        if not math.isfinite(value):
            raise ObserverError(f'nonfinite original KPAD field at offset {offset:#x}')
        return dict(value=value, bits='0x' + raw[offset:offset + 4].hex())
    def vector(offset, axes='xyz'):
        return {axis: real(offset + i * 4) for i, axis in enumerate(axes)}
    sample = {name: struct.unpack_from('>I', raw, offset)[0] for offset, name in
              ((0, 'hold'), (4, 'trig'), (8, 'release'), (236, 'unk_EC'))}
    for offset, name in ((24, 'acc_value'), (28, 'acc_speed'), (48, 'speed'),
                         (68, 'hori_speed'), (72, 'dist'), (76, 'dist_vec'), (80, 'dist_speed')):
        sample[name] = real(offset)
    sample['acc'] = vector(12)
    for offset, name in ((32, 'pos'), (40, 'vec'), (52, 'horizon'), (60, 'hori_vec'), (84, 'acc_vertical')):
        sample[name] = vector(offset, 'xy')
    device, error, valid, fmt = struct.unpack_from('>BbbB', raw, 92)
    sample.update(dev_type=device, wpad_err=error, dpd_valid_fg=valid, data_format=fmt)
    if device in (1, 6):
        sample['ex_status'] = dict(fs=dict(stick=vector(96, 'xy'), acc=vector(104),
                                          acc_value=real(116), acc_speed=real(120)))
    elif device in (2, 7):
        cl = {name: struct.unpack_from('>I', raw, offset)[0] for offset, name in
              ((96, 'hold'), (100, 'trig'), (104, 'release'))}
        cl.update(lstick=vector(108, 'xy'), rstick=vector(116, 'xy'),
                  ltrigger=real(124), rtrigger=real(128))
        sample['ex_status'] = dict(cl=cl)
    elif device in (0, 5, 252, 253, 255):
        sample['ex_status'] = {}
    else:
        raise ObserverError(f'unsupported original KPAD extension {device}')
    sample['mpls'] = dict(angularVelocity=vector(176), angle=vector(188),
                          direction={axis: vector(offset) for axis, offset in (('X', 200), ('Y', 212), ('Z', 224))})
    contract.encode_sample(sample)
    return sample


def read_connection(remote, channel):
    # WPADProbe only reads these fields inside an interrupt guard; the CPU is stopped here.
    m = Memory(remote)
    control = m.pointer(CONTROL_BLOCKS + channel * 4)
    error = signed(m.u32(control + 0x900))
    device = m.read(control + 0x905, 1)[0]
    if error != -1:
        if device == 253:
            error = -1
        elif m.u32(control + 0x920) == 0:
            error = -2
    if not -128 <= error <= 127:
        raise ObserverError('original WPADProbe status outside native interchange range')
    return dict(probe_error=error, device_type=device)


class ObservedInput:
    def __init__(self, output):
        self.output = Path(output)
        self.file = self.output.with_suffix('.jsonl.partial').open('w')
        self.breakpoints = set()
        self.pending = []
        self.probe_sha256 = PROBE_SHA256
        self.probe_counts = {}
        self.max_probe_age = 0
        self.connection_transitions = []
        self.connection_method = "post-KPAD read-only WPADProbe-equivalent snapshot; complete SDK function hash verified"
        self.ordinal = 0
        self.reads = 0
        self.script = None
        self.revisions = []
        self.resume_diversions = []
        self.deferred_stop = False

    def add(self, remote, pc):
        if pc not in self.breakpoints:
            remote.ok(f'Z0,{pc:x},4')
            self.breakpoints.add(pc)

    def remove(self, remote, pc):
        if pc in self.breakpoints:
            remote.ok(f'z0,{pc:x},4')
            self.breakpoints.remove(pc)

    def install(self, remote):
        if remote.read(ENTRY, 8).hex() != '38e000014800000c':
            raise ObserverError('original KPADReadEx signature mismatch')
        if hashlib.sha256(remote.read(PROBE, 0x8c)).hexdigest() != PROBE_SHA256:
            raise ObserverError('original WPADProbe complete function signature mismatch')
        self.add(remote, ENTRY)

    def connection(self, remote, channel):
        result = read_connection(remote, channel)
        self.probe_counts[channel] = self.probe_counts.get(channel, 0) + 1
        return result

    def handle(self, remote, pc, frame):
        m = Memory(remote)
        if pc == ENTRY:
            channel = remote.reg(3)
            if channel >= 4:
                raise ObserverError('original SDK controller channel out of range')
            lr = Memory.checked_pointer(remote.reg(67), pc)
            if lr in (ENTRY, SEAM):
                raise ObserverError('original SDK return aliases a fixed observer seam')
            pointer = Memory.checked_pointer(remote.reg(4), pc)
            capacity = remote.reg(5)
            if not 1 <= capacity <= 120:
                raise ObserverError('original KPAD request exceeds supported capacity')
            call = dict(channel=channel, pointer=pointer, lr=lr, capacity=capacity,
                        error=Memory.checked_pointer(remote.reg(6), pc),
                        connection=self.connection(remote, channel))
            self.pending.append(call)
            self.add(remote, lr)
        elif self.pending and pc == self.pending[-1]['lr']:
            call = self.pending.pop()
            channel = call['channel']
            count, error = signed(remote.reg(3)), signed(m.u32(call['error']))
            if not 0 <= count <= call['capacity'] or not -128 <= error <= 127:
                raise ObserverError('original KPAD count/error outside native interchange range')
            connection = self.connection(remote, channel)
            if call['connection'] != connection:
                self.connection_transitions.append(dict(frame=frame, read_ordinal=self.ordinal,
                                                        channel=channel, before=call['connection'], after=connection))
            raw = m.read(call['pointer'], 240 * max(count, 1))
            row = dict(frame=frame, read_ordinal=self.ordinal, channel=channel,
                       requested_length=call['capacity'], returned_count=count, error=error,
                       connection=connection,
                       samples=[decode_sample(raw[i:i + 240]) for i in range(0, len(raw), 240)])
            self.file.write(json.dumps(row, separators=(',', ':'), allow_nan=False) + '\n')
            self.file.flush()
            self.ordinal += 1
            self.reads += 1
            if not any(item['lr'] == pc for item in self.pending):
                self.remove(remote, pc)
        else:
            return False
        keep = pc in self.breakpoints
        temporary = pc + 4 not in self.breakpoints
        if temporary:
            self.add(remote, pc + 4)
        self.remove(remote, pc)
        remote.stop('c')
        next_pc = remote.reg(64)
        if temporary:
            self.remove(remote, pc + 4)
        if keep:
            self.add(remote, pc)
        self.deferred_stop = next_pc != pc + 4
        if self.deferred_stop:
            if next_pc == pc or next_pc not in self.breakpoints | {SEAM}:
                raise ObserverError(f'original SDK resume reached unexpected PC {next_pc:#x}')
            self.resume_diversions.append(dict(frame=frame, expected_pc=f'{pc + 4:08x}', stopped_pc=f'{next_pc:08x}'))
        return True

    def end_frame(self):
        if self.pending:
            raise ObserverError('game-loop seam inside an observed SDK controller call')
        self.ordinal = 0

    def close(self, remote, complete=False):
        if remote:
            for pc in list(self.breakpoints):
                try:
                    self.remove(remote, pc)
                except Exception:
                    pass
        self.file.close()
        if complete:
            self.output.with_suffix('.jsonl.partial').rename(self.output)
