import copy
import hashlib
from pathlib import Path
import tempfile
import unittest
from observer_standalone_caller import consumed_md,inspect_otool,execute_after_qualification,bounded_report

class CallerModels(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name).resolve()
        self.header=self.root/'sdk-header.h';self.header.write_text('synthetic SDK header\n')
        self.inventory={'records':{str(self.header):{'sha256':hashlib.sha256(self.header.read_bytes()).hexdigest()}}}
    def dep(self,text):p=self.root/'fixture.d';p.write_text(text);return p
    def test_consumed_system_dependency_must_match_preinventory(self):
        r=consumed_md(self.dep('fixture.o: '+str(self.header)+'\n'),self.root,self.inventory,lambda:None)
        self.assertEqual(len(r['dependencies']),1)
        self.header.write_text('changed')
        with self.assertRaises(RuntimeError):consumed_md(self.root/'fixture.d',self.root,self.inventory,lambda:None)
    def test_unbound_empty_or_malformed_md_rejected(self):
        for text in ('fixture.o: /unbound/sdk.h\n','fixture.o:\n','not a depfile'):
            with self.assertRaises(RuntimeError):consumed_md(self.dep(text),self.root,self.inventory,lambda:None)
    def test_otool_exact_target_and_dynamic_cpp(self):
        p=self.root/'otool.log';exe=self.root/'control';p.write_text(str(exe)+':\n\t/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1.0.0)\n')
        self.assertEqual(len(inspect_otool(p,exe)['libraries']),1)
        p.write_text(str(exe)+':\n\t/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 1.0.0)\n')
        with self.assertRaises(RuntimeError):inspect_otool(p,exe)
    def test_no_qualification_pin_means_no_file_lock_or_spawn(self):
        with self.assertRaisesRegex(RuntimeError,'UNQUALIFIED'):execute_after_qualification(None,None,None,None,None,None,'fake')
    def test_report_never_overwrites_existing_evidence(self):
        p=self.root/'report.json';bounded_report(p,{'status':'synthetic'})
        with self.assertRaises(FileExistsError):bounded_report(p,{'status':'replacement'})

if __name__=='__main__':unittest.main()
