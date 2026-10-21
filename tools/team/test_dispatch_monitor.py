"""Synthetic metadata-only controls. No archive, process, signal or native lock."""
import json
from pathlib import Path
import tempfile
import time
import types
import unittest

import dispatch_monitor as monitor


class MonitorSourceModels(unittest.TestCase):
    def setup_stage(self, root, name):
        work=root/name;work.mkdir()
        cleanup={'all_owned_groups_absent':True,'remaining_pgids':[],'failures':[], 'execution_error':None,'events':[]}
        budget={'root':str(root),'failure':None,'initial_allocated_bytes':0,
                'cleanup':{'status':'CONFIRMED','base_report':cleanup}}
        (work/'disk-budget.json').write_bytes(monitor.encoded(budget))
        (work/'owned-process-cleanup.json').write_bytes(monitor.encoded(cleanup))
        return types.SimpleNamespace(work=work,groups={},start=time.monotonic(),seconds=600)

    def test_complete_joins_teardown_bytes(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);job=self.setup_stage(root,'execution')
            path=monitor.complete('execution',root,'a'*64,job,{'actual_result_sha256':'b'*64})
            completion=json.loads(path.read_text())
            self.assertEqual(completion['budget_sha256'],monitor.sha((job.work/'disk-budget.json').read_bytes()))
            self.assertEqual(completion['cleanup_sha256'],monitor.sha((job.work/'owned-process-cleanup.json').read_bytes()))

    def test_failed_teardown_no_completion(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);job=self.setup_stage(root,'execution');job.groups={707:None}
            with self.assertRaises(RuntimeError):monitor.complete('execution',root,'a'*64,job,{'actual_result_sha256':'b'*64})
            self.assertFalse((job.work/'completion.json').exists())

    def test_nonfinite_clock_rejects(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);job=self.setup_stage(root,'execution');job.start=float('nan')
            with self.assertRaises(RuntimeError):monitor.complete('execution',root,'a'*64,job,{'actual_result_sha256':'b'*64})

    def test_terminal_metadata_only_detached(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);out=root/'package';out.mkdir()
            for stage, name in [('execution','execution'),('packaging','packaging-monitor')]:
                job=self.setup_stage(root,name)
                payload={'actual_result_sha256':'a'*64} if stage=='execution' else {'primary_encoded_sha256':'b'*64,'secondary_encoded_sha256':'c'*64}
                monitor.complete(stage,root,'d'*64,job,payload)
            contract={'terminal_metadata':{'maximum_allocated_bytes':16777216,'reserved_bytes':16777216},
                      'required_live_budget':{'maximum_owned_growth_bytes':4*1024**3,'minimum_free_bytes':0}}
            receipt={'synthetic_metadata_only':True};transfer={}
            pin=monitor.terminal_publish(root,out,contract,'d'*64,receipt,transfer,{})
            publication=json.loads((out/'terminal-publication.json').read_text())
            self.assertEqual(pin,monitor.sha((out/'terminal-publication.json').read_bytes()))
            self.assertEqual(len(publication['files']),8)
            self.assertNotIn('terminal-publication.json',publication['files'])
            self.assertEqual(publication['receipt_sha256'],monitor.sha((out/'receipt.json').read_bytes()))
            self.assertEqual(publication['transfer_sha256'],monitor.sha((out/'transfer.json').read_bytes()))


if __name__=='__main__':unittest.main()
