"""Fail-closed tests for actual artifact verification and landing contract binding."""
import base64
import copy
import io
import json
from pathlib import Path
import sys
import tarfile
import tempfile
import unittest
from unittest.mock import patch
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parent))
import remoteproof as rp
from test_objsame import build_elf


class RemoteProofTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name).resolve()
        obj = self.root / 'fixture.o'
        build_elf(obj)
        good = obj.read_bytes()
        build_elf(obj, text1=b'\x60\x00\x00\x01')
        bad = obj.read_bytes()
        self.rel = 'src/Game/Test.o'
        self.spec = {'schema': 1, 'base': 'base', 'before': {'src': 'old'},
                     'after': dict(src='src', include='include', libs='libs', config='config'),
                     'toolchain': {'compiler': 'hash'}, 'generators': {'objsame': 'hash'},
                     'units': {self.rel: {'source': 'src/Game/Test.cpp', 'options': {'cflags': ['-O4']}}}}
        self.members = {'before/' + self.rel: good, 'after/' + self.rel: good,
                        'control/original.o': good, 'control/perturbed.o': bad, 'control/restored.o': good}
        cert = b'base src include libs\n'
        (self.root / 'proof.cert').write_bytes(cert)
        source_hash = rp.digest(b'source')
        self.receipt = {'schema': 1, 'status': 'PASS', 'host': 'M1', 'spec': copy.deepcopy(self.spec),
                        'restored_trees': self.spec['after'], 'clean': {'before': True, 'after': True},
                        'builds': dict(before=0, after=0, control=0, restore=0),
                        'certificate': {'path': 'proof.cert', 'sha256': rp.digest(cert)},
                        'control': {'source': 'src/Game/Test.cpp', 'object': self.rel,
                                    'source_sha256': source_hash, 'restored_source_sha256': source_hash}}
        data = b'actual proof provenance'
        (self.root / 'provenance.txt').write_bytes(data)
        self.receipt['provenance'] = {key: dict(path='provenance.txt', sha256=rp.digest(data)) for key in
            ('before_log', 'after_log', 'control_log', 'recipe', 'unit_manifest', 'spec_comparison')}
        self.pack()

    def tearDown(self):
        self.tmp.cleanup()

    def pack(self, extra=None):
        out = io.BytesIO()
        with tarfile.open(fileobj=out, mode='w:gz') as tar:
            for name, data in self.members.items():
                info = tarfile.TarInfo(name)
                info.size = len(data)
                tar.addfile(info, io.BytesIO(data))
            if extra:
                tar.addfile(extra)
        raw = out.getvalue()
        encoded = base64.b64encode(raw)
        (self.root / 'objects.b64').write_bytes(encoded)
        self.receipt['archive'] = dict(path='objects.b64', sha256=rp.digest(raw), encoded_sha256=rp.digest(encoded))
        self.receipt['files'] = {p: rp.digest(d) for p, d in self.members.items()}

    def verify(self):
        with patch.object(rp, 'git', return_value='src/Game/Test.cpp'), \
             patch.object(rp.subprocess, 'check_output', return_value=b'source'):
            return rp.verify(self.receipt, self.root, self.spec, 'M1')

    def rejects(self):
        with self.assertRaises((ValueError, KeyError)):
            self.verify()

    def test_actual_objects_and_control_pass(self):
        self.assertEqual(self.verify(), b'base src include libs\n')

    def test_stale_contract_fields(self):
        for field in ('base', 'before', 'after', 'units', 'toolchain', 'generators'):
            with self.subTest(field=field):
                saved = self.receipt['spec'][field]
                self.receipt['spec'][field] = 'stale'
                self.rejects()
                self.receipt['spec'][field] = saved

    def test_wrong_host(self):
        self.receipt['host'] = 'elsewhere'
        self.rejects()

    def test_failed_or_unclean_build(self):
        for phase in self.receipt['builds']:
            self.receipt['builds'][phase] = 1
            self.rejects()
            self.receipt['builds'][phase] = 0
        self.receipt['clean']['after'] = False
        self.rejects()

    def test_wrong_certificate_even_with_updated_hash(self):
        data = b'other src include libs\n'
        (self.root / 'proof.cert').write_bytes(data)
        self.receipt['certificate']['sha256'] = rp.digest(data)
        self.rejects()

    def test_missing_and_extra_objects(self):
        del self.members['before/' + self.rel]
        self.pack()
        self.rejects()
        self.members['unexpected.o'] = b'extra'
        self.pack()
        self.rejects()

    def test_object_hash_tamper(self):
        self.receipt['files']['before/' + self.rel] = '0' * 64
        self.rejects()

    def test_semantic_difference_with_valid_hashes(self):
        self.members['after/' + self.rel] = self.members['control/perturbed.o']
        self.pack()
        self.rejects()

    def test_unchanged_control(self):
        self.members['control/perturbed.o'] = self.members['control/original.o']
        self.pack()
        self.rejects()

    def test_unrestored_control(self):
        self.members['control/restored.o'] = self.members['control/perturbed.o']
        self.pack()
        self.rejects()

    def test_unrestored_source(self):
        self.receipt['control']['restored_source_sha256'] = 'bad'
        self.rejects()

    def test_unrelated_control(self):
        self.receipt['control']['source'] = 'unrelated.cpp'
        self.rejects()

    def test_archive_traversal_symlink_and_duplicate(self):
        for name, kind in (('../escape', tarfile.REGTYPE), ('link', tarfile.SYMTYPE),
                           ('control/original.o', tarfile.REGTYPE)):
            with self.subTest(name=name):
                info = tarfile.TarInfo(name)
                info.type = kind
                info.linkname = '/etc/passwd' if kind == tarfile.SYMTYPE else ''
                self.pack(extra=info)
                self.rejects()

    def test_missing_or_tampered_provenance(self):
        del self.receipt['provenance']['recipe']
        self.rejects()

    def test_archive_hash_tamper(self):
        self.receipt['archive']['sha256'] = 'bad'
        self.rejects()

    def test_certificate_path_escape(self):
        self.receipt['certificate']['path'] = '../proof.cert'
        self.rejects()

    def test_artifact_symlink(self):
        (self.root / 'alias.cert').symlink_to(self.root / 'proof.cert')
        self.receipt['certificate']['path'] = 'alias.cert'
        self.rejects()

    def equivalent_fixture(self):
        self.spec['before'] = dict(src='old', include='old-include', libs='old-libs', config='old-config')
        self.receipt['spec'] = copy.deepcopy(self.spec)
        self.spec['base'] = 'current'

    def equivalence_git(self, *args):
        if args[0] == 'diff':
            return 'PLAY.md\nnative/play/PLAY.md' if args[2] == 'base' else 'src/Game/Test.cpp'
        if args[0] == 'rev-parse':
            path = args[1].split(':', 1)[1]
            return self.spec['before'].get(path, 'same-' + path)
        self.fail(args)

    def test_docs_base_equivalence_preserves_original_proof(self):
        self.equivalent_fixture()
        original = copy.deepcopy(self.receipt)
        with patch.object(rp, 'git', side_effect=self.equivalence_git), \
             patch.object(rp.subprocess, 'run', return_value=SimpleNamespace(returncode=0)), \
             patch.object(rp.subprocess, 'check_output', return_value=b'source'):
            result = rp.verify(self.receipt, self.root, self.spec, 'M1', equivalent_base='base')
        self.assertEqual(result, b'current src include libs\n')
        self.assertEqual(self.receipt, original)
        self.assertEqual((self.root / 'proof.cert').read_bytes(), b'base src include libs\n')

    def test_base_equivalence_requires_explicit_original_base(self):
        self.equivalent_fixture()
        for original in (None, 'another-base'):
            with self.subTest(original=original), self.assertRaises(ValueError):
                rp.base_equivalence(self.receipt['spec'], self.spec, original)

    def test_base_equivalence_rejects_other_contract_changes(self):
        self.equivalent_fixture()
        for field in ('before', 'after', 'units', 'toolchain', 'generators'):
            saved = self.receipt['spec'][field]
            self.receipt['spec'][field] = 'changed'
            with self.subTest(field=field), self.assertRaises(ValueError):
                rp.base_equivalence(self.receipt['spec'], self.spec, 'base')
            self.receipt['spec'][field] = saved

    def test_base_equivalence_rejects_nonancestor(self):
        self.equivalent_fixture()
        with patch.object(rp.subprocess, 'run', return_value=SimpleNamespace(returncode=1)), \
             self.assertRaises(ValueError):
            rp.base_equivalence(self.receipt['spec'], self.spec, 'base')

    def test_base_equivalence_rejects_source_advance(self):
        self.equivalent_fixture()
        with patch.object(rp, 'git', return_value='native/app/main.cpp'), \
             patch.object(rp.subprocess, 'run', return_value=SimpleNamespace(returncode=0)), \
             self.assertRaises(ValueError):
            rp.base_equivalence(self.receipt['spec'], self.spec, 'base')

    def test_base_equivalence_rejects_compile_input_tree_changes(self):
        self.equivalent_fixture()
        for changed_path in (*rp.INPUTS, 'tools', 'configure.py'):
            def changed_git(*args):
                if args == ('rev-parse', 'current:' + changed_path):
                    return 'different'
                return self.equivalence_git(*args)
            with self.subTest(path=changed_path), \
                 patch.object(rp, 'git', side_effect=changed_git), \
                 patch.object(rp.subprocess, 'run', return_value=SimpleNamespace(returncode=0)), \
                 self.assertRaises(ValueError):
                rp.base_equivalence(self.receipt['spec'], self.spec, 'base')

    def test_base_equivalence_rejects_unbound_before_tree(self):
        self.equivalent_fixture()
        def unbound_git(*args):
            if args[0] == 'rev-parse' and args[1].endswith(':src'):
                return 'same-but-unbound'
            return self.equivalence_git(*args)
        with patch.object(rp, 'git', side_effect=unbound_git), \
             patch.object(rp.subprocess, 'run', return_value=SimpleNamespace(returncode=0)), \
             self.assertRaises(ValueError):
            rp.base_equivalence(self.receipt['spec'], self.spec, 'base')

    def test_sanction_metadata_requires_explicit_opt_in(self):
        self.receipt['sanctioned_delta'] = {}
        self.rejects()

    def test_native52_exact_commits_and_paths(self):
        self.equivalent_fixture()
        self.receipt['spec']['base'] = rp.mario52.BASE
        def approved_git(*args):
            if args[0] == 'rev-list':
                return '\n'.join(rp.APPROVED_NATIVE_52)
            if args[0] == 'diff-tree':
                return '\n'.join(rp.APPROVED_NATIVE_52[args[-1]])
            return self.equivalence_git(*args)
        with patch.object(rp, 'git', side_effect=approved_git), \
             patch.object(rp.subprocess, 'run', return_value=SimpleNamespace(returncode=0)):
            result = rp.base_equivalence(self.receipt['spec'], self.spec, rp.mario52.BASE, True)
        self.assertTrue(result['approved_native_52'])

    def test_native52_rejects_additional_commit(self):
        self.equivalent_fixture()
        self.receipt['spec']['base'] = rp.mario52.BASE
        def extra_git(*args):
            if args[0] == 'rev-list':
                return '\n'.join([*rp.APPROVED_NATIVE_52, 'unreviewed-native-change'])
            return self.equivalence_git(*args)
        with patch.object(rp, 'git', side_effect=extra_git), \
             patch.object(rp.subprocess, 'run', return_value=SimpleNamespace(returncode=0)), \
             self.assertRaisesRegex(ValueError, 'unapproved native base commits'):
            rp.base_equivalence(self.receipt['spec'], self.spec, rp.mario52.BASE, True)

    def test_native52_rejects_wrong_paths(self):
        self.equivalent_fixture()
        self.receipt['spec']['base'] = rp.mario52.BASE
        def wrong_git(*args):
            if args[0] == 'rev-list':
                return '\n'.join(rp.APPROVED_NATIVE_52)
            if args[0] == 'diff-tree':
                return 'native/unreviewed.cpp'
            return self.equivalence_git(*args)
        with patch.object(rp, 'git', side_effect=wrong_git), \
             patch.object(rp.subprocess, 'run', return_value=SimpleNamespace(returncode=0)), \
             self.assertRaisesRegex(ValueError, 'path mismatch'):
            rp.base_equivalence(self.receipt['spec'], self.spec, rp.mario52.BASE, True)


if __name__ == '__main__':
    unittest.main()
