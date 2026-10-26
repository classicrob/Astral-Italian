#!/usr/bin/env python3
"""Safely collect evidence and replay an Astral Italian tester report. See INTAKE.md."""
import argparse
import contextlib
import csv
import hashlib
import importlib.util
import json
import math
import os
from pathlib import Path, PurePosixPath, PureWindowsPath
import re
import shlex
import shutil
import signal
import stat
import struct
import subprocess
import sys
import time
from types import SimpleNamespace
import zipfile

REPO = Path(__file__).resolve().parents[2]
TEAM = Path.home() / 'MacioGalaxy2-team'
CACHE = TEAM / 'intake'
MAX_BYTES = 4 * 1024**3
MAX_FILES = 10000
MIN_FREE = 50 * 1024**3
UUID = re.compile(r'^[0-9a-fA-F]{8}(?:-[0-9a-fA-F]{4}){3}-[0-9a-fA-F]{12}$')
COMMIT = re.compile(r'^[0-9a-fA-F]{40}$')
BUILD_KEYS = ('report_format', 'report_time', 'app_version', 'source_commit', 'executable_uuid',
              'app_path', 'macos', 'model', 'chip', 'memory_gb', 'newest_tape', 'tape_included', 'tape_skipped_reason')


class IntakeError(Exception):
    pass


def read_text(path, limit=256 * 1024):
    if not path.is_file():
        return ''
    with path.open('rb') as f:
        data = f.read(limit)
        truncated = bool(f.read(1))
    return data.decode('utf-8', errors='replace') + ('\n[truncated]\n' if truncated else '')


def read_json(path):
    if path.stat().st_size > 4 * 1024**2:
        raise ValueError(f'{path.name} is too large for metadata')
    value = json.loads(path.read_text())
    if not isinstance(value, dict):
        raise ValueError(f'{path.name} must contain a JSON object')
    return value


def safe_name(name):
    p = PurePosixPath(name)
    if not name or '\\' in name or p.is_absolute() or PureWindowsPath(name).drive or '..' in p.parts:
        raise IntakeError(f'Unsafe report path: {name!r}. Ask for a new report zip.')
    return p


def unpack_report(source, dest, max_bytes=MAX_BYTES, max_files=MAX_FILES):
    """Copy regular files only, preflight size/count and enforce actual streamed byte limits."""
    entries = []
    archive = None
    try:
        if source.is_symlink():
            raise IntakeError('The report must not be a symbolic link.')
        if source.is_dir():
            for parent, dirs, files in os.walk(source, followlinks=False):
                for name in dirs + files:
                    path = Path(parent) / name
                    mode = path.lstat().st_mode
                    if not (stat.S_ISREG(mode) or stat.S_ISDIR(mode)):
                        raise IntakeError(f'Report contains a link or special file: {path.name}')
                    rel = safe_name(path.relative_to(source).as_posix())
                    entries.append((rel, path, path.stat().st_size if path.is_file() else 0, path.is_dir()))
        else:
            archive = zipfile.ZipFile(source)
            for info in archive.infolist():
                rel = safe_name(info.filename)
                mode = info.external_attr >> 16
                kind = stat.S_IFMT(mode)
                if kind not in (0, stat.S_IFREG, stat.S_IFDIR):
                    raise IntakeError(f'Report zip contains a link or special file: {info.filename}')
                entries.append((rel, info, info.file_size, info.is_dir()))
        if len(entries) > max_files or sum(e[2] for e in entries) > max_bytes:
            raise IntakeError(f'Report is too large (limit {max_files} entries / {max_bytes} bytes).')
        seen = set()
        for rel, _, _, _ in entries:
            key = rel.as_posix().casefold()
            if key in seen:
                raise IntakeError(f'Report has duplicate paths: {rel}')
            seen.add(key)
        dest.mkdir()
        total = 0
        for rel, item, _, directory in entries:
            target = dest / rel
            if directory:
                target.mkdir(parents=True, exist_ok=True)
                continue
            target.parent.mkdir(parents=True, exist_ok=True)
            with (archive.open(item) if archive else item.open('rb')) as src, target.open('xb') as out:
                while True:
                    block = src.read(1024 * 1024)
                    if not block:
                        break
                    total += len(block)
                    if total > max_bytes:
                        raise IntakeError('Report expands beyond the size limit.')
                    out.write(block)
        roots = [dest] + [p for p in dest.iterdir() if p.is_dir() and p.name != '__MACOSX']
        markers = ('build.json', 'system.txt', 'note.txt', 'logs', 'captures', 'crashes', 'tapes', 'settings')
        candidates = [p for p in roots if any((p / marker).exists() for marker in markers)]
        if len(candidates) != 1:
            # A folder without identity or a tape is still useful for logs/crashes.
            if len(roots) == 2 and not list(dest.glob('*.txt')):
                return roots[1]
            if len(roots) == 1:
                return dest
            raise IntakeError('Cannot identify one report folder in the zip.')
        return candidates[0]
    finally:
        if archive:
            archive.close()


