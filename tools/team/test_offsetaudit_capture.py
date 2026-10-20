#!/usr/bin/env python3
"""Exact production-token mapping and opt-in compiler guard regressions."""
from pathlib import Path
import contextlib, importlib.util, io, json, sys, tempfile, unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parent))
import offsetaudit as audit
import offsetaudit_capture as capture


class CaptureTests(unittest.TestCase):
    def setup_tree(self, directory, original, encoded):
        capture.ROOT=Path(directory)/'root';capture.LEGACY=Path(directory)/'legacy'
        for root in [capture.ROOT,capture.LEGACY]:root.mkdir(exist_ok=True)
        (capture.ROOT/'x.cpp').write_text(original)
        prefix='#include <petari/utf16.h>\n#include <petari/msl_stdio.h>\n#line 1 "x.cpp"\n'
        (capture.LEGACY/'x.cpp').write_text(prefix+encoded)
        capture.AUD=audit;capture.MAPS={}
        return len(prefix.encode())

    def test_utf16_and_literal_expansion_exact_token_mapping(self):
        with tempfile.TemporaryDirectory() as tmp:
            raw='wchar_t names[4]; auto s="日本語"; p->_2C=4;'
            enc='PetariChar16 names[4]; auto s="\\223\\372\\226\\173\\214\\352"; p->_2C=4;'
            prefix=self.setup_tree(tmp,raw,enc)
            self.assertIsNotNone(capture.tokenmap('x.cpp'))
            rawpos=raw.encode().index(b'_2C');encpos=prefix+enc.encode().index(b'_2C')
            self.assertEqual(capture.original_offset('x.cpp',encpos),rawpos)
            self.assertEqual(capture.original_offset('x.cpp',encpos+3,True),rawpos+3)

    def test_unapproved_token_change_cannot_map(self):
        with tempfile.TemporaryDirectory() as tmp:
            self.setup_tree(tmp,'p->_2C=4;','p->_2D=4;')
            self.assertIsNone(capture.tokenmap('x.cpp'))

    def test_macro_expansion_retains_provenance_and_does_not_validate_as_field_token(self):
        with tempfile.TemporaryDirectory() as tmp:
            prefix=self.setup_tree(tmp,'GET;','GET;')
            capture.BYFILE={'x.cpp':[{'id':'row','byte_offset':0,'end_byte_offset':3}]}
            loc={'file':str(capture.LEGACY/'x.cpp'),'offset':prefix,'tokLen':3}
            node={'id':'node','kind':'MemberExpr','name':'flag','type':{'qualType':'bool'},
                  'range':{'begin':{'spellingLoc':loc,'expansionLoc':loc},
                           'end':{'spellingLoc':loc,'expansionLoc':loc}}}
            facts,decls,errors=capture.reduce_ast(node)
            self.assertEqual(len(facts),1)
            self.assertIn('macro_provenance',facts[0])
            self.assertFalse(facts[0]['token_validation'])

    def test_cached_dependency_drift_rejects(self):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp)/'type.hpp';p.write_text('changed')
            with self.assertRaisesRegex(ValueError,'dependency drift'):
                capture.verify_dependencies({str(p):{'sha256':'old'}})

    def test_no_compiler_in_dry_run(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);(root/'inventory.json').write_text(json.dumps({'files':{'x.cpp':'fixture'},'rows':[{'id':'x','file':'x.cpp','byte_offset':0}]}))
            plan={'schema':1,'root':tmp,'inventory':'inventory.json','command':['clang++','-fsyntax-only']}
            (root/'plan.json').write_text(json.dumps(plan))
            with patch.object(capture.subprocess,'run',side_effect=AssertionError('no implicit compiler')):
                with contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(capture.main(['--plan',str(root/'plan.json'),'--out',str(root/'out')]),0)

    def test_stale_source_pin_rejects_before_compiler(self):
        with tempfile.TemporaryDirectory() as tmp:
            capture.ROOT=Path(tmp);(capture.ROOT/'x.cpp').write_text('changed')
            capture.INPUT={'files':{'x.cpp':'old'}}
            with self.assertRaisesRegex(ValueError,'source pin drift'):
                capture.verify_source_pins()


if __name__=='__main__':unittest.main()
