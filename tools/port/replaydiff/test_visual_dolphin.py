import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

import visual_dolphin


class Remote:
    def reg(self, number):
        return visual_dolphin.collector.SEAM if number == 64 else visual_dolphin.collector.CALLER


class FakeSnapshot:
    def __init__(self):
        self.m = type('Memory', (), dict(remote=Remote()))()

    def take(self, frame):
        if frame == 0:
            return dict(frame=frame,scene_valid=0,player_valid=0,scene_epoch=0,scenario='',
                        init_state='',scene='',stage='',observer_error='')
        return dict(frame=frame,scene_valid=1,player_valid=1,scene_epoch=1,scenario=1,
                    init_state=6,scene='Game',stage='Synthetic',observer_error='')


class AdapterTests(unittest.TestCase):
    def test_wraps_actual_source_rows_and_restores_observer(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            (output/'manifest.json').write_text(json.dumps(dict(dolphin_pid=123,dolphin_sha256='a'*64)))
            seen=[]
            def collect():
                snapshot=visual_dolphin.collector.Snapshot()
                for frame in range(602): snapshot.take(frame)
                return 0
            def image(pid,exe,sha,path,request,window):
                self.assertEqual(pid,123)
                self.assertEqual(sha,'a'*64)
                self.assertEqual(window,7)
                seen.append(request)
                return dict(status='UNQUALIFIED_WINDOW_CAPTURE',request=request)
            argv=['visual_dolphin.py','--sync-stage','Synthetic','--sync-scenario','1',
                  '--output',str(output),'--dolphin','/synthetic','--visual-window-id','7']
            with patch.object(visual_dolphin.collector,'Snapshot',FakeSnapshot), \
                 patch.object(visual_dolphin.collector,'game_private_path',side_effect=Path), \
                 patch.object(visual_dolphin.dolphin_stage_entry,'main',side_effect=collect), \
                 patch.object(visual_dolphin,'capture_window',side_effect=image), \
                 patch.object(sys,'argv',argv):
                self.assertEqual(visual_dolphin.main(),0)
                self.assertIs(visual_dolphin.collector.Snapshot,FakeSnapshot)
            receipt=json.loads((output/'visual-captures.json').read_text())
            self.assertEqual(receipt['qualification'],'BLOCKED')
            self.assertEqual(receipt['origin']['source_frame'],1)
            self.assertEqual([x['capture_source_frame'] for x in seen],[61,301,601])
            self.assertEqual(len(receipt['captures']),3)
            self.assertIn('UNKNOWN',receipt['rendered_frame_binding'])


if __name__=='__main__': unittest.main()