def load_build(root, warnings):
    try:
        build = read_json(root / 'build.json')
    except (OSError, ValueError) as e:
        warnings.append(f'Build identity unavailable: {e}')
        return {}
    if build.get('report_format') != 1:
        warnings.append(f'Expected report_format 1; received {build.get("report_format")!r}.')
    missing = [k for k in BUILD_KEYS if k not in build or build[k] in ('', None)]
    if missing:
        warnings.append('Missing or empty build values: ' + ', '.join(missing))
    return build


def verify_build(build, tape, registry, repo=REPO, override=None):
    problems = []
    ids, commits = set(), set()
    for origin, data in (('build.json', build), ('tape sidecar', tape)):
        for key, pattern, values in (('executable_uuid', UUID, ids), ('source_commit', COMMIT, commits)):
            value = data.get(key)
            if value in ('', None):
                continue
            if not isinstance(value, str) or not pattern.fullmatch(value):
                problems.append(f'{origin}: invalid {key}.')
            else:
                values.add(value.upper() if key == 'executable_uuid' else value.lower())
    if build.get('report_format') not in (1, '', None):
        problems.append('Unsupported build.json report format; expected format 1.')
    if len(ids) > 1 or len(commits) > 1:
        problems.append('The report and tape disagree about build identity.')
    with registry.open(newline='') as f:
        rows = list(csv.DictReader(f, delimiter='\t'))
    matched = [r for r in rows if r['executable_uuid'].upper() in ids]
    if ids and len({r['executable_uuid'].upper() for r in matched}) != len(ids):
        problems.append('Tester executable UUID is not in builds.tsv; this executable has not been released in this registry.')
    mapped = {r['source_commit'].lower() for r in matched}
    if len(mapped) > 1:
        problems.append('The registry maps this executable UUID to multiple source commits.')
    if mapped and commits and mapped != commits:
        problems.append('Executable UUID and source commit disagree with builds.tsv.')
    if not ids:
        matched = [r for r in rows if r['source_commit'].lower() in commits]
        mapped = {r['source_commit'].lower() for r in matched}
        if not matched:
            problems.append('No released executable or source commit could be identified in builds.tsv.')
    resolved = next(iter(mapped or commits), None) if len(mapped or commits) == 1 else None
    if not ids and not commits:
        problems.append('The report and tape contain no executable UUID or source commit.')
    if override:
        if not COMMIT.fullmatch(override):
            raise IntakeError('--commit must be a full 40-character source commit.')
        if resolved and resolved != override.lower():
            problems.append('The explicit --commit differs from the tester build source; replay is unverified.')
        resolved = override.lower()
    if resolved:
        check = subprocess.run(['git', 'cat-file', '-e', resolved + '^{commit}'], cwd=repo, capture_output=True)
        if check.returncode:
            problems.append(f'Source commit {resolved} is not available in this repository.')
            resolved = None
    return dict(known=not problems, commit=resolved, executable_uuids=sorted(ids),
                reported_commits=sorted(commits), released=bool(matched) if ids else None, source_registered=bool(matched), registry=matched,
                problems=problems, override=override)


