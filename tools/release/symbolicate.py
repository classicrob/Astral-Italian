#!/usr/bin/env python3
"""UUID-checked arm64 crash symbolication. JSON schema is shared with intake.py.

Addresses and offsets are hex strings; line is an integer or null, file/symbol
are strings (empty when unavailable). Inline entries repeat the physical index;
inlined=true marks inline functions; the final entry is the physical frame. JSON diagnostics go to stderr.
"""
import argparse
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import zipfile
from contextlib import ExitStack


def uuid_key(value):
    return value.replace('-', '').upper()


def command(args):
    try:
        result = subprocess.run(args, capture_output=True, text=True, timeout=30)
        return result.stdout if result.returncode == 0 else ''
    except (OSError, subprocess.TimeoutExpired):
        return ''


def frame(index, image, address, offset):
    return dict(index=index, image=image, address=hex(address), offset=hex(offset),
                symbol='', file='', line=None, inlined=False)


def parse_report(text):
    """Return public JSON plus private image and original-line information."""
    result = {'build': dict(app_version='', source_commit='', executable_uuid='', macos='', model=''),
              'crash': {'signal': '', 'fault_address': ''}, 'frames': []}
    images, locations = [], []
    lines = text.splitlines()
    if text.lstrip().startswith('{'):
        decoder, objects, rest = json.JSONDecoder(), [], text.lstrip()
        while rest:
            value, end = decoder.raw_decode(rest)
            objects.append(value)
            rest = rest[end:].lstrip()
        data = objects[-1]  # .ips usually has a metadata JSON line before the body.
        for entry in data.get('usedImages', []):
            images.append(dict(name=entry.get('name', ''), path=entry.get('path', ''),
                               uuid=entry.get('uuid', ''), base=entry.get('base', 0)))
        main = next((i for i in images if i['name'] == data.get('procName')), images[0] if images else None)
        if main:
            result['build']['executable_uuid'] = main['uuid']
        bundle = data.get('bundleInfo', {})
        result['build']['app_version'] = bundle.get('CFBundleShortVersionString', '')
        result['build']['macos'] = data.get('osVersion', {}).get('train', '')
        result['build']['model'] = data.get('modelCode', '')
        exception = data.get('exception', {})
        result['crash']['signal'] = exception.get('signal', exception.get('type', ''))
        fault = re.search(r'0x[0-9a-fA-F]+', exception.get('subtype', ''))
        result['crash']['fault_address'] = fault.group() if fault else ''
        threads = data.get('threads', [])
        triggered = next((t for t in threads if t.get('triggered')), None)
        if triggered is None and threads:
            triggered = threads[data.get('faultingThread', 0)]
        for n, item in enumerate((triggered or {}).get('frames', [])):
            image_index = item.get('imageIndex', -1)
            image = images[image_index] if 0 <= image_index < len(images) else dict(name='???', base=0)
            offset = item.get('imageOffset', 0)
            f = frame(n, image['name'], image['base'] + offset, offset)
            f['symbol'] = item.get('symbol', '')
            result['frames'].append(f)
        return result, images, locations, lines

    fields = {'app version': 'app_version', 'source commit': 'source_commit',
              'executable UUID': 'executable_uuid', 'macOS': 'macos', 'model': 'model'}
    in_images, old_thread = False, False
    for n, line in enumerate(lines):
        key, _, value = line.partition(': ')
        if key in fields:
            result['build'][fields[key]] = value.strip()
        if line.startswith('signal: '):
            result['crash']['signal'] = value.split(' ')[0]
            fault = re.search(r'fault address (0x[\da-fA-F]+)', line)
            if fault:
                result['crash']['fault_address'] = fault[1]
        if line.startswith('Exception Type:'):
            result['crash']['signal'] = line.split(':', 1)[1].strip()
        if line == 'Binary Images:':
            in_images = True
        if in_images:
            m = re.match(r'(0x[\da-fA-F]+)\s+([\da-fA-F-]{32,36})\s+(.+)', line)
            old = re.match(r'\s*(0x[\da-fA-F]+)\s+-\s+0x[\da-fA-F]+\s+\+?(\S+)\s+\S+\s+<([\da-fA-F-]+)>\s+(.+)', line)
            if m:
                images.append(dict(base=int(m[1], 16), uuid=m[2], path=m[3], name=Path(m[3]).name))
            elif old:
                images.append(dict(base=int(old[1], 16), uuid=old[3], path=old[4], name=old[2]))
        m = re.match(r'#(\d+) (0x[\da-fA-F]+)  (.+) \+0x([\da-fA-F]+)$', line)
        if m:
            result['frames'].append(frame(int(m[1]), m[3], int(m[2], 16), int(m[4], 16)))
            locations.append(n)
        if re.match(r'Thread \d+ Crashed:', line):
            old_thread = True
        elif old_thread:
            m = re.match(r'\s*(\d+)\s+(\S+)\s+(0x[\da-fA-F]+)\s+(0x[\da-fA-F]+)\s+\+\s+(\d+)', line)
            if m:
                result['frames'].append(frame(int(m[1]), m[2], int(m[3], 16), int(m[5])))
                locations.append(n)
            elif not line.strip():
                old_thread = False
    if not result['build']['executable_uuid'] and images:
        # Old .crash files put the process executable first in Binary Images.
        result['build']['executable_uuid'] = images[0]['uuid']
    if not images or not result['frames']:
        raise ValueError('No crash frames or Binary Images were found. Send a complete v2 report or macOS crash report.')
    return result, images, locations, lines


