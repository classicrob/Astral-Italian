import copy
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
from test_visual_resolution import synthetic_images,synthetic_manifest
import native_visual_receipt as provider


def fixture(root):
    images=synthetic_images(root/'images')
    pair=synthetic_manifest(root,(images['identical'],images['identical']))['pairs'][0]
    native=pair['native']
    producer={k:native[k] for k in ('host','executable_sha256','source_revision','renderer','image_geometry')}
    producer['input_sha256']=native['input']['sha256']
    resolve=dict(copy_id=3,resource_id='run-local-texture-1',resource_generation=1,resource_revision=7,
                 command_buffer_id='buffer-2',operation_index=100,renderer_packet_id=42)
    selection={k:resolve[k] for k in ('copy_id','resource_id','resource_generation','resource_revision')}
    selection.update(source_frame=60,vi_configured=True,vi_black=False,vi_dimmed=False,policy='latest',
                     free_aspect=False,includes_chrome=False,includes_overlay=False,rml_replaces_game=False,
                     color_encoding='srgb-code-values-preserved',image_geometry=producer['image_geometry'])
    return dict(synthetic=True,stage='Synthetic',scenario=1,offset=60,source_trace=native['trace'],image=native['image'],input=native['input'],producer=producer,
        input_delivery=dict(status='captured',method='native-consumed-read-log',input_sha256=producer['input_sha256'],frames=600,read_count=2400,source_frame_start=1,source_frame_end=600),
        draw=dict(id=8,phase='observed-draw-state',source_frame=60,scene_epoch=0,stage='Synthetic',scenario=1),
        copy=dict(id=3,draw_id=8,kind='display',fifo_tag=3,issued_source_frame=60),resolve=resolve,selection=selection,
        readback_encode=dict(**{k:resolve[k] for k in ('copy_id','resource_id','resource_generation','resource_revision','command_buffer_id','renderer_packet_id')},operation_index=150,width=320,height=180),
        submit=dict(id='submit-4',command_buffer_id='buffer-2',success=True,validation_scope_result='no-error',device_lost=False),mapped=dict(submit_id='submit-4',copy_id=3,renderer_packet_id=42,success=True,callback_completed=True,width=320,height=180),
        png=dict(write_success=True,close_success=True,atomic_rename_success=True,sha256=native['image']['sha256'],width=320,height=180),
        resolve_coverage_complete=True,resource_writes=[copy.deepcopy(resolve)])


class NativeProviderContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=tempfile.TemporaryDirectory();cls.home=Path(cls.temp.name).resolve()
        root=cls.home/'MacioGalaxy2-gamedata';root.mkdir();cls.base=fixture(root)

    @classmethod
    def tearDownClass(cls):cls.temp.cleanup()

    def review(self,evidence):
        with patch('native_visual_receipt.Path.home',return_value=self.home):return provider.review(evidence)

    def test_consistent_chain_is_only_a_proposal_never_a_capture(self):
        r=self.review(self.base)
        self.assertEqual(r['status'],'SOURCE_CHAIN_CONSISTENT')
        self.assertEqual(r['measurement_status'],'NOTMEASURED')
        self.assertEqual(r['qualification'],'BLOCKED');self.assertIsNone(r['measured_qualified_frames'])
        self.assertFalse(r['runtime_provider_accepted'])
        self.assertEqual(r['presentation_candidate']['status'],'proposal-only')
        self.assertEqual(r['presentation_candidate']['rendered_frame_id'],42)
        self.assertIsNone(r['presentation_candidate']['physical_display_time'])

    def test_missing_real_producer_data_is_not_inferred(self):
        for field in ('draw','copy','resolve','selection','readback_encode','submit','mapped','png','input_delivery'):
            with self.subTest(field=field):
                e=copy.deepcopy(self.base);del e[field]
                r=self.review(e);self.assertEqual(r['status'],'BLOCKED');self.assertIsNone(r['presentation_candidate'])

    def test_phase_lag_and_queued_counters_are_not_fit(self):
        for record,key,value in [('draw','phase','queued-counter'),('draw','source_frame',59),('copy','issued_source_frame',61),('selection','source_frame',62),('copy','kind','texture'),('copy','fifo_tag',4)]:
            with self.subTest(record=record,key=key):
                e=copy.deepcopy(self.base);e[record][key]=value
                self.assertEqual(self.review(e)['status'],'BLOCKED')

    def test_same_destination_or_handle_does_not_prove_copy_generation(self):
        for record,key in [('selection','copy_id'),('readback_encode','resource_revision'),('selection','resource_generation')]:
            e=copy.deepcopy(self.base);e[record][key]+=1
            self.assertEqual(self.review(e)['status'],'BLOCKED')

    def test_texture_reuse_overwrite_before_readback_is_rejected(self):
        e=copy.deepcopy(self.base);other=copy.deepcopy(e['resolve']);other.update(copy_id=4,resource_revision=8,operation_index=120)
        e['resource_writes'].append(other)
        self.assertIn('overwritten',self.review(e)['reasons'][0])

    def test_resize_or_incomplete_write_coverage_blocks(self):
        e=copy.deepcopy(self.base);other=copy.deepcopy(e['resolve']);other['resource_generation']+=1;e['resource_writes'].append(other)
        self.assertIn('reset',self.review(e)['reasons'][0])
        e=copy.deepcopy(self.base);e['resolve_coverage_complete']=False
        self.assertIn('coverage',self.review(e)['reasons'][0])

    def test_cpu_fifo_completion_or_separate_readback_submit_is_insufficient(self):
        for record,key,value in [('readback_encode','command_buffer_id','later-independent-encoder'),('readback_encode','operation_index',99),('submit','success',False),('mapped','success',False),('mapped','submit_id','other-submit'),('submit','validation_scope_result','error'),('submit','device_lost',True),('mapped','callback_completed',False)]:
            e=copy.deepcopy(self.base);e[record][key]=value
            self.assertEqual(self.review(e)['status'],'BLOCKED')

    def test_png_existence_is_not_close_or_content_proof(self):
        for field,value in [('write_success',False),('close_success',False),('atomic_rename_success',False),('sha256','0'*64),('width',321)]:
            e=copy.deepcopy(self.base);e['png'][field]=value
            self.assertEqual(self.review(e)['status'],'BLOCKED')

    def test_black_dim_overlay_color_unknown_all_block_raw_xfb_candidate(self):
        for key,value in [('vi_black',True),('vi_dimmed',True),('vi_configured',False),('includes_chrome',True),('includes_overlay',True),('rml_replaces_game',True),('free_aspect',True),('color_encoding','UNKNOWN')]:
            e=copy.deepcopy(self.base);e['selection'][key]=value
            self.assertEqual(self.review(e)['status'],'BLOCKED')

    def test_bad_stage_or_scenario_does_not_select_another_source(self):
        for key,value in [('stage','Other'),('scenario',2),('offset',61)]:
            e=copy.deepcopy(self.base);e[key]=value
            self.assertEqual(self.review(e)['status'],'BLOCKED')

    def test_raw_state_and_render_packet_ids_are_distinct(self):
        r=self.review(self.base);candidate=r['presentation_candidate']
        self.assertEqual(candidate['capture_source_frame'],60)
        self.assertEqual(candidate['rendered_frame_id'],42)
        self.assertEqual(candidate['copy_id'],3)


if __name__=='__main__':unittest.main()