def load_replay(path=REPO / 'tools/port/replay_tape.py'):
    spec = importlib.util.spec_from_file_location('intake_replay', path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def prepare_tape(root, dest, replay):
    sidecars = sorted((root / 'tapes').glob('*/*.tape.json'))
    if not sidecars:
        return None, {}, 'TAPE_MISSING', 'No session tape was included. Ask the tester to collect a report after playing.'
    if len(sidecars) != 1:
        return None, {}, 'TAPE_INVALID', 'Expected one session tape; found multiple sidecars.'
    meta = {}
    try:
        sidecar = sidecars[0]
        meta = read_json(sidecar)
        if meta.get('schema') != 1:
            raise ValueError('unsupported tape schema (expected 1)')
        if not isinstance(meta.get('complete'), bool):
            raise ValueError('missing complete flag')
        for key in ('seams', 'unencodable_samples', 'lossy_samples'):
            if type(meta.get(key)) is not int or meta[key] < 0:
                raise ValueError(f'invalid {key}')
        if meta['lossy_samples'] or meta['unencodable_samples']:
            raise ValueError('lossy or unencodable controller samples prevent exact replay')
        if not isinstance(meta.get('mods'), list) or any(not isinstance(m, str) or not re.fullmatch(r'[A-Za-z0-9_]+', m) for m in meta['mods']):
            raise ValueError('invalid recorded mods')
        replay_env = meta.get('replay_env') or {}
        if not isinstance(replay_env, dict) or any(k != 'PETARI_TEST_RETURN_TO_SHIP' or not isinstance(v, str) or not re.fullmatch(r'[0-9]+(?::[0-9]+)?', v) for k, v in replay_env.items()):
            raise ValueError('unsupported replay environment; review it before replaying')
        segments = meta.get('segments') or []
        if not segments and meta.get('play_ready_seams'):
            s = meta['play_ready_seams'][0]
            segments = [dict(start_row=s['row'], end_row=meta['seams'] - 1, stage=s['stage'], scenario=s['scenario'], rng_seed=s['rng_seed'])]
        previous_end = -1
        for s in segments:
            if any(type(s[k]) is not int for k in ('start_row', 'end_row', 'scenario', 'rng_seed')) or not 0 <= s['start_row'] <= s['end_row'] < meta['seams']:
                raise ValueError('invalid segment rows or seed')
            if s['start_row'] <= previous_end or s['scenario'] < 1 or not 0 <= s['rng_seed'] <= 0xffffffff:
                raise ValueError('overlapping segments, invalid scenario or RNG seed')
            previous_end = s['end_row']
            if not isinstance(s['stage'], str) or not re.fullmatch(r'[A-Za-z0-9_]+', s['stage']):
                raise ValueError('invalid segment stage')
        replay.plan_segments(segments, meta['seams'])
        dest.mkdir()
        base = sidecar.name[:-len('.tape.json')]
        # Ignore tester paths entirely: replay only the sibling files that were collected.
        for key, suffix in (('route_csv', '.csv'), ('kpd', '.kpd'), ('actions', '.actions')):
            source = sidecar.parent / (base + suffix)
            if not source.is_file():
                raise ValueError(f'missing tape file {source.name}')
            target = dest / source.name
            shutil.copyfile(source, target)
            meta[key] = str(target)
        required = {'stage', 'scenario', 'valid', 'x', 'y', 'z', 'grounded', 'dead', 'move_x', 'move_y', 'jump', 'spin', 'crouch', 'shoot'}
        with Path(meta['route_csv']).open(newline='') as f:
            reader = csv.DictReader(f)
            if not required <= set(reader.fieldnames or []):
                raise ValueError('route CSV is missing required columns: ' + ', '.join(sorted(required - set(reader.fieldnames or []))))
            count = 0
            for row in reader:
                count += 1
                if None in row or any(row[k] is None for k in required):
                    raise ValueError(f'malformed route row {count - 1}')
                if row['valid'] not in ('0', '1'):
                    raise ValueError(f'invalid player-valid flag at route row {count - 1}')
                if row['valid'] == '1' and any(not math.isfinite(float(row[k])) for k in ('x', 'y', 'z')):
                    raise ValueError(f'non-finite position at route row {count - 1}')
            if count < meta['seams']:
                raise ValueError('route CSV ends before the last recorded checkpoint')
        validate_kpd(Path(meta['kpd']), allow_trailing=not meta['complete'])
        replay.parse_actions(Path(meta['actions']).read_text())
        meta['segments'] = segments
        staged = dest / sidecar.name
        staged.write_text(json.dumps(meta, indent=2) + '\n')
        return staged, meta, None, None
    except (OSError, ValueError, KeyError, TypeError, IndexError, OverflowError, struct.error) as e:
        return None, meta, 'TAPE_INVALID', f'The session tape cannot be replayed: {e}'


def validate_kpd(path, allow_trailing=False):
    """Check lengths before replay's parser allocates its sample lists."""
    size = path.stat().st_size
    record = struct.Struct('>QIIIiiiII')
    with path.open('rb') as f:
        header = f.read(24)
        if len(header) != 24 or header[:8] != b'SMG2KPD1':
            raise ValueError('bad or truncated KPAD header')
        _, count = struct.unpack('>QQ', header[8:])
        if count > (size - 24) // record.size:
            raise ValueError('truncated KPAD records')
        for _ in range(count):
            fields = record.unpack(f.read(record.size))
            sample_bytes = fields[-1] * 240
            if sample_bytes > size - f.tell():
                raise ValueError('truncated KPAD samples')
            f.seek(sample_bytes, 1)
        if not allow_trailing and f.tell() != size:
            raise ValueError('trailing KPAD tape data')


def require_space(path):
    if shutil.disk_usage(path).free < MIN_FREE:
        raise IntakeError('Less than 50 GB is free. Free disk space before building headless.')


def cached_binary(commit, cache=CACHE):
    binary = cache / 'headless' / commit / 'petari_headless'
    info = binary.with_name('BUILDINFO.json')
    if not binary.is_file() or not info.is_file():
        return None
    data = read_json(info)
    if data.get('source_commit') != commit or data.get('sha256') != hashlib.sha256(binary.read_bytes()).hexdigest():
        raise IntakeError(f'Cached headless identity does not match {info}. Remove that cache entry and rebuild.')
    return binary


def build_worker(commit, log_path):
    if not COMMIT.fullmatch(commit):
        raise IntakeError('Invalid build commit.')
    if cached_binary(commit):
        return
    CACHE.mkdir(parents=True, exist_ok=True)
    require_space(CACHE)
    src = CACHE / 'src' / commit
    build = CACHE / 'src' / (commit + '-build')
    src.parent.mkdir(parents=True, exist_ok=True)
    if src.exists() or build.exists():
        raise IntakeError(f'Scratch already exists at {src} or {build}; inspect it before retrying.')
    deps = TEAM / 'scratch-port/solA/build-ov-gx/_deps'
    if not all((deps / (d + '-src')).is_dir() for d in ('imgui', 'aurora', 'dawn_prebuilt', 'sdl', 'tracy')):
        raise IntakeError(f'Pinned native dependency sources are missing under {deps}.')
    commands = []
    added = False
    started = time.time()
    with Path(log_path).open('a') as log:
        def run(cmd):
            commands.append(cmd)
            log.write('$ ' + shlex.join(cmd) + '\n')
            log.flush()
            subprocess.run(cmd, cwd=REPO, stdout=log, stderr=subprocess.STDOUT, check=True)
        try:
            run(['git', 'worktree', 'add', '--detach', str(src), commit])
            added = True
            cmd = ['nice', '-n', '19', 'cmake', '-S', str(src), '-B', str(build), '-DCMAKE_BUILD_TYPE=Release',
                   '-DPETARI_SANITIZERS=OFF', '-DBUILD_TESTING=OFF', '-DPETARI_BUILD_HEADLESS=ON', '-DPETARI_BUILD_GX_PROBE=OFF',
                   '-DFETCHCONTENT_FULLY_DISCONNECTED=ON', f'-DPETARI_IMGUI_SOURCE_DIR={deps / "imgui-src"}']
            for d in ('aurora', 'dawn_prebuilt', 'imgui', 'sdl', 'tracy'):
                cmd.append(f'-DFETCHCONTENT_SOURCE_DIR_{d.upper()}={deps / (d + "-src")}')
            compiler = subprocess.check_output(['clang', '--version'], text=True)
            major = re.search(r'clang version (\d+)', compiler)
            if major and int(major[1]) < 17:
                cmd.append('-DCMAKE_CXX_FLAGS=-fexperimental-library -include TargetConditionals.h')
            run(cmd)
            require_space(CACHE)
            run(['nice', '-n', '19', 'cmake', '--build', str(build), '--target', 'petari_headless', '-j4'])
            built = build / 'native/app/headless/petari_headless'
            target = CACHE / 'headless' / commit
            target.mkdir(parents=True, exist_ok=True)
            shutil.copy2(built, target / 'petari_headless.tmp')
            (target / 'petari_headless.tmp').replace(target / 'petari_headless')
            info = dict(source_commit=commit, sha256=hashlib.sha256(built.read_bytes()).hexdigest(),
                        seconds=time.time() - started, built_at=time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()),
                        commands=commands, compiler=compiler, dependencies=str(deps))
            (target / 'BUILDINFO.json').write_text(json.dumps(info, indent=2) + '\n')
        finally:
            shutil.rmtree(build, ignore_errors=True)
            if added:
                subprocess.run(['git', 'worktree', 'remove', '--force', str(src)], cwd=REPO,
                               stdout=log, stderr=subprocess.STDOUT, check=True)