def dwarf_files(path):
    path = Path(path).expanduser()
    bundles = [path] if path.name.endswith('.dSYM') else sorted(path.rglob('*.dSYM')) if path.is_dir() else []
    for bundle in bundles:
        dwarf = bundle / 'Contents/Resources/DWARF'
        if dwarf.is_dir():
            yield from sorted(p for p in dwarf.iterdir() if p.is_file())


def file_uuids(path):
    return [uuid_key(m[1]) for m in re.finditer(r'UUID: ([\da-fA-F-]+) \(arm64\)', command(['dwarfdump', '--uuid', str(path)]))]


def find_dsym(wanted, paths, stack, diagnostics):
    seen = set()
    def search(path, explicit=False):
        path = Path(path).expanduser()
        if str(path) in seen:
            return None
        seen.add(str(path))
        if path.is_file() and path.name.endswith('.zip'):
            temporary = Path(stack.enter_context(tempfile.TemporaryDirectory(prefix='petari-dsym-')))
            with zipfile.ZipFile(path) as archive:
                # Release archives are data: never extract outside the temp dir.
                for entry in archive.infolist():
                    target = (temporary / entry.filename).resolve()
                    if not target.is_relative_to(temporary.resolve()):
                        raise ValueError('The dSYM zip contains an unsafe path.')
                archive.extractall(temporary)
            path = temporary
        for dwarf in dwarf_files(path):
            uuids = file_uuids(dwarf)
            if wanted in uuids:
                return dwarf
            if explicit:
                diagnostics.append(f'Refusing mismatched dSYM {dwarf}: UUID {", ".join(uuids) or "unavailable"}; report requires {wanted}.')
        return None
    for path in paths:
        found = search(path, True)
        if found:
            return found
    found = search(Path.home() / 'MacioGalaxy2-team/releases/dsym')
    if found:
        return found
    if wanted:
        pretty = '-'.join((wanted[:8], wanted[8:12], wanted[12:16], wanted[16:20], wanted[20:]))
        for path in command(['mdfind', f'com_apple_xcode_dsym_uuids == {pretty}']).splitlines():
            found = search(path)
            if found:
                return found
    return None


