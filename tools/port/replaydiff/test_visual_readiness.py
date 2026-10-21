import unittest
from visual_readiness import ReadySchedule, presentation_receipt


def row(frame, ready=False, player=1, epoch=0):
    return dict(frame=frame, scene_valid=1, player_valid=player, init_state=6 if ready else 5,
                scene_epoch=epoch, scenario=1, scene='Game', stage='Synthetic')


class ScheduleTests(unittest.TestCase):
    def test_source_offsets_are_exact_and_no_auto_reanchor(self):
        schedule=ReadySchedule('Synthetic',1,(2,4))
        self.assertIsNone(schedule.observe(row(0),0))
        self.assertIsNone(schedule.observe(row(1,True),1))
        self.assertIsNone(schedule.observe(row(2,True),2))
        request=schedule.observe(row(3,True),3)
        self.assertEqual(request['ready_source_frame'],1)
        self.assertEqual(request['capture_source_frame'],3)
        self.assertEqual(request['status'],'PENDING_PRESENTATION')
        self.assertIsNone(schedule.observe(row(4,True,epoch=1),4))
        self.assertIsNone(schedule.observe(row(5,True),5))
        self.assertIsNotNone(schedule.cancelled)

    def test_unavailable_boot_fields_are_not_invented(self):
        schedule=ReadySchedule('Synthetic',1,(1,))
        absent=dict(frame=0,scene_valid=0,player_valid=0,scene_epoch=0,
                    init_state='',scenario='',scene='',stage='')
        self.assertIsNone(schedule.observe(absent,0))
        self.assertIsNone(schedule.origin)
        schedule.observe(row(1,True),1)
        self.assertEqual(schedule.origin['source_frame'],1)
        absent['frame']=2
        self.assertIsNone(schedule.observe(absent,2))
        self.assertIn('unavailable',schedule.cancelled['reason'])
        self.assertIsNone(schedule.observe(row(3,True),3))
        self.assertEqual(schedule.requests,[])

    def test_first_init6_is_not_shifted_to_player_valid(self):
        schedule=ReadySchedule('Synthetic',1,(1,))
        schedule.observe(row(0,True,player=0),0)
        request=schedule.observe(row(1,True,player=1),1)
        self.assertEqual(schedule.origin['source_frame'],0)
        self.assertEqual(request['status'],'BLOCKED')

    def test_skipped_and_rebased_sources_fail(self):
        schedule=ReadySchedule('Synthetic',1)
        with self.assertRaises(ValueError): schedule.observe(row(1,True),1)
        with self.assertRaises(ValueError): schedule.observe(row(8,True),0)

    def test_unknown_and_wrong_presented_frames_never_accepted(self):
        request=dict(status='PENDING_PRESENTATION',capture_source_frame=60,source_scene_epoch=0)
        for copy,readback in [({},{}), (dict(frame_id=1,source_frame=59,scene_epoch=0),dict(frame_id=1)),
                               (dict(frame_id=1,source_frame=60,scene_epoch=0),dict(frame_id=2))]:
            with self.assertRaises(ValueError): presentation_receipt(request,copy,readback,'unused','native-renderer-readback')
        with self.assertRaises(ValueError): presentation_receipt(dict(status='BLOCKED'),{}, {},'unused','native-renderer-readback')


if __name__ == '__main__': unittest.main()