def get_headless(commit, no_build, work):
    binary = cached_binary(commit)
    if binary:
        return binary
    if no_build:
        raise IntakeError('Matching headless is not cached. Remove --no-build or supply --headless PATH.')
    lock = TEAM / 'slots/intake-build.lock'
    lock.parent.mkdir(parents=True, exist_ok=True)
    log = work / 'build.log'
    cmd = ['lockf', '-k', str(lock), sys.executable, '-I', str(Path(__file__).resolve()), '--_build-worker', commit, str(log)]
    (work / 'build-command.json').write_text(json.dumps(cmd, indent=2) + '\n')
    with log.open('a') as out:
        result = subprocess.run(cmd, stdout=out, stderr=subprocess.STDOUT)
    if result.returncode:
        raise IntakeError(f'Headless build failed (exit {result.returncode}); see {log}.')
    binary = cached_binary(commit)
    if binary is None:
        raise IntakeError(f'The build did not produce a verified cache entry; see {log}.')
    return binary


def tail_lines(path, n=25):
    if not path.is_file():
        return []
    with path.open('rb') as f:
        f.seek(max(0, path.stat().st_size - 32768))
        return f.read().decode('utf-8', errors='replace').splitlines()[-n:]


def replay_tape(module, tape, binary, disc, work, max_frames=None):
    out = work / 'replay'
    args = [str(tape.parent), '--headless', str(binary), '--disc', str(disc), '--out', str(out), '--allow-incomplete']
    if max_frames is not None:
        args += ['--max-frames', str(max_frames)]
    cmd = [sys.executable, '-I', str(REPO / 'tools/port/replay_tape.py')] + args
    result = dict(command=cmd, headless_exit=None, exit_status=None)
    # Capture the actual child status: replay_tape turns negative signals into its own exit 1.
    def run_headless(*a, **kw):
        kw['cwd'] = out
        child = subprocess.run(*a, **kw)
        result['headless_exit'] = child.returncode
        return child
    module.subprocess = SimpleNamespace(run=run_headless, STDOUT=subprocess.STDOUT)
    started = time.monotonic()
    with (work / 'replay.log').open('w') as log, contextlib.redirect_stdout(log), contextlib.redirect_stderr(log):
        try:
            result['exit_status'] = module.main(args)
        except SystemExit as e:
            result['exit_status'] = e.code if isinstance(e.code, int) else 1
            result['error'] = str(e)
            print(e)
        except (ValueError, KeyError, IndexError, OSError) as e:
            result.update(exit_status=1, error=str(e))
            print(e)
    result['seconds'] = time.monotonic() - started
    code = result['headless_exit']
    result['signal'] = signal.Signals(-code).name if code is not None and code < 0 else None
    result['last_log_lines'] = tail_lines(out / 'headless.log')
    result['summary'] = read_json(out / 'first-divergence.json') if (out / 'first-divergence.json').is_file() else None
    result['timing'] = parse_timing(out)
    result['max_frames'] = max_frames
    return result


