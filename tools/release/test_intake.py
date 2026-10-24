#!/usr/bin/env python3
"""Synthetic intake reports: no disc data or native build required. Run with python3 -I."""
import contextlib
import csv
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import signal
import stat
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile

spec = importlib.util.spec_from_file_location('intake', Path(__file__).with_name('intake.py'))
it = importlib.util.module_from_spec(spec)
spec.loader.exec_module(it)
with (it.REPO / 'tools/release/builds.tsv').open() as f:
    RELEASE = next(csv.DictReader(f, delimiter='\t'))

FAKE_HEADLESS = r'''#!/usr/bin/env python3
import csv, json, os, pathlib, signal, sys
args=sys.argv[1:]
def arg(key):return args[args.index(key)+1]
trace=pathlib.Path(arg('--trace'))
mode=MODE
if mode=='crash-report':
    crashdir=pathlib.Path(arg('--user'))/'Crashes'
    crashdir.mkdir(parents=True)
    (crashdir/'petari-crash-synthetic.txt').write_text('synthetic native crash report')
if mode in ('crash','crash-report'):
    print('headless: synthetic fatal signal',flush=True)
    os.kill(os.getpid(),signal.SIGTERM)
if mode=='failure':
    print('headless: controlled replay failure',flush=True)
    sys.exit(2)
segments=[s.split() for s in pathlib.Path(arg('--segments')).read_text().splitlines() if s and not s.startswith('#')]
start=0
with trace.open('w') as f, pathlib.Path(str(trace)+'.segments.csv').open('w') as sf, pathlib.Path(str(trace)+'.timing.csv').open('w') as tf:
    writer=csv.writer(f); writer.writerow(['stage','scenario','player_valid','x','y','z','movement_lo','dead'])
    sw=csv.writer(sf); sw.writerow(['trace_start_row','rows'])
    tw=csv.writer(tf); tw.writerow(['frame','vi_count','ticks'])
    for i,(stage,scenario,seed,n) in enumerate(segments):
        for row in range(int(n)+1):
            writer.writerow([stage,scenario,1,5 if mode=='diverge' and row==1 else 0,0,0,1<<30,0])
            tw.writerow([start+row,start+row,(start+row)*100])
        sw.writerow([start,int(n)+1])
        start+=int(n)+1
        pathlib.Path(str(trace)+'.sync'+(str(i) if i else '')+'.json').write_text(json.dumps({'rng_seed':int(seed)}))
pathlib.Path(str(trace)+'.complete').touch()
print('recorded mods:',os.environ.get('PETARI_MODS'))
'''


class Fixture(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)
        self.report = self.base / 'report'
        self.report.mkdir()
        self.disc = self.base / 'disc'
        (self.disc / 'files').mkdir(parents=True)
        self.build = dict(report_format=1, executable_uuid=RELEASE['executable_uuid'], source_commit=RELEASE['source_commit'])
        self.write_build()
        (self.report / 'note.txt').write_text('Mario froze after the switch.\n')
        self.tape = self.report / 'tapes/session'
        self.tape.mkdir(parents=True)
        self.meta = dict(schema=1, complete=True, seams=3, mods=['Cappy'], unencodable_samples=0, lossy_samples=0,
                         # Deliberately inaccessible tester paths: only collected siblings may be read.
                         route_csv='/tester/private/route.csv', kpd='/tester/private/route.kpd', actions='/tester/private/route.actions',
                         segments=[dict(start_row=0, end_row=2, stage='RedBlueExGalaxy', scenario=1, rng_seed=123)], replay_env={})
        self.write_meta()
        self.rt = it.load_replay()
        fields = ('stage', 'scenario', 'valid', 'x', 'y', 'z', 'grounded', 'dead', 'move_x', 'move_y', 'jump', 'spin', 'crouch', 'shoot')
        with (self.tape / 'route.csv').open('w') as f:
            w = csv.DictWriter(f, fieldnames=fields)
            w.writeheader()
            for _ in range(3):
                row = dict.fromkeys(fields, '0')
                row.update(stage='RedBlueExGalaxy', scenario='1', valid='1', grounded='1')
                w.writerow(row)
        records = [dict(frame=f, ordinal=0, channel=0, capacity=120, returned=0, error=0, probe=0, device=1, samples=[]) for f in (1, 2)]
        (self.tape / 'route.kpd').write_bytes(self.rt.pack_kpd(2, records))
        (self.tape / 'route.actions').write_text('# no mod presses\n')

    def write_build(self):
        (self.report / 'build.json').write_text(json.dumps(self.build))

    def write_meta(self):
        (self.tape / 'route.tape.json').write_text(json.dumps(self.meta))

    def fake(self, mode='match'):
        p = self.base / ('fake-headless-' + mode)
        p.write_text(FAKE_HEADLESS.replace('MODE', repr(mode)))
        p.chmod(0o755)
        return p

    def run_intake(self, mode='match', extra=()):
        work = self.base / 'work'
        with contextlib.redirect_stdout(io.StringIO()):
            rc = it.main([str(self.report), '--work', str(work), '--disc', str(self.disc), '--headless', str(self.fake(mode)), *extra])
        return rc, json.loads((work / 'triage.json').read_text()), (work / 'triage.md').read_text()


