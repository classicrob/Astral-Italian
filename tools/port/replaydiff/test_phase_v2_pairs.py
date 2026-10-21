import copy
import csv
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
from PIL import Image
from test_dolphin_visual_contract import fixture,bind
import phase_v2_pairs as pairs


def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n');return bind(path)


def provider(root,role,origin):
    e=fixture(root,origin=origin)
    if role=='native':
        # This is synthetic proposed producer evidence, not installed instrumentation.
        closure=json.loads(Path(e['input_export']['closed_receipt']['path']).read_text())
        closure['method']='native-consumed-read-log'
        e['input_export']['closed_receipt']=write(root/'closed-receipt.json',closure)
        d=e['draw_coverage']['draws'][0]
        d.update(entry_hook='after-beginRender/before-GameSystem::draw',exit_hook='after-GameSystem::draw/before-endRender')
        e['copy'].update(kind='display',fifo_tag=e['copy']['id'])
        resolve=dict(copy_id=72,resource_id='texture-local',resource_generation=1,resource_revision=8,
                     command_buffer_id='buffer-local',operation_index=10,renderer_packet_id=333)
        e.update(resolve=resolve,resolve_coverage_complete=True,resource_writes=[copy.deepcopy(resolve)],
                 readback_encode=dict(resolve,operation_index=20,width=32,height=18),
                 submit=dict(id=401,command_buffer_id='buffer-local',success=True,validation_scope_result='no-error',device_lost=False),
                 mapped=dict(id=501,submit_id=401,copy_id=72,renderer_packet_id=333,success=True,callback_completed=True,width=32,height=18))
        e['selection']=dict(copy_id=72,resource_id='texture-local',resource_generation=1,resource_revision=8,policy='latest',
                            event=110,observed_source_seam=origin+61,vi_configured=True,vi_black=False,vi_dimmed=False,
                            free_aspect=False,includes_chrome=False,includes_overlay=False,rml_replaces_game=False,
                            image_geometry=e['image_geometry'],color_encoding='srgb-code-values-preserved')
        e['selection_history']=dict(complete=True,events=[copy.deepcopy(e['selection'])])
        e['png']=dict(readback_id=501,copy_id=72,write_success=True,flush_success=True,close_success=True,
                      verified_temp_bytes=True,published_inode_verified=True,temp_link_removed=True,
                      publication_method='same-directory-no-replace-link',sha256=e['image']['sha256'],width=32,height=18)
    source_file=root/'synthetic-source.cpp';source_file.write_text('synthetic proposed provider; no runtime hooks\n')
    source=write(root/'source-manifest.json',dict(semantic_base_revision=pairs.NATIVE_PINS['revision'] if role=='native' else pairs.shared.PINS['dolphin_revision'],source_revision=('c' if role=='native' else 'd')*40,files=[bind(source_file)]))
    exe=root/'synthetic-executable';exe.write_bytes(b'synthetic non-executable identity only')
    inst=write(root/'instrumentation.json',dict(phase_contract=pairs.CONTRACT,hooks={name:dict(site='synthetic-'+name,source_file=bind(source_file)) for name in ('draw_entry','draw_exit','copy_tag','selection','readback','publication')}))
    build=write(root/'build.json',dict(status='fixture-only',source_manifest_sha256=source['sha256'],executable_sha256=bind(exe)['sha256'],instrumentation_manifest_sha256=inst['sha256']))
    calibration=write(root/'color-validation.json',dict(role=role,status='fixture-only',method='rgb-code-ramp-and-transfer',source_manifest_sha256=source['sha256'],executable_sha256=bind(exe)['sha256'],image_geometry=e['image_geometry'],color_transfer=e['color_transfer']))
    e['color_validation']=calibration
    if role=='native':
        with Path(e['trace']['path']).open() as f:reader=csv.DictReader(f);fields=reader.fieldnames;rows=list(reader)
        for row in rows:
            if row['scene']=='Game':row['scene_epoch']='9'
        trace=root/'source.csv'
        with trace.open('w') as f:writer=csv.DictWriter(f,fieldnames=fields);writer.writeheader();writer.writerows(rows)
        e['trace']=bind(trace)
        e['draw_coverage']['draws'][0]['snapshot']['scene_epoch']=9
        e['draw_coverage']['draws'][0]['exit_snapshot']['scene_epoch']=9
    origin_record=dict(phase='post-endFrame',stage='Synthetic',scenario=1,source_frame=origin,source_scene_epoch=9 if role=='native' else 2,player_valid=1)
    e['ready_origin']=origin_record
    groups={**pairs.COMMON,**pairs.ROLE_GROUPS[role]}
    artifacts={name:write(root/(name+'.json'),{key:e[key] for key in keys}) for name,keys in groups.items()}
    prod=write(root/'producer.json',dict(role=role,status='fixture-only',synthetic=True,host='synthetic-'+role,run_id='fixture-'+role,
        source_manifest=source,executable=bind(exe),instrumentation=inst,build_receipt=build,
        selection_policy='latest' if role=='native' else 'VI',clock=dict(status='unavailable',reason='synthetic clocks intentionally unknown'),
        counters=e['counters']))
    return dict(producer=prod,artifacts=artifacts)