def parse_timing(out):
    summaries = []
    for path in sorted(out.glob('*timing*.csv')):
        count, flagged, samples, first, last = 0, [], [], None, None
        with path.open(newline='') as f:
            for row in csv.DictReader(f):
                count += 1
                first = first or row
                last = row
                if len(samples) < 20:
                    samples.append(row)
                if any(k.lower() in ('stall', 'stalled', 'stall_ms') and str(v).lower() not in ('', '0', '0.0', 'false', 'no', 'none') for k, v in row.items() if k):
                    if len(flagged) < 20:
                        flagged.append(row)
        summaries.append(dict(file=str(path), rows=count, first=first, last=last, sample_rows=samples, stall_rows=flagged,
                              note='VI/ticks are emulated time; they do not measure wall-clock stalls.' if first and set(first) == {'frame', 'vi_count', 'ticks'} else ''))
    return summaries


def parse_log(path):
    result = dict(file=str(path), lines=0, stalls=0, stall_lines=[], stall_phases={}, stall_causes={}, errors=[], shader_compile_lines=0,
                  audio_reports=0, audio_underrun_reports=0, audio_underrun_frames=0, audio_worst_underrun_frames=0, audio_lines=[])
    with path.open(errors='replace') as f:
        for line in f:
            line = line.rstrip()[:2000]
            result['lines'] += 1
            if '[stall]' in line:
                result['stalls'] += 1
                if len(result['stall_lines']) < 8:
                    result['stall_lines'].append(line)
                for key in ('phase', 'cause'):
                    m = re.search(r'\b' + key + r'=([^\s,]+)', line)
                    if m:
                        counts = result['stall_' + key + 's']
                        counts[m[1]] = counts.get(m[1], 0) + 1
            if re.search(r'PETARI|\berror\b|\babort\w*\b|\bfatal\b', line, re.I) and len(result['errors']) < 20:
                result['errors'].append(line)
            if re.search(r'shader.*compil|compil.*shader', line, re.I):
                result['shader_compile_lines'] += 1
            # the sink's periodic "[audio] level ..., underrun N frames, ..." report and its end-of-run "audio underrun: N silent frames"
            m = re.search(r'\[audio\] level .*?underrun (\d+) frames', line) or re.search(r'audio underrun: (\d+) silent frames', line)
            if m:
                frames = int(m[1])
                result['audio_reports'] += 1
                if frames:
                    result['audio_underrun_reports'] += 1
                    result['audio_underrun_frames'] += frames
                    result['audio_worst_underrun_frames'] = max(result['audio_worst_underrun_frames'], frames)
                    if len(result['audio_lines']) < 8:
                        result['audio_lines'].append(line)
    return result


