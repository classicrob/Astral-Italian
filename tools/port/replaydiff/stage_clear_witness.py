"""Read-only stage-result witness bound to retail SB4E01 getters and stores."""
import json
import struct

from dolphin_collect import ObserverError

SIGNATURES = {
    0x804C8EC0: '386300124e800020',
    0x804C9000: '908300484e800020',
    0x804C9010: '806300484e800020',
    0x804C8E90: '388000003800ffff98830042900300489083005c988300754e800020',
    0x804D6AE0: '9421fff07c0802a6900100144bffeef54bfdd4a180010014886300427c0803a6382100104e800020',
    0x804D6B10: '9421fff07c0802a6900100144bffeec54bfdd47180010014886300437c0803a6382100104e800020',
}


def decode(raw):
    if len(raw) != 0x4c:
        raise ObserverError('stage-result holder read length mismatch')
    name = raw[0x12:0x42]
    if b'\0' not in name or raw[0x42] > 1 or raw[0x43] > 1:
        raise ObserverError('invalid stage-result holder string/boolean')
    return dict(result_stage=name.split(b'\0')[0].decode('ascii'),
                stage_result=bool(raw[0x42]), new_power_star=bool(raw[0x43]),
                cleared_star_id=struct.unpack_from('>i', raw, 0x48)[0])


class ClearWitness:
    def __init__(self, output):
        self.path = output / 'stage-clear.jsonl'
        self.file = self.path.with_suffix('.jsonl.partial').open('x')
        self.previous = None
        self.bound = False
        self.receipt = dict(schema_version=1, status='incomplete', observation='read-only post-endFrame',
                            proof='retail getters/setter and stage-result boolean loads verified',
                            signatures={f'{address:08x}': code for address, code in SIGNATURES.items()},
                            yoshi_star_result=None, starship_ready=None)

    def bind(self, remote, holder):
        for address, code in SIGNATURES.items():
            if remote.read(address, len(bytes.fromhex(code))).hex() != code:
                raise ObserverError('retail stage-result witness signature mismatch')
        self.holder = holder
        self.bound = True

    def sample(self, remote, row):
        if not self.bound:
            return
        result = decode(remote.read(self.holder, 0x4c))
        context = {key: row[key] for key in ('frame', 'scene', 'stage', 'scenario', 'init_state')}
        state = dict(result, **{key: context[key] for key in context if key != 'frame'})
        if state != self.previous:
            self.file.write(json.dumps(dict(context, **result)) + '\n')
            self.file.flush()
            self.previous = state
        if result['result_stage'] == 'YosshiHomeGalaxy' and result['stage_result'] and result['cleared_star_id'] == 1:
            if self.receipt['yoshi_star_result'] is None:
                self.receipt['yoshi_star_result'] = dict(context, **result)
        if row['stage'] == 'MarioFaceShipGalaxy' and row['init_state'] == 6:
            if self.receipt['starship_ready'] is None:
                self.receipt['starship_ready'] = context

    def close(self, complete):
        self.file.close()
        if complete:
            self.path.with_suffix('.jsonl.partial').rename(self.path)
            self.receipt['status'] = 'captured'
        self.receipt['reproduces_clear'] = bool(complete and self.receipt['yoshi_star_result'] and self.receipt['starship_ready'])
        return self.receipt
