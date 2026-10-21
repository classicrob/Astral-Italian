"""Read-only decoder qualification; not a production proof verifier."""
import argparse
import datetime
import json
from pathlib import Path
import struct

import dispatchownership as own
from dispatchproof import dol_read, sha, require

BASELINE = {
    'TU13': 'e67b1555fc049b89028b29dfa772771caffb3e1a0057bd583af212a6fd697ab2',
    'TU14': '2bf0c053980c2d8ad6bad9e72e3554ae15fc037a0795cd561cfabbb97d74e244',
}


def qualify(dol_path, objects):
    data = dol_path.read_bytes()
    report = {'status': 'READ_ONLY_DECODER_QUALIFICATION_ONLY', 'production_acceptance': False,
              'candidate_compilation': False, 'fresh_candidate_qualification': 'PENDING',
              'time': datetime.datetime.now(datetime.timezone.utc).isoformat(),
              'original_dol_sha256': sha(data), 'original_chains': {}, 'known_bad_objects': {},
              'semantic_negative_controls': []}
    for scope in ('TU13', 'TU14'):
        report['original_chains'][scope] = own.decode_original_dol(data, scope)
        path = objects[scope]
        before = sha(path.read_bytes())
        require(before == BASELINE[scope], 'wrong qualification baseline object')
        result = own.decode_object(path, scope)
        require(len(result['mismatches']) == 2, 'expected two measured old dispatch mismatches')
        try:
            own.require_candidate(path, scope)
        except ValueError as error:
            reason = str(error)
        else:
            raise ValueError('known bad dispatch was accepted as candidate')
        require(sha(path.read_bytes()) == before, 'baseline changed during qualification')
        report['known_bad_objects'][scope] = {'sha256': before, 'mismatches': result['mismatches'],
                                             'candidate_rejected': True, 'reason': reason, 'object_unchanged': True}
    def form(op, rt, ra, immediate):
        return op << 26 | rt << 21 | ra << 16 | (immediate & 65535)
    def jump(pc, target, link=False):
        return 18 << 26 | ((target - pc) & 0x03fffffc) | int(link)
    mutations = [
        ('TU13', 'wrong_executor_base', 0x804f9730, form(32, 3, 3, 0)),
        ('TU13', 'wrong_vtable_slot', 0x807031a0, 0x804f9710),
        ('TU13', 'wrong_ctor_store', 0x804f96e8, form(36, 4, 3, 4)),
        ('TU13', 'wrong_virtual_slot', 0x802431f4, form(32, 12, 12, 12)),
        ('TU13', 'wrong_selected_singleton', 0x804f93a8, form(14, 4, 13, 0)),
        ('TU14', 'mislabeled_BL_at_ADDI', 0x804f9978, jump(0x804f9978, 0x804f99a0, True)),
        ('TU14', 'stale_deadflag', 0x804f97d0, form(14, 0, 0, 0)),
        ('TU14', 'wrong_wait_target', 0x804f99c4, jump(0x804f99c4, 0x804f9854)),
    ]
    for scope, name, pc, replacement in mutations:
        def read(address):
            return replacement if address == pc else struct.unpack('>I', dol_read(data, address, 4))[0]
        try:
            own.decode_original_words(read, scope)
        except ValueError as error:
            reason = str(error)
        else:
            raise ValueError('semantic mutation accepted: ' + name)
        report['semantic_negative_controls'].append({'scope': scope, 'control': name,
            'instruction_or_slot_address': hex(pc), 'rejected': True,
            'outer_DOL_hash_bypassed_to_reach_semantic_check': True, 'reason': reason})
    require(sha(dol_path.read_bytes()) == own.DOL_SHA, 'private DOL changed during qualification')
    report['tools'] = {name: sha(Path(__file__).with_name(name).read_bytes()) for name in
                       ('dispatchownership.py', 'dispatchproof.py', 'objsame.py', 'qualify_dispatchownership.py')}
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dol', required=True, type=Path)
    parser.add_argument('--tu13', required=True, type=Path)
    parser.add_argument('--tu14', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    require(not args.output.exists(), 'qualification output already exists')
    report = qualify(args.dol, {'TU13': args.tu13, 'TU14': args.tu14})
    with args.output.open('x') as target:
        target.write(json.dumps(report, indent=2) + '\n')
    print('Decoder qualified; two original chains, two rejecting old objects, eight semantic controls. No proof acceptance.')


if __name__ == '__main__':
    main()
