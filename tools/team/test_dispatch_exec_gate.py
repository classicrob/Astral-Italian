"""No-process gate models: never call gate.main, exec, spawn or send a signal."""
import json
import os
from pathlib import Path
import sys
import tempfile
import types
import unittest
from unittest.mock import patch

import dispatch_exec_gate as gate
import dispatch_full_budget as full


class GateSourceModels(unittest.TestCase):
    def request(self):
        return {'nonce':'a'*32,'argv':['/bin/sh','-c','NEVER_EXECUTE'], 'cwd':str(Path.cwd().resolve()),
                'env':dict(os.environ),'target':str(Path('/bin/sh').resolve()),
                'target_sha256':gate.digest('/bin/sh'),'gate_sha256':gate.digest(gate.__file__),
                'python_sha256':gate.digest(sys.executable)}

    def test_original_argv_unchanged(self):
        request = self.request()
        old = list(request['argv'])
        with patch.object(gate.os, 'execv') as execute:
            self.assertEqual(gate.check_request(request), request['target'])
            self.assertEqual(request['argv'], old)
            execute.assert_not_called()

    def test_wrong_environment_rejects(self):
        request = self.request(); request['env'] = {}
        with self.assertRaises(RuntimeError):gate.check_request(request)

    def test_wrong_target_hash_rejects(self):
        request = self.request(); request['target_sha256'] = '0'*64
        with self.assertRaises(RuntimeError):gate.check_request(request)

    def test_wrong_gate_hash_rejects(self):
        request = self.request(); request['gate_sha256'] = '0'*64
        with self.assertRaises(RuntimeError):gate.check_request(request)

    def test_stale_record_not_overwritten(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'record.json'; gate.publish(path, {'old':True})
            with self.assertRaises(RuntimeError):gate.publish(path, {'new':True})
            self.assertEqual(json.loads(path.read_text()), {'old':True})

    def model_ack(self, mode):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);work=root/'work';work.mkdir();directory=work/'launch-0001';directory.mkdir()
            request=self.request();gate.publish(directory/'request.json',request)
            pin=gate.digest(directory/'request.json')
            job=full.FullBudgetJob(work,3,root,0,1024**2,0.25)
            job.remaining=lambda:3
            if mode=='cleanup':job.in_cleanup=True
            job.fixture_identity_path=directory/'identity.json'
            job.groups[707]=types.SimpleNamespace(pid=707)
            job.active_launch={'directory':directory,'request':request,'pin':pin,'acknowledged':False}
            if mode!='missing':
                gate.publish(directory/'identity.json',{'pid':707,'pgid':707,'sid':707,'uid':os.getuid()+(mode=='wrong-uid')})
                gate.publish(directory/'target.json',{'pid':707,'request_sha256':'0'*64 if mode=='wrong-target' else pin,
                    'target':request['target'],'target_sha256':request['target_sha256'],'cwd':request['cwd']})
            with patch.object(full.accepted.os,'getpgid',return_value=707),patch.object(full.accepted.os,'getsid',return_value=707),patch.object(full.accepted.os,'killpg') as signal:
                if mode in ('wrong-uid','wrong-target'):
                    with self.assertRaises(RuntimeError):job.observe_fixture()
                else:job.observe_fixture()
                if mode=='valid':
                    self.assertEqual(json.loads((directory/'ack.json').read_text()),{'pid':707,'request_sha256':pin})
                elif mode=='cleanup':
                    self.assertFalse((directory/'ack.json').exists())
                    self.assertEqual(job.ownership[707]['status'],'VERIFIED')
                else:
                    self.assertFalse((directory/'ack.json').exists())
                    with self.assertRaises(RuntimeError):job.require_signal_ownership(707)
                signal.assert_not_called()

    def test_verified_ack(self):self.model_ack('valid')
    def test_missing_identity_no_ack_or_signal(self):self.model_ack('missing')
    def test_uid_mismatch_no_ack_or_signal(self):self.model_ack('wrong-uid')
    def test_target_mismatch_no_ack_or_signal(self):self.model_ack('wrong-target')
    def test_cleanup_never_acknowledges_new_exec(self):self.model_ack('cleanup')


if __name__ == '__main__':unittest.main()
