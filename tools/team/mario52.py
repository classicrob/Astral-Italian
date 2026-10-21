"""The single preapproved build-5.2 Mario function proof exception.

All other object semantics remain strict. Evidence scripts are never executed.
Original-game reference bytes and instruction comparison output stay private.
"""
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile

import objsame

PLAN_SHA = 'e607209219b7f7a0d3bfed265e5f596bce8629c4048e08bcc5dcb202ffab3975'
BASE = '885b0ceca53a907136fe4b77b86f41566f82ae1b'
TIP = '99ed05340dcbab79120ac70298562b0b47375f07'
OBJECT = 'src/Game/Player/Mario.o'
SYMBOL = 'fn_803A22D0__5MarioFfl'
TEXT = '.text#8'
BOUNDS = (
    (260, '5861bd1f05a8f2bf1545f0ba3525da394eeab26992ce150862b5b3ace546945f',
     '8d9fdf7ddefc79aa94d445d6c70f0b92535fbbb4111033858bb0bc42d208ff74'),
    (268, '7b543f33c35bae1c4f3e025b736789fdd4a66f4b7377f20e95273df37f36451c',
     '5b7d39ddfd2a9e927bb5cf42eebbac0e72c1c1c8c0f0d0bcafc79b9cca9d13a4'))
DOL_SHA = 'abea6b0c498c4af90b17cfa08d54a01781186f097bee3e0761dbfb4838cbc281'
DOL_BODY_SHA = '338f1923477ed39c09f1989423a114993ef20facd42358ad5fdf982db983f126'
REFERENCE_SHA = '12739075be69638a324afe0f483c71056a7b55ee1f09abd3a6ec7de04d5773bc'
OBJDIFF_SHA = '4ef0896ba8986f1adea6202d6e52a471deb1d126b071d23e88e8c00c250da7fd'


def require(ok, message):
    if not ok:
        raise ValueError('Mario52: ' + message)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def inspect(path, size):
    sections, symbols = objsame.read_object(path)
    keys = objsame.section_keys(sections)
    functions = [s for s in symbols if s['name'] == SYMBOL]
    require(len(functions) == 1, 'approved symbol is not unique')
    function = functions[0]
    require(function['info'] & 15 == 2 and function['size'] == size, 'wrong function type/size')
    require(0 < function['shndx'] < len(sections) and keys[function['shndx']] == TEXT,
            'wrong section occurrence')
    require(function['value'] == 0x2200, 'function moved')
    return sections, symbols, function


def envelope(before, after):
    summaries = []
    for path, size in ((before, 260), (after, 268)):
        sections, _, function = inspect(path, size)
        data = sections[function['shndx']]['data']
        padding = 0x2304 if size == 260 else 0x230c
        require(data[padding:0x2310] == bytes(0x2310 - padding), 'alignment padding is not zero')
        summary = objsame.summarize(path)
        key = 'section ' + TEXT
        typ, flags, length, align, _ = summary[key]
        summary[key] = (typ, flags, length, align, data[:0x2200] + bytes(268) + data[0x230c:])
        summary['named symbols'] = tuple(sorted(
            (name, info, other, section, value, 260 if name == SYMBOL else symbol_size)
            for name, info, other, section, value, symbol_size in summary['named symbols']))
        key = 'relocations for ' + TEXT
        summary[key] = tuple(row for row in summary[key] if not 0x2200 <= row[0] < 0x230c)
        summaries.append(summary)
    differences = sorted(k for k in summaries[0].keys() | summaries[1].keys()
                         if summaries[0].get(k) != summaries[1].get(k))
    require(not differences, 'outside-envelope difference: ' + ', '.join(differences))