def crash_evidence(paths, dsyms, symbolicator=None):
    evidence = []
    for path in paths:
        item = dict(file=str(path), header=read_text(path, 16000), frames=[], symbolication=None)
        if symbolicator and symbolicator.is_file():
            cmd = [sys.executable, '-I', str(symbolicator), str(path), '--json']
            for dsym in dsyms:
                cmd += ['--dsym', str(dsym)]
            run = subprocess.run(cmd, capture_output=True, text=True)
            item['symbolication'] = dict(command=cmd, exit_status=run.returncode, stderr=run.stderr[-4000:])
            try:
                item['symbolicated'] = json.loads(run.stdout)
                item['frames'] = item['symbolicated'].get('frames', [])[:12]
            except (ValueError, AttributeError):
                item['symbolication']['error'] = run.stdout[-4000:] or 'No JSON output.'
        else:
            item['symbolication'] = dict(error='symbolicate.py is unavailable; retain the crash and locate its matching dSYM.')
        evidence.append(item)
    return evidence


def verdicts(build, tape_issue, replay, crashes):
    codes = []
    if not build['known']:
        codes.append('BUILD_UNKNOWN')
    if tape_issue:
        codes.append(tape_issue)
    if crashes:
        codes.append('NATIVE_CRASH')
    if replay:
        summary = replay.get('summary') or {}
        if replay.get('signal') or replay.get('crashes'):
            codes.append('HEADLESS_CRASH')
        elif summary.get('first_divergence') is not None:
            codes.append('DIVERGED')
        elif summary.get('all_segments_matched'):
            codes.append('REPLAY_MATCHED')
        else:
            codes.append('TAPE_INVALID')
    priority = ('BUILD_UNKNOWN', 'HEADLESS_CRASH', 'NATIVE_CRASH', 'TAPE_INVALID', 'TAPE_MISSING', 'DIVERGED', 'REPLAY_MATCHED')
    return next((v for v in priority if v in codes), None), codes


def evidence_summary(data):
    """The at-a-glance block: machine, M captures, stalls, audio. Audio cannot be replayed headless (no device), so it is read
    from the tester's own log only."""
    system = data.get('system') or {}
    lines = ['**Mac:** ' + ', '.join(x for x in (system.get('model'), system.get('chip'), f"{system['memory_gb']} GB" if system.get('memory_gb') else '', 'macOS ' + system['macos'] if system.get('macos') else '') if x) or '(not recorded)']
    captures = data.get('captures') or []
    lines.append('**M/F9 captures:** ' + (', '.join(c.get('time', '?') for c in captures) if captures else 'none'))
    logs = data.get('logs') or []
    stalls = sum(l['stalls'] for l in logs)
    lines.append(f'**Log stalls:** {stalls}' + (f' in {len(logs)} log(s)' if logs else ' (no log included)'))
    reports = sum(l['audio_reports'] for l in logs)
    under = sum(l['audio_underrun_reports'] for l in logs)
    frames = sum(l['audio_underrun_frames'] for l in logs)
    worst = max([l['audio_worst_underrun_frames'] for l in logs] or [0])
    lines.append(f'**Audio (tester log; not replayable headless):** {under} of {reports} audio reports show underruns, {frames} silent frames in total, worst single report {worst} frames' if reports else '**Audio (tester log):** no audio reports in the included log(s)')
    return lines + ['']


