#!/usr/bin/env python3
"""No-build capture intake: explicit frames only; malformed/unfinished observations fail closed."""
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from record_pipeline_deadlines import observations, disagreements, seed_identity, replay_checks, map_observation, qualify_cold


class Captures(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.path = Path(self.tmp.name) / 'capture.jsonl'
        self.rows = [{'event': 'header', 'version': 3, 'gx_config_version': 13,
                      'clock': 'active-frame-packet-id', 'source': 'find_pipeline-before-memo', 'origin_policy': 'next-whole-packet'},
                     {'event': 'stage_begin', 'stage': 'A', 'visit': 1, 'request_packet': 479, 'origin_frame': 480},
                     {'event': 'first_use', 'stage': 'A', 'visit': 1, 'origin_frame': 480,
                      'frame': 480, 'relative_frame': 0, 'config': '0000000000000001'},
                     {'event': 'end', 'complete': True, 'records': 1, 'clock_first_frame': 480,
                      'clock_last_frame': 490, 'clock_transitions': 1, 'lookup_calls': 2,
                      'stage_requests': 1, 'stage_begins': 1, 'pending_stage_requests': 0}]

    def tearDown(self):
        self.tmp.cleanup()

    def read(self):
        self.path.write_text(''.join(json.dumps(r) + '\n' for r in self.rows))
        return observations(self.path, 'A')

    def test_exact(self):
        self.assertEqual(self.read(), {'0000000000000001': 0})

    def test_missing_footer(self):
        self.rows.pop()
        with self.assertRaises(ValueError): self.read()

    def test_wrong_origin(self):
        self.rows[2]['relative_frame'] = 1
        with self.assertRaises(ValueError): self.read()

    def test_duplicate(self):
        self.rows.insert(3, dict(self.rows[2])); self.rows[-1]['records'] = 2
        with self.assertRaises(ValueError): self.read()

    def test_wrong_clock(self):
        self.rows[0]['clock'] = 'host-time'
        with self.assertRaises(ValueError): self.read()

    def test_frozen_clock_rejected(self):
        self.rows[-1].update(clock_last_frame=480, clock_transitions=0)
        with self.assertRaises(ValueError): self.read()

    def test_old_zero_clock_protocol_rejected(self):
        self.rows[0].update(version=1, clock='fifo-renderer-frame-id')
        with self.assertRaises(ValueError): self.read()

    def test_mid_packet_stage_boundary_rejected(self):
        self.rows[1]['request_packet'] = 480
        with self.assertRaises(ValueError): self.read()

    def test_lookup_without_boundary_rejected(self):
        self.rows.pop(1)
        with self.assertRaises(ValueError): self.read()

    def test_incomplete(self):
        self.rows[-1]['complete'] = False
        with self.assertRaises(ValueError): self.read()

    def test_disagreement_retained(self):
        self.assertEqual(disagreements({'a': 0, 'b': 1}, {'a': 1, 'c': 0}),
                         {'a': [0, 1], 'b': [1, None], 'c': [None, 0]})

    def test_missing_stage_seed_keeps_recording_case(self):
        import sqlite3
        app = Path(self.tmp.name) / 'Petari.app'
        seeds = app / 'Contents/Resources/pipeline-seeds'
        seeds.mkdir(parents=True)
        with sqlite3.connect(seeds / '__global__.db') as db:
            db.execute('CREATE TABLE petari_cache_build(build_key TEXT)')
            db.execute('INSERT INTO petari_cache_build VALUES(?)', ('a' * 64,))
        db.close()
        result = seed_identity(app, 'StaffRollGalaxy')
        self.assertTrue(result['stage_seed_missing'])
        self.assertIsNone(result['stage_seed_sha256'])
        self.assertEqual(result['renderer_key'], 'a' * 64)
        self.assertEqual(result['identity_seed'], '__global__.db')
        self.assertFalse((seeds / 'StaffRollGalaxy.db').exists())


class ColdAnchors(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.root=Path(self.tmp.name)
        for name,shift in [('warm',0),('cold',27)]:
            rows=[{'event':'header','version':3,'gx_config_version':13,'clock':'active-frame-packet-id','source':'find_pipeline-before-memo','origin_policy':'next-whole-packet'},
                  {'event':'stage_begin','stage':'A','visit':1,'request_packet':479,'origin_frame':480},
                  {'event':'first_use','stage':'A','visit':1,'origin_frame':480,'frame':480,'relative_frame':0,'config':'0000000000000001'},
                  {'event':'first_use','stage':'A','visit':1,'origin_frame':480,'frame':481+shift,'relative_frame':1+shift,'config':'0000000000000002'},
                  {'event':'end','complete':True,'records':2,'clock_first_frame':480,'clock_last_frame':490+shift,'clock_transitions':2,'lookup_calls':3,'stage_requests':1,'stage_begins':1,'pending_stage_requests':0}]
            (self.root/(name+'.jsonl')).write_text(''.join(json.dumps(r)+'\n' for r in rows))
            (self.root/(name+'.tape.json')).write_text(json.dumps({'complete':True,'seams':491+shift,
                'ready_seams':[{'stage':'A','scenario':1,'row':480+shift}],
                'play_ready_seams':[{'stage':'A','scenario':1,'row':484+shift}]}))

    def tearDown(self):self.tmp.cleanup()

    def qualify(self):
        return qualify_cold(self.root/'warm.jsonl',self.root/'warm.tape.json',self.root/'cold.jsonl',self.root/'cold.tape.json','A')

    def test_load_extension_uses_exact_native_anchors(self):
        result=self.qualify();self.assertTrue(result['passed'])
        self.assertEqual(len(result['raw_absolute_disagreements']),1)
        self.assertEqual(result['exact_phase_offset_disagreements'],{})
        self.assertEqual(result['cold_anchors']['init_end_packet'],508)

    def test_post_init_one_packet_error_is_not_relaxed(self):
        p=self.root/'cold.jsonl';rows=[json.loads(l) for l in p.read_text().splitlines()]
        rows[3]['frame']+=1;rows[3]['relative_frame']+=1
        p.write_text(''.join(json.dumps(r)+'\n' for r in rows))
        self.assertFalse(self.qualify()['passed'])

    def test_phase_change_fails(self):
        p=self.root/'cold.tape.json';d=json.loads(p.read_text());d['ready_seams'][0]['row']+=1;p.write_text(json.dumps(d))
        self.assertFalse(self.qualify()['passed'])

    def test_missing_anchor_fails(self):
        p=self.root/'cold.tape.json';d=json.loads(p.read_text());d['play_ready_seams']=[];p.write_text(json.dumps(d))
        with self.assertRaises(ValueError):self.qualify()

    def test_ambiguous_init_end_fails(self):
        p=self.root/'cold.tape.json';d=json.loads(p.read_text());d['ready_seams']*=2;p.write_text(json.dumps(d))
        with self.assertRaises(ValueError):self.qualify()


class MapWindow(unittest.TestCase):
    def rows(self):
        return [{'frame':str(n),'stage':'WorldMap01Galaxy' if n>=10 else 'FileSelect',
                 'scenario':'1','valid':'1'} for n in range(981)]

    def test_fixed_window(self):
        result=map_observation(self.rows(),'WorldMap01Galaxy',970)
        self.assertTrue(all(result['checks'].values()))
        self.assertEqual(result['requested_end_route_row'],980)

    def test_short_window_fails(self):
        result=map_observation(self.rows()[:-1],'WorldMap01Galaxy',970)
        self.assertFalse(result['checks']['fixed_window_complete'])

    def test_scene_departure_fails(self):
        rows=self.rows();rows[600]['stage']='MarioFaceShipGalaxy'
        self.assertFalse(map_observation(rows,'WorldMap01Galaxy',970)['checks']['no_scene_departure'])

    def test_input_fails(self):
        rows=self.rows();rows[600]['jump']='1'
        self.assertFalse(map_observation(rows,'WorldMap01Galaxy',970)['checks']['no_input'])

    def test_missing_map_fails(self):
        self.assertFalse(all(map_observation(self.rows(),'WorldMap02Galaxy',970)['checks'].values()))

    def test_nonmap_or_shorter_configuration_rejected(self):
        for stage,count in [('WorldMap09Galaxy',970),('YosshiHomeGalaxy',970),('WorldMap01Galaxy',969)]:
            with self.assertRaises(ValueError):map_observation(self.rows(),stage,count)


class TapeReplay(unittest.TestCase):
    def report(self):
        return {'mode':'whole-session','finished':True,'stopped':False,
                'anchor_policy':'interactive-and-play-ready-v1','interactive_position_bit_exact':True,
                'segments':[{'started':True,'completed':True,'diverged':False,'identity_rows_skipped':1,
                    'rows_compared':10,'tape_frames':10,'pointer_rows_compared':10,'max_position_error':0,
                    'identity_offset_checked':True,'identity_offset_passed':True,'identity_offset_limit':30,
                    'identity_frame_offset':1,'play_ready_required':True,
                    'play_ready_state_check':{'checked':True,'passed':True,'position_bit_exact':True,'rng':{'verified':True}}}]}

    def test_strict_whole_session(self):
        self.assertTrue(all(replay_checks(self.report()).values()))

    def test_missing_pointer_row_fails(self):
        data=self.report();data['segments'][0]['pointer_rows_compared']=9
        self.assertFalse(all(replay_checks(data).values()))

    def test_missing_checkpoint_rng_fails(self):
        data=self.report();data['segments'][0]['play_ready_state_check']['rng']={}
        self.assertFalse(all(replay_checks(data).values()))

    def test_boundary_over_bound_fails(self):
        data=self.report();data['segments'][0]['identity_frame_offset']=31
        self.assertFalse(all(replay_checks(data).values()))

    def test_position_difference_fails(self):
        data=self.report();data['segments'][0]['max_position_error']=0.001
        self.assertFalse(all(replay_checks(data).values()))


if __name__ == '__main__': unittest.main()