def negative_controls(before, after):
    """Mutate actual ELF structure; do not use pinned-hash rejection as a control."""
    data = Path(after).read_bytes()
    shoff = struct.unpack_from('>I', data, 0x20)[0]
    entsize, count = struct.unpack_from('>HH', data, 0x2e)
    headers = [struct.unpack_from('>10I', data, shoff + i * entsize) for i in range(count)]
    sections, symbols, function = inspect(after, 268)
    keys = objsame.section_keys(sections)
    other = next(s for s in symbols if s['name'] != SYMBOL and s['info'] & 15 == 2
                 and s['shndx'] == function['shndx'] and s['size'] > 0)
    pool = next(i for i, s in enumerate(sections) if s['name'] in ('.rodata', '.sdata2') and s['size'])
    symtab = next(i for i, s in enumerate(sections) if s['type'] == objsame.SHT_SYMTAB)
    mutations = {'other_function_text_byte': headers[function['shndx']][4] + other['value'],
                 'constant_pool_byte': headers[pool][4],
                 'other_function_symbol_size': headers[symtab][4] + symbols.index(other) * 16 + 11}
    for i, section in enumerate(sections):
        if section['type'] != objsame.SHT_RELA or section['name'] in objsame.SKIP:
            continue
        source = keys[section['info']]
        if source.split('#')[0] in objsame.SKIP:
            continue
        for row in range(section['size'] // 12):
            offset = struct.unpack_from('>I', section['data'], row * 12)[0]
            if source == TEXT and 0x2200 <= offset < 0x230c:
                continue
            mutations['outside_relocation_addend'] = headers[i][4] + row * 12 + 11
            break
        if 'outside_relocation_addend' in mutations:
            break
    require(len(mutations) == 4, 'missing independent mutation site')
    with tempfile.TemporaryDirectory(prefix='mario52-controls-') as directory:
        for name, offset in mutations.items():
            mutated = bytearray(data)
            mutated[offset] ^= 1
            target = Path(directory) / (name + '.o')
            target.write_bytes(mutated)
            try:
                envelope(before, target)
            except ValueError:
                continue
            raise ValueError('Mario52: control accepted: ' + name)
    return sorted(mutations)


def verify_objects(before, after):
    for path, (size, body_sha, object_sha) in zip((before, after), BOUNDS):
        require(sha(Path(path).read_bytes()) == object_sha, 'unreviewed full object bytes')
        sections, symbols, function = inspect(path, size)
        body = sections[function['shndx']]['data'][0x2200:0x2200 + size]
        require(sha(body) == body_sha, 'unreviewed function bytes')
        require(sum(s['info'] & 15 == 2 and 0 < s['shndx'] < len(sections) for s in symbols) == 623,
                'function inventory differs')
    envelope(before, after)
    return negative_controls(before, after)


def verify_reference(dol, reference, tool):
    data = dol.read_bytes()
    require(sha(data) == DOL_SHA, 'original DOL hash mismatch')
    offsets = struct.unpack_from('>18I', data, 0)
    addresses = struct.unpack_from('>18I', data, 0x48)
    sizes = struct.unpack_from('>18I', data, 0x90)
    locations = [offset + 0x803a22d0 - address
                 for offset, address, size in zip(offsets, addresses, sizes)
                 if address <= 0x803a22d0 and 0x803a22d0 + 276 <= address + size]
    require(locations == [3791952], 'DOL function mapping mismatch')
    require(sha(data[3791952:3791952 + 276]) == DOL_BODY_SHA, 'DOL function bytes mismatch')
    require(sha(reference.read_bytes()) == REFERENCE_SHA, 'reference object hash mismatch')
    require(sha(tool.read_bytes()) == OBJDIFF_SHA, 'objdiff executable hash mismatch')


def verify_evidence(receipt, root, confined):
    require(receipt['spec']['base'] == BASE, 'wrong original proof base')
    wanted = {'plan', 'comparison', 'matching', 'helper', 'reference_recipe', 'measurement_recipe'}
    records = receipt['sanctioned_delta']
    require(set(records) == wanted, 'missing or unexpected sanction evidence')
    for record in records.values():
        require(set(record) == {'path', 'sha256'}, 'malformed sanction artifact record')
        data = confined(root, record['path']).read_bytes()
        require(data and sha(data) == record['sha256'], 'sanction artifact hash mismatch')
    require(records['plan']['sha256'] == PLAN_SHA, 'unapproved sanction plan')
    plan = json.loads(confined(root, records['plan']['path']).read_text())
    require(plan['base'] == BASE and plan['tip'] == TIP and plan['object'] == OBJECT,
            'sanction source binding mismatch')
    require(receipt['spec']['after'] == {
        'src': 'c3e6fe506cb2f2fe873024663de1950a61992b67',
        'include': 'b72499674cbd183a43e95c1a258bc25b21217a57',
        'libs': '1beea762e1cd327a9aeebe42e39a778f0fa0fa02',
        'config': 'b3413d55c3308c4556962b831ddf47bfb92cdb9a'}, 'unreviewed candidate trees')
    require(OBJECT in receipt['spec']['units'], 'sanctioned object absent from proof')


def measure(before, after, repository, published):
    """Recompute both metrics locally with pinned tools and private references."""
    reference = repository / 'build/SB4E01/obj/Game/Player/Mario.o'
    tool = repository / 'build/tools/objdiff-cli'
    dol = Path.home() / 'MacioGalaxy2-gamedata/SB4E01-extracted/sys/main.dol'
    verify_reference(dol, reference, tool)
    require(published['status'] == 'PASS' and published['tool_sha256'] == OBJDIFF_SHA,
            'unbound published metrics')
    require(published['reference']['reference_sha256'] == REFERENCE_SHA,
            'published reference mismatch')
    configs = published['reference']['source_config']
    names = ('config.yml', 'symbols.txt', 'splits.txt', 'ldscript.lcf')
    require(len(configs) == 4 and {Path(p).name for p in configs} == set(names),
            'reference config inventory mismatch')
    for name, expected_hash in configs.items():
        require(sha((repository / 'config/SB4E01' / Path(name).name).read_bytes()) == expected_hash,
                'reference source config mismatch')
    private = Path.home() / 'MacioGalaxy2-gamedata/solL'
    private.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='proof52-local-metrics-', dir=private))
    output.chmod(0o700)
    scores, metrics = {}, {}
    for phase, obj in (('before', before), ('after', after)):
        directory = output / phase
        directory.mkdir()
        diff = directory / 'function-private.json'
        subprocess.run([str(tool), '-L', 'error', 'diff', '-1', str(reference), '-2', str(obj),
                        '-o', str(diff)], check=True, timeout=60)
        parsed = json.loads(diff.read_text())
        functions = [s for s in parsed['left']['symbols'] if s.get('instructions')
                     and not s['name'].startswith(('@', 'lbl_', '...', '.', 'gap_'))]
        scores[phase] = {s['name']: float(s['match_percent']) for s in functions}
        require(len(functions) == len(scores[phase]) == 551, 'scored function inventory mismatch')
        config = {'build_base': False, 'build_target': False, 'units': [
            {'name': 'Game/Player/Mario', 'target_path': str(reference), 'base_path': str(obj)}]}
        (directory / 'objdiff.json').write_text(json.dumps(config))
        report = directory / 'tu-private.json'
        subprocess.run([str(tool), '-L', 'error', 'report', 'generate', '-d', '-p', str(directory),
                        '-o', str(report)], check=True, timeout=60)
        tu = float(json.loads(report.read_text())['units'][0]['measures']['fuzzy_match_percent'])
        metrics[phase] = {'object_sha256': sha(Path(obj).read_bytes()),
                          'function_match_percent': scores[phase][SYMBOL],
                          'tu_fuzzy_match_percent': tu, 'function_count': 551}
        for key, value in metrics[phase].items():
            require(published['phases'][phase][key] == value, 'published/local metric mismatch: ' + key)
    changed = sorted(n for n in scores['before'].keys() | scores['after'].keys()
                     if scores['before'].get(n) != scores['after'].get(n))
    require(changed == [SYMBOL], 'other function scores changed')
    require(metrics['after']['function_match_percent'] > metrics['before']['function_match_percent'],
            'DOL function matching did not improve')
    require(metrics['after']['tu_fuzzy_match_percent'] >= metrics['before']['tu_fuzzy_match_percent'],
            'whole-TU matching decreased')
    all_scores = {name: {phase: scores[phase][name] for phase in ('before', 'after')}
                  for name in sorted(scores['before'])}
    require(published['all_function_scores'] == all_scores, 'published function scores mismatch')
    result = {'status': 'PASS', 'plan_sha256': PLAN_SHA, 'phases': metrics,
              'unchanged_other_function_scores': 550, 'changed_functions': changed,
              'private_evidence': str(output), 'reference_sha256': REFERENCE_SHA,
              'original_dol_sha256': DOL_SHA, 'tool_sha256': OBJDIFF_SHA}
    (output / 'scalar-receipt.json').write_text(json.dumps(result, indent=2) + '\n')
    (repository / 'build/remote-proof-mario52.json').write_text(json.dumps(result, indent=2) + '\n')
    return result
