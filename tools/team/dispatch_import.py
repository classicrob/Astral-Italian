"""Independent DISPATCH_PROOF_V1 verifier; no production hook or certificate writes.

All trust pins and command expectations come from the operator's independently
qualified local inputs, never from the uploaded recipe. No uploaded code runs.
"""
import base64
import copy
import hashlib
import io
import json
from pathlib import Path, PurePosixPath
import re
import shlex
import struct
import subprocess
import tarfile
import tempfile

import dispatchownership as ownership
import dispatchproof as dispatch
import objsame

TYPE = 'DISPATCH_PROOF_V1'
SCOPES = ('TU13', 'TU14')
PROVENANCE = {'before_log', 'after_log', 'control_log', 'recipe',
              'unit_manifest', 'spec_comparison'}
DELTA = {'schema', 'plans', 'original_ownership', 'strict_remainder',
         'effective_reference_closure', 'symbol_map', 'secondary_control',
         'actual_negative_controls'}
RECEIPT = {'schema', 'type', 'status', 'host', 'spec', 'restored_trees', 'clean',
           'builds', 'archive', 'files', 'certificate', 'provenance', 'control',
           'dispatch_delta', 'monitors'}
POSITIONS = {
    'TU13': ((0x370, 8, 0x390, 8), (0x390, 8, 0x370, 8)),
    'TU14': ((0x240, 12, 0x230, 8), (0x230, 8, 0x240, 12)),
}


def need(ok, reason):
    if not ok:
        raise ValueError('dispatch import: ' + reason)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':')).encode()


def exact(value, keys, reason):
    need(isinstance(value, dict) and set(value) == set(keys), reason)


def json_bytes(data):
    def pairs(rows):
        result = {}
        for key, value in rows:
            need(key not in result, 'duplicate JSON key')
            result[key] = value
        return result
    return json.loads(data, object_pairs_hook=pairs,
                      parse_constant=lambda x: need(False, 'nonfinite JSON'))


def relative(name):
    need(isinstance(name, str) and name and '\\' not in name, 'invalid artifact name')
    path = PurePosixPath(name)
    need(not path.is_absolute() and '..' not in path.parts and str(path) == name,
         'noncanonical artifact path')
    return path


def artifact(root, record):
    exact(record, {'path', 'sha256'}, 'artifact record fields')
    relative(record['path'])
    need(re.fullmatch('[0-9a-f]{64}', record['sha256'] or '') is not None,
         'artifact hash format')
    root = Path(root).resolve(strict=True)
    path = root / record['path']
    # Reject every symlink component, including an in-root alias.
    cursor = root
    for part in PurePosixPath(record['path']).parts:
        cursor = cursor / part
        need(not cursor.is_symlink(), 'symlink artifact component')
    need(path.resolve(strict=True).is_relative_to(root) and path.is_file(),
         'missing or escaped artifact')
    need(path.stat().st_size <= 1024**3, 'artifact too large')
    data = path.read_bytes()
    need(sha(data) == record['sha256'], 'artifact hash mismatch')
    return data


def metadata(root, record):
    return json_bytes(artifact(root, record))


def unpack(root, archive, members, destination):
    exact(archive, {'path', 'sha256', 'encoded_sha256'}, 'archive record fields')
    need(isinstance(members, dict) and members, 'empty member manifest')
    for name, digest in members.items():
        relative(name)
        need(re.fullmatch('[0-9a-f]{64}', digest or '') is not None, 'member hash format')
    encoded = artifact(root, {'path': archive['path'], 'sha256': archive['encoded_sha256']})
    raw = base64.b64decode(encoded, validate=True)
    need(sha(raw) == archive['sha256'], 'decoded archive hash')
    seen, total = set(), 0
    with tarfile.open(fileobj=io.BytesIO(raw), mode='r:gz') as stream:
        for member in stream:
            relative(member.name)
            need(member.type in (tarfile.REGTYPE, tarfile.AREGTYPE) and not member.pax_headers and member.name in members
                 and member.name not in seen, 'unsafe/duplicate/unexpected archive member')
            need(0 < member.size <= 64*1024**2, 'member size')
            total += member.size
            need(total <= 2*1024**3, 'expanded archive limit')
            content = stream.extractfile(member).read(member.size + 1)
            need(len(content) == member.size and sha(content) == members[member.name],
                 'member size/hash mismatch')
            target = Path(destination) / member.name
            target.parent.mkdir(parents=True, exist_ok=True)
            need(not target.exists() and not target.is_symlink() and
                 target.parent.resolve().is_relative_to(Path(destination).resolve()), 'archive overwrite/parent escape')
            target.write_bytes(content)
            seen.add(member.name)
    need(seen == set(members), 'missing archive members')