class EndToEnd(Fixture):
    def test_matched_report_and_runnable_receipt(self):
        original = (self.tape / 'route.tape.json').read_bytes()
        rc, data, md = self.run_intake()
        self.assertEqual(rc, 0)
        self.assertEqual(data['primary'], 'REPLAY_MATCHED')
        self.assertIn('Mario froze after the switch.', md)
        self.assertEqual(data['replay']['timing'][0]['rows'], 3)
        self.assertIn('Cappy,-OdysseyJumpDive', '\n'.join(data['replay']['last_log_lines']))
        self.assertEqual((self.tape / 'route.tape.json').read_bytes(), original)
        command = data['replay']['command'][:]
        command[command.index('--out') + 1] = str(self.base / 'another-replay')
        replay = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(replay.returncode, 0, replay.stderr)

    def test_diverged(self):
        rc, data, md = self.run_intake('diverge')
        self.assertEqual(rc, 0)
        self.assertEqual(data['primary'], 'DIVERGED')
        self.assertEqual(data['replay']['summary']['first_divergence']['route_row'], 1)
        self.assertIn('--max-frames 61', md)
        self.assertIn('RedBlueExGalaxy', md)

    def test_signal_not_hidden_by_replay_exit(self):
        rc, data, _ = self.run_intake('crash')
        self.assertEqual(rc, 0)
        self.assertEqual(data['primary'], 'HEADLESS_CRASH')
        self.assertEqual(data['replay']['headless_exit'], -signal.SIGTERM)
        self.assertEqual(data['replay']['signal'], 'SIGTERM')
        self.assertIn('exit -15', data['replay']['error'])

    def test_headless_crash_report_survives_user_cleanup(self):
        rc, data, _ = self.run_intake('crash-report')
        self.assertEqual(rc, 0)
        self.assertEqual(data['primary'], 'HEADLESS_CRASH')
        self.assertEqual(len(data['headless_crashes']), 1)
        self.assertTrue(Path(data['headless_crashes'][0]['file']).is_file())
        self.assertFalse((self.base / 'work/replay/user').exists())

    def test_partial_tape(self):
        self.meta['complete'] = False
        self.write_meta()
        with (self.tape / 'route.kpd').open('ab') as f:
            f.write(b'uncheckpointed tail')
        rc, data, _ = self.run_intake(extra=('--max-frames', '1'))
        self.assertEqual(rc, 0)
        self.assertEqual(data['primary'], 'REPLAY_MATCHED')
        self.assertFalse(data['tape']['complete'])
        self.assertEqual(data['replay']['summary']['tape_frames'], 1)

    def test_missing_tape(self):
        (self.tape / 'route.tape.json').unlink()
        rc, data, _ = self.run_intake()
        self.assertEqual(rc, 0)
        self.assertEqual(data['primary'], 'TAPE_MISSING')
        self.assertIsNone(data['replay'])

    def test_wrong_uuid_can_still_replay_explicit_headless(self):
        self.build['executable_uuid'] = '00000000-0000-0000-0000-000000000001'
        self.write_build()
        rc, data, _ = self.run_intake()
        self.assertEqual(rc, 0)
        self.assertEqual(data['primary'], 'BUILD_UNKNOWN')
        self.assertIn('REPLAY_MATCHED', data['verdicts'])
        self.assertIn('has not been released', ' '.join(data['build']['problems']))

    def test_native_crash_combines_with_match(self):
        (self.report / 'crashes').mkdir()
        (self.report / 'crashes/petari-crash-test.txt').write_text('Petari crash\nsignal=11\n')
        # Optional symbolicator may land later; exercise the absence deterministically.
        with patch.object(it, 'crash_evidence', wraps=it.crash_evidence) as evidence:
            rc, data, _ = self.run_intake()
        self.assertEqual(rc, 0)
        self.assertEqual(data['primary'], 'NATIVE_CRASH')
        self.assertIn('REPLAY_MATCHED', data['verdicts'])
        self.assertTrue(evidence.called)

    def test_corrupt_tape_is_a_verdict(self):
        (self.tape / 'route.kpd').write_bytes(b'bad')
        rc, data, _ = self.run_intake()
        self.assertEqual(rc, 0)
        self.assertEqual(data['primary'], 'TAPE_INVALID')

    def test_unsafe_replay_environment_not_executed(self):
        self.meta['replay_env'] = {'DYLD_INSERT_LIBRARIES': '/tester/payload.dylib'}
        self.write_meta()
        rc, data, _ = self.run_intake()
        self.assertEqual(rc, 0)
        self.assertEqual(data['primary'], 'TAPE_INVALID')

    def test_missing_binary_is_tool_error(self):
        with patch.object(it, 'cached_binary', return_value=None), contextlib.redirect_stdout(io.StringIO()):
            rc = it.main([str(self.report), '--work', str(self.base / 'work'), '--disc', str(self.disc), '--no-build'])
        self.assertEqual(rc, 2)
        data = json.loads((self.base / 'work/triage.json').read_text())
        self.assertIsNone(data['primary'])
        self.assertEqual(data['verdicts'], [])
        self.assertIn('--no-build', data['tool_errors'][0])

    def test_cli_isolated_json(self):
        cmd = [sys.executable, '-I', str(Path(it.__file__)), str(self.report), '--work', str(self.base / 'cli'),
               '--disc', str(self.disc), '--headless', str(self.fake()), '--json']
        run = subprocess.run(cmd, capture_output=True, text=True)
        self.assertEqual(run.returncode, 0, run.stderr)
        self.assertEqual(json.loads(run.stdout)['primary'], 'REPLAY_MATCHED')

    def test_controlled_failure_is_not_a_signal_crash(self):
        rc, data, _ = self.run_intake('failure')
        self.assertEqual(rc, 0)
        self.assertEqual(data['primary'], 'TAPE_INVALID')
        self.assertEqual(data['replay']['headless_exit'], 2)
        self.assertIsNone(data['replay']['signal'])

    def test_capture_note_on_first_screen(self):
        capture = self.report / 'captures/now'
        capture.mkdir(parents=True)
        (capture / 'state.txt').write_text('stage: RedBlueExGalaxy\nnote: Switch appeared late\n')
        (self.report / 'note.txt').write_text('')
        rc, data, md = self.run_intake()
        self.assertEqual(rc, 0)
        self.assertIn('Switch appeared late', md[:1000])
        self.assertEqual(data['captures'][0]['notes'], ['Switch appeared late'])

    def test_recorded_return_hook(self):
        self.meta['replay_env'] = {'PETARI_TEST_RETURN_TO_SHIP': '600:2'}
        self.write_meta()
        rc, data, _ = self.run_intake()
        self.assertEqual(rc, 0)
        self.assertEqual(data['primary'], 'REPLAY_MATCHED')
        cmd = json.loads((self.base / 'work/replay/command.json').read_text())
        self.assertEqual(cmd['env']['PETARI_TEST_RETURN_TO_SHIP'], '600:2')

    def test_unknown_build_does_not_build_automatically(self):
        self.build['executable_uuid'] = '00000000-0000-0000-0000-000000000001'
        self.write_build()
        with patch.object(it, 'get_headless', side_effect=AssertionError('must not build')), contextlib.redirect_stdout(io.StringIO()):
            rc = it.main([str(self.report), '--work', str(self.base / 'unknown')])
        self.assertEqual(rc, 0)
        data = json.loads((self.base / 'unknown/triage.json').read_text())
        self.assertEqual(data['primary'], 'BUILD_UNKNOWN')
        self.assertIsNone(data['replay'])

    def test_nonfinite_position_is_invalid(self):
        p = self.tape / 'route.csv'
        p.write_text(p.read_text().replace('RedBlueExGalaxy,1,1,0,', 'RedBlueExGalaxy,1,1,nan,', 1))
        rc, data, _ = self.run_intake()
        self.assertEqual(rc, 0)
        self.assertEqual(data['primary'], 'TAPE_INVALID')
        self.assertIn('non-finite', data['tape']['error'])

    def test_logs_only_report_missing_identity_and_tape(self):
        import shutil
        shutil.rmtree(self.report / 'tapes')
        (self.report / 'build.json').unlink()
        (self.report / 'logs').mkdir()
        (self.report / 'logs/astral.log').write_text('PETARI fatal error\n')
        rc, data, _ = self.run_intake()
        self.assertEqual(rc, 0)
        self.assertEqual(data['primary'], 'BUILD_UNKNOWN')
        self.assertIn('TAPE_MISSING', data['verdicts'])
        self.assertEqual(len(data['logs']), 1)


