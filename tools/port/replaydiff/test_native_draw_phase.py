import hashlib
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import native_draw_phase as phase


class DrawPhaseTests(unittest.TestCase):
    def test_all_modes_keep_draw_unconditional_even_when_render_flag_false(self):
        for mode in (1,2,3):
            for previous in (False,True):
                trace=phase.frame_order(mode,previous,counter=0,interval=3)
                self.assertFalse(trace['current_render'])
                self.assertTrue(trace['draw_called'])
                self.assertEqual(trace['events'].count('GameSystem.draw.call-unconditional'),1)
                self.assertIn('endRender.endGX',trace['events'])

    def test_double_uses_previous_flag_before_new_flag(self):
        t=phase.frame_order(2,True,counter=0,interval=3)['events']
        self.assertLess(t.index('GXCopyDisp.double-before-current-draw'),t.index('beginRender.recompute-render-flag'))
        self.assertLess(t.index('GXCopyDisp.double-before-current-draw'),t.index('GameSystem.draw.call-unconditional'))
        self.assertNotIn('clearEfb.current-render',t)

    def test_new_render_does_not_imply_old_exchange(self):
        t=phase.frame_order(2,False)['events']
        self.assertNotIn('GXCopyDisp.double-before-current-draw',t)
        self.assertIn('clearEfb.current-render',t)

    def test_double_index_condition_clears_instead_of_copy(self):
        t=phase.frame_order(2,True,drawn_equals_displaying=False)['events']
        self.assertNotIn('GXCopyDisp.double-before-current-draw',t)
        self.assertIn('clearEfb.double-no-exchange',t)

    def test_invalid_drawing_index_suppresses_conditional_copy(self):
        for mode,copy in [(2,'GXCopyDisp.double-before-current-draw'),(3,'GXCopyDisp.triple-after-current-draw')]:
            self.assertNotIn(copy,phase.frame_order(mode,True,copy_drawing_valid=False)['events'])

    def test_triple_copies_after_draw_before_update(self):
        t=phase.frame_order(3,True)['events']
        self.assertLess(t.index('proposed.draw-exit-observation'),t.index('GXCopyDisp.triple-after-current-draw'))
        self.assertLess(t.index('GXCopyDisp.triple-after-current-draw'),t.index('update'))
        self.assertLess(t.index('update'),t.index('post-endFrame.ready-observation'))

    def test_single_retrace_copy_is_not_assigned_fixed_frame_or_draw(self):
        t=phase.frame_order(1,True)
        self.assertFalse(any('GXCopyDisp' in e for e in t['events']))
        self.assertIn('UNBOUND',t['single_copy']['relative_to_current_draw'])
        self.assertIn('retrace',t['single_copy']['thread'])
        self.assertNotIn('pre-copy-callback.if-installed',t['events'])

    def test_pre_copy_callback_is_not_uniform_draw_hook(self):
        for mode in (2,3):
            e=phase.frame_order(mode,True)['events']
            callback=e.index('pre-copy-callback.if-installed')
            draw=e.index('GameSystem.draw.call-unconditional')
            self.assertEqual(callback<draw,mode==2)
        self.assertFalse(phase.proposal()['existing_uniform_hook'])

    def test_invalid_symbolic_parameters_rejected(self):
        for args in [(0,True),(4,True),(True,True),(3,1),(3,True,0,0)]:
            with self.assertRaises(ValueError):phase.frame_order(*args)

    def good(self):
        return dict(scene_valid=1,scene='Game',stage='Synthetic',placed_scenario=1,init_state=6,
                    current_scene_nonnull=True,executed_scene_is_current=True,canonical_player_valid=True,
                    observation_phase='before-GameSystem::draw',game_cpu_owned=True,
                    scene_epoch=2,scene_epoch_kind='sampled-identity')

    def test_typed_candidate_never_claims_epoch_or_gpu_qualification(self):
        r=phase.typed_draw_eligibility(self.good(),'Synthetic',1)
        self.assertTrue(r['typed_candidate']);self.assertFalse(r['scene_epoch_authoritative'])
        self.assertFalse(r['runtime_token_producer']);self.assertEqual(r['qualification'],'BLOCKED')
        self.assertIsNone(r['qualified_frames'])

    def test_ready_metadata_alone_does_not_prove_current_scene_drawn(self):
        for key,value in [('executed_scene_is_current',False),('current_scene_nonnull',False),('scene','FileSelect'),('init_state',5),('canonical_player_valid',False),('game_cpu_owned',False),('observation_phase','post-endFrame'),('scene_epoch',None),('scene_epoch_kind','authoritative-by-pointer')]:
            d=self.good();d[key]=value
            self.assertFalse(phase.typed_draw_eligibility(d,'Synthetic',1)['typed_candidate'])

    def test_selected_scenario_is_not_placed_scenario(self):
        d=self.good();del d['placed_scenario'];d['selected_scenario']=1
        self.assertFalse(phase.typed_draw_eligibility(d,'Synthetic',1)['typed_candidate'])
        d=self.good();d['placed_scenario']=2;d['selected_scenario']=1
        self.assertFalse(phase.typed_draw_eligibility(d,'Synthetic',1)['typed_candidate'])

    def test_invalid_target_cannot_become_a_typed_candidate(self):
        for stage,scenario in [('',1),(None,1),('Synthetic',True),('Synthetic',0)]:
            with self.assertRaises(ValueError):phase.typed_draw_eligibility(self.good(),stage,scenario)

    def test_exact_blob_pins_reject_drift_and_missing_sources(self):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp)/'source.cpp';p.write_text('synthetic source fixture')
            pins=dict(revision='synthetic',files={'source.cpp':hashlib.sha256(p.read_bytes()).hexdigest()})
            with patch.object(phase,'PINS',pins):
                self.assertEqual(phase.audit_sources(tmp)['revision'],'synthetic')
                p.write_text('changed')
                with self.assertRaisesRegex(ValueError,'drift'):phase.audit_sources(tmp)
                p.unlink()
                with self.assertRaises(FileNotFoundError):phase.audit_sources(tmp)

    def test_proposal_keeps_phase_mapping_and_all_runtime_joins_unknown(self):
        p=phase.proposal()
        self.assertFalse(p['runtime_token_producer']);self.assertFalse(p['source_frame_is_drawn_state'])
        self.assertIsNone(p['fixed_frame_lag']);self.assertIsNone(p['qualified_frames'])
        self.assertIn('GAME',p['required_owner']);self.assertIn('UNBOUND',p['copy_to_draw_binding'])
        self.assertEqual(p['qualification'],'BLOCKED')


if __name__=='__main__':unittest.main()