class GitSources:
    """Read immutable local Git objects, without checkout or uploaded programs."""
    def __init__(self, repo):
        self.repo = Path(repo).resolve(strict=True)

    def read(self, revision, path):
        need(re.fullmatch('[0-9a-f]{40}', revision or '') is not None, 'source revision')
        relative(path)
        return subprocess.check_output(['git', 'show', revision + ':' + path], cwd=self.repo)


def plans_check(plans, pins, spec, sources, tip):
    exact(plans, SCOPES, 'separate plan scopes required')
    exact(pins, SCOPES, 'independent plan pins required')
    need(len(spec['units']) == 2426, 'canonical full inventory')
    for scope in SCOPES:
        plan = plans[scope]
        dispatch.validate_plan(plan, pins[scope])
        need(plan['scope'] == scope and set(plan['units']) == set(spec['units']), 'plan scope/units')
        binding = plan['bindings']
        need(binding['base'] == spec['base'] and binding['tip'] == tip, 'plan revision')
        need(binding['source_trees'] == {p: spec[p] for p in ('before', 'after')}, 'plan trees')
        need(binding['toolchain'] == spec['toolchain'] and binding['generators'] == spec['generators'],
             'plan tools/generators')
        for phase, revision in [('before', spec['base']), ('after', tip)]:
            for category in ('source_files', 'config_files'):
                for path, digest in binding[category][phase].items():
                    need(sha(sources.read(revision, path)) == digest, 'local source/config hash')
        # Operator-pinned plan AND independently fixed exact ELF envelopes.
        for name, expected in zip(dispatch.SCOPES[scope]['symbols'], POSITIONS[scope]):
            for phase, value, size in [('before', *expected[:2]), ('after', *expected[2:])]:
                need(plan['symbols'][name][phase] == [18, 0, '.text#0', value, size],
                     'unqualified ELF envelope')


def symbol_maps(before, after, plans):
    names = {n: maps for plan in plans.values() for n, maps in plan['symbol_map'].items()}
    remainder = []
    configs = {}
    for phase, text in [('before', before), ('after', after)]:
        kept, found, config = [], set(), {}
        for line in text.splitlines(keepends=True):
            if not line.strip() or line.lstrip().startswith('#'):
                kept.append(line)
                continue
            match = re.match(r'(.+?) = ([^:;]+):0x([0-9a-fA-F]+);', line)
            need(match is not None, 'unparsed symbol map')
            name = match[1]
            config.setdefault(name, []).append((match[2], int(match[3], 16)))
            if name not in names:
                kept.append(line)
                continue
            need(name not in found, 'duplicate execute map')
            mapped = re.fullmatch(re.escape(name) + r' = \.text:0x([0-9a-fA-F]+); // type:function size:0x([0-9a-fA-F]+)\n?', line)
            need(mapped is not None and [int(v, 16) for v in mapped.groups()] == names[name][phase],
                 'execute mapped address/size')
            found.add(name)
        need(found == set(names), 'missing execute map')
        remainder.append(kept)
        configs[phase] = config
    need(remainder[0] == remainder[1], 'unapproved config record change')
    return configs


def census(summaries, plans, config, phase):
    endpoints, definitions = {}, {}
    names = {name for plan in plans.values() for name in plan['symbols']}
    expected = sorted(tuple(row) for plan in plans.values() for row in plan['references'])
    for unit, summary in summaries.items():
        for name, info, other, section, value, size in summary.get('named symbols', ()):
            need(not section.startswith('bad'), 'invalid symbol section identity')
            if section == 'UND':
                continue
            definitions.setdefault(name, []).append((unit, section, value))
            if name in names:
                need(name not in endpoints, 'duplicate execute definition')
                endpoints[name] = (unit, section, value, size)
    need(set(endpoints) == names, 'execute definitions incomplete')
    observed = []
    for unit, summary in summaries.items():
        for key, rows in summary.items():
            if not key.startswith('relocations for '):
                continue
            section = key[len('relocations for '):]
            for offset, kind, addend, target in rows:
                hits = set()
                if target[0] == 'sym':
                    name = target[1]
                    if name in names:
                        hits.add(name)
                    candidates = definitions.get(name, ())
                    for config_section, address in config.get(name, ()):
                        for plan in plans.values():
                            for execute, mapping in plan['symbol_map'].items():
                                start, width = mapping[phase]
                                if config_section == '.text' and start <= address+addend < start+width:
                                    hits.add(execute)
                elif target[0] == 'local':
                    candidates = [(unit, target[1], target[2])]
                else:
                    raise ValueError('unknown relocation target kind')
                for owner, target_section, value in candidates:
                    need(not target_section.startswith('bad'), 'invalid effective section identity')
                    if target_section == 'special0xfff1':
                        for plan in plans.values():
                            for name, mapping in plan['symbol_map'].items():
                                start, width = mapping[phase]
                                if start <= value+addend < start+width:
                                    hits.add(name)
                    for name, (own, sec, start, width) in endpoints.items():
                        if owner == own and target_section == sec and start <= value+addend < start+width:
                            hits.add(name)
                for name in hits:
                    need(target == ('sym', name) and addend == 0, 'alias/midbody execute reference')
                    observed.append((unit, section, offset, kind, addend, *target))
    need(sorted(observed) == expected, 'full effective reference closure')
    return sorted(observed)