def markdown(data):
    build, replay = data['build'], data.get('replay')
    summary = (replay or {}).get('summary') or {}
    descriptions = dict(REPLAY_MATCHED='Headless matched the recorded trajectory. Inspect rendering, timing and stalls.',
                        DIVERGED='Headless departed from the recorded trajectory. Inspect the first differing state below.',
                        HEADLESS_CRASH='The headless replay failed. Inspect its exit status, log and crash frames.',
                        NATIVE_CRASH='The tester app crashed. Inspect its crash frames and matching dSYM.',
                        TAPE_MISSING='No session tape was included. Ask the tester to collect a report after playing.',
                        TAPE_INVALID='The tape did not produce a complete replay comparison. Inspect the reason below.',
                        BUILD_UNKNOWN='The tester build could not be verified. Resolve build identity before trusting a replay.')
    note = data['note'].strip() or '\n'.join(n for c in data['captures'] for n in c['notes']) or '(no note supplied)'
    if replay and replay.get('max_frames') and summary.get('all_segments_matched'):
        descriptions['REPLAY_MATCHED'] = 'Headless matched the requested prefix only. Rerun without --max-frames to check the rest of the session.'
    lines = ['# Triage: ' + (data['primary'] or 'could not finish'), '', '**Verdicts:** ' + ', '.join(data['verdicts']), '', descriptions.get(data['primary'], 'Intake could not finish. Resolve the tool errors below and run again.'), '',
             '**Tester note:**', '', *['> ' + s for s in note.splitlines()], '',
             '**Build:** ' + (build.get('commit') or 'unknown') + '; UUID ' + (', '.join(build['executable_uuids']) or 'missing') +
             '; released in registry: ' + str(build['released']), '',
             *evidence_summary(data), '**Look next:**', '']
    if data['primary'] in ('NATIVE_CRASH', 'HEADLESS_CRASH'):
        kind = 'native_crashes' if data['primary'] == 'NATIVE_CRASH' else 'headless_crashes'
        for crash in data[kind][:3]:
            lines += [crash['file'], '', '```json', json.dumps(crash['frames'][:6] or crash['symbolication'], indent=2), '```', '']
            if crash['symbolication'].get('command'):
                lines += ['```sh', shlex.join(crash['symbolication']['command']), '```', '']
        if not data[kind]:
            lines += ['No headless crash report was written; inspect replay/headless.log and the exit signal below.', '']
    if summary.get('first_divergence') is not None:
        divergence = summary['first_divergence']
        segment = summary.get('first_divergence_segment')
        stage = next((s.get('stage') for s in summary.get('segments', []) if s.get('segment') == segment), None)
        lines += [f'Segment {segment}, stage {stage}, route row {divergence.get("route_row")}, segment frame {divergence.get("trace_row")}.', '',
                  '```json', json.dumps(divergence, indent=2), '```', '']
    if replay:
        cmd = list(replay['command'])
        cmd[cmd.index('--out') + 1] = str(Path(data['work']) / 'replay-inspect')
        if summary.get('first_divergence'):
            d = summary['first_divergence']
            preceding = sum(s.get('tape_frames', 0) for s in summary.get('segments', []) if s.get('segment', 0) < summary.get('first_divergence_segment', 0))
            if '--max-frames' in cmd:
                i = cmd.index('--max-frames')
                del cmd[i:i+2]
            cmd += ['--max-frames', str(preceding + int(d.get('trace_row', 0)) + 60)]
        lines += ['Replay command (fresh output; uses the prepared local tape):', '', '```sh', shlex.join(cmd), '```', '',
                  f'Replay exit: {replay["exit_status"]}; headless exit: {replay["headless_exit"]}; signal: {replay["signal"]}; seconds: {replay["seconds"]:.2f}.', '']
        if replay.get('max_frames'):
            lines += ['Replay was limited by --max-frames; a match covers only that prefix.', '']
        if replay.get('error'):
            lines += [replay['error'], '']
        lines += ['Last headless log lines:', '', '```text', *replay['last_log_lines'], '```', '']
    else:
        lines += ['Verify identity in tools/release/builds.tsv, then rerun intake with --commit FULL_COMMIT or --headless PATH as needed.', '']
    for warning in data['warnings'] + build['problems']:
        lines += ['- ' + warning]
    for error in data['tool_errors']:
        lines += ['- Intake could not finish: ' + error]
    lines += ['', '## Report evidence', '', '**System:** ' + json.dumps(data['system'], ensure_ascii=False), '',
              '```text', data['system_text'], '```', '']
    for capture in data['captures']:
        lines += ['Capture ' + capture.get('time', '') + ': ' + capture['file'], '', '```text', capture['state'], '```', '']
    for log in data['logs']:
        lines += [f'Log {log["file"]}: {log["stalls"]} stalls; {log["shader_compile_lines"]} shader compile log lines; {log["audio_underrun_reports"]}/{log["audio_reports"]} audio reports with underruns ({log["audio_underrun_frames"]} silent frames).', '',
                  '```text', *log['stall_lines'], *log['audio_lines'], *log['errors'], '```', '']
    for kind in ('native_crashes', 'headless_crashes'):
        for crash in data[kind]:
            lines += ['Crash: ' + crash['file'], '', '```json', json.dumps(crash['frames'] or crash['symbolication'], indent=2), '```', '']
    lines += ['Full evidence: triage.json; replay/first-divergence.json; replay/headless.log; replay.log.', '']
    return '\n'.join(lines)


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    if argv[:1] == ['--_build-worker']:
        build_worker(argv[1], argv[2])
        return 0
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('report', type=Path)
    ap.add_argument('--work', type=Path, help='fresh output directory (must not exist)')
    ap.add_argument('--disc', type=Path, default=Path.home() / 'MacioGalaxy2-gamedata/SB4E01-extracted')
    ap.add_argument('--headless', type=Path)
    ap.add_argument('--commit', help='explicit full source commit for an unverified build')
    ap.add_argument('--no-build', action='store_true')
    ap.add_argument('--dsym', type=Path, action='extend', nargs='+', default=[])
    ap.add_argument('--max-frames', type=int)
    ap.add_argument('--keep', action='store_true', help='keep private replay preparation and user directory')
    ap.add_argument('--json', action='store_true', help='print triage JSON instead of a short verdict')
    a = ap.parse_args(argv)
    if a.max_frames is not None and a.max_frames < 1:
        ap.error('--max-frames must be positive')
    source = a.report.expanduser().absolute()
    if source.is_symlink():
        raise IntakeError('The report must not be a symbolic link.')
    source = source.resolve()
    name = re.sub(r'[^A-Za-z0-9_.-]', '_', source.stem if source.is_file() else source.name)
    work = (a.work.expanduser() if a.work else CACHE / name).resolve()
    if work.exists():
        raise IntakeError(f'Output already exists: {work}. Choose a fresh --work directory.')
    if source.is_dir() and (work == source or source in work.parents):
        raise IntakeError('--work must be outside the report directory.')
    work.parent.mkdir(parents=True, exist_ok=True)
    work.mkdir()
    try:
        root = unpack_report(source, work / 'report')
    except Exception:
        if not a.keep:
            shutil.rmtree(work)
        raise
    warnings = []
    raw_build = load_build(root, warnings)
    module = load_replay()
    tape, meta, issue, reason = prepare_tape(root, work / 'replay-tape', module)
    if reason:
        warnings.append(reason)
    build = verify_build(raw_build, meta, REPO / 'tools/release/builds.tsv', override=a.commit)
    symbolicator = REPO / 'tools/release/symbolicate.py'
    crashes = crash_evidence(sorted(p for p in (root / 'crashes').glob('*') if p.is_file() and
                                   (p.suffix.lower() in ('.ips', '.crash') or p.name.startswith('petari-crash-'))), a.dsym, symbolicator)
    data = dict(format=1, work=str(work), report=str(root), source_report=str(source), build=build, build_json=raw_build,
                note=read_text(root / 'note.txt'), warnings=warnings, tool_errors=[], replay=None,
                system={k: raw_build.get(k, '') for k in ('chip', 'memory_gb', 'macos', 'model')},
                system_text=read_text(root / 'system.txt'), native_crashes=crashes, headless_crashes=[],
                captures=[dict(file=str(p), time=p.parent.name, state=read_text(p), notes=re.findall(r'^note:\s*(.*)$', read_text(p), re.M | re.I)) for p in sorted((root / 'captures').glob('*/state.txt'))],
                logs=[parse_log(p) for p in sorted((root / 'logs').glob('*')) if p.is_file()],
                tape=dict(complete=meta.get('complete'), seams=meta.get('seams'), mods=meta.get('mods'), error=reason))
    try:
        if tape and (a.headless or (build['commit'] and (build['known'] or a.commit))):
            disc = a.disc.expanduser().absolute()
            if not (disc / 'files').is_dir():
                raise IntakeError(f'No extracted disc at {disc}. Pass --disc DIR containing files/.')
            binary = a.headless.expanduser().resolve() if a.headless else get_headless(build['commit'], a.no_build, work)
            if not binary.is_file() or not os.access(binary, os.X_OK):
                raise IntakeError(f'Headless is not executable: {binary}')
            # Report attachments are never executable inputs, even with --headless.
            if root in binary.parents or source == binary or (source.is_dir() and source in binary.parents):
                raise IntakeError('--headless must be a trusted binary outside the report.')
            data['replay'] = replay_tape(module, tape, binary, disc, work, a.max_frames)
            headless_reports = [p for p in (work / 'replay').rglob('*') if p.is_file() and (p.suffix in ('.ips', '.crash') or p.name.startswith('petari-crash-'))]
            saved_reports = []
            for index, report in enumerate(headless_reports):
                saved = work / 'replay/crashes' / (str(index) + '-' + report.name)
                saved.parent.mkdir(exist_ok=True)
                shutil.copyfile(report, saved)
                saved_reports.append(saved)
            data['headless_crashes'] = crash_evidence(saved_reports, a.dsym, symbolicator)
            data['replay']['crashes'] = [c['file'] for c in data['headless_crashes']]
    except (IntakeError, OSError, ValueError) as e:
        data['tool_errors'].append(str(e))
    data['primary'], data['verdicts'] = verdicts(build, issue, data['replay'], crashes)
    (work / 'triage.json').write_text(json.dumps(data, indent=2) + '\n')
    (work / 'triage.md').write_text(markdown(data))
    if not a.keep:
        shutil.rmtree(work / 'replay/user', ignore_errors=True)
    print(json.dumps(data, indent=2) if a.json else f'{data["primary"]}: {", ".join(data["verdicts"])}\n{work / "triage.md"}')
    return 2 if data['tool_errors'] else 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (IntakeError, OSError, ValueError, zipfile.BadZipFile, subprocess.CalledProcessError) as error:
        print(f'intake: {error}', file=sys.stderr)
        sys.exit(2)
