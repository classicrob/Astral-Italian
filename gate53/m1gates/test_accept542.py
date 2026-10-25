import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch
spec=importlib.util.spec_from_file_location('accept542',Path(__file__).with_name('accept542.py'))
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class Commands(unittest.TestCase):
 def test_full_plan_roles_and_warm_paths(self):
  with tempfile.TemporaryDirectory() as d:
   root=Path(d);out=root/'cohort'
   args=['accept542','--baseline-dir','/read-only/541','--candidate-dir','/read-only/542','--disc','/disc','--tier','m4',
         '--out',str(out),'--heavy-lock','/heavy','--native-lock','/native','--baseline-id','dev.petari.Petari.perftest541',
         '--candidate-id','dev.petari.Petari.perftest542','--spots','--plan-only']
   with patch.object(m,'ROOT',root),patch.object(sys,'argv',args),patch.object(m.subprocess,'run') as run:
    self.assertEqual(m.main(),0);run.assert_not_called()
   plan=json.loads((out/'COMMANDS.json').read_text())['plan'];self.assertEqual(len(plan),30)
   direct=[a for a in plan if a['kind']=='direct'];self.assertEqual(len(direct),20)
   for a in plan:
    self.assertEqual('--require-boot-responsiveness' in a['command'],a['build']=='542')
    self.assertEqual('--baseline' in a['command'],a['build']=='541')
    self.assertEqual(a['command'][a['command'].index('--tier')+1],'m4')
   for a in direct:
    self.assertEqual(a['command'][:3],['/usr/bin/lockf','-k','/heavy'])
    if 'spot-9' in a['name']:self.assertIn('IslandFleetGalaxy',a['folder'])
    if 'spot-10' in a['name']:self.assertIn('VsKoopaLv1Galaxy',a['folder'])
   for i,a in enumerate(plan):
    if a['name']=='warm-second-r1':
     self.assertEqual(plan[i-1]['build'],a['build'])
     self.assertEqual(a['command'][a['command'].index('--warm-from')+1],plan[i-1]['folder'])
     self.assertIn('--keep-cache',a['command'])
if __name__=='__main__':unittest.main()