def compare(before, after, plans, configs):
    need(set(before) == set(after) == set(plans['TU13']['units']) == set(plans['TU14']['units'])
         and len(before) == 2426, 'full object inventory')
    for phase, summaries in [('before', before), ('after', after)]:
        census(summaries, plans, configs[phase], phase)
    scopes = {dispatch.SCOPES[s]['object']: s for s in SCOPES}
    for unit in before:
        a, b = copy.deepcopy(before[unit]), copy.deepcopy(after[unit])
        if unit in scopes:
            plan = plans[scopes[unit]]
            for phase, summary in [('before', a), ('after', b)]:
                for name, record in plan['symbols'].items():
                    need([list(row[1:]) for row in summary['named symbols'] if row[0] == name]
                         == [record[phase]], 'actual execute outside pinned envelope')
            b['named symbols'] = tuple(sorted(
                (r[0], *plan['symbols'][r[0]]['before']) if r[0] in plan['symbols'] else r
                for r in b['named symbols']))
        need(a == b, 'unapproved object remainder: ' + unit)
    return {'strict_objects': 2424, 'scopes': list(SCOPES)}


def control_paths(scope):
    prefix = 'control' if scope == 'TU13' else 'control-tu14'
    return {prefix + '/' + name + '.o' for name in ('original', 'perturbed', 'restored')}


def certificate_check(cert, receipt, pins, spec_sha256):
    expected = {'type': TYPE, 'canonical_spec_sha256': spec_sha256,
                'base': receipt['spec']['base'], 'after_trees': receipt['spec']['after'],
                'TU13_plan_sha256': pins['TU13'], 'TU14_plan_sha256': pins['TU14'],
                'secondary_control_sha256': receipt['dispatch_delta']['secondary_control']['sha256'],
                'fullcomparison_sha256': receipt['dispatch_delta']['strict_remainder']['sha256']}
    need(cert == expected, 'wrong tagged certificate')


def expected_negatives(path, scope):
    """Reconstruct exact isolated ELF mutations without producer code."""
    raw = path.read_bytes()
    table, step, count = struct.unpack_from('>I', raw, 32)[0], *struct.unpack_from('>HH', raw, 46)
    headers = [struct.unpack_from('>10I', raw, table+i*step) for i in range(count)]
    sections, symbols = objsame.read_object(path)
    keys = objsame.section_keys(sections)
    symtab = next(h for h in headers if h[1] == 2)
    names = dispatch.SCOPES[scope]['symbols']
    indices = [i for i, s in enumerate(symbols) if s['name'] in names]
    need(len(indices) == 2, 'negative execute identities')
    output = {}
    def change(label, at, fmt, value):
        data = bytearray(raw)
        struct.pack_into(fmt, data, at, value)
        output[label] = bytes(data)
    text = keys.index('.text#0')
    at = headers[text][4]
    change('first-text', at, '>B', raw[at] ^ 1)
    at = headers[keys.index('.data#0')][4]
    change('outside-data', at, '>B', raw[at] ^ 1)
    for i in indices:
        for label, offset in [('value', 4), ('size', 8)]:
            at = symtab[4]+16*i+offset
            change('execute-'+str(i)+'-'+label, at, '>I', struct.unpack_from('>I', raw, at)[0]+4)
    i = next(i for i, s in enumerate(symbols) if s['name'] and s['name'] not in names
             and s['info'] & 15 == 2 and s['shndx'] == text)
    change('outside-symbol-size', symtab[4]+i*16+8, '>I', symbols[i]['size']+4)
    for h in headers:
        if h[1] != 4 or keys[h[7]] != '.data#0':
            continue
        for at in range(h[4], h[4]+h[5], 12):
            if struct.unpack_from('>I', raw, at)[0] not in ({0x48, 0x68} if scope == 'TU13' else {0x58, 0x68}):
                continue
            change('own-slot-addend-'+str(at), at+8, '>i', 4)
            info = struct.unpack_from('>I', raw, at+4)[0]
            need(info >> 8 in indices, 'negative owning slot target')
            other = next(i for i in indices if i != info >> 8)
            change('own-slot-crossed-'+str(at), at+4, '>I', other << 8 | info & 255)
    at = next(h[4] for h in headers if h[1] == 4 and h[5] >= 12 and keys[h[7]] == '.text#0')
    info = struct.unpack_from('>I', raw, at+4)[0]
    change('extra-execute-reference', at+4, '>I', indices[0] << 8 | info & 255)
    local = next(i for i, s in enumerate(symbols) if s['shndx'] == text and
                 (s['info'] & 15 == 3 or not s['name'] or objsame.LOCAL_LABEL.match(s['name'])))
    data = bytearray(raw)
    struct.pack_into('>I', data, at+4, local << 8 | info & 255)
    struct.pack_into('>i', data, at+8, symbols[indices[0]]['value']+4-symbols[local]['value'])
    output['extra-local-midbody-alias'] = bytes(data)
    need(len(output) == 13, 'negative artifact count')
    return output


