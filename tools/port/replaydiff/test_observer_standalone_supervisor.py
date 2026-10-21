import copy
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from observer_standalone_supervisor import checked_plan,load_leaves,LABELS

class StandaloneSourceControls(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name)
        self.plan=dict(commands=[dict(label=x,argv=['chosen','-MD']) for x in LABELS],limits=dict(wall_seconds=120,parallel_jobs=1,nice=19,private_output_bytes=268435456))
    def check(self,plan,pin=None):
        p=self.root/'plan.json';p.write_text(json.dumps(plan));return checked_plan(p,pin or hashlib.sha256(p.read_bytes()).hexdigest())
    def test_exact_inventory_and_limits_model_only(self):self.assertEqual(len(self.check(self.plan)['commands']),14)
    def test_unknown_extra_or_reordered_command_rejected(self):
        for mutate in (lambda p:p['commands'].append(p['commands'][0]),lambda p:p['commands'].reverse(),lambda p:p['commands'][0].update(label='app')):
            p=copy.deepcopy(self.plan);mutate(p)
            with self.assertRaises(RuntimeError):self.check(p)
    def test_larger_budget_and_missing_system_dependencies_rejected(self):
        p=copy.deepcopy(self.plan);p['limits']['wall_seconds']=121
        with self.assertRaises(RuntimeError):self.check(p)
        p=copy.deepcopy(self.plan);p['commands'][0]['argv']=['chosen','-MMD']
        with self.assertRaises(RuntimeError):self.check(p)
    def test_rebound_plan_cannot_replace_operator_digest(self):
        with self.assertRaises(RuntimeError):self.check(self.plan,'0'*64)
    def test_missing_leaf_is_not_a_spawn_fallback(self):
        with self.assertRaises(FileNotFoundError):load_leaves(self.root)

if __name__=='__main__':unittest.main()
