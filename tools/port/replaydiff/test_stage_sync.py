import csv
import json
from pathlib import Path
from types import SimpleNamespace
import tempfile
import unittest
from unittest.mock import patch

import dolphin_collect as collector
from kpad_script import ENTRY


def source_row(frame, **changes):
    row = dict.fromkeys(collector.COLUMNS, '')
    row.update(frame=frame, scene_epoch=4, scene_valid=1, scene='Game',
               stage='SkyIslandGalaxy', scenario=1, init_state=6, player_valid=0,
               rng_valid=1, rng_seed=0xDEADBEEF, observer_error='')
    row.update(changes)
    return row


class StageSyncTests(unittest.TestCase):
    def test_first_matching_seam_does_not_wait_for_player(self):
        sync = collector.StageSync('SkyIslandGalaxy', 1)
        for changes in ({'scene_valid': 0}, {'scene': 'Logo'}, {'init_state': 5},
                        {'stage': 'FileSelect'}, {'scenario': 2}):
            self.assertIsNone(sync.select(source_row(7, **changes)))
        original = source_row(8)
        row = sync.select(original)
        self.assertEqual((row['frame'], row['scene_epoch'], row['player_valid']), (0, 0, 0))
        self.assertEqual(original['frame'], 8)
        self.assertEqual(sync.origin['source_frame'], 8)
        self.assertEqual(sync.origin['rng_seed'], 0xDEADBEEF)
        self.assertIsNone(sync.origin['source_ticks'])
        self.assertIs(sync.origin['rng_valid'], True)
        self.assertFalse(sync.origin['rng_forced'])
        later = sync.select(source_row(9, scene_epoch=5, stage='OtherGalaxy', rng_seed=123))
        self.assertEqual((later['frame'], later['scene_epoch'], later['rng_seed']), (1, 1, 123))
        self.assertEqual(sync.origin['rng_seed'], 0xDEADBEEF)

    def test_missing_rng_at_first_match_fails_instead_of_shifting_origin(self):
        with self.assertRaisesRegex(collector.ObserverError, 'first stage-sync'):
            collector.StageSync('SkyIslandGalaxy', 1).select(source_row(7, rng_valid=0))

    def test_collection_keeps_full_input_clock_and_emits_only_synced_rows(self):
        self.collect_synced()

    def test_actor_sidecars_share_exact_rebased_origin_and_success_manifest(self):
        self.collect_synced(actors=True)

    def test_actor_finalization_failure_keeps_main_capture_incomplete(self):
        self.collect_synced(actors=True, fail_actor_close=True)

    def test_missing_input_export_binding_keeps_main_and_actors_incomplete(self):
        self.collect_synced(actors=True, fail_input_receipt=True)

    def collect_synced(self, actors=False, fail_actor_close=False, fail_input_receipt=False):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            script = root / 'script.json'
            script.write_text(json.dumps(dict(version=1, frames=5, segments=[])))
            args = SimpleNamespace(output=str(root / 'capture'), socket=str(root / 'gdb.sock'),
                                   dolphin=None, frames=2, input_script=str(script), live_input_script=False,
                                   gui=True, movie=None, connect_timeout=1, stop_timeout=1, wall_timeout=1,
                                   sync_stage='SkyIslandGalaxy', sync_scenario=1)
            args.actors = actors
            handled = []
            class Stream:
                def connect(self, _): pass
                def settimeout(self, _): pass
                def setsockopt(self, *_): pass
                def close(self): pass
            class Remote:
                step = False
                first = True
                pc = collector.SEAM
                def __init__(self, _): pass
                def stop(self, command, **_kwargs):
                    if command != 'c': return
                    if self.first:
                        self.pc, self.first = ENTRY, False
                    elif self.pc == ENTRY:
                        self.pc = collector.SEAM
                    else:
                        self.pc = collector.SEAM + (4 if self.step else 0)
                def read(self, address, size):
                    return bytes.fromhex({**collector.SIGNATURES, **collector.ACTOR_SIGNATURES}[address])
                def reg(self, number):
                    return collector.CALLER if number == 67 else self.pc
                def ok(self, command):
                    self.assert_no_write(command)
                    if command.startswith('Z0,804be284'): self.step = True
                    if command.startswith('z0,804be284'): self.step = False
                @staticmethod
                def assert_no_write(command):
                    if command.startswith(('M', 'P', 'G')):
                        raise AssertionError('observer wrote guest state')
            class Input:
                reads = 0
                revisions = []
                script = {}
                def __init__(self, path, frames, output, live):
                    self.frames = frames
                    self.output = Path(output)
                    if frames != 5: raise AssertionError('script clock truncated')
                def install(self, _): pass
                deferred_stop = False
                def handle(self, remote, pc, frame):
                    if pc == ENTRY:
                        remote.stop('c')
                        self.deferred_stop = True
                        return True
                    self.deferred_stop = False
                    handled.append(frame)
                    return False
                def end_frame(self): pass
                def close(self, remote, complete):
                    if not complete: raise AssertionError('collection did not complete')
                    # This mock emitted zero SDK reads; the negative control contradicts that count.
                    self.output.write_bytes(b'{}\n' if fail_input_receipt else b'')
            class Snapshot:
                def __init__(self, _, actors=False): self.actor_rows = []
                def take(self, frame):
                    row = source_row(frame, init_state=5 if frame < 3 else 6,
                                     rng_seed=100 + frame, actor_count_valid=1,
                                     actor_registered=1, actor_living=1)
                    actor = dict.fromkeys(collector.ACTOR_COLUMNS, '')
                    actor.update(schema_version=1, **{key: row[key] for key in collector.ACTOR_CONTEXT},
                                 producer_handle=99, actor_id='player', identity_kind='player',
                                 identity_status='source-key', dead=0)
                    self.actor_rows = [actor]
                    return row
            class FailingSidecars(collector.ActorSidecars):
                def close(self, complete):
                    super().close(False)
                    raise OSError('synthetic actor rename failure')
            sidecars = FailingSidecars if fail_actor_close else collector.ActorSidecars
            with patch.object(collector.socket, 'socket', return_value=Stream()), \
                 patch.object(collector, 'RSP', Remote), patch.object(collector, 'Snapshot', Snapshot), \
                 patch.object(collector, 'ActorSidecars', sidecars), \
                 patch('kpad_script.ScriptedInput', Input):
                if fail_actor_close:
                    with self.assertRaisesRegex(collector.ObserverError, 'actor finalization failed'):
                        collector.collect(args)
                    manifest = json.loads((root / 'capture/manifest.json').read_text())
                    self.assertEqual(manifest['status'], 'incomplete')
                    self.assertEqual(manifest['actors']['status'], 'incomplete')
                    self.assertTrue((root / 'capture/oracle.csv.actors.csv.partial').exists())
                    self.assertFalse((root / 'capture/oracle.csv.actors.csv').exists())
                    return
                if fail_input_receipt:
                    with self.assertRaisesRegex(collector.ObserverError, 'input cleanup failed'):
                        collector.collect(args)
                    manifest = json.loads((root / 'capture/manifest.json').read_text())
                    self.assertEqual(manifest['status'], 'incomplete')
                    self.assertEqual(manifest['input_export']['status'], 'incomplete')
                    self.assertEqual(manifest['actors']['status'], 'incomplete')
                    self.assertTrue((root / 'capture/oracle.csv.actors.csv.partial').exists())
                    self.assertFalse((root / 'capture/oracle.csv.actors.csv').exists())
                    return
                collector.collect(args)
            self.assertEqual(handled, [0, 1, 2, 3, 4])
            with (root / 'capture/oracle.csv').open() as file:
                rows = list(csv.DictReader(file))
            self.assertEqual([row['frame'] for row in rows], ['0', '1'])
            self.assertEqual([row['rng_seed'] for row in rows], ['103', '104'])
            self.assertEqual(rows[0]['player_valid'], '0')
            sync = json.loads((root / 'capture/oracle.csv.sync.json').read_text())
            self.assertEqual((sync['source_frame'], sync['rng_seed']), (3, 103))
            manifest = json.loads((root / 'capture/manifest.json').read_text())
            self.assertEqual((manifest['frames'], manifest['source_frames']), (2, 5))
            self.assertEqual(manifest['input_export']['status'], 'captured')
            self.assertEqual(manifest['input_export']['path'], 'inputs.jsonl')
            self.assertEqual(manifest['input_export']['bytes'], 0)
            self.assertEqual(manifest['input_export']['read_count'], 0)
            if actors:
                for suffix in ('actors.csv', 'actor-frames.csv'):
                    with (root / ('capture/oracle.csv.' + suffix)).open() as stream:
                        actor_rows = list(csv.DictReader(stream))
                    self.assertEqual([r['frame'] for r in actor_rows], ['0', '1'])
                    self.assertEqual([r['scene_epoch'] for r in actor_rows], ['0', '0'])
                self.assertEqual(manifest['actors']['frames'], 2)
                self.assertEqual(manifest['actors']['status'], 'captured')
                self.assertEqual(len(manifest['actors']['files']), 2)
            else:
                self.assertNotIn('actors', manifest)
                self.assertFalse(list((root / 'capture').glob('*actors*')))


if __name__ == '__main__':
    unittest.main()
