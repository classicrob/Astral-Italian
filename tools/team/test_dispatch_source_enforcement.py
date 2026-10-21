"""No-process source/temporary-file controls for two full-only enforcement seams."""
import ast
import hashlib
from pathlib import Path
import tempfile
import unittest
import dispatch_fullproof as full

class SourceEnforcement(unittest.TestCase):
    def test_transform_exact_and_drift(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);path=root/'tools/transform_dep.py';path.parent.mkdir();path.write_bytes(b'exact')
            contract={'execution_sources':{'before':{'tools/transform_dep.py':hashlib.sha256(b'exact').hexdigest()}}}
            full.check_execution_sources(root,'before',contract)
            path.write_bytes(b'drift')
            with self.assertRaises(AssertionError):full.check_execution_sources(root,'before',contract)
            path.unlink()
            with self.assertRaises(AssertionError):full.check_execution_sources(root,'before',contract)
            path.symlink_to(root/'missing')
            with self.assertRaises(AssertionError):full.check_execution_sources(root,'before',contract)
    def test_phase_pin_and_extra_source_reject(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);path=root/'tools/transform_dep.py';path.parent.mkdir();path.write_bytes(b'before')
            c={'execution_sources':{'after':{'tools/transform_dep.py':hashlib.sha256(b'after').hexdigest()}}}
            with self.assertRaises(AssertionError):full.check_execution_sources(root,'after',c)
            c['execution_sources']['after']['other']='x'
            with self.assertRaises(AssertionError):full.check_execution_sources(root,'after',c)
    def test_package_lock_encloses_budget_and_terminal(self):
        source=(Path(full.__file__).parent/'dispatch_package.py').read_text();tree=ast.parse(source)
        main=next(x for x in tree.body if isinstance(x,ast.FunctionDef) and x.name=='main')
        guarded=next(x for x in main.body if isinstance(x,ast.Try) and x.finalbody)
        first=ast.unparse(guarded.body[0])
        self.assertIn('fcntl.LOCK_EX | fcntl.LOCK_NB',first)
        self.assertEqual(ast.unparse(guarded.finalbody[0]),'lock.close()')
        guarded_source='\n'.join(ast.unparse(x) for x in guarded.body)
        self.assertIn('budget.FullBudgetJob(',guarded_source)
        self.assertIn('monitoring.terminal_publish(',guarded_source)
        self.assertNotIn('monitoring.terminal_publish(', '\n'.join(ast.unparse(x) for x in main.body if x is not guarded))

if __name__=='__main__':unittest.main()