class Safety(Fixture):
    def test_zip_paths(self):
        for n, name in enumerate(('../escape', '/absolute', 'report/../../escape', 'C:/windows', '..\\escape')):
            with self.subTest(name=name):
                archive = self.base / f'unsafe{n}.zip'
                with zipfile.ZipFile(archive, 'w') as z:
                    z.writestr(name, 'bad')
                with self.assertRaises(it.IntakeError):
                    it.unpack_report(archive, self.base / f'out{n}')
        self.assertFalse((self.base / 'escape').exists())

    def test_zip_symlink(self):
        archive = self.base / 'link.zip'
        with zipfile.ZipFile(archive, 'w') as z:
            i = zipfile.ZipInfo('report/link')
            i.create_system = 3
            i.external_attr = (stat.S_IFLNK | 0o777) << 16
            z.writestr(i, '/etc/passwd')
        with self.assertRaises(it.IntakeError):
            it.unpack_report(archive, self.base / 'out')

    def test_folder_symlink(self):
        (self.report / 'link').symlink_to(self.base)
        with self.assertRaises(it.IntakeError):
            it.unpack_report(self.report, self.base / 'out')

    def test_oversize_and_file_count(self):
        with self.assertRaises(it.IntakeError):
            it.unpack_report(self.report, self.base / 'out-size', max_bytes=5)
        with self.assertRaises(it.IntakeError):
            it.unpack_report(self.report, self.base / 'out-count', max_files=1)
        archive = self.base / 'large.zip'
        with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED) as z:
            z.writestr('report/large', b'x' * 10000)
        with self.assertRaises(it.IntakeError):
            it.unpack_report(archive, self.base / 'out-zip', max_bytes=100)

    def test_duplicate_casefold_paths(self):
        archive = self.base / 'duplicate.zip'
        with zipfile.ZipFile(archive, 'w') as z:
            z.writestr('report/A', 'a')
            z.writestr('report/a', 'b')
        with self.assertRaises(it.IntakeError):
            it.unpack_report(archive, self.base / 'out')

    def test_valid_wrapped_zip(self):
        archive = self.base / 'good.zip'
        with zipfile.ZipFile(archive, 'w') as z:
            for path in self.report.rglob('*'):
                if path.is_file():
                    z.write(path, 'Wrapped/' + str(path.relative_to(self.report)))
        root = it.unpack_report(archive, self.base / 'out')
        self.assertEqual(root.name, 'Wrapped')
        self.assertEqual(json.loads((root / 'build.json').read_text()), self.build)

    def test_malicious_kpd_sample_length(self):
        path = self.tape / 'route.kpd'
        path.write_bytes(b'SMG2KPD1' + struct.pack('>QQ', 2, 1) + struct.pack('>QIIIiiiII', 0, 0, 0, 1, 1, 0, 0, 1, 2**32-1))
        with self.assertRaisesRegex(ValueError, 'samples'):
            it.validate_kpd(path)

    def test_work_inside_report_refused(self):
        with self.assertRaises(it.IntakeError):
            it.main([str(self.report), '--work', str(self.report / 'work')])

    def test_report_binary_via_symlink_refused(self):
        headless = self.report / 'payload'
        headless.write_text('#!/bin/sh\nexit 99\n')
        headless.chmod(0o755)
        link = self.base / 'link-to-payload'
        link.symlink_to(headless)
        with contextlib.redirect_stdout(io.StringIO()):
            rc = it.main([str(self.report), '--work', str(self.base / 'safe'), '--disc', str(self.disc), '--headless', str(link)])
        self.assertEqual(rc, 2)
        data = json.loads((self.base / 'safe/triage.json').read_text())
        self.assertIn('trusted binary outside', data['tool_errors'][0])