def resolve_frames(result, images, dwarf, app=None, return_addresses=True):
    wanted = uuid_key(result['build']['executable_uuid'])
    output, needed, resolved = [], 0, 0
    for original in result['frames']:
        matching = next((i for i in images if i['name'] == original['image'] and
                         i['base'] + int(original['offset'], 16) == int(original['address'], 16)), None)
        executable = bool(matching and wanted and uuid_key(matching['uuid']) == wanted)
        if executable:
            needed += 1
        binary = dwarf if executable else Path(matching['path']) if matching and matching['path'] else None
        # --app helps resolve embedded libraries, but never substitutes an
        # unchecked executable or a wrong UUID for missing debug symbols.
        if not executable and matching and app:
            candidates = Path(app).rglob(original['image'])
            binary = next((p for p in candidates if p.is_file() and uuid_key(matching['uuid']) in file_uuids(p)), binary)
        if not executable and binary and matching and uuid_key(matching['uuid']) not in file_uuids(binary):
            binary = None
        symbols = ''
        if binary and matching:
            # Saved LR/FP and macOS imageOffset values are return addresses.
            # Attribute callers to the preceding arm64 call instruction, especially
            # after noreturn calls whose return PC can have DWARF line zero.
            address = int(original['address'], 16)
            if return_addresses and original['index'] > 0 and address >= 4:
                address -= 4
            symbols = command(['atos', '-arch', 'arm64', '-i', '-d', '\t', '-o', str(binary),
                               '-l', hex(matching['base']), hex(address)]).strip()
        entries = []
        for symbol in symbols.replace('\n', '\t').split('\t'):
            if not symbol or re.fullmatch(r'0x[\da-fA-F]+', symbol):
                continue
            f = dict(original)
            f['inlined'] = False
            source = re.search(r'\(([^()]+?):(\d+)(?::\d+)?\)$', symbol)
            f['symbol'] = re.sub(r' \(in [^()]+\).*$', '', symbol)
            if source:
                f['file'], f['line'] = source[1], int(source[2]) or None
            entries.append(f)
        for inline in entries[:-1]:
            inline['inlined'] = True
        if executable and any(f['file'] and f['line'] for f in entries):
            resolved += 1
        output.extend(entries or [original])
    result['frames'] = output
    return needed > 0 and resolved == needed


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('report', type=Path)
    parser.add_argument('--dsym', action='append', default=[], metavar='PATH_OR_DIR')
    parser.add_argument('--app', type=Path)
    parser.add_argument('--json', action='store_true')
    args = parser.parse_args(argv)
    try:
        text = args.report.read_text()
        result, images, locations, lines = parse_report(text)
        wanted = uuid_key(result['build']['executable_uuid'])
        if not re.fullmatch(r'[0-9A-F]{32}', wanted):
            raise ValueError('The report has no valid executable UUID. Send a complete crash report.')
        with ExitStack() as stack:
            diagnostics = []
            dwarf = find_dsym(wanted, args.dsym, stack, diagnostics)
            if not dwarf:
                diagnostics.append(f'No matching arm64 dSYM for executable UUID {wanted}. Pass the dSYM saved for this release with --dsym.')
            ok = resolve_frames(result, images, dwarf, args.app)
        for diagnostic in diagnostics:
            print(diagnostic, file=sys.stderr)
        if args.json:
            print(json.dumps(result, indent=2))
        elif locations:
            for location, index in zip(locations, dict.fromkeys(f['index'] for f in result['frames'])):
                annotations = [f for f in result['frames'] if f['index'] == index]
                for f in annotations:
                    if f['symbol']:
                        source = f" ({f['file']}:{f['line']})" if f['file'] else ''
                        lines[location] += f"\n    {'[inlined] ' if f['inlined'] else ''}{f['symbol']}{source}"
            print('\n'.join(lines))
        else:
            print(text.rstrip())
            print('\nSymbolicated crashed thread:')
            for f in result['frames']:
                source = f" ({f['file']}:{f['line']})" if f['file'] else ''
                print(f"#{f['index']} {f['address']}  {f['image']} +{f['offset']}  "
                      f"{'[inlined] ' if f['inlined'] else ''}{f['symbol']}{source}")
        if not ok:
            print('Executable frames could not all be resolved to source lines. Check that the release dSYM contains debug information.', file=sys.stderr)
        return 0 if ok else 1
    except (OSError, ValueError, TypeError, AttributeError, KeyError, IndexError, zipfile.BadZipFile) as error:
        print(f'symbolicate: {error}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
