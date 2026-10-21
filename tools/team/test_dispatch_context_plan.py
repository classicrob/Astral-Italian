"""Static/source-only disabled plan checks; never call runner.main or spawn."""
import ast
import hashlib
import json
from pathlib import Path
import unittest
import dispatch_context_check as runner

class ContextPlanModels(unittest.TestCase):
    def test_disabled_scope_and_pins(self):
        base=Path(runner.__file__).parent
        plan=json.loads((base/'DISPATCH_CONTEXT_OBSERVATION_PLAN.json').read_text())
        self.assertIs(plan['execution_enabled'],False)
        self.assertEqual(plan['root'],str(runner.ROOT))
        self.assertEqual(plan['commands'],runner.COMMANDS)
        self.assertEqual(len(runner.COMMANDS),1)
        self.assertEqual(runner.COMMANDS[0][1:],['-c','import os; print("SOLP_EXEC_IDENTITY",os.getpid(),os.getpgrp(),os.getsid(0),os.getuid())'])
        for name,pin in plan['source_files'].items():
            self.assertEqual(hashlib.sha256((base/name).read_bytes()).hexdigest(),pin)
        self.assertEqual([plan[n] for n in ('total_seconds','work_seconds','cleanup_seconds','maximum_owned_growth_bytes','minimum_free_bytes')],[60,20,4,64*1024**2,50*1024**3])
    def test_disabled_precedes_lock_and_output(self):
        tree=ast.parse(Path(runner.__file__).read_text())
        main=next(n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='main')
        guard=next(n for n in main.body if isinstance(n,ast.If) and 'execution_enabled' in ast.unparse(n.test))
        lock=next(n for n in ast.walk(main) if isinstance(n,ast.Call) and ast.unparse(n.func)=='fcntl.lockf')
        self.assertLess(guard.lineno,lock.lineno)
        self.assertIn('raise RuntimeError',ast.unparse(guard))
        self.assertNotIn('command-1.log',Path(runner.__file__).read_text())

if __name__=='__main__':unittest.main()
