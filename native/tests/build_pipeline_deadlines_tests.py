#!/usr/bin/env python3
import csv
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import build_pipeline_deadlines as builder
import pipeline_deadlines as deadlines
import seedpack


class Build(unittest.TestCase):
    def setUp(self):
        self.stage = 'A'
        self.tmp = tempfile.TemporaryDirectory(); self.root = Path(self.tmp.name)
        self.seeds = self.root / 'seeds'; self.seeds.mkdir()
        import sqlite3
        self.rows = [(1, 11, 13, 3, b'abc', 82), (1, 12, 13, 3, b'def', 83)]
        for name in ('__global__', 'A'):
            with sqlite3.connect(self.seeds / (name + '.db')) as db:
                db.executescript(seedpack.SCHEMA); db.execute('INSERT INTO aurora_schema VALUES(1)')
                db.executemany('INSERT INTO pipeline_cache VALUES(?,?,?,?,?,?)', self.rows)
            db.close()
        self.batch = self.root / 'batch'; self.batch.mkdir()
        self.arms = []
        for index in (1, 2):
            name = f'warm-{index}'; folder = self.batch / 'A' / name; folder.mkdir(parents=True)
            capture = [{'event': 'header', 'version': 3, 'gx_config_version': 13,
                        'clock': 'active-frame-packet-id', 'source': 'find_pipeline-before-memo', 'origin_policy': 'next-whole-packet'},
                       {'event': 'stage_begin', 'stage': 'A', 'visit': 1, 'request_packet': 479, 'origin_frame': 480}]
            capture += [{'event': 'first_use', 'stage': 'A', 'visit': 1, 'origin_frame': 480, 'frame': 480+n,
                         'relative_frame': n, 'config': f'{key:016x}'} for n, key in enumerate((11, 12))]
            capture += [{'event': 'end', 'complete': True, 'records': 2, 'clock_first_frame': 480,
                         'clock_last_frame': 490, 'clock_transitions': 1, 'lookup_calls': 2,
                         'stage_requests': 1, 'stage_begins': 1, 'pending_stage_requests': 0}]
            (folder / 'first-use.jsonl').write_text(''.join(json.dumps(r)+'\n' for r in capture))
            (folder / 'app.log').write_text('[input-script] done after 970 frames\n')
            (folder / 'frames.csv').write_text('frame,pipeline_wait_ms\n1,0.000\n')
            (folder / 'route.tape.json').write_text(json.dumps({'complete':True,'seams':491,
                'ready_seams':[{'stage':'A','scenario':1,'row':480}],
                'play_ready_seams':[{'stage':'A','scenario':1,'row':484}]}))
            (folder / 'launch.json').write_text(json.dumps({'exe_sha256': '4'*64, 'renderer_key': '3'*64, 'environment': {}}))
            self.arms.append({'passed': True, 'name': name, 'recording_exe_sha256': '4'*64, 'renderer_key': '3'*64,
                              'checks': dict(valid_capture=True, no_pipeline_waits=True, no_pop_in=True)})
            self.rehash(index)
        self.data = {'version': 1, 'recording_only': True, 'stages_planned': ['A'],
                     'recording_build_metadata': {'source': '1'*40, 'binary_sha256': '2'*64, 'renderer_key': '3'*64},
                     'recording_build_metadata_sha256': 'unit-metadata', 'stage_results': [
                         {'stage': 'A', 'passed': True, 'warm_disagreements': {}, 'cold_disagreements': None, 'arms': self.arms}]}
        self.source_pack = self.root / 'v13.xz'
        self.bind_pack()

    def bind_pack(self):
        seedpack.pack(str(self.seeds), str(self.source_pack))
        self.data['recording_build_metadata']['seed_pack_sha256'] = builder.sha(self.source_pack)

    def tearDown(self): self.tmp.cleanup()

    def rehash(self, index):
        arm = self.arms[index-1]; folder = self.batch / self.stage / arm['name']
        for name, field in [('first-use.jsonl', 'capture_sha256'), ('app.log', 'app_log_sha256'), ('launch.json', 'launch_sha256')]:
            arm[field] = builder.sha(folder / name)

    def run_build(self, expected=None, require_cold=False, fallback=()):
        path = self.batch / 'PROVENANCE.json'; path.write_text(json.dumps(self.data))
        return builder.build(self.seeds, [path], self.root / 'out', self.root / 'report.json', expected,
                             source_pack=self.source_pack, require_cold_confirmation=require_cold, fallback_stages=fallback)

    def test_exact_pair(self):
        result = self.run_build(); self.assertTrue(result['passed'])
        self.assertEqual(deadlines.read(self.root / 'out/A.first-use.bin', 'A', self.rows), {11: 0, 12: 1})
        self.assertTrue(result['all_database_rows_unchanged'])

    def test_edited_raw(self):
        (self.batch / 'A/warm-2/app.log').write_text('changed')
        self.assertFalse(self.run_build()['passed'])

    def test_raw_wait_overrides_claim(self):
        (self.batch / 'A/warm-2/frames.csv').write_text('frame,pipeline_wait_ms\n1,0.001\n')
        self.assertFalse(self.run_build()['passed'])

    def test_disagreement_overrides_claim(self):
        path = self.batch / 'A/warm-2/first-use.jsonl'
        rows = [json.loads(line) for line in path.read_text().splitlines()]
        rows[3].update(frame=482, relative_frame=2)
        path.write_text(''.join(json.dumps(r)+'\n' for r in rows)); self.rehash(2)
        result = self.run_build(); self.assertFalse(result['passed'])
        self.assertTrue(result['stage_results'][0]['disagreements'])

    def test_unknown_config(self):
        for index in (1, 2):
            path = self.batch / f'A/warm-{index}/first-use.jsonl'
            path.write_text(path.read_text().replace('000000000000000c', '00000000000003e7')); self.rehash(index)
        self.assertFalse(self.run_build()['passed'])

    def test_partial_batch_cannot_pass_complete_sweep(self):
        result = self.run_build(['A', 'B'])
        self.assertFalse(result['passed'])
        self.assertEqual(result['unrecorded_stages'], ['B'])
        self.assertEqual(result['missing_or_failed_stages'], ['B'])

    def test_renderer_must_match_recording_build(self):
        self.data['recording_build_metadata']['renderer_key'] = '5'*64
        self.assertFalse(self.run_build()['passed'])

    def test_additive_global_closure_preserves_every_source_row(self):
        import sqlite3
        extra = (1, 13, 13, 3, b'new', 900)
        with sqlite3.connect(self.seeds / '__global__.db') as db:
            db.execute('INSERT INTO pipeline_cache VALUES(?,?,?,?,?,?)', extra)
        db.close()
        self.bind_pack()
        for index in (1, 2):
            path = self.batch / f'A/warm-{index}/first-use.jsonl'
            path.write_text(path.read_text().replace('000000000000000c', '000000000000000d')); self.rehash(index)
        result = self.run_build()
        self.assertTrue(result['passed'])
        self.assertFalse(result['all_database_rows_unchanged'])
        self.assertTrue(result['all_existing_rows_and_ranks_byte_identical'])
        self.assertEqual(set(seedpack.database_rows(str(self.root / 'out/A.db'))), set(self.rows + [extra]))
        self.assertEqual(result['stage_results'][0]['manifest_closure']['added_count'], 1)
        pack = str(self.root / 'v14.xz')
        seedpack.pack(str(self.root / 'out'), pack)
        self.assertEqual(seedpack.check(pack), [])
        seedpack.expand(pack, str(self.root / 'expanded'))
        self.assertEqual(seedpack.verify(str(self.root / 'out'), str(self.root / 'expanded')), 0)

    def test_seed_rank_edit_rejected_against_original_pack(self):
        import sqlite3
        with sqlite3.connect(self.seeds / 'A.db') as db:
            db.execute('UPDATE pipeline_cache SET first_frame_used=99 WHERE hash=11')
        db.close()
        with self.assertRaisesRegex(ValueError, 'rows/ranks differ'):
            self.run_build()

    def test_pack_must_match_recording_build(self):
        self.data['recording_build_metadata']['seed_pack_sha256'] = '0'*64
        with self.assertRaisesRegex(ValueError, 'differs from recording build'):
            self.run_build()

    def test_full_pack_requires_a_raw_cold_confirmation(self):
        result = self.run_build(require_cold=True)
        self.assertFalse(result['passed'])
        self.assertTrue(result['cold_confirmation_required'])
        self.assertEqual(result['cold_confirmed_stages'], [])

    def test_raw_cold_disagreement_overrides_summary(self):
        import shutil
        folder = self.batch / 'A/cold-confirm'
        shutil.copytree(self.batch / 'A/warm-1', folder)
        arm = dict(self.arms[0], name='cold-confirm', cold_confirmation=True)
        self.arms.append(arm)
        launch = json.loads((folder / 'launch.json').read_text()); launch['cold_namespace'] = True
        (folder / 'launch.json').write_text(json.dumps(launch))
        path = folder / 'first-use.jsonl'
        rows = [json.loads(line) for line in path.read_text().splitlines()]
        rows[3].update(frame=482, relative_frame=2)
        path.write_text(''.join(json.dumps(r)+'\n' for r in rows)); self.rehash(3)
        result = self.run_build()
        self.assertFalse(result['passed'])
        self.assertIn('raw cold confirmation phase/offset groups differ', result['stage_results'][0]['reason'])

    def test_raw_cold_equal_pair_admitted(self):
        import shutil
        folder = self.batch / 'A/cold-confirm'
        shutil.copytree(self.batch / 'A/warm-1', folder)
        self.arms.append(dict(self.arms[0], name='cold-confirm', cold_confirmation=True))
        launch = json.loads((folder / 'launch.json').read_text()); launch['cold_namespace'] = True
        (folder / 'launch.json').write_text(json.dumps(launch)); self.rehash(3)
        result = self.run_build(require_cold=True)
        self.assertTrue(result['passed'])
        self.assertEqual(result['stage_results'][0]['cold_confirmation_checks'][0]['disagreements'], {})

    def test_explicit_fallback_preserves_original_failure(self):
        self.data['stage_results'][0]['passed']=False
        result=self.run_build(fallback=['A'])
        self.assertTrue(result['passed'])
        self.assertFalse(result['all_requested_deadlines_qualified'])
        self.assertEqual(result['missing_or_failed_stages'],['A'])
        self.assertEqual(result['explicit_whole_stage_fallbacks'],['A'])
        self.assertFalse(result['stage_results'][0]['admitted'])
        self.assertFalse((self.root/'out/A.first-use.bin').exists())

    def test_unrecorded_stage_cannot_be_explicit_fallback(self):
        with self.assertRaisesRegex(ValueError,'attempted, unqualified'):
            self.run_build(expected=['A','B'],fallback=['B'])

    def test_planned_unstarted_stage_does_not_count_as_attempt(self):
        self.data['stages_planned']=['A','B']
        result=self.run_build(expected=['A','B'])
        self.assertEqual(result['attempted_stages'],['A'])
        self.assertEqual(result['unrecorded_stages'],['B'])

    def test_planned_unstarted_stage_cannot_be_waived(self):
        self.data['stages_planned']=['A','B']
        with self.assertRaisesRegex(ValueError,'attempted, unqualified'):
            self.run_build(expected=['A','B'],fallback=['B'])

    def test_cold_native_clock_qualification_keeps_original_fail(self):
        import shutil
        folder=self.batch/'A/cold-confirm';shutil.copytree(self.batch/'A/warm-1',folder)
        arm=dict(self.arms[0],name='cold-confirm',cold_confirmation=True);self.arms.append(arm)
        launch=json.loads((folder/'launch.json').read_text());launch['cold_namespace']=True
        (folder/'launch.json').write_text(json.dumps(launch))
        path=folder/'first-use.jsonl';rows=[json.loads(l) for l in path.read_text().splitlines()]
        rows[3]['frame']+=27;rows[3]['relative_frame']+=27;rows[-1]['clock_last_frame']+=27
        path.write_text(''.join(json.dumps(r)+'\n' for r in rows));self.rehash(3)
        path=folder/'route.tape.json';side=json.loads(path.read_text());side['seams']+=27
        side['ready_seams'][0]['row']+=27;side['play_ready_seams'][0]['row']+=27;path.write_text(json.dumps(side))
        self.data['stage_results'][0].update(passed=False,cold_disagreements={'000000000000000c':[1,28]})
        result=self.run_build(require_cold=True)
        self.assertTrue(result['passed']);item=result['stage_results'][0]
        self.assertFalse(item['raw_recording_passed']);self.assertTrue(item['cold_definition_requalified'])
        q=item['cold_confirmation_checks'][0]['warm_pair_qualifications']
        self.assertEqual(len(q),2)
        self.assertEqual(len(q[0]['raw_absolute_disagreements']),1)
        self.assertEqual(q[0]['exact_phase_offset_disagreements'],{})

    def test_warm_native_clock_qualification_retains_failed_raw_pair(self):
        folder=self.batch/'A/warm-2';path=folder/'first-use.jsonl'
        rows=[json.loads(line) for line in path.read_text().splitlines()]
        rows[3]['frame']+=27;rows[3]['relative_frame']+=27;rows[-1]['clock_last_frame']+=27
        path.write_text(''.join(json.dumps(row)+'\n' for row in rows));self.rehash(2)
        path=folder/'route.tape.json';side=json.loads(path.read_text());side['seams']+=27
        side['ready_seams'][0]['row']+=27;side['play_ready_seams'][0]['row']+=27;path.write_text(json.dumps(side))
        self.data['stage_results'][0].update(passed=False,warm_disagreements={'000000000000000c':[1,28]})
        result=self.run_build();self.assertTrue(result['passed']);item=result['stage_results'][0]
        self.assertFalse(item['raw_recording_passed']);self.assertTrue(item['warm_definition_requalified'])
        self.assertTrue(item['warm_packet_order_preserved'])
        self.assertEqual(deadlines.read(self.root/'out/A.first-use.bin','A',self.rows),{11:0,12:1})

    def test_warm_missing_native_anchor_fails_closed(self):
        path=self.batch/'A/warm-2/route.tape.json';side=json.loads(path.read_text())
        side['play_ready_seams']=[];path.write_text(json.dumps(side))
        self.assertFalse(self.run_build()['passed'])

    def test_native_order_proof_preserves_ties(self):
        with self.assertRaisesRegex(ValueError,'group order'):
            builder.recording.prove_order({'a':1,'b':1},{'a':('scene',0),'b':('scene',1)})

    def test_retry_cannot_replace_success(self):
        path=self.batch/'PROVENANCE.json';path.write_text(json.dumps(self.data))
        with self.assertRaisesRegex(ValueError,'already admitted'):
            builder.build(self.seeds,[path,path],self.root/'out',self.root/'report.json',source_pack=self.source_pack)

    def test_retry_keeps_failed_original_visible(self):
        import shutil
        retry=self.root/'retry';shutil.copytree(self.batch,retry)
        success=retry/'PROVENANCE.json';success.write_text(json.dumps(self.data))
        self.data['stage_results'][0]['passed']=False
        original=self.batch/'PROVENANCE.json';original.write_text(json.dumps(self.data))
        result=builder.build(self.seeds,[original,success],self.root/'out',self.root/'report.json',source_pack=self.source_pack)
        self.assertTrue(result['passed'])
        self.assertEqual([r['admitted'] for r in result['stage_results']],[False,True])
        self.assertEqual(result['stage_results'][1]['previous_failed_attempts'],[str(original.resolve())])

    def prepare_map(self):
        self.stage='WorldMap01Galaxy'
        (self.seeds/'A.db').rename(self.seeds/(self.stage+'.db'))
        (self.batch/'A').rename(self.batch/self.stage)
        self.data.update(mode='no-input-map',map_frames=970,stages_planned=[self.stage])
        self.data['stage_results'][0]['stage']=self.stage
        for index,arm in enumerate(self.arms,1):
            folder=self.batch/self.stage/arm['name']
            capture=folder/'first-use.jsonl';capture.write_text(capture.read_text().replace('"A"','"'+self.stage+'"'))
            (folder/'app.log').write_text(''.join(f'[gx stage config] stage={self.stage} config={key:016x} phase=menu\n' for key in (11,12))+'Petari: quitting; close the window again to quit immediately\n')
            launch=json.loads((folder/'launch.json').read_text());launch.update(mode='no-input-map',map_frames=970)
            (folder/'launch.json').write_text(json.dumps(launch))
            rows=[{'frame':str(n),'stage':self.stage if n>=479 else 'FileSelect','scenario':'1','valid':'1','jump':'0'} for n in range(1450)]
            with (folder/'route.csv').open('w') as file:
                writer=csv.DictWriter(file,fieldnames=rows[0]);writer.writeheader();writer.writerows(rows)
            (folder/'route.tape.json').write_text(json.dumps({'complete':True,'ready_seams':[{'stage':self.stage,'scenario':1,'row':480}]}))
            (folder/'orderly-quit.json').write_text(json.dumps({'exit':0}))
            arm.update(quit_error=None,map_quit_request=builder.recording.map_observation(rows,self.stage,970),
                       route_csv_sha256=builder.sha(folder/'route.csv'),route_sidecar_sha256=builder.sha(folder/'route.tape.json'),
                       orderly_quit_sha256=builder.sha(folder/'orderly-quit.json'))
            self.rehash(index)
        self.bind_pack()

    def test_title_closure_omits_uncovered_global_hashes_and_quit_only_addition(self):
        import sqlite3
        self.stage='FileSelect'
        (self.seeds/'A.db').rename(self.seeds/'FileSelect.db');(self.batch/'A').rename(self.batch/'FileSelect')
        with sqlite3.connect(self.seeds/'__global__.db') as db:
            db.executemany('INSERT INTO pipeline_cache VALUES(?,?,?,?,?,?)',[(1,13,13,3,b'new',900),(1,14,13,3,b'end',901)])
        db.close();self.data.update(mode='no-input-title',stages_planned=['FileSelect'])
        case=self.data['stage_results'][0];case.update(stage='FileSelect',passed=False,warm_disagreements={'000000000000000e':[3,4]})
        for index in (1,2):
            folder=self.batch/self.stage/f'warm-{index}';path=folder/'first-use.jsonl'
            rows=[json.loads(l) for l in path.read_text().replace('"A"','"FileSelect"').splitlines()]
            rows[3]['config']='000000000000000d'
            rows[4:4]=[dict(rows[3],config='4562580ba2df567a',frame=482,relative_frame=2),
                       dict(rows[3],config='000000000000000e',frame=482+index,relative_frame=2+index)]
            rows[-1].update(records=4,lookup_calls=5)
            path.write_text(''.join(json.dumps(r)+'\n' for r in rows))
            (folder/'app.log').write_text(''.join(f'[gx stage config] stage=FileSelect config={k:016x} phase=menu\n' for k in (11,13,int('4562580ba2df567a',16)))+
                'Petari: quitting; close the window again to quit immediately\n[gx stage config] stage=FileSelect config=000000000000000e phase=menu\n')
            launch=json.loads((folder/'launch.json').read_text());launch['mode']='no-input-title'
            (folder/'launch.json').write_text(json.dumps(launch));self.rehash(index)
        self.bind_pack();result=self.run_build(expected=[]);item=result['stage_results'][0]
        self.assertTrue(item['admitted']);self.assertEqual(item['manifest_closure']['added_hashes'],['000000000000000d'])
        self.assertEqual(item['uncovered_configs_absent_global'],['4562580ba2df567a'])
        after=seedpack.database_rows(str(self.root/'out/FileSelect.db'))
        self.assertEqual(deadlines.read(self.root/'out/FileSelect.first-use.bin','FileSelect',after),{11:0,13:1})
        self.assertNotIn(14,[r[1] for r in after])

    def test_title_unapproved_global_gap_fails_closed(self):
        with self.assertRaisesRegex(ValueError,'unapproved uncovered'):
            builder.title_covered_values(self.rows,{11:0,999:1})

    def test_explicit_map_observation(self):
        self.prepare_map()
        self.assertTrue(self.run_build()['passed'])

    def test_map_post_quit_sets_must_match_even_when_frames_match(self):
        self.prepare_map()
        folder=self.batch/self.stage/'warm-2';path=folder/'app.log'
        lines=path.read_text().splitlines();path.write_text('\n'.join([lines[0],lines[2],lines[1]])+'\n');self.rehash(2)
        result=self.run_build();self.assertFalse(result['passed'])
        self.assertIn('post-quit first-use sets differ',result['stage_results'][0]['reason'])

    def test_map_missing_native_quit_marker_fails(self):
        self.prepare_map();path=self.batch/self.stage/'warm-1/app.log'
        path.write_text(path.read_text().split('Petari: quitting')[0]);self.rehash(1)
        self.assertFalse(self.run_build()['passed'])

    def test_map_duplicate_config_binding_fails(self):
        self.prepare_map();path=self.batch/self.stage/'warm-1/app.log'
        path.write_text(path.read_text()+path.read_text().splitlines()[0]+'\n');self.rehash(1)
        self.assertFalse(self.run_build()['passed'])

    def test_map_duplicate_native_quit_marker_fails(self):
        self.prepare_map();path=self.batch/self.stage/'warm-1/app.log'
        path.write_text(path.read_text()+'Petari: quitting\n');self.rehash(1)
        self.assertFalse(self.run_build()['passed'])

    def test_shutdown_bound_interval_is_not_an_exact_packet_claim(self):
        self.prepare_map();folder=self.batch/self.stage/'warm-1'
        values,report=builder.recording.shutdown_observations(folder/'first-use.jsonl',folder/'app.log',self.stage)
        self.assertEqual(len(values),2);self.assertEqual(report['packet_interval_inclusive'],[481,490])
        self.assertFalse(report['exact_quit_packet_known'])

    def test_map_raw_input_overrides_passed_summary(self):
        self.prepare_map();folder=self.batch/self.stage/'warm-2';path=folder/'route.csv'
        with path.open() as file:rows=list(csv.DictReader(file))
        rows[700]['jump']='1'
        with path.open('w') as file:
            writer=csv.DictWriter(file,fieldnames=rows[0]);writer.writeheader();writer.writerows(rows)
        self.arms[1]['route_csv_sha256']=builder.sha(path)
        self.assertFalse(self.run_build()['passed'])

    def test_map_needs_actual_ready_scene(self):
        self.prepare_map();path=self.batch/self.stage/'warm-2/route.tape.json'
        path.write_text(json.dumps({'complete':True,'ready_seams':[]}))
        self.arms[1]['route_sidecar_sha256']=builder.sha(path)
        self.assertFalse(self.run_build()['passed'])

    def test_map_failed_orderly_quit_rejected(self):
        self.prepare_map();path=self.batch/self.stage/'warm-2/orderly-quit.json'
        path.write_text(json.dumps({'exit':1}));self.arms[1]['orderly_quit_sha256']=builder.sha(path)
        self.assertFalse(self.run_build()['passed'])

    def test_equal_unknown_maps_do_not_hide_missing_bootstrap_scope(self):
        for index in (1,2):
            path = self.batch / f'A/warm-{index}/first-use.jsonl'
            rows = [json.loads(line) for line in path.read_text().splitlines()]
            for row in rows[1:-1]: row['visit'] = 2
            rows[1:1] = [
                {'event':'stage_begin','stage':'Unknown','visit':1,'request_packet':5,'origin_frame':6},
                {'event':'first_use','stage':'Unknown','visit':1,'origin_frame':6,'frame':6,'relative_frame':0,'config':'000000000000000b'}]
            rows[-1].update(records=3,clock_first_frame=6,clock_transitions=2,lookup_calls=3,stage_requests=2,stage_begins=2)
            path.write_text(''.join(json.dumps(row)+'\n' for row in rows));self.rehash(index)
        result = self.run_build()
        self.assertTrue(result['passed']) # target A remains independently qualified
        unknown = next(r for r in result['frontend_results'] if r['stage']=='Unknown')
        self.assertFalse(unknown['admitted'])
        self.assertFalse(unknown['bootstrap_scope_qualified'])
        self.assertFalse((self.root / 'out/Unknown.first-use.bin').exists())


if __name__ == '__main__': unittest.main()