class PhaseV2Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory();cls.home=Path(cls.tmp.name).resolve()
        cls.root=cls.home/'MacioGalaxy2-gamedata';cls.root.mkdir()
        cls.base=dict(schema_version=2,contract=pairs.CONTRACT,phase_contract_status='proposed',synthetic=True,
            pairs=[dict(stage='Synthetic',scenario=1,offset=60,native=provider(cls.root/'native','native',50),dolphin=provider(cls.root/'dolphin','dolphin',5))])
        cls.case=0
    @classmethod
    def tearDownClass(cls):cls.tmp.cleanup()
    def run_case(self,data):
        type(self).case+=1;root=self.root/f'case-{self.case}';root.mkdir()
        manifest=root/'manifest.json';write(manifest,data)
        pin=write(root/'operator-pin.json',dict(schema_version=1,scope='operator-integrity-pin',contract=pairs.CONTRACT,manifest_sha256=bind(manifest)['sha256']))
        with patch('phase_v2_pairs.Path.home',return_value=self.home):return pairs.compare_manifest(manifest,root/'report',pin['path'],pin['sha256'])
    def changed(self,data,role,group,mutate):
        type(self).case+=1;path=self.root/f'mutated-{self.case}.json'
        record=data['pairs'][0][role]['artifacts'][group];value=json.loads(Path(record['path']).read_text());mutate(value)
        data['pairs'][0][role]['artifacts'][group]=write(path,value)
    def blocked(self,data,category=None):
        row=self.run_case(data)['results'][0]
        self.assertEqual(row['measurement_status'],'NOTMEASURED',row);self.assertNotIn('metrics',row)
        if category:self.assertEqual(row['failure_category'],category,row)
        return row
    def test_consistent_pair_metrics_remain_proposal_only_with_independent_origins(self):
        r=self.run_case(self.base);row=r['results'][0]
        self.assertEqual(row['measurement_status'],'SYNTHETIC_METRIC_ONLY',row)
        self.assertEqual(row['status'],'PROPOSED_CONTENT_PAIR');self.assertEqual(row['metrics']['visual_distance'],0)
        self.assertEqual(r['qualification'],'BLOCKED');self.assertIsNone(r['qualified_frames']);self.assertFalse(r['runtime_provider_accepted'])
        self.assertEqual(row['providers']['native']['ready_origin']['source_frame'],50)
        self.assertEqual(row['providers']['dolphin']['ready_origin']['source_frame'],5)
        self.assertNotEqual(row['providers']['native']['sampled_epoch']['value'],row['providers']['dolphin']['sampled_epoch']['value'])
        self.assertNotEqual(row['providers']['native']['requested_state'],row['providers']['native']['drawn_state'])
        self.assertFalse(row['providers']['native']['drawn_state']['full_draw_state_observed'])
        self.assertIsNone(row['providers']['native']['sampled_epoch']['authoritative_lifecycle_id'])
        self.assertEqual(len(r['results']),3);self.assertEqual(r['results'][1]['measurement_status'],'NOTMEASURED')
    def test_legacy_or_approval_waiver_manifest_rejected(self):
        for key,value in [('schema_version',1),('contract','legacy'),('phase_contract_status','approved')]:
            e=copy.deepcopy(self.base);e[key]=value
            with self.assertRaises(ValueError):self.run_case(e)
    def test_missing_separately_pinned_provider_artifact_blocks(self):
        for group in pairs.COMMON:
            e=copy.deepcopy(self.base);del e['pairs'][0]['native']['artifacts'][group];self.blocked(e,'PROVIDER_PROVENANCE')
    def test_binary_source_build_or_color_file_drift_is_not_accepted(self):
        for key in ('executable','source_manifest','build_receipt'):
            e=copy.deepcopy(self.base);p=json.loads(Path(e['pairs'][0]['native']['producer']['path']).read_text())
            p[key]['sha256']='f'*64
            path=self.root/f'bad-{key}.json';e['pairs'][0]['native']['producer']=write(path,p);self.blocked(e,'PROVIDER_PROVENANCE')
        e=copy.deepcopy(self.base);self.changed(e,'native','color',lambda x:x.update(color_transfer='UNKNOWN'));self.blocked(e,'PROVIDER_PROVENANCE')
    def test_shifted_ready_anchor_or_later_draw_is_not_fit(self):
        e=copy.deepcopy(self.base);self.changed(e,'native','ready',lambda x:x['ready_origin'].update(source_frame=51));self.blocked(e)
        e=copy.deepcopy(self.base);self.changed(e,'dolphin','draw',lambda x:x.update(selected_draw_id=18));self.blocked(e)
    def test_ready_rng_cannot_be_substituted_for_unknown_source_observation(self):
        e=copy.deepcopy(self.base)
        self.changed(e,'native','ready',lambda x:x['ready_origin'].update(rng_seed=123))
        self.blocked(e)
    def test_draw_or_copy_after_stop_and_extra_emulation_block(self):
        for role in ('native','dolphin'):
            e=copy.deepcopy(self.base);self.changed(e,role,'draw',lambda x:x['draw_coverage']['draws'][0].update(after_source_seam=99999));self.blocked(e)
            e=copy.deepcopy(self.base);self.changed(e,role,'ready',lambda x:x['cpu_stop'].update(emulation_advanced_after_stop=True));self.blocked(e)
    def test_native_overwrite_and_unknown_gpu_result_block(self):
        e=copy.deepcopy(self.base)
        def overwrite(x):
            w=copy.deepcopy(x['resolve']);w.update(operation_index=15,copy_id=99);x['resource_writes'].append(w)
        self.changed(e,'native','copy',overwrite);self.blocked(e)
        e=copy.deepcopy(self.base);self.changed(e,'native','readback',lambda x:x['submit'].update(validation_scope_result='unknown'));self.blocked(e)
    def test_selection_changed_policy_or_stitched_xfb_blocks(self):
        e=copy.deepcopy(self.base);self.changed(e,'native','selection',lambda x:x['selection'].update(policy='latched'));self.blocked(e)
        e=copy.deepcopy(self.base);self.changed(e,'dolphin','selection',lambda x:x['selection'].update(stitched=True));self.blocked(e)
    def test_unchecked_publication_or_wrong_readback_blocks(self):
        for key,value in [('flush_success',False),('readback_id',999),('copy_id',999)]:
            e=copy.deepcopy(self.base);self.changed(e,'native','publication',lambda x,k=key,v=value:x['png'].update({k:v}));self.blocked(e)
    def test_missing_prefix_with_self_updated_export_hashes_still_blocks(self):
        e=copy.deepcopy(self.base)
        def remove_prefix(x):
            actual=Path(x['actual_input']['path']);rows=[json.loads(line) for line in actual.read_text().splitlines()]
            rows=[row for row in rows if row['frame']>=51]
            p=self.root/'prefix-deleted.jsonl';p.write_text(''.join(json.dumps(row)+'\n' for row in rows))
            x['actual_input']=bind(p);x['input_export'].update(sha256=bind(p)['sha256'],bytes=p.stat().st_size,read_count=len(rows))
        self.changed(e,'native','input',remove_prefix);row=self.blocked(e,'INPUT_DELIVERY')
        self.assertIn('original raw export',row['error'])
    def test_missing_tail_and_extra_source_seam_are_both_notmeasured(self):
        for extra in (False,True):
            e=copy.deepcopy(self.base)
            def change(x):
                with Path(x['trace']['path']).open() as f:reader=csv.DictReader(f);fields=reader.fieldnames;rows=list(reader)
                if extra:
                    row=dict(rows[-1]);row['frame']=str(int(row['frame'])+1);rows.append(row)
                else:rows.pop()
                p=self.root/('extra-tail.csv' if extra else 'missing-tail.csv')
                with p.open('w') as f:writer=csv.DictWriter(f,fieldnames=fields);writer.writeheader();writer.writerows(rows)
                x['trace']=bind(p)
            self.changed(e,'native','ready',change);self.blocked(e)
    def test_valid_content_difference_is_not_input_or_gameplay_failure(self):
        e=copy.deepcopy(self.base)
        p=self.root/'different-content.png';Image.new('RGB',(32,18),(200,20,20)).save(p)
        def change(x):x['image']=bind(p);x['png']['sha256']=bind(p)['sha256']
        self.changed(e,'native','publication',change)
        row=self.run_case(e)['results'][0]
        self.assertEqual(row['measurement_status'],'SYNTHETIC_METRIC_ONLY',row)
        self.assertGreater(row['metrics']['visual_distance'],0)
        self.assertNotIn('failure_category',row);self.assertEqual(row['qualification'],'BLOCKED')
    def test_artifact_change_during_metrics_cannot_leave_measured_result(self):
        e=copy.deepcopy(self.base)
        # Mutate an owned duplicate artifact, leaving class fixtures intact.
        self.changed(e,'native','publication',lambda x:None)
        path=Path(e['pairs'][0]['native']['artifacts']['publication']['path'])
        real=pairs.metrics
        def changed(*args,**kwargs):
            result=real(*args,**kwargs);path.write_text(path.read_text()+' ');return result
        with patch.object(pairs,'metrics',side_effect=changed):self.blocked(e)
    def test_later_pair_cannot_leave_earlier_drifted_artifact_measured(self):
        e=copy.deepcopy(self.base);e['pairs'].append(copy.deepcopy(e['pairs'][0]))
        image=Path(json.loads(Path(e['pairs'][0]['native']['artifacts']['publication']['path']).read_text())['image']['path'])
        original=image.read_bytes();real=pairs.metrics;calls=0
        def mutate(*args):
            nonlocal calls
            result=real(*args);calls+=1
            if calls==2:image.write_bytes(original+b'drift')
            return result
        try:
            with patch('phase_v2_pairs.metrics',side_effect=mutate):r=self.run_case(e)
            self.assertTrue(all(row['measurement_status']=='NOTMEASURED' for row in r['results']))
            self.assertEqual(r['results'][0]['failure_category'],'PROVENANCE_DRIFT')
        finally:image.write_bytes(original)
    def test_self_rebound_stop_evidence_cannot_replace_operator_integrity_root(self):
        e=copy.deepcopy(self.base);root=self.root/'external-pin-test';root.mkdir()
        manifest=root/'manifest.json';write(manifest,e)
        pin=write(root/'operator-pin.json',dict(schema_version=1,scope='operator-integrity-pin',contract=pairs.CONTRACT,manifest_sha256=bind(manifest)['sha256']))
        self.changed(e,'native','ready',lambda x:x['cpu_stop'].update(event=10001))
        write(manifest,e)
        with patch('phase_v2_pairs.Path.home',return_value=self.home):
            with self.assertRaisesRegex(ValueError,'operator pin'):pairs.compare_manifest(manifest,root/'report',pin['path'],pin['sha256'])
            with self.assertRaisesRegex(ValueError,'digest required'):pairs.compare_manifest(manifest,root/'report',pin['path'],None)
    def test_unknown_counts_or_forged_synthetic_identity_block(self):
        e=copy.deepcopy(self.base);prod=json.loads(Path(e['pairs'][0]['native']['producer']['path']).read_text());prod['counters']['vi']=None
        e['pairs'][0]['native']['producer']=write(self.root/'unknown-counter.json',prod);self.blocked(e,'PROVIDER_PROVENANCE')
        e=copy.deepcopy(self.base);e['synthetic']=False;self.blocked(e,'PROVIDER_PROVENANCE')


if __name__=='__main__':unittest.main()
