import copy
import csv
import hashlib
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch
from PIL import Image
from kpad_script import contract
import dolphin_visual_contract as visual


def bind(path):return dict(path=str(path),sha256=visual.digest(path))


def fixture(root,origin=5):
    root.mkdir(parents=True,exist_ok=True)
    image=root/'synthetic.png';Image.new('RGB',(32,18),(40,110,170)).save(image)
    trace=root/'source.csv'
    with trace.open('w') as f:
        fields=['frame','scene_valid','player_valid','scene_epoch','scene','stage','scenario','init_state']
        writer=csv.DictWriter(f,fieldnames=fields);writer.writeheader()
        for frame in range(origin+617):
            ready=frame>=origin
            writer.writerow(dict(frame=frame,scene_valid=int(ready),player_valid=int(ready),scene_epoch=2 if ready else 1,
                                 scene='Game' if ready else 'Boot',stage='Synthetic',scenario=1,init_state=6 if ready else 0))
    canonical=root/'idle-tail.jsonl';actual=root/'actual-original.jsonl'
    def row(frame,channel):
        error=0 if channel==0 else -1
        return dict(frame=frame,read_ordinal=channel,channel=channel,requested_length=120,
                    returned_count=1 if channel==0 else 0,error=error,
                    connection=dict(probe_error=error,device_type=1 if channel==0 else 253),
                    samples=[contract.idle_sample(channel==0)])
    with canonical.open('w') as f:
        for frame in range(616):
            for channel in range(4):f.write(json.dumps(row(frame,channel))+'\n')
    with actual.open('w') as f:
        for frame in range(origin+617):
            for channel in range(4):f.write(json.dumps(row(frame,channel))+'\n')
    original=root/'original-closed.jsonl';original.write_bytes(actual.read_bytes())
    census=root/'read-census.json';census.write_text(json.dumps([dict(source_frame=i,reads=4) for i in range(origin+617)]))
    closure=root/'closed-receipt.json';closure.write_text(json.dumps(dict(status='closed',method='dolphin-actual-export',sha256=visual.digest(original),bytes=original.stat().st_size,read_count=(origin+617)*4,source_start=0,source_end=origin+616,census_sha256=visual.digest(census))))
    result=dict(synthetic=True,cpu_stop=dict(status='stopped',phase='post-endFrame',source_frame=origin+616,terminal_source_frame=origin+616,resume_after_stop_count=0,emulation_advanced_after_stop=False,input_frames=616,event=10000),stage='Synthetic',scenario=1,offset=60,tail_frames=16,
        sampling_definition='post-seam-request/next-observed-draw-v2',source_revision=visual.PINS['dolphin_revision'],
        executable_sha256='a'*64,instrumentation_manifest_sha256='b'*64,trace=bind(trace),input=bind(canonical),
        actual_input=bind(actual),input_export=dict(status='captured',sha256=visual.digest(actual),bytes=actual.stat().st_size,read_count=(origin+617)*4,original_raw_export=bind(original),closed_receipt=bind(closure),read_census=bind(census)),
        selected_draw_id=17,draw_coverage=dict(complete=True,request_source_frame=origin+60,request_event=100,next_source_frame=origin+61,next_seam_event=106,draws=[dict(
            id=17,after_source_seam=origin+60,before_source_seam=origin+61,entry_event=101,selected_scene_event=102,exit_event=103,entry_pc='804b6a90',exit_pc='804b6a94',
            selected_scene_pc='804ba798',guest_cpu_stopped=True,snapshot=dict(scene_valid=1,scene='Game',stage='Synthetic',
                placed_scenario=1,init_state=6,current_scene_nonnull=True,executed_scene_is_current=True,canonical_player_valid=True,
                observation_phase='before-GameSystem::draw',game_cpu_owned=True,scene_epoch=2,scene_epoch_kind='sampled-identity'))]),
        copy=dict(id=72,draw_id=17,kind='xfb',buffer_mode=3,current_render=True,lineage_complete=True,
                  observed_source_seam=origin+60,clear_before_draw_proven=True,efb_writers=['current-draw','endGX'],endGX_event=104,event=105,xfb_entry_id=600),
        selection=dict(reason='VideoInterface',duplicate=False,stitched=False,copy_ids=[72],xfb_entry_id=600,
                       settings_unchanged=True,originating_source_seam=origin+61,event=110,present_id=900),
        dump=dict(id=10,present_id=900,xfb_entry_id=600,copy_ids=[72],resolution_policy='XFBRawResolution',
                  includes_host_overlay=False,pending_overwrite_observed=False,staging_generation=4,event=111,source_rect=[0,0,32,18],target_rect=[0,0,32,18]),
        gpu=dict(dump_id=10,staging_generation=4,backend='Metal',flush_returned=True,map_success=True,error_status_observed=True,
                 completion_status='command-buffer-completed-no-error',event=112),image=bind(image),
        image_geometry=dict(kind='game-viewport',pixel_aspect_ratio=[1,1],display_aspect_ratio=[16,9],color_space='sRGB'),
        color_transfer='recorded-and-validated-srgb-code-values',
        png=dict(dump_id=10,checked_write=True,checked_flush=True,checked_close=True,atomic_publish=True,
                 sha256=visual.digest(image),width=32,height=18,event=115),
        counters=dict(loop=70,draw=17,update=68,vi=311,present=900,dump=10))
    result['selection_history']=dict(complete=True,events=[copy.deepcopy(result['selection'])])
    draw=result['draw_coverage']['draws'][0]
    draw['exit_snapshot']=dict(draw['snapshot'],observation_phase='after-GameSystem::draw')
    return result


class DolphinVisualContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory();cls.home=Path(cls.tmp.name).resolve()
        cls.base=fixture(cls.home/'MacioGalaxy2-gamedata')
    @classmethod
    def tearDownClass(cls):cls.tmp.cleanup()
    def review(self,e):
        with patch('dolphin_visual_contract.Path.home',return_value=self.home):return visual.review(e)
    def blocked(self,e):
        r=self.review(e);self.assertEqual(r['status'],'BLOCKED',r);self.assertIsNone(r['presentation_candidate'])
    def test_consistent_fixture_stays_proposal_only_with_distinct_counters(self):
        r=self.review(self.base);self.assertEqual(r['status'],'PROPOSAL_CHAIN_CONSISTENT',r)
        self.assertEqual(r['measurement_status'],'NOTMEASURED');self.assertEqual(r['qualification'],'BLOCKED')
        self.assertFalse(r['runtime_provider_accepted']);self.assertIsNone(r['qualified_frames'])
        self.assertEqual(r['presentation_candidate']['status'],'proposal-only')
        self.assertEqual(len(set(r['presentation_candidate']['counters'].values())),6)
        self.assertEqual(r['ready_origin']['source_frame'],5)
    def test_all_fixed_offsets_use_their_own_observed_request(self):
        for offset in (60,300,600):
            e=copy.deepcopy(self.base);e['offset']=offset;e['draw_coverage']['request_source_frame']=5+offset;e['draw_coverage']['next_source_frame']=6+offset
            e['draw_coverage']['draws'][0].update(after_source_seam=5+offset,before_source_seam=6+offset)
            e['copy']['observed_source_seam']=5+offset
            e['selection']['originating_source_seam']=6+offset;e['selection_history']['events']=[copy.deepcopy(e['selection'])]
            self.assertEqual(self.review(e)['status'],'PROPOSAL_CHAIN_CONSISTENT')
    def test_draw_exit_identity_change_is_not_hidden(self):
        e=copy.deepcopy(self.base);e['draw_coverage']['draws'][0]['exit_snapshot']['scene_epoch']=3;self.blocked(e)
    def test_ready_epoch_change_or_invalid_first_player_does_not_reanchor(self):
        for field,value in [('scene_epoch','3'),('player_valid','0')]:
            with tempfile.TemporaryDirectory(dir=self.home/'MacioGalaxy2-gamedata') as tmp:
                p=Path(tmp)/'source.csv'
                with Path(self.base['trace']['path']).open() as f:reader=csv.DictReader(f);fields=reader.fieldnames;rows=list(reader)
                rows[5][field]=value
                with p.open('w') as f:writer=csv.DictWriter(f,fieldnames=fields);writer.writeheader();writer.writerows(rows)
                e=copy.deepcopy(self.base);e['trace']=bind(p);self.blocked(e)
    def test_missing_image_producer_links_block(self):
        for field in ('draw_coverage','copy','selection','dump','gpu','png','actual_input','input_export'):
            e=copy.deepcopy(self.base);del e[field];self.blocked(e)
    def test_wrong_phase_offset_source_or_request_is_not_fit(self):
        for key,value in [('sampling_definition','same-frame'),('offset',61),('tail_frames',0),('source_revision','f'*40)]:
            e=copy.deepcopy(self.base);e[key]=value;self.blocked(e)
        e=copy.deepcopy(self.base);e['draw_coverage']['request_source_frame']+=1;self.blocked(e)
    def test_later_ready_draw_does_not_replace_actual_next_draw(self):
        e=copy.deepcopy(self.base);later=copy.deepcopy(e['draw_coverage']['draws'][0]);later.update(id=18,entry_event=120,selected_scene_event=121,exit_event=122)
        e['draw_coverage']['draws'].append(later);e['selected_draw_id']=18;self.blocked(e)
        e=copy.deepcopy(self.base);e['draw_coverage']['draws'][0]['snapshot']['executed_scene_is_current']=False;self.blocked(e)
    def test_draw_guard_or_order_unknown_is_not_observed_boundary(self):
        for key,value in [('entry_pc','804b6a94'),('guest_cpu_stopped',False),('selected_scene_pc','804ba794'),('selected_scene_event',100)]:
            e=copy.deepcopy(self.base);e['draw_coverage']['draws'][0][key]=value;self.blocked(e)
        e=copy.deepcopy(self.base);e['draw_coverage']['complete']=False;self.blocked(e)
    def test_epoch_or_selected_scenario_cannot_replace_placed_identity(self):
        for key,value in [('scene_epoch',3),('placed_scenario',2),('observation_phase','post-endFrame')]:
            e=copy.deepcopy(self.base);e['draw_coverage']['draws'][0]['snapshot'][key]=value;self.blocked(e)
    def test_copy_order_modes_and_mixed_efb_writes_block(self):
        for key,value in [('buffer_mode',2),('current_render',False),('event',102),('endGX_event',100),('lineage_complete',False),('efb_writers',['current-draw','foreign-draw']),('clear_before_draw_proven',False)]:
            e=copy.deepcopy(self.base);e['copy'][key]=value;self.blocked(e)
    def test_vi_duplicate_immediate_stitched_or_foreign_copy_blocks(self):
        for key,value in [('reason','Immediate'),('duplicate',True),('stitched',True),('copy_ids',[71,72]),('xfb_entry_id',601),('settings_unchanged',False)]:
            e=copy.deepcopy(self.base);e['selection'][key]=value;self.blocked(e)
    def test_later_selection_or_incomplete_history_is_not_fit(self):
        e=copy.deepcopy(self.base);earlier=copy.deepcopy(e['selection']);earlier.update(event=108,present_id=899,duplicate=True)
        e['selection_history']['events'].insert(0,earlier);self.blocked(e)
        e=copy.deepcopy(self.base);e['selection_history']['complete']=False;self.blocked(e)
    def test_pending_dump_overwrite_or_staging_reuse_blocks(self):
        e=copy.deepcopy(self.base);e['dump']['pending_overwrite_observed']=True;self.blocked(e)
        e=copy.deepcopy(self.base);e['gpu']['staging_generation']=5;self.blocked(e)
    def test_dump_counter_is_not_copy_id_or_request_id(self):
        for key,value in [('present_id',17),('xfb_entry_id',72),('copy_ids',[10]),('resolution_policy','WindowResolution'),('includes_host_overlay',True)]:
            e=copy.deepcopy(self.base);e['dump'][key]=value;self.blocked(e)
    def test_always_mapped_is_not_gpu_completion(self):
        for key,value in [('flush_returned',False),('error_status_observed',False),('completion_status','unknown'),('dump_id',11),('event',100)]:
            e=copy.deepcopy(self.base);e['gpu'][key]=value;self.blocked(e)
    def test_request_completed_or_file_exists_does_not_prove_png_close(self):
        for key,value in [('checked_close',False),('checked_flush',False),('checked_write',False),('atomic_publish',False),('width',33),('sha256','f'*64)]:
            e=copy.deepcopy(self.base);e['png'][key]=value;e['png']['screenshot_completed_event']=True;self.blocked(e)
    def test_color_or_crop_is_not_inferred(self):
        e=copy.deepcopy(self.base);e['color_transfer']='unknown';self.blocked(e)
        e=copy.deepcopy(self.base);e['dump']['source_rect']=[1,0,33,18];self.blocked(e)
    def test_actual_input_export_hash_and_read_count_required(self):
        for key,value in [('status','incomplete'),('read_count',1),('sha256','f'*64),('bytes',1)]:
            e=copy.deepcopy(self.base);e['input_export'][key]=value;self.blocked(e)
    def test_changed_actual_connection_is_delivery_failure_not_gameplay(self):
        with tempfile.TemporaryDirectory(dir=self.home/'MacioGalaxy2-gamedata') as tmp:
            p=Path(tmp)/'actual.jsonl';lines=Path(self.base['actual_input']['path']).read_text().splitlines()
            row=json.loads(lines[24]);row['connection']['probe_error']=-1;lines[24]=json.dumps(row);p.write_text('\n'.join(lines)+'\n')
            e=copy.deepcopy(self.base);e['actual_input']=bind(p);e['input_export'].update(sha256=visual.digest(p),bytes=p.stat().st_size)
            r=self.review(e);self.assertEqual(r['status'],'BLOCKED');self.assertIn('delivery differs',r['reasons'][0])
    def test_missing_actual_read_is_not_filled(self):
        with tempfile.TemporaryDirectory(dir=self.home/'MacioGalaxy2-gamedata') as tmp:
            p=Path(tmp)/'actual.jsonl';lines=Path(self.base['actual_input']['path']).read_text().splitlines();del lines[24];p.write_text('\n'.join(lines)+'\n')
            e=copy.deepcopy(self.base);e['actual_input']=bind(p);e['input_export'].update(sha256=visual.digest(p),bytes=p.stat().st_size,read_count=len(lines));self.blocked(e)
    def test_missing_original_prefix_cannot_be_hidden_by_new_export_hashes(self):
        for also_rebind_closure in (False,True):
            with tempfile.TemporaryDirectory(dir=self.home/'MacioGalaxy2-gamedata') as tmp:
                root=Path(tmp);e=copy.deepcopy(self.base)
                rows=[json.loads(line) for line in Path(e['actual_input']['path']).read_text().splitlines()]
                rows=[row for row in rows if row['frame']>=6]
                p=root/'prefix-deleted.jsonl';p.write_text(''.join(json.dumps(row)+'\n' for row in rows))
                e['actual_input']=bind(p);e['input_export'].update(sha256=visual.digest(p),bytes=p.stat().st_size,read_count=len(rows))
                if also_rebind_closure:
                    e['input_export']['original_raw_export']=bind(p)
                    receipt=json.loads(Path(e['input_export']['closed_receipt']['path']).read_text())
                    receipt.update(sha256=visual.digest(p),bytes=p.stat().st_size,read_count=len(rows))
                    closed=root/'changed-closure.json';closed.write_text(json.dumps(receipt));e['input_export']['closed_receipt']=bind(closed)
                r=self.review(e);self.assertEqual(r['status'],'BLOCKED')
                self.assertIn('census differs' if also_rebind_closure else 'original raw export',r['reasons'][0])
    def test_exact_tail_trace_and_cpu_stop_are_required(self):
        for extra in (False,True):
            with tempfile.TemporaryDirectory(dir=self.home/'MacioGalaxy2-gamedata') as tmp:
                e=copy.deepcopy(self.base);p=Path(tmp)/'source.csv'
                with Path(e['trace']['path']).open() as f:reader=csv.DictReader(f);fields=reader.fieldnames;rows=list(reader)
                if extra:
                    last=dict(rows[-1]);last['frame']=str(int(last['frame'])+1);rows.append(last)
                else:rows.pop()
                with p.open('w') as f:writer=csv.DictWriter(f,fieldnames=fields);writer.writeheader();writer.writerows(rows)
                e['trace']=bind(p);self.blocked(e)
        for key,value in [('terminal_source_frame',622),('resume_after_stop_count',1),('emulation_advanced_after_stop',True)]:
            e=copy.deepcopy(self.base);e['cpu_stop'][key]=value;self.blocked(e)
    def test_draw_or_vi_past_source_bound_is_not_gpu_drain(self):
        e=copy.deepcopy(self.base);e['draw_coverage']['draws'][0]['after_source_seam']=622;self.blocked(e)
        e=copy.deepcopy(self.base);e['selection']['originating_source_seam']=622;e['selection_history']['events']=[copy.deepcopy(e['selection'])];self.blocked(e)
    def test_gpu_drain_after_stop_is_allowed_without_emulation_advance(self):
        e=copy.deepcopy(self.base);e['gpu']['event']=11000;e['png']['event']=11001
        self.assertEqual(self.review(e)['status'],'PROPOSAL_CHAIN_CONSISTENT')
    def test_source_blob_and_dol_drift_fail_before_binding(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);(root/'source.cpp').write_text('synthetic source');(root/'symbols').write_text('synthetic symbols')
            b=bytearray(260);struct.pack_into('>I',b,0,256);struct.pack_into('>I',b,0x48,0x80001000);struct.pack_into('>I',b,0x90,4);b[256:260]=bytes.fromhex('48000001');(root/'game.dol').write_bytes(b)
            pins=dict(dolphin_revision='synthetic',files={'source.cpp':visual.digest(root/'source.cpp')},retail=dict(dol_sha256=visual.digest(root/'game.dol'),symbols_sha256=visual.digest(root/'symbols'),guards={'site':dict(address='80001000',word='48000001')}))
            with patch.object(visual,'PINS',pins):
                self.assertFalse(visual.audit_sources(root,root/'game.dol',root/'symbols')['runtime_observed'])
                (root/'source.cpp').write_text('changed')
                with self.assertRaisesRegex(ValueError,'source drift'):visual.audit_sources(root,root/'game.dol',root/'symbols')
                (root/'source.cpp').write_text('synthetic source');(root/'game.dol').write_bytes(b'bad')
                with self.assertRaisesRegex(ValueError,'DOL hash'):visual.audit_sources(root,root/'game.dol',root/'symbols')


if __name__=='__main__':unittest.main()
