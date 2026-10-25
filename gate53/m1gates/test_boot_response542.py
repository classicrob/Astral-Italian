import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import boot_response542 as response

class ProbeResult(unittest.TestCase):
    def make(self,folder):
        p=object.__new__(response.Probe)
        p.folder=Path(folder);p.pid=123;p.app=Path('/owned/Petari.app')
        class Finished:
            def set(self): pass
            def join(self,timeout): pass
            def is_alive(self): return False
        p.stop=p.thread=Finished();p.samples=[{'passed':True}];p.error=None
        p.shown=p.ended=True;p.screenshot={'exit':0}
        (p.folder/'boot-splash.png').write_bytes(b'actual-image-fixture')
        return p
    def test_complete_response(self):
        with tempfile.TemporaryDirectory() as d:self.assertTrue(self.make(d).finish()['passed'])
    def test_missing_response(self):
        with tempfile.TemporaryDirectory() as d:
            p=self.make(d);p.samples=[];self.assertFalse(p.finish()['passed'])
    def test_one_timeout(self):
        with tempfile.TemporaryDirectory() as d:
            p=self.make(d);p.samples.append({'passed':False,'status':-1712});self.assertFalse(p.finish()['passed'])
    def test_missing_visible_evidence(self):
        with tempfile.TemporaryDirectory() as d:
            p=self.make(d);p.screenshot=None;self.assertFalse(p.finish()['passed'])
    def test_fourcc(self): self.assertEqual(response.fourcc('kpid'),0x6b706964)

if __name__=='__main__': unittest.main()
