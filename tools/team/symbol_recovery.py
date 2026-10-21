"""Read-only qualification and transaction planning for exact frozen symbols.

This module does not configure, compile, checkout, rename, delete or acquire locks.
Its output is a reviewed plan; allocation and an execution controller are separate.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shlex
import tarfile
import zipfile

from retention_paths import canonical_path


def digest(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for block in iter(lambda: f.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def archive_members(path):
    """Read regular archive contents without extracting or following links."""
    if str(path).endswith('.zip'):
        with zipfile.ZipFile(path) as z:
            for m in z.infolist():
                if not m.is_dir():
                    yield m.filename, z.read(m)
    else:
        with tarfile.open(path) as t:
            for m in t:
                if m.isfile():
                    yield m.name, t.extractfile(m).read()


def patched_sdl(data, relative, patches):
    """Reproduce only the two pinned literal replacements, entirely in memory."""
    if relative == 'src/joystick/SDL_gamepad.c':
        script = patches['apply-sdl3-android-nintendo-auto-mapping.cmake']
        blocks = dict(re.findall(r'set\((_(?:old|new)_block) \[=\[\n(.*?)\]=\]\)', script, re.S))
        if set(blocks) != {'_old_block', '_new_block'}:
            raise ValueError('unrecognized Nintendo patch syntax')
        old, new = (blocks[n].encode() for n in ('_old_block', '_new_block'))
    elif relative == 'android-project/app/src/main/java/org/libsdl/app/SDLActivity.java':
        script = patches['apply-sdl3-android-security-exception.cmake']
        blocks = dict(re.findall(r'set\((_(?:old|new)) "([^"]+)"\)', script))
        if set(blocks) != {'_old', '_new'}:
            raise ValueError('unrecognized SecurityException patch syntax')
        old, new = blocks['_old'].encode(), blocks['_new'].encode()
    else:
        return data
    if old not in data:
        raise ValueError('approved patch input missing')
    return data.replace(old, new)


def verify_dependency(record, patches=None):
    archive, source = Path(record['archive']), Path(record['path'])
    if digest(archive) != record['archive_sha256']:
        raise ValueError('dependency archive hash mismatch')
    members = list(archive_members(archive))
    prefix = Path(members[0][0]).parts[0]
    strip = not (source / prefix).exists() and all(Path(n).parts[0] == prefix for n, _ in members)
    hashes, mismatches = {}, []
    for name, data in members:
        rel = Path(*Path(name).parts[1:]) if strip else Path(name)
        if rel.is_absolute() or '..' in rel.parts:
            raise ValueError('unsafe archive member')
        path = source / rel
        expected = patched_sdl(data, rel.as_posix(), patches) if patches else data
        expected_sha = hashlib.sha256(expected).hexdigest()
        actual_sha = digest(path) if path.is_file() else None
        hashes[rel.as_posix()] = {'expected': expected_sha, 'actual': actual_sha}
        if actual_sha != expected_sha:
            mismatches.append(rel.as_posix())
    if len(members) != record['archive_files_verified']:
        raise ValueError('historical member-count mismatch')
    return {'archive_sha256': digest(archive), 'members': len(members),
            'mismatches': mismatches, 'member_hashes': hashes,
            'status': 'MATCH' if not mismatches else 'REJECT'}


def configure_argv(script, home):
    """Extract the first configure invocation as data, never execute shell text."""
    logical = script.replace('\\\n', ' ')
    for line in logical.splitlines():
        if line.startswith('cmake --preset '):
            result = []
            for token in shlex.split(line):
                if token in ('>', '2>', '2>&1'):
                    break
                if '$(' in token or '`' in token:
                    raise ValueError('shell substitution in configure recipe')
                token = token.replace('$HOME', str(home)).replace('${HOME}', str(home))
                if '$' in token:
                    raise ValueError('unresolved variable in configure argv')
                result.append(token)
            return result
    raise ValueError('missing historical configure command')


def target_contract(root, preserved, script, home):
    root, preserved = canonical_path(root), canonical_path(preserved)
    cache = (preserved / 'CMakeCache.txt').read_text()
    headless = 'PETARI_BUILD_HEADLESS:BOOL=ON' in cache
    canonical = root / 'build' / ('macos-arm64-headless' if headless else 'macos-arm64-release')
    rel = 'native/app/headless/petari_headless' if headless else 'native/app/Petari.app/Contents/MacOS/Petari'
    link = preserved / ('native/app/headless/CMakeFiles/petari_headless.dir/link.txt' if headless else 'native/app/CMakeFiles/petari.dir/link.txt')
    commands = json.loads((preserved / 'compile_commands.json').read_text())
    if any(not canonical_path(c['directory']).is_relative_to(canonical) for c in commands):
        raise ValueError('historical build path mismatch')
    return {'source_root': str(root), 'canonical_build': str(canonical),
            'preserved_build': str(preserved), 'image_relative': rel,
            'configure_argv': configure_argv(Path(script).read_text(), home),
            'build_argv': ['cmake', '--build', str(canonical), '--target',
                           'petari_headless' if headless else 'petari', '-j6'],
            'compile_commands_sha256': digest(preserved / 'compile_commands.json'),
            'compile_argv': [shlex.split(c['command']) for c in commands],
            'link_argv': shlex.split(link.read_text()), 'link_file_sha256': digest(link),
            'cache_sha256': digest(preserved / 'CMakeCache.txt')}


def recovery_plan(contract, source_commit, expected, attempt, available_bytes,
                  source_clean, runtime_released=False, allocated=False):
    """Prepare preservation/restore actions; never perform them."""
    if not source_clean:
        raise ValueError('source checkout is not clean')
    root = canonical_path(contract['source_root'])
    canonical = canonical_path(contract['canonical_build'])
    attempt = canonical_path(attempt)
    if canonical not in {root / 'build/macos-arm64-release', root / 'build/macos-arm64-headless'}:
        raise ValueError('noncanonical build path')
    if attempt == root or attempt.is_relative_to(canonical) or canonical.is_relative_to(attempt):
        raise ValueError('attempt overlaps protected build')
    if attempt.exists():
        raise ValueError('attempt output must be fresh')
    if not re.fullmatch('[0-9a-f]{40}', source_commit):
        raise ValueError('full source commit required')
    if not re.fullmatch('[0-9a-f]{64}', expected['sha256']):
        raise ValueError('full frozen hash required')
    if not re.fullmatch('[0-9a-fA-F-]{36}', expected['uuid']):
        raise ValueError('frozen UUID required')
    if available_bytes < 65 * 1024**3:
        raise ValueError('insufficient preservation reserve')
    protected = canonical.with_name(canonical.name + '.recovery-protected-' + attempt.name)
    if protected.exists():
        raise ValueError('preservation path already exists')
    return {'schema': 1, 'status': 'PLAN_ONLY',
            'execution_allowed': False,
            'allocation_flags_recorded': bool(runtime_released and allocated),
            'source_commit': source_commit, 'source_root': str(root),
            'attempt': str(attempt), 'expected_image': expected,
            'contract': contract,
            'journal_required_before_mutation': ['original_head', 'original_branch',
                'original_status', 'all_protected_image_hashes_and_UUIDs',
                'recovered_object_manifest', 'preservation_rename_map', 'owner_pid'],
            'preserve': {'from': str(canonical), 'to': str(protected)},
            'restore_in_order': [
                {'action': 'verify_ownership_and_protected_hashes'},
                {'action': 'preserve_attempt_build', 'from': str(canonical),
                 'to': str(attempt / 'attempt-build')},
                {'action': 'restore_original_checkout_from_journal'},
                {'action': 'restore_protected_build', 'from': str(protected), 'to': str(canonical)},
                {'action': 'reverify_original_status_and_all_protected_hashes_and_UUIDs'}],
            'acceptance': ['exact full image SHA256', 'exact LC_UUID',
                'exact missing-object SHA256s', 'matching dSYM UUID', 'line-resolution qualification'],
            'budgets': {'configure_seconds': 180, 'build_seconds': 1200,
                        'qualification_seconds': 180, 'additional_bytes': 15 * 1024**3,
                        'minimum_free_bytes': 50 * 1024**3},
            'executor_implemented': False,
            'blockers': ['D explicit release', 'PORT allocation',
                         'historical toolchain hash qualification',
                         'reviewed execution/restore journal', 'ordinary link reproducibility']}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('contract', type=Path)
    p.add_argument('request', type=Path)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    if a.output.exists():
        raise ValueError('refusing to overwrite report')
    plan = recovery_plan(json.loads(a.contract.read_text()), **json.loads(a.request.read_text()))
    a.output.write_text(json.dumps(plan, indent=2) + '\n')


def restore_preflight(journal, actual_head, source_clean):
    """Validate a future restore before emitting any mutation instructions."""
    if journal.get('schema') != 1 or journal.get('kind') != 'FROZEN_SYMBOL_RECOVERY':
        raise ValueError('not an owned recovery journal')
    if not source_clean or actual_head not in {journal['attempt_commit'], journal['original_head']}:
        raise ValueError('checkout changed outside the owned transaction')
    root = canonical_path(journal['source_root'])
    build = canonical_path(journal['canonical_build'])
    protected = canonical_path(journal['protected_build'])
    attempt = canonical_path(journal['attempt_output'])
    if build not in {root / 'build/macos-arm64-release', root / 'build/macos-arm64-headless'}:
        raise ValueError('invalid canonical path')
    if protected.parent != build.parent or not protected.name.startswith(build.name + '.recovery-protected-'):
        raise ValueError('invalid protected path')
    if attempt.is_relative_to(build) or attempt.is_relative_to(protected) or root.is_relative_to(attempt):
        raise ValueError('restore output overlaps protected closure')
    if not protected.is_dir() or (attempt / 'attempt-build').exists():
        raise ValueError('missing original or occupied restore destination')
    for rel, expected in journal['protected_files'].items():
        path = canonical_path(protected / rel)
        if not path.is_relative_to(protected) or digest(path) != expected:
            raise ValueError('protected closure changed or escaped root')
    if not journal['protected_files']:
        raise ValueError('empty protected hash manifest')
    return [
        {'program': 'git', 'argv': ['-C', str(root), 'checkout', journal['original_branch']]
         if journal['original_branch'] else ['-C', str(root), 'checkout', '--detach', journal['original_head']],
         'postcondition': 'HEAD equals original_head and status equals original clean status'},
        {'rename': [str(build), str(attempt / 'attempt-build')],
         'condition': 'only if canonical build exists; preserve failed attempt'},
        {'rename': [str(protected), str(build)], 'condition': 'canonical destination absent'},
        {'verify': 'all original protected hashes, UUIDs and checkout journal; keep all evidence'}]


if __name__ == '__main__':
    main()