def commands_check(text, expected_tokens):
    """Expectations must come from operator-qualified official graph dry-runs."""
    rows = [shlex.split(line) for line in text.splitlines() if line.strip()]
    need(rows == expected_tokens and rows, 'independent compile command mismatch')


def dependency_check(text, hashes, phase_root, revision, sources, toolchain):
    names = {line.strip() for line in text.splitlines() if line.startswith('    ')}
    need(names and names == set(hashes), 'dependency census/hash inventory')
    for name, digest in hashes.items():
        from dispatch_commands import dependency_name
        name = dependency_name(name, phase_root)
        relative(name)
        if name in toolchain:
            wanted = toolchain[name]
        else:
            wanted = sha(sources.read(revision, name))
        need(digest == wanted, 'dependency source/tool hash mismatch')


def final_hook(record, pinned_record_sha256, current_spec, current_tools, current_tip):
    """Pure final-hook contract check. Does not invoke land.sh or write a cert."""
    exact(record, {'type', 'status', 'receipt_sha256', 'spec', 'spec_sha256', 'plans',
                   'contract_sha256', 'certificate_sha256', 'verifier_tools', 'candidate_tip',
                   'operator_context_sha256'}, 'hook record fields')
    need(sha(canonical(record)) == pinned_record_sha256, 'unreviewed import record')
    need(record['type'] == TYPE and record['status'] == 'VERIFIED_NOT_LANDED', 'wrong import mode/status')
    need(record['candidate_tip'] == current_tip and re.fullmatch('[0-9a-f]{40}', current_tip or '')
         and record['spec'] == current_spec and record['verifier_tools'] == current_tools,
         'source/tool drift after independent verification')
    exact(record['plans'], SCOPES, 'hook scope pins')
    need(all(re.fullmatch('[0-9a-f]{64}', v or '') for v in record['plans'].values()), 'hook plan hash format')
    need(all(re.fullmatch('[0-9a-f]{64}', record[k] or '') for k in
             ('receipt_sha256', 'spec_sha256', 'contract_sha256', 'certificate_sha256', 'operator_context_sha256')),
         'hook identity format')
    return {'status': 'FINAL_CONTRACT_MATCH_ONLY', 'landing_authorized': False}


def verifier_tools():
    root = Path(__file__).resolve().parent
    names = ('dispatch_import.py', 'dispatchproof.py', 'dispatchownership.py',
             'dispatch-tu13-ownership-v1.json', 'objsame.py', 'dispatch_commands.py', 'dispatch_monitors.py')
    return {n: sha((root/n).read_bytes()) for n in names}


def negative_checks(root, rows, before, after, extracted, plans, configs):
    need(isinstance(rows, list) and len(rows) == 26, 'all negative ELF artifacts required')
    expected = {}
    for scope in SCOPES:
        path = extracted/'after'/dispatch.SCOPES[scope]['object']
        for label, content in expected_negatives(path, scope).items():
            expected[(scope, label)] = content
    seen, paths = set(), set()
    for row in rows:
        exact(row, {'scope', 'label', 'path', 'sha256', 'rejected'}, 'negative fields')
        key = (row['scope'], row['label'])
        need(key in expected and key not in seen and row['path'] not in paths
             and row['rejected'] is True, 'negative scope/identity/duplicates')
        content = artifact(root, {'path': row['path'], 'sha256': row['sha256']})
        need(content == expected[key], 'negative mutation not independently reproduced')
        # Fresh private file; never load or execute producer helpers.
        target = extracted/('independent-negative-' + str(len(seen)) + '.o')
        target.write_bytes(content)
        changed = dict(after)
        changed[dispatch.SCOPES[row['scope']]['object']] = objsame.summarize(target)
        try:
            compare(before, changed, plans, configs)
        except ValueError:
            pass
        else:
            raise ValueError('dispatch import: real negative passed semantic gate')
        if row['label'] in ('extra-execute-reference', 'extra-local-midbody-alias') or row['label'].startswith('own-slot'):
            try:
                census(changed, plans, configs['after'], 'after')
            except ValueError:
                pass
            else:
                raise ValueError('dispatch import: real alias/slot negative passed census')
        seen.add(key)
        paths.add(row['path'])
    need(seen == set(expected), 'incomplete negative matrix')


