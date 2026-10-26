#!/usr/bin/env python3
"""CPU-only checks of baseline-control construction; no native process or cache."""
import importlib.util
import tempfile
import unittest
from unittest import mock
from pathlib import Path

spec=importlib.util.spec_from_file_location('proof',Path(__file__).with_name('check_stale_fixtures543.py'))
proof=importlib.util.module_from_spec(spec);spec.loader.exec_module(proof)

class BaselineControlTests(unittest.TestCase):
    def test_unavailable_never_becomes_an_executed_or_passing_check(self):
        line='NOT RUN native_skip_donor_integration: UNAVAILABLE IN SMG2; native/STUBS.md; docs/port/HOOK_TRANSFER.md'
        row=proof.unavailable_checks(line)[0]
        self.assertEqual(row['status'],'NOT_RUN')
        self.assertFalse(row['executed']);self.assertFalse(row['passed'])
        self.assertIn('STUBS.md',row['reason']);self.assertIn('HOOK_TRANSFER.md',row['reason'])
        with self.assertRaisesRegex(RuntimeError,'duplicate'):
            proof.unavailable_checks(line+'\n'+line)
        with self.assertRaisesRegex(RuntimeError,'malformed'):
            proof.unavailable_checks('NOT RUN missing-reason')

    def test_separate_donor_row_requires_77_and_explicit_unavailable_citations(self):
        line='NOT RUN native_skip_donor_integration: UNAVAILABLE IN SMG2; native/STUBS.md; docs/port/HOOK_TRANSFER.md'
        record={'exit':77,'timed_out':False}
        row=proof.donor_unavailable(record,line)
        self.assertEqual(row['ctest_status'],'SKIPPED');self.assertFalse(row['passed']);self.assertFalse(row['executed'])
        for wrong in ({'exit':0,'timed_out':False},{'exit':77,'timed_out':True}):
            with self.assertRaisesRegex(RuntimeError,'SKIPPED'):
                proof.donor_unavailable(wrong,line)
        for wrong in ('',line.replace('native_skip_donor_integration','different'),line.replace('native/STUBS.md','missing')):
            with self.assertRaises(RuntimeError):proof.donor_unavailable(record,wrong)

    def test_supported_mod_fix_binds_every_shipping_row_mapping_blob(self):
        fix='fd3d120e8057695948a831d036159f2b402751a8'
        paths=proof.PRODUCTION_FIXES[fix]
        self.assertIn('native/input/mods.cpp',paths)
        self.assertIn('native/home_menu/hbm_bridge.cpp',paths)
        self.assertIn('native/home_menu/mods_menu.cpp',paths)
        self.assertIn('native/home_menu/include/petari/mods_menu.hpp',paths)
        def git(*args):
            if args[0]=='merge-base':return (proof.BASE+'\n').encode()
            if args[0]=='diff':return ('\n'.join(paths)+'\n').encode()
            if args[0]=='show':return args[1].split(':',1)[1].encode()
            self.fail(args)
        with mock.patch.object(proof,'git',side_effect=git):
            allowed,blobs=proof.production_scope([fix])
        self.assertEqual(allowed,set(paths));self.assertEqual(set(blobs),set(paths))

    def test_production_scope_refuses_unknown_or_different_shipping_blob(self):
        with self.assertRaisesRegex(RuntimeError,'unapproved'):
            proof.production_scope(['unknown'])
        fix=next(iter(proof.PRODUCTION_FIXES))
        def git(*args):
            if args[0]=='merge-base':return (proof.BASE+'\n').encode()
            if args[0]=='diff':return b'native/platform/dvd/dvd_host.cpp\n'
            if args[0]=='show':return b'approved' if args[1].startswith(fix+':') else b'different'
            self.fail(args)
        with mock.patch.object(proof,'git',side_effect=git):
            with self.assertRaisesRegex(RuntimeError,'blob differs'):
                proof.production_scope([fix])

    def test_production_scope_keeps_an_explicit_approved_hash_binding(self):
        fix=next(iter(proof.PRODUCTION_FIXES))
        def git(*args):
            if args[0]=='merge-base':return (proof.BASE+'\n').encode()
            if args[0]=='diff':return b'native/platform/dvd/dvd_host.cpp\n'
            if args[0]=='show':return b'exact approved source'
            self.fail(args)
        with mock.patch.object(proof,'git',side_effect=git):
            allowed,blobs=proof.production_scope([fix])
        self.assertEqual(allowed,{'native/platform/dvd/dvd_host.cpp'})
        self.assertEqual(blobs,{'native/platform/dvd/dvd_host.cpp':proof.hashlib.sha256(b'exact approved source').hexdigest()})

    def test_production_scope_refuses_a_different_base(self):
        fix=next(iter(proof.PRODUCTION_FIXES))
        with mock.patch.object(proof,'git',return_value=b'different-base\n'):
            with self.assertRaisesRegex(RuntimeError,'pinned main'):
                proof.production_scope([fix])

    def test_compile_uses_exact_baseline_and_preserves_fixture_relative_includes(self):
        row={'directory':'/private/build','file':'/checkout/native/tests/input_tests.cpp',
             'arguments':['clang++','-I/production','-DKEEP=1','-MD','-MF','old.d','-o','old.o','-c','/checkout/native/tests/input_tests.cpp']}
        args=proof.compile_control(row,Path('/private/main.cpp'),Path('/private/main.o'))
        self.assertEqual(args,['clang++','-I/production','-DKEEP=1','-c','-iquote/checkout/native/tests','/private/main.cpp','-o','/private/main.o'])

    def test_main_input_uses_original_archive_without_new_sdk_wrapper(self):
        with tempfile.TemporaryDirectory(prefix='stale-fixture-proof-') as tmp:
            root=Path(tmp).resolve();directory=root/'CMakeFiles/petari_input_tests.dir';directory.mkdir(parents=True)
            obj=directory/'input_tests.cpp.o';wrapper=directory/'kpad_test_support.cpp.o';archive=root/'libpetari_input.a'
            (directory/'link.txt').write_text(f'clang++ {obj} {wrapper} {archive} -o petari_input_tests')
            row={'directory':str(root),'file':'/checkout/native/tests/input_tests.cpp',
                 'arguments':['clang++','-o',str(obj),'-c','/checkout/native/tests/input_tests.cpp']}
            args,cwd=proof.link_control(row,'petari_input_tests',root/'main.o',root/'main')
            self.assertEqual(cwd,root)
            self.assertIn(str(archive),args);self.assertIn(str(root/'main.o'),args)
            self.assertNotIn(str(obj),args);self.assertNotIn(str(wrapper),args)
            self.assertEqual(args[-2:],['-o',str(root/'main')])
            (directory/'link.txt').write_text(f'clang++ {obj} {archive} -o petari_input_tests')
            with self.assertRaisesRegex(RuntimeError,'SDK wrapper linkage'):
                proof.link_control(row,'petari_input_tests',root/'main.o',root/'main')

    def test_unrelated_object_cannot_be_replaced_as_fixture(self):
        with tempfile.TemporaryDirectory(prefix='stale-fixture-proof-') as tmp:
            root=Path(tmp).resolve();directory=root/'CMakeFiles/unit.dir';directory.mkdir(parents=True)
            (directory/'link.txt').write_text('clang++ another.o -o unit')
            row={'directory':str(root),'file':'fixture.cpp','arguments':['clang++','-o',str(directory/'fixture.o'),'-c','fixture.cpp']}
            with self.assertRaisesRegex(RuntimeError,'uniquely replaced'):
                proof.link_control(row,'unit',root/'main.o',root/'main')

if __name__=='__main__':unittest.main()
