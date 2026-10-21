import copy
import unittest
from native_draw_observations import inspect


def fixture():
    key=dict(key_version=1,scene_hex=b'Game'.hex(),stage_hex=b'RedBlueExGalaxy'.hex(),placed_scenario=1,selected_scenario=1)
    source=dict(frame=20,sampled_epoch=7,valid=1,authority='caller-sampled-identity-not-lifecycle',key=key)
    snapshot=dict(key=key,init_state=6,buffer_mode=3,controller_present=1,current_scene_nonnull=1,executed_scene_nonnull=1,executed_scene_is_current=1,initialized_game=1,canonical_player_valid=1,framework_present=1,xfb_present=1,render_flag=0,invalid=0)
    events=[]
    for phase in range(4):
        s=copy.deepcopy(source)
        if phase==3:s['frame']+=1
        events.append(dict(event_serial=phase+1,loop_serial=1,draw_serial=int(phase!=0),phase=phase,source=s,source_key_matches=int(phase!=3),snapshot=copy.deepcopy(snapshot)))
    return dict(schema_version=1,status='detached-observations-only',qualification='BLOCKED',qualified_frames=None,runtime_provider_accepted=False,fault=0,capacity=4096,event_count=4,events=events)


class DrawObservationTests(unittest.TestCase):
    def test_consistent_journal_never_provider(self):
        r=inspect(fixture());self.assertEqual(r['pilot_metadata_candidates'],1);self.assertFalse(r['phase_v2_provider_emitted']);self.assertIsNone(r['qualified_frames'])
    def test_initialized_storage_not_selected_is_preserved(self):
        d=fixture();d['events'][1]['snapshot']['executed_scene_is_current']=0;r=inspect(d)
        self.assertEqual(r['initialized_not_selected'],1);self.assertEqual(r['pilot_metadata_candidates'],0)
    def test_unready_selected_scene_is_not_no_draw(self):
        d=fixture();d['events'][1]['snapshot'].update(initialized_game=0,init_state=0);r=inspect(d)
        self.assertEqual(r['selected_unready'],1);self.assertEqual(r['loops'],1)
    def test_render_flag_false_does_not_remove_draw(self):
        self.assertEqual(inspect(fixture())['loops'],1)
    def test_selected_scenario_changes_do_not_get_hidden_in_epoch(self):
        d=fixture();d['events'][1]['snapshot']['key']=dict(d['events'][1]['snapshot']['key'],selected_scenario=2)
        with self.assertRaises(ValueError):inspect(d)
        d['events'][1]['source_key_matches']=0;self.assertEqual(inspect(d)['pilot_metadata_candidates'],0)
    def test_protocol_fault_overflow_partial_and_qualification_rejected(self):
        for k,v in [('fault',1),('event_count',3),('capacity',4097),('qualification','PASS'),('runtime_provider_accepted',True)]:
            d=fixture();d[k]=v
            with self.assertRaises(ValueError):inspect(d)
        d=fixture();d['events'].pop();d['event_count']=3
        with self.assertRaises(ValueError):inspect(d)
    def test_shifted_source_and_serials_rejected(self):
        for mutate in (lambda e:e[1].update(event_serial=3),lambda e:e[1]['source'].update(frame=21),lambda e:e[3]['source'].update(frame=22),lambda e:e[2].update(draw_serial=2)):
            d=fixture();mutate(d['events'])
            with self.assertRaises(ValueError):inspect(d)
    def test_invalid_bytes_noncanonical_boolean_and_lifecycle_claim_rejected(self):
        for mutate in (lambda e:e[1]['snapshot']['key'].update(stage_hex='a'*128),lambda e:e[1]['snapshot'].update(canonical_player_valid=2),lambda e:e[0]['source'].update(authority='lifecycle')):
            d=fixture();mutate(d['events'])
            with self.assertRaises(ValueError):inspect(d)

if __name__=='__main__':unittest.main()