def config_negatives(rows, texts, plans):
    need(isinstance(rows, list) and len(rows) == 8, 'all eight config controls required')
    expected = {(n, f) for p in plans.values() for n in p['symbol_map'] for f in (0, 1)}
    seen = set()
    for row in rows:
        exact(row, {'execute', 'field', 'rejected'}, 'config negative fields')
        key = (row['execute'], row['field'])
        need(key in expected and key not in seen and row['rejected'] is True, 'config negative identity')
        name, field = key
        mapping = next(p['symbol_map'][name] for p in plans.values() if name in p['symbol_map'])
        values = list(mapping['after'])
        values[field] += 4
        lines = texts['after'].splitlines(keepends=True)
        i = next(i for i, l in enumerate(lines) if l.startswith(name + ' = '))
        lines[i] = name+' = .text:0x%08X; // type:function size:0x%X\n' % tuple(values)
        try:
            symbol_maps(texts['before'], ''.join(lines), plans)
        except ValueError:
            pass
        else:
            raise ValueError('dispatch import: wrong config address/size accepted')
        seen.add(key)
    need(seen == expected, 'incomplete config matrix')


def verify(receipt_bytes, root, trust, sources, original_dol):
    """Offline verification only. No checkout, hook installation, builds or landing.

    trust is operator-owned and separately reviewed. Its command_tokens are exact
    independent official-graph dry-run tokens for both phases and source controls;
    receipt-derived expectations are forbidden. Missing qualification rejects.
    """
    exact(trust, {'spec_bytes', 'tip', 'host', 'plan_pins', 'contract_sha256',
                  'verifier_tools', 'command_tokens', 'phase_roots', 'terminal_publication_sha256'}, 'operator trust contract fields')
    need(trust['verifier_tools'] == verifier_tools(), 'independent verifier tool pin drift')
    exact(trust['command_tokens'], {'before', 'after', 'TU13', 'TU14'}, 'independent command qualification missing')
    need(all(trust['command_tokens'].values()), 'empty independent command expectation')
    exact(trust['phase_roots'], {'before', 'after'}, 'exact operator phase roots required')
    from dispatch_commands import dependency_name
    for phase, path in trust['phase_roots'].items():
        need(PurePosixPath(path).parts[-2:] == ('execution', phase), 'operator phase root shape')
        dependency_name('source.hpp', path)
    need(PurePosixPath(trust['phase_roots']['before']).parent ==
         PurePosixPath(trust['phase_roots']['after']).parent, 'incompatible operator phase roots')
    spec = json_bytes(trust['spec_bytes'])
    need(len(spec['units']) == 2426 and sha(original_dol) == dispatch.DOL_SHA,
         'canonical inventory/original DOL pin')
    r = json_bytes(receipt_bytes)
    exact(r, RECEIPT, 'receipt fields/mutually incompatible modes')
    need(type(r['schema']) is int and r['schema'] == 1 and r['type'] == TYPE and
         r['status'] == 'COMPLETE_AWAITING_INDEPENDENT_IMPORT', 'receipt mode/status')
    need(r['host'] == trust['host'] and r['spec'] == spec and r['restored_trees'] == spec['after'],
         'host/spec/restoration mismatch')
    need(r['clean'] == {'before': True, 'after': True} and
         r['builds'] == {'before': 0, 'after': 0, 'control': 0, 'restore': 0}, 'build/restore failed')
    exact(r['provenance'], PROVENANCE, 'six provenance records required')
    evidence = {k: artifact(root, v) for k, v in r['provenance'].items()}
    need(all(evidence.values()), 'empty provenance')
    delta = r['dispatch_delta']
    exact(delta, DELTA, 'dispatch evidence fields')
    need(delta['schema'] == 1, 'dispatch evidence version')
    exact(delta['plans'], SCOPES, 'missing plan scope')
    plans = {s: metadata(root, delta['plans'][s]) for s in SCOPES}
    plans_check(plans, trust['plan_pins'], spec, sources, trust['tip'])
    recipe = json_bytes(evidence['recipe'])
    exact(recipe, {'contract', 'actual_result', 'owned_process_cleanup', 'import_status',
                   'source_controls_append_bytes'}, 'recipe fields')
    need(recipe['import_status'] == 'PENDING_INDEPENDENT_L_DISPATCH_IMPORT', 'recipe import mode')
    contract = recipe['contract']
    need(sha(canonical(contract)) == trust['contract_sha256'], 'unreviewed producer contract')
    need(contract['plans'] == trust['plan_pins'] and contract['before'] == spec['base']
         and contract['after'] == trust['tip'] and contract['canonical_spec_sha256'] == sha(trust['spec_bytes'])
         and contract['certificate_type'] == TYPE, 'contract source/spec/scope pins')
    need(recipe['source_controls_append_bytes'] == contract['source_controls'], 'perturb contract drift')
    result = recipe['actual_result']
    exact(result, {'status', 'contract_sha256', 'phases', 'controls', 'ownership',
                   'actual_ELF_controls', 'actual_config_controls', 'seconds'}, 'actual result fields')
    need(result['status'] == 'FULL_PAIR_COMPONENTS_COMPLETE_IMPORT_PENDING' and
         result['contract_sha256'] == trust['contract_sha256'], 'incomplete actual result')
    exact(result['phases'], {'before', 'after'}, 'actual phase scope')
    cleanup = recipe['owned_process_cleanup']
    need(cleanup['all_owned_groups_absent'] is True and not cleanup['remaining_pgids']
         and not cleanup['failures'] and cleanup['execution_error'] is None, 'incomplete owned cleanup')
    need(contract.get('execution_enabled') is True, 'disabled execution contract')
    from dispatch_monitors import check as check_monitors
    exact(r['monitors'], {'execution', 'packaging'}, 'successful final monitors required')
    monitor_bytes = {}
    for stage, records in r['monitors'].items():
        exact(records, {'budget', 'cleanup', 'completion'}, 'final monitor artifact fields')
        monitor_bytes[stage] = {name: artifact(root, rec) for name, rec in records.items()}
    # The secondary archive record is still fully verified/extracted below.
    monitor_secondary = metadata(root, delta['secondary_control'])
    need(json_bytes(monitor_bytes['execution']['cleanup']) == cleanup,
         'recipe/final monitor cleanup disagreement')
    monitor_result = check_monitors(r['monitors'], monitor_bytes, contract, trust['contract_sha256'],
                   trust['phase_roots'], {
                       'execution': {'actual_result_sha256': sha(canonical(result))},
                       'packaging': {'primary_encoded_sha256': r['archive']['encoded_sha256'],
                                     'secondary_encoded_sha256': monitor_secondary['archive']['encoded_sha256']}})
    publication_bytes = artifact(root, {'path': 'terminal-publication.json',
                                         'sha256': trust['terminal_publication_sha256']})
    publication = json_bytes(publication_bytes)
    publication_files = {name: artifact(root, {'path': name, 'sha256': rec['sha256']})
                         for name, rec in publication['files'].items()}
    from dispatch_monitors import terminal_check
    terminal_check(publication_bytes, trust['terminal_publication_sha256'], receipt_bytes,
                   r, contract, monitor_result['root'], monitor_result['initial_allocated_bytes'],
                   publication_files, monitor_secondary['archive'])
    cert = metadata(root, r['certificate'])
    certificate_check(cert, r, trust['plan_pins'], sha(trust['spec_bytes']))
    exact(r['control'], {'source', 'object', 'source_sha256', 'restored_source_sha256'}, 'primary control fields')
    primary = {phase+'/'+unit for phase in ('before', 'after') for unit in spec['units']}
    primary |= control_paths('TU13')
    need(set(r['files']) == primary and len(primary) == 4855, 'primary exact4855 members')
    secondary = metadata(root, delta['secondary_control'])
    exact(secondary, {'source', 'append', 'source_sha256', 'perturbed_source_sha256', 'unit',
                      'restored_source_sha256', 'ELFs', 'compiler_command_sha256', 'dependencies_sha256',
                      'dependency_header_hashes_sha256', 'compile_exit_codes', 'semantic_perturb_rejected',
                      'restored_raw_identical', 'archive', 'manifest', 'context', 'logs'}, 'secondary fields')
    secondary_members = metadata(root, secondary['manifest'])
    need(set(secondary_members) == control_paths('TU14'), 'secondary exact three controls')
    need(secondary['context'] == contract['context'], 'secondary tool provenance drift')
    units = json_bytes(evidence['unit_manifest'])
    exact(units, {'spec_units', 'phases', 'qualified_object_pins', 'actual_evidence',
                  'primary_control_evidence'}, 'unit manifest fields')
    exact(units['actual_evidence'], {'before', 'after'}, 'phase provenance scope')
    need(units['spec_units'] == spec['units'] and units['phases'] == result['phases']
         and units['qualified_object_pins'] == contract['qualified_objects'], 'unit provenance')
    spec_compare = json_bytes(evidence['spec_comparison'])
    exact(spec_compare, {'original_spec_sha256', 'independent_spec_sha256', 'parsed_equal',
                         'actual_phase_specs'}, 'independent spec evidence fields')
    need(spec_compare['original_spec_sha256'] == sha(trust['spec_bytes'])
         and spec_compare['independent_spec_sha256'] == sha(trust['spec_bytes'])
         and spec_compare['parsed_equal'] is True, 'independent spec comparison')
    for phase in ('before', 'after'):
        expected = copy.deepcopy(spec)
        if phase == 'before':
            expected['after'] = expected['before']
        need(spec_compare['actual_phase_specs'][phase] == expected, 'actual phase spec')
    texts = {phase: sources.read(rev, 'config/SB4E01/symbols.txt').decode()
             for phase, rev in [('before', spec['base']), ('after', trust['tip'])]}
    configs = symbol_maps(texts['before'], texts['after'], plans)
    with tempfile.TemporaryDirectory(prefix='dispatch-offline-import-') as temporary:
        extracted = Path(temporary)
        unpack(root, r['archive'], r['files'], extracted)
        unpack(root, secondary['archive'], secondary_members, extracted)
        trees = {phase: {u: objsame.summarize(extracted/phase/u) for u in spec['units']}
                 for phase in ('before', 'after')}
        comparison = compare(trees['before'], trees['after'], plans, configs)
        effective = {phase: census(trees[phase], plans, configs[phase], phase) for phase in ('before', 'after')}
        need(metadata(root, delta['effective_reference_closure']) == json_bytes(canonical(effective)),
             'reported reference closure differs from independent census')
        reported_comparison = metadata(root, delta['strict_remainder'])
        exact(reported_comparison, {'status', 'strict_objects', 'scoped_objects',
                                    'effective_references', 'production_acceptance'}, 'full comparison fields')
        need(reported_comparison == {'status': 'FULL_SEMANTIC_COMPONENTS_ONLY_NOT_IMPORT_ACCEPTANCE',
             'strict_objects': 2424, 'scoped_objects': sorted(dispatch.SCOPES[s]['object'] for s in SCOPES),
             'effective_references': json_bytes(canonical(effective)), 'production_acceptance': False},
             'reported comparison differs')
        owners = {}
        for scope in SCOPES:
            unit = dispatch.SCOPES[scope]['object']
            old = ownership.decode_object(extracted/'before'/unit, scope)
            need(len(old['mismatches']) == 2, 'known-bad baseline ownership')
            owners[scope] = {'before': old, 'after': ownership.require_candidate(extracted/'after'/unit, scope),
                             'original': ownership.decode_original_dol(original_dol, scope)}
        need(metadata(root, delta['original_ownership']) == json_bytes(canonical(owners))
             and result['ownership'] == json_bytes(canonical(owners)), 'original/object ownership provenance')
        need(metadata(root, delta['symbol_map']) == {s: plans[s]['symbol_map'] for s in SCOPES}, 'symbol-map evidence')
        negatives = metadata(root, delta['actual_negative_controls'])
        exact(negatives, {'ELF', 'config'}, 'negative matrix fields')
        need(len(result['actual_ELF_controls']) == 26, 'actual negative provenance count')
        for packaged, actual in zip(negatives['ELF'], result['actual_ELF_controls']):
            need({k:v for k,v in packaged.items() if k != 'path'} ==
                 {k:v for k,v in actual.items() if k != 'path'} and
                 packaged['path'] == 'dispatch/' + actual['path'], 'packaged negative provenance join')
        negative_checks(root, negatives['ELF'], trees['before'], trees['after'], extracted, plans, configs)
        config_negatives(negatives['config'], texts, plans)
        need(negatives['config'] == result['actual_config_controls'], 'actual config control provenance')
        exact(result['controls'], SCOPES, 'both source controls required')
        for phase, rev in [('before', spec['base']), ('after', trust['tip'])]:
            exact(result['phases'][phase], {'exit_code', 'clean', 'objects', 'source', 'graph_sha256',
                  'manifest_sha256', 'dependency_hashes_sha256'}, 'actual phase fields')
            scoped = {dispatch.SCOPES[s]['object'] for s in SCOPES}
            scoped.add('src/Game/System/GrandGalaxyMapSequence.o')
            qualified = contract['qualified_objects'][phase]
            need(set(qualified) == {'build/SB4E01/' + u for u in scoped}, 'qualified raw object inventory')
            for rel, digest in qualified.items():
                need(sha((extracted/phase/rel.removeprefix('build/SB4E01/')).read_bytes()) == digest,
                     'qualified scoped/Sequence raw object drift')
            records = units['actual_evidence'][phase]
            exact(records, {'build.ninja', 'source-graph.json', 'commands.txt', 'deps.txt',
                            'dependency-hashes.json', 'objects.json'}, 'phase evidence fields')
            actual = {n: artifact(root, rec) for n, rec in records.items()}
            graph = sha(actual['build.ninja'])
            need(all(graph == plans[s]['bindings']['graphs'][phase] for s in SCOPES)
                 and result['phases'][phase]['graph_sha256'] == graph, 'actual graph hash')
            need(result['phases'][phase]['dependency_hashes_sha256'] == sha(actual['dependency-hashes.json']),
                 'reported dependency manifest hash')
            commands_check(actual['commands.txt'].decode(), trust['command_tokens'][phase])
            dependency_check(actual['deps.txt'].decode(), json_bytes(actual['dependency-hashes.json']),
                             trust['phase_roots'][phase], rev, sources, spec['toolchain'])
            hashes = {u: sha((extracted/phase/u).read_bytes()) for u in spec['units']}
            need(json_bytes(actual['objects.json']) == hashes and
                 result['phases'][phase]['manifest_sha256'] == sha(actual['objects.json']), 'full object manifest')
            need(result['phases'][phase]['exit_code'] == 0 and result['phases'][phase]['clean'] is True
                 and result['phases'][phase]['objects'] == 2426 and result['phases'][phase]['source'] == rev,
                 'actual clean phase identity')
        for scope in SCOPES:
            control = result['controls'][scope]
            exact(control, {'source', 'append', 'source_sha256', 'perturbed_source_sha256', 'unit',
                            'restored_source_sha256', 'ELFs', 'compiler_command_sha256', 'dependencies_sha256',
                            'dependency_header_hashes_sha256', 'compile_exit_codes', 'semantic_perturb_rejected',
                            'restored_raw_identical'}, 'source control fields')
            ctl_contract = contract['source_controls'][scope]
            need(all(control[k] == v for k, v in ctl_contract.items()), 'source perturb recipe mismatch')
            unit, source = dispatch.SCOPES[scope]['object'], dispatch.SCOPES[scope]['source']
            original_source = sources.read(trust['tip'], source)
            need(control['unit'] == unit and control['source'] == source and
                 sha(original_source) == control['source_sha256'] == control['restored_source_sha256']
                 and sha(original_source+control['append'].encode()) == control['perturbed_source_sha256'],
                 'source perturb/restore provenance')
            prefix = 'control' if scope == 'TU13' else 'control-tu14'
            blobs = {n: (extracted/prefix/(n+'.o')).read_bytes() for n in ('original', 'perturbed', 'restored')}
            need(control['ELFs'] == {n: sha(data) for n, data in blobs.items()} and
                 blobs['original'] == blobs['restored'] == (extracted/'after'/unit).read_bytes(), 'source control baseline/restore')
            need(objsame.compare(extracted/prefix/'original.o', extracted/prefix/'perturbed.o'),
                 'actual source control failed to change semantics')
            changed = dict(trees['after'])
            changed[unit] = objsame.summarize(extracted/prefix/'perturbed.o')
            try:
                compare(trees['before'], changed, plans, configs)
            except ValueError:
                pass
            else:
                raise ValueError('dispatch import: actual source perturb accepted')
            need(control['compile_exit_codes'] == [0, 0] and control['semantic_perturb_rejected'] is True
                 and control['restored_raw_identical'] is True, 'source control failed')
            if scope == 'TU13':
                need(r['control'] == {'source': source, 'object': unit, 'source_sha256': control['source_sha256'],
                                     'restored_source_sha256': control['restored_source_sha256']}, 'primary control join')
                records = units['primary_control_evidence']
                exact(records, {'control-command.txt', 'control-deps.txt', 'control-dependency-hashes.json'}, 'primary control provenance')
                command = artifact(root, records['control-command.txt'])
                deps = artifact(root, records['control-deps.txt'])
                headers = artifact(root, records['control-dependency-hashes.json'])
            else:
                need(all(secondary[k] == v for k, v in control.items()), 'secondary control result join')
                exact(secondary['logs'], {'perturb', 'restore', 'command', 'deps', 'headers'}, 'secondary provenance')
                need(artifact(root, secondary['logs']['perturb']) and artifact(root, secondary['logs']['restore']), 'empty secondary source logs')
                command, deps, headers = [artifact(root, secondary['logs'][n]) for n in ('command', 'deps', 'headers')]
            need((sha(command), sha(deps), sha(headers)) == (control['compiler_command_sha256'],
                 control['dependencies_sha256'], control['dependency_header_hashes_sha256']), 'control command/header provenance')
            commands_check(command.decode(), trust['command_tokens'][scope])
            dependency_check(deps.decode(), json_bytes(headers), trust['phase_roots']['after'], trust['tip'], sources, spec['toolchain'])
    need(trust['verifier_tools'] == verifier_tools(), 'verifier drift during import')
    return {'type': TYPE, 'status': 'VERIFIED_NOT_LANDED', 'receipt_sha256': sha(receipt_bytes),
            'spec': spec, 'spec_sha256': sha(trust['spec_bytes']), 'plans': trust['plan_pins'],
            'contract_sha256': trust['contract_sha256'], 'certificate_sha256': r['certificate']['sha256'],
            'verifier_tools': trust['verifier_tools'], 'candidate_tip': trust['tip'],
            'operator_context_sha256': sha(canonical({'command_tokens': trust['command_tokens'],
                                                     'phase_roots': trust['phase_roots'],
                                                     'terminal_publication_sha256': trust['terminal_publication_sha256']}))}
