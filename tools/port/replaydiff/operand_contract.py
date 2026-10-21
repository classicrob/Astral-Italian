#!/usr/bin/env python3
"""Offline retail operand-site binding and a non-controlling read-only port."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct

DOL_SHA256 = 'abea6b0c498c4af90b17cfa08d54a01781186f097bee3e0761dbfb4838cbc281'
NATIVE_SOURCE = '6b78fad2261e08a3ef031b7d2677905b205ddf3f'
DOLPHIN_SOURCE = '409881c34357d0f105fd473167d15ab0bd9c1628'
FUNCTIONS = {
    'actor_init_after': (0x803B7BB0, 0x11C),
    'actor_init_member': (0x803BEF00, 0x7EC),
    'actor_camera_reset': (0x803BFBB0, 0x54),
    'actor_force_matrix': (0x803BDB40, 0xC0),
    'access_keep_up': (0x803B5560, 0xB8),
    'actor_camera_update': (0x803C6670, 0x168),
    'mario_constructor': (0x80386D60, 0xCDC),
    'mario_set_front': (0x8038B000, 0x154),
    'mario_set_head': (0x8038AFC0, 0x38),
    'mario_keep_up': (0x8038B3D0, 0x168),
    'mario_keep_side': (0x8038B540, 0x118),
    'mario_fix_gravity': (0x803898A0, 0x4F0),
    'mario_blend_head': (0x803B23E0, 0x88),
    'mario_quaternion_head': (0x803B1EF0, 0x280),
    'blend_sphere': (0x8003F840, 0x230),
}
# Each pair brackets an original BL, not a substituted guest function call.
CALLS = {
    'camera_blend': ('actor_camera_update', 0x803C6788, 0x8003F840, 'r3=dst=actor+618,r4=from=actor+618,r5=to=actor+648; f1=rate PS0 binary64'),
    'camera_fallback_copy': ('actor_camera_update', 0x803C679C, 0x8000E740, 'r3=actor+618,r4=actor+648; only on blend false'),
    'force_matrix_y': ('actor_force_matrix', 0x803BDBAC, 0x8000E260, 'r3=matrix (48 bytes),r4=actor+618'),
    'access_up_copy': ('access_keep_up', 0x803B55E0, 0x8000E740, 'r3=actor+618,r4=mario+1f0'),
    'camera_reset_copy': ('actor_camera_reset', 0x803BFBDC, 0x8000E740, 'r3=actor+618,r4=actor+648'),
    'init_after_up_copy': ('actor_init_after', 0x803B7C30, 0x8000E740, 'r3=actor+618,r4=actor+648'),
    'init_member_up_zero': ('actor_init_member', 0x803BF07C, 0x8000DBB0, 'r3=actor+618'),
    'front_projected_copy': ('mario_set_front', 0x8038B108, 0x8000E740, 'r3=mario+208,r4=stack projected front'),
    'front_guarded_copy': ('mario_set_front', 0x8038B118, 0x8000E740, 'r3=mario+208,r4=input front'),
    'front_direct_copy': ('mario_set_front', 0x8038B128, 0x8000E740, 'r3=mario+208,r4=input front'),
    'front_normalize': ('mario_set_front', 0x8038B130, 0x8003E500, 'r3=mario+208; save pointer before callee clobbers r3'),
    'keep_up_side_normalize': ('mario_keep_up', 0x8038B49C, 0x8003E500, 'r3=stack side (sp+2c)'),
    'keep_up_saved_side_normalize': ('mario_keep_up', 0x8038B4B0, 0x8003E500, 'r3=mario+310'),
    'head_blend': ('mario_blend_head', 0x803B2414, 0x8003F840, 'r3=stack output,r4=mario+1f0,r5=direction; f1=rate PS0 binary64'),
    'head_normalize': ('mario_blend_head', 0x803B2448, 0x8003E500, 'r3=mario+1f0'),
}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def section_bytes(data, address, size):
    if len(data) < 0x100 or address % 4 or size < 0:
        raise ValueError('invalid DOL header or range')
    for i in range(18):
        offset, base, length = (struct.unpack_from('>I', data, k+4*i)[0] for k in (0, 0x48, 0x90))
        if length and base <= address and address+size <= base+length:
            start = offset+address-base
            if start+size > len(data):
                raise ValueError('truncated DOL section')
            return data[start:start+size]
    raise ValueError('address outside DOL sections')


def branch_link_target(pc, word):
    if word >> 26 != 18 or word & 3 != 1:
        raise ValueError('expected relative branch-and-link')
    displacement = word & 0x03FFFFFC
    if displacement & 0x02000000:
        displacement -= 0x04000000
    return (pc+displacement) & 0xFFFFFFFF


def inspect_dol(path):
    data = path.read_bytes()
    if digest(data) != DOL_SHA256:
        raise ValueError('retail DOL identity mismatch')
    functions = {name: {'entry': f'0x{a:08x}', 'size': n,
                        'sha256': digest(section_bytes(data, a, n)),
                        'execution': 'UNOBSERVED'} for name, (a, n) in FUNCTIONS.items()}
    sites = {}
    for name, (function, pc, target, operands) in CALLS.items():
        entry, size = FUNCTIONS[function]
        if not entry <= pc < pc+4 < entry+size:
            raise ValueError('call/return outside reviewed function')
        instruction = section_bytes(data, pc, 4)
        if branch_link_target(pc, int.from_bytes(instruction, 'big')) != target:
            raise ValueError('reviewed call target mismatch: '+name)
        sites[name] = {'function': function, 'pre_pc': f'0x{pc:08x}',
                       'post_pc': f'0x{pc+4:08x}', 'target': f'0x{target:08x}',
                       'call_sha256': digest(instruction),
                       'post_instruction_sha256': digest(section_bytes(data, pc+4, 4)),
                       'operands': operands, 'execution': 'UNOBSERVED'}
    return {'schema_version': 1, 'status': 'STATIC_SITES_VERIFIED_NOT_RUNTIME_ACCEPTED',
            'qualification': 'BLOCKED', 'source_defect': None,
            'dol_sha256': DOL_SHA256, 'native_source': NATIVE_SOURCE,
            'dolphin_source': DOLPHIN_SOURCE, 'functions': functions, 'calls': sites,
            'target_trace_frames': [887, 888, 889, 903, 904],
            'guest_layout_only': {'actor_mario_pointer': '0x584', 'actor_up': '0x618',
                'actor_up_target': '0x648', 'actor_camera_counter': '0x660:u16',
                'actor_remote_matrix_flag': '0x4e1:u8', 'actor_constants_pointer': '0x590',
                'constants_table_index': '0xc:u32', 'table_camera_timer': '0x7a0:u16',
                'mario_head': '0x1f0', 'mario_front': '0x208', 'mario_side': '0x310'},
            'native_layout': 'DWARF typed fields or native offsetof assertions; never reuse guest offsets',
            'ps1': 'UNAVAILABLE through pinned GDB p interface; no injected mffs/stores or guest calls',
            'launcher_or_installer': False}


NATIVE_SHA256 = '059e7594ea56370693748688f3500869146f57257f6fc0af7b11e091a568756f'
NATIVE_CALLS = {
    'camera_blend_wrapper': (0x1016CA3A4, 0x102DB1BB8,
        'x0=from,x1=to,x2=dst; s0=f32 rate; stub symbol is const/from,const/to,dst wrapper'),
    'front_normalize': (0x1015132CC, 0x1022583AC, 'x0=in-place front vector'),
}


def macho_bytes(data, address, size):
    if len(data) < 32 or data[:4] != b'\xcf\xfa\xed\xfe':
        raise ValueError('expected little-endian Mach-O64')
    ncmds = struct.unpack_from('<I', data, 16)[0]
    offset = 32
    for _ in range(ncmds):
        command, length = struct.unpack_from('<II', data, offset)
        if length < 8 or offset+length > len(data):
            raise ValueError('invalid Mach-O load command')
        if command == 0x19:
            vmaddr, vmsize, fileoff, filesize = struct.unpack_from('<QQQQ', data, offset+24)
            if vmaddr <= address and address+size <= vmaddr+filesize:
                start = fileoff+address-vmaddr
                if start+size > len(data):
                    raise ValueError('truncated Mach-O segment')
                return data[start:start+size]
        offset += length
    raise ValueError('native site outside file-backed segment')


def inspect_native(path):
    data = path.read_bytes()
    if digest(data) != NATIVE_SHA256:
        raise ValueError('native binary identity mismatch')
    sites = {}
    for name, (pc, target, operands) in NATIVE_CALLS.items():
        code = macho_bytes(data, pc, 4)
        word = int.from_bytes(code, 'little')
        if word >> 26 != 0b100101:
            raise ValueError('native site is not ARM64 BL')
        displacement = word & 0x03ffffff
        if displacement & 0x02000000:
            displacement -= 0x04000000
        if pc+displacement*4 != target:
            raise ValueError('native call target mismatch')
        sites[name] = {'file_pre_pc': f'0x{pc:x}', 'file_post_pc': f'0x{pc+4:x}',
                       'file_call_target': f'0x{target:x}', 'operands': operands,
                       'call_sha256': digest(code),
                       'post_instruction_sha256': digest(macho_bytes(data,pc+4,4)),
                       'execution': 'UNOBSERVED; resolve actual module ASLR slide before use'}
    return {'sha256': NATIVE_SHA256, 'source': NATIVE_SOURCE, 'calls': sites,
            'source_frame_file_address': '0x10374c060',
            'source_frame_symbol': '__ZN12_GLOBAL__N_112gSourceFrameE',
            'read_boundary_file_address': '0x1025edf58',
            'read_boundary_symbol': '__ZN12PetariNative8Headless6Replay7consumeEjj',
            'frame_semantics': 'gSourceFrame is stored from mSourceFrame by Driver::wait(true) after the preceding emitted seam; bind actual sync origin, never call exported getter in inferior.',
            'dwarf': 'Offline camera address lookup returned no DIE; typed-field access not demonstrated. ABI argument captures do not depend on guest layout.',
            'mode': 'Raw debugger FPCR AND FPSR reads before/after observation; no expressions, inferior function calls, FP formatting or register writes.'}


class ReadPort:
    """Caller supplies a stopped transport; this port cannot control execution."""
    def __init__(self, request):
        self._request = request

    def query(self, command):
        if not re.fullmatch(r'p[0-9a-f]+|m[0-9a-f]+,[0-9a-f]+', command):
            raise ValueError('only individual register and memory reads permitted')
        if command.startswith('p'):
            if int(command[1:], 16) > 70:
                raise ValueError('unreviewed register')
        else:
            address, size = (int(x, 16) for x in command[1:].split(','))
            if not 0 < size <= 256 or not any(lo <= address and address+size <= hi for lo,hi in
                    [(0x80000000,0x81800000),(0x90000000,0x94000000)]):
                raise ValueError('invalid bounded RAM query')
        return self._request(command)

    def register(self, number):
        if type(number) is not int or not 0 <= number <= 70:
            raise ValueError('unreviewed register')
        text = self.query(f'p{number:x}')
        width = 16 if 32 <= number < 64 else 8
        if not isinstance(text, str) or not re.fullmatch('[0-9a-fA-F]{'+str(width)+'}', text):
            raise ValueError('bad register response')
        return text.lower()

    def memory(self, address, size):
        if type(address) is not int or type(size) is not int or not 0 < size <= 256:
            raise ValueError('invalid bounded memory read')
        if not any(lo <= address and address+size <= hi for lo, hi in
                   [(0x80000000,0x81800000),(0x90000000,0x94000000)]):
            raise ValueError('non-RAM or crossing memory read')
        text = self.query(f'm{address:x},{size:x}')
        if not isinstance(text, str) or not re.fullmatch('[0-9a-fA-F]{'+str(size*2)+'}', text):
            raise ValueError('bad memory response')
        return text.lower()


def passive_snapshot(port, expected_pc, ranges):
    """No connect, breakpoint, continue, writes, float formatting or guest calls."""
    before = {str(i): port.register(i) for i in range(71)}
    if int(before['64'],16) != expected_pc:
        raise ValueError('unexpected stopped PC')
    memory = {name: port.memory(address, size) for name, (address,size) in ranges.items()}
    after = {str(i): port.register(i) for i in range(71)}
    if before != after:
        raise ValueError('exposed guest registers changed during read-only observation')
    return {'registers': before, 'memory_hex': memory, 'ps1': None,
            'mode': {'fpscr_bits': before['70'], 'phase': 'exact stopped PC'},
            'execution_control': 'external; not supplied by this module'}


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--dol', required=True, type=Path)
    p.add_argument('--out', required=True, type=Path)
    p.add_argument('--native', type=Path)
    args = p.parse_args()
    result = inspect_dol(args.dol)
    if args.native:
        result['native_bridge'] = inspect_native(args.native)
    with args.out.open('x') as f:
        json.dump(result, f, indent=2); f.write('\n')
    print(json.dumps({'status':result['status'],'calls':len(result['calls']),'out':str(args.out)}))
