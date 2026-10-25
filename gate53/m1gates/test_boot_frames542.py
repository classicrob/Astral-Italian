import copy
import csv
import ctypes
import json
from pathlib import Path
import struct
import tempfile
import unittest
import zlib
import boot_frames542 as boot

def png(path,black=False):
    w,h=400,300;raw=bytearray()
    for y in range(h):
        raw.append(0)
        for x in range(w):
            pixel=(255,255,255) if 305<=x<375 and 282<=y<288 else (80,140,220) if 305<=x<375 and y==290 else (0,0,0)
            raw.extend((0,0,0) if black else pixel)
    def chunk(k,b):return struct.pack('>I',len(b))+k+b+struct.pack('>I',zlib.crc32(k+b)&0xffffffff)
    Path(path).write_bytes(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',w,h,8,2,0,0,0))+chunk(b'IDAT',zlib.compress(raw))+chunk(b'IEND',b''))

class Frames(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name)
        png(self.root/'boot-splash.png')
        self.text=(
          '[gx boot splash] event=shown published=0 total=0 monotonic_ns=100 wall_ns=100 mode=normal-frames\n'
          '[gx boot splash window] id=42 pid=123\n'
          '[gx boot splash present] frame=0 uptime_ns=1000000000 success=1\n'
          '[gx frontend release] finished_launching=1 window_onscreen=1 alpha=1 bounds_nonzero=1 first_present=1 presented_frames=1\n'
          '[gx boot splash present] frame=1 uptime_ns=1016666666 success=1\n'
          f'[gx boot splash capture] result=written frame=1 width=400 height=300 source=presented-surface after_present=1 path={self.root}/boot-splash.png\n'
          '[gx boot splash present] frame=2 uptime_ns=1033333332 success=1\n'
          '[gx boot splash] event=handed_off published=0 total=0 monotonic_ns=400 wall_ns=400 mode=normal-frames\n'
          '[gx frontend pre-present] version=2 result=ready keys=54 completed=54 pending=0 failed=0 manifest_errors=0 elapsed_ms=50 cap_ms=5000 workers=4 pumps=10 pump_max_gap_ms=20 main_thread=1 presented_frames=3 key_list=unused\n'
          '[gx boot splash frames] presented=3 csv_intervals=2 first_serial=0 last_serial=2 elapsed_ms=50 game_advanced=0 route_advanced=0\n'
          '[gx startup frame mapping] prep_csv_first=1 prep_csv_last=2 game_csv_offset=3 route_offset=0 policy=continuous-intervals\n')
        self.rows=[dict(frame=i,phase='startup' if i<3 else 'menu',interval_ms=16.667) for i in (1,2,3)]
    def write(self):
        (self.root/'app.log').write_text(self.text)
        with (self.root/'frames.csv').open('w') as f:
            out=csv.DictWriter(f,fieldnames=self.rows[0].keys());out.writeheader();out.writerows(self.rows)
    def check(self):self.write();return boot.validate_frames(self.root)
    def test_complete(self):self.assertTrue(self.check()['passed'])
    def v3(self):
        self.text=self.text.replace('version=2 result=ready','version=3 result=ready').replace('cap_ms=5000','cap_ms=7000')
        names=('app_menu_setup_enter','app_menu_setup_exit','hidden_bootstrap_enter',
            'first_run_loop_enter','first_run_loop_exit','first_sdl_pump_enter','first_sdl_pump_exit',
            'first_aurora_update_enter','input_initialize_enter','sdl_input_init_enter','sdl_input_init_exit',
            'input_initialize_exit','first_aurora_update_exit','hidden_bootstrap_exit','window_show_enter','window_shown')
        self.text+=''.join(f'[gx startup call] event={name} monotonic_ns={i}\n' for i,name in enumerate(names,1))
        self.text+='[gx startup call] event=first_successful_present monotonic_ns=999999999\n'
    def test_version3_keeps_same_coverage_and_responsiveness(self):
        self.v3()
        self.assertTrue(self.check()['passed'])
        self.text=self.text.replace('pump_max_gap_ms=20','pump_max_gap_ms=100.001')
        self.assertFalse(self.check()['passed'])
    def test_bootstrap_missing_or_ambiguous_is_not_proof(self):
        self.v3()
        line='[gx startup call] event=sdl_input_init_exit monotonic_ns=11\n'
        self.assertIn(line,self.text)
        original=self.text
        self.text=original.replace(line,'');self.assertFalse(self.check()['passed'])
        self.text=original+line;self.assertFalse(self.check()['passed'])
    def test_visible_input_init_is_not_hidden_bootstrap(self):
        self.v3()
        self.text=self.text.replace('event=sdl_input_init_exit monotonic_ns=11','event=sdl_input_init_exit monotonic_ns=101')
        self.assertFalse(self.check()['passed'])
    def test_black(self):
        png(self.root/'boot-splash.png',True);self.assertFalse(self.check()['passed'])
    def test_truncated_png(self):
        (self.root/'boot-splash.png').write_bytes(b'\x89PNG\r\n\x1a\n');self.assertFalse(self.check()['passed'])
    def test_png_crc(self):
        p=self.root/'boot-splash.png';data=bytearray(p.read_bytes());data[-1]^=1;p.write_bytes(data)
        self.assertFalse(self.check()['passed'])
    def test_missing_capture(self):
        self.text='\n'.join(l for l in self.text.splitlines() if 'splash capture' not in l)
        self.assertFalse(self.check()['passed'])
    def test_no_successful_present(self):
        self.text=self.text.replace('frame=1 uptime_ns=1016666666 success=1','frame=1 uptime_ns=1016666666 success=0')
        self.assertFalse(self.check()['passed'])
    def test_frame_omission(self):
        self.rows.pop(0);self.assertFalse(self.check()['passed'])
    def test_interval_hidden(self):
        self.rows[0]['interval_ms']=1;self.assertFalse(self.check()['passed'])
    def test_startup_phase_required(self):
        self.rows[0]['phase']='gameplay';self.assertFalse(self.check()['passed'])
    def test_mapping_required(self):
        self.text=self.text.replace('game_csv_offset=3','game_csv_offset=0');self.assertFalse(self.check()['passed'])
    def test_lost_long_interval(self):
        self.text=self.text.replace('1016666666','1400000000').replace('1033333332','1416666666')
        self.rows[0]['interval_ms']=400
        # Coverage accepts the REAL long row; unchanged gate542m fails >250.
        self.assertTrue(self.check()['passed']);self.assertEqual(self.check()['cadence']['max_ms'],400)
    def probe(self):
        self.write();p=object.__new__(boot.Probe);p.folder=self.root;p.pid=123;p.app=Path('/owned/Petari.app')
        class Finished:
            def set(self):pass
            def join(self,timeout):pass
            def is_alive(self):return False
        p.stop=p.thread=Finished();p.error=None;p.shown=p.ended=True;p.appleevent=None;p.extra_screen=None
        p.samples=[dict(passed=True,windowserver=dict(passed=True,wall_ns=200))]
        return p
    def test_full_proof(self):self.assertTrue(self.probe().finish()['passed'])
    def test_no_window_evidence(self):
        p=self.probe();p.samples=[];self.assertFalse(p.finish()['passed'])
    def test_appleevent_denial_observation_only(self):
        p=self.probe();p.appleevent=dict(passed=False,status=-1701,reply_error_number=-1744)
        result=p.finish();self.assertTrue(result['passed']);self.assertFalse(result['appleevent_gated'])
    def test_turn_gap_threshold(self):
        self.text=self.text.replace('pump_max_gap_ms=20','pump_max_gap_ms=100.001');self.assertFalse(self.check()['passed'])
        self.text=self.text.replace('pump_max_gap_ms=100.001','pump_max_gap_ms=100');self.assertTrue(self.check()['passed'])
    def test_window_failure_stays_failure(self):
        p=self.probe();p.samples[0]['passed']=False;self.assertFalse(p.finish()['passed'])
    def test_after_prep_not_proof(self):
        p=self.probe();p.samples[0]['windowserver']['wall_ns']=401;self.assertFalse(p.finish()['passed'])
    def test_duplicate_boundary(self):
        self.text+=self.text.splitlines()[0]+'\n';self.assertFalse(self.probe().finish()['passed'])
    def test_sdk_packing_and_prompt_flags(self):
        self.assertEqual(ctypes.sizeof(boot.Desc),12);self.assertEqual(boot.Desc.dataHandle.offset,4)
        self.assertEqual(boot.SEND_MODE,0x20013)
        self.assertEqual(boot.fourcc('kpid'),0x6b706964)
if __name__=='__main__':unittest.main()