class Parsing(Fixture):
    def test_uuid_maps_missing_commit(self):
        result = it.verify_build({'executable_uuid': RELEASE['executable_uuid']}, {}, it.REPO / 'tools/release/builds.tsv')
        self.assertTrue(result['known'])
        self.assertEqual(result['commit'], RELEASE['source_commit'])

    def test_sidecar_identity(self):
        result = it.verify_build({}, self.build, it.REPO / 'tools/release/builds.tsv')
        self.assertTrue(result['known'])

    def test_identity_conflict(self):
        meta = dict(self.build, source_commit='f'*40)
        result = it.verify_build(self.build, meta, it.REPO / 'tools/release/builds.tsv')
        self.assertFalse(result['known'])
        self.assertIn('disagree', ' '.join(result['problems']))

    def test_unavailable_commit(self):
        result = it.verify_build({'source_commit': 'f'*40}, {}, it.REPO / 'tools/release/builds.tsv', override='f'*40)
        self.assertFalse(result['known'])
        self.assertIsNone(result['commit'])

    def test_override_different_from_registered_build_is_unverified(self):
        other = subprocess.check_output(['git', 'rev-parse', RELEASE['source_commit'] + '^'], cwd=it.REPO, text=True).strip()
        result = it.verify_build(self.build, {}, it.REPO / 'tools/release/builds.tsv', override=other)
        self.assertFalse(result['known'])
        self.assertEqual(result['commit'], other)
        self.assertIn('explicit --commit differs', ' '.join(result['problems']))

    def test_missing_build_values(self):
        warnings = []
        it.load_build(self.report, warnings)
        self.assertIn('Missing or empty', warnings[0])
        (self.report / 'build.json').write_text('[]')
        self.assertEqual(it.load_build(self.report, warnings), {})

    def test_stalls_and_shader_logs(self):
        p = self.base / 'log'
        p.write_text('[stall] phase=render cause=shader 200ms\n[stall] phase=render cause=shader 100ms\nShader compiled 1\nPETARI fatal abort\n')
        parsed = it.parse_log(p)
        self.assertEqual(parsed['stalls'], 2)
        self.assertEqual(parsed['stall_phases'], {'render': 2})
        self.assertEqual(parsed['stall_causes'], {'shader': 2})
        self.assertEqual(parsed['shader_compile_lines'], 1)
        self.assertEqual(parsed['errors'], ['PETARI fatal abort'])

    def test_timing_wall_duration_is_not_itself_a_stall(self):
        (self.base / 'timing.csv').write_text('frame,wall_ms,stall\n0,16,0\n1,90,1\n')
        result = it.parse_timing(self.base)[0]
        self.assertEqual(result['rows'], 2)
        self.assertEqual(len(result['sample_rows']), 2)
        self.assertEqual([row['frame'] for row in result['stall_rows']], ['1'])

    def test_optional_symbolication_stub(self):
        p = self.base / 'crash.txt'
        p.write_text('synthetic crash')
        stub = self.base / 'symbolicate.py'
        stub.write_text("import json,sys\nassert '--json' in sys.argv\nassert '--dsym' in sys.argv\nprint(json.dumps({'build':{},'crash':{},'frames':[{'index':0,'symbol':'Mario::update','file':'Mario.cpp','line':42}]}))\n")
        result = it.crash_evidence([p], [self.base / 'Petari.dSYM'], stub)
        self.assertEqual(result[0]['frames'][0]['symbol'], 'Mario::update')
        self.assertEqual(result[0]['symbolication']['exit_status'], 0)
        self.assertIn('unavailable', it.crash_evidence([p], [], self.base / 'absent')[0]['symbolication']['error'])

    def test_cache_identity(self):
        cache = self.base / 'cache'
        commit = RELEASE['source_commit']
        entry = cache / 'headless' / commit
        entry.mkdir(parents=True)
        (entry / 'petari_headless').write_bytes(b'fake')
        (entry / 'BUILDINFO.json').write_text(json.dumps({'source_commit':commit, 'sha256':hashlib.sha256(b'fake').hexdigest()}))
        self.assertEqual(it.cached_binary(commit, cache), entry / 'petari_headless')
        (entry / 'petari_headless').write_bytes(b'changed')
        with self.assertRaises(it.IntakeError):
            it.cached_binary(commit, cache)


if __name__ == '__main__':
    unittest.main()
