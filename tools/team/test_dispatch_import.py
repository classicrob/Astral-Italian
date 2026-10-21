"""Asset-free import/transport/final-hook controls. No production acceptance."""
import base64
import copy
import io
import json
from pathlib import Path
import sys
import tarfile
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import dispatch_import as d
import dispatchproof as dp
import test_dispatchproof as fixtures


class Transport(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.dest = self.root/'unpacked'
        self.dest.mkdir()

    def archive(self, rows):
        stream = io.BytesIO()
        with tarfile.open(fileobj=stream, mode='w:gz') as tar:
            for name, content, kind in rows:
                info = tarfile.TarInfo(name)
                info.type = kind
                info.size = len(content)
                tar.addfile(info, io.BytesIO(content))
        raw = stream.getvalue()
        encoded = base64.b64encode(raw)
        (self.root/'archive.b64').write_bytes(encoded)
        return {'path': 'archive.b64', 'sha256': d.sha(raw), 'encoded_sha256': d.sha(encoded)}

    def test_valid_exact_regular_archive(self):
        arc = self.archive([('control/original.o', b'ELF', tarfile.REGTYPE)])
        d.unpack(self.root, arc, {'control/original.o': d.sha(b'ELF')}, self.dest)
        self.assertEqual((self.dest/'control/original.o').read_bytes(), b'ELF')

    def test_archive_escape_duplicate_links_extra_missing(self):
        cases = [
            [('../escape', b'ELF', tarfile.REGTYPE)],
            [('/absolute', b'ELF', tarfile.REGTYPE)],
            [('a', b'ELF', tarfile.SYMTYPE)],
            [('a', b'ELF', tarfile.LNKTYPE)],
            [('a', b'ELF', tarfile.REGTYPE), ('a', b'ELF', tarfile.REGTYPE)],
            [('unexpected', b'ELF', tarfile.REGTYPE)], [],
        ]
        for rows in cases:
            with self.subTest(rows=rows), tempfile.TemporaryDirectory() as out:
                with self.assertRaises((ValueError, tarfile.TarError)):
                    d.unpack(self.root, self.archive(rows), {'a': d.sha(b'ELF')}, Path(out))

    def test_corrupt_archive_hash_and_member_hash(self):
        for field in ('sha256', 'encoded_sha256'):
            arc = self.archive([('a', b'ELF', tarfile.REGTYPE)])
            arc[field] = '0'*64
            with self.assertRaises(ValueError):
                d.unpack(self.root, arc, {'a': d.sha(b'ELF')}, self.dest)
        with self.assertRaises(ValueError):
            d.unpack(self.root, self.archive([('a', b'ELF', tarfile.REGTYPE)]),
                     {'a': '0'*64}, self.dest)

    def test_archive_does_not_overwrite(self):
        (self.dest/'a').write_bytes(b'protected')
        with self.assertRaises(ValueError):
            d.unpack(self.root, self.archive([('a', b'ELF', tarfile.REGTYPE)]),
                     {'a': d.sha(b'ELF')}, self.dest)
        self.assertEqual((self.dest/'a').read_bytes(), b'protected')

    def test_metadata_paths_symlink_and_unknown_record_reject(self):
        (self.root/'real').mkdir()
        (self.root/'real/data').write_bytes(b'x')
        (self.root/'alias').symlink_to(self.root/'real', target_is_directory=True)
        for name in ('../x', '/x', 'alias/data', 'real//data', './real/data', 'missing'):
            with self.subTest(name=name), self.assertRaises((ValueError, FileNotFoundError)):
                d.artifact(self.root, {'path': name, 'sha256': d.sha(b'x')})
        with self.assertRaises(ValueError):
            d.artifact(self.root, {'path': 'real/data', 'sha256': d.sha(b'x'), 'execute': True})

    def test_duplicate_json_key_and_nonfinite_reject(self):
        for raw in (b'{"x":1,"x":2}', b'{"x":NaN}', b'{"x":Infinity}'):
            with self.assertRaises(ValueError):
                d.json_bytes(raw)

    def test_missing_secondary_and_negative_artifact_reject(self):
        for name in ('secondary.tar.gz.b64', 'dispatch/negative-ELFs/TU13-first-text.o'):
            with self.assertRaises(FileNotFoundError):
                d.artifact(self.root, {'path': name, 'sha256': '0'*64})
        with self.assertRaises(ValueError):
            d.negative_checks(self.root, [], {}, {}, self.dest, {}, {})


class HookContract(unittest.TestCase):
    def setUp(self):
        self.spec = {'base': 'a'*40, 'before': {}, 'after': {}}
        self.tools = {'dispatch_import.py': 'b'*64}
        self.record = {'type': d.TYPE, 'status': 'VERIFIED_NOT_LANDED',
                       'receipt_sha256': 'a'*64, 'spec': self.spec,
                       'spec_sha256': 'b'*64, 'plans': {'TU13': 'c'*64, 'TU14': 'd'*64},
                       'contract_sha256': 'e'*64, 'certificate_sha256': 'f'*64,
                       'verifier_tools': self.tools, 'candidate_tip': 'c'*40}

    def check(self, record=None, spec=None, tools=None, pinned=None):
        return d.final_hook(record or self.record,
                            pinned or d.sha(d.canonical(self.record)),
                            self.spec if spec is None else spec,
                            self.tools if tools is None else tools, 'c'*40)

    def test_exact_contract_not_landing_authority(self):
        self.assertFalse(self.check()['landing_authorized'])

    def test_stale_source_tool_record_unknown_missing_and_modes_reject(self):
        for key, value in [('type', 'PASS'), ('status', 'PASS'), ('mario52', True),
                           ('candidate_tip', '0'*40), ('spec', {'base': '0'*40}), ('plans', {'TU13': 'c'*64})]:
            record = copy.deepcopy(self.record)
            record[key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.check(record=record)
        record = copy.deepcopy(self.record)
        record.pop('certificate_sha256')
        with self.assertRaises(ValueError):
            self.check(record=record)
        with self.assertRaises(ValueError):
            self.check(tools={'dispatch_import.py': '0'*64})
        with self.assertRaises(ValueError):
            self.check(spec={'base': '0'*40})

    def test_rehashed_incompatible_mode_still_rejects(self):
        for extra in ('sanctioned_delta', 'approved_native_52', 'generic_cert'):
            record = {**self.record, extra: True}
            with self.assertRaises(ValueError):
                self.check(record=record, pinned=d.sha(d.canonical(record)))

    def test_commands_and_dependency_expectations_not_receipt_authority(self):
        d.commands_check('compiler -c src/a.cpp\n', [['compiler', '-c', 'src/a.cpp']])
        with self.assertRaises(ValueError):
            d.commands_check('compiler -c src/evil.cpp', [['compiler', '-c', 'src/a.cpp']])
        with self.assertRaises(ValueError):
            d.commands_check('', [])

    def test_wrong_certificate_plan_scope_hash_and_extra_reject(self):
        pins = {'TU13': '1'*64, 'TU14': '2'*64}
        receipt = {'spec': self.spec, 'dispatch_delta': {
            'secondary_control': {'sha256': '3'*64}, 'strict_remainder': {'sha256': '4'*64}}}
        cert = {'type': d.TYPE, 'canonical_spec_sha256': '5'*64, 'base': self.spec['base'],
                'after_trees': {}, 'TU13_plan_sha256': pins['TU13'],
                'TU14_plan_sha256': pins['TU14'], 'secondary_control_sha256': '3'*64,
                'fullcomparison_sha256': '4'*64}
        d.certificate_check(cert, receipt, pins, '5'*64)
        for key in cert:
            changed = copy.deepcopy(cert)
            changed[key] = 'wrong'
            with self.subTest(key=key), self.assertRaises(ValueError):
                d.certificate_check(changed, receipt, pins, '5'*64)
        with self.assertRaises(ValueError):
            d.certificate_check({**cert, 'generic': True}, receipt, pins, '5'*64)


class SemanticInventory(unittest.TestCase):
    def fixture(self):
        tests = fixtures.DispatchTests('test_tu13_classifies_only_pinned_symbol_changes')
        tests.setUp()
        self.addCleanup(tests.doCleanups)
        before, after, plans = {}, {}, {}
        for scope in d.SCOPES:
            tests.make_case(scope)
            # make_case creates new plan; regenerate both synthetic files.
            tests.build('before')
            tests.build('after')
            unit = tests.unit
            plans[scope] = copy.deepcopy(tests.plan)
            before[unit] = d.objsame.summarize(tests.root/'before.o')
            after[unit] = d.objsame.summarize(tests.root/'after.o')
        for i in range(2424):
            before[str(i)+'.o'] = after[str(i)+'.o'] = {'section .text#0': b'strict'}
        for plan in plans.values():
            plan['units'] = sorted(before)
        return before, after, plans

    def test_full_synthetic_inventory_and_extra_object_or_delta_reject(self):
        before, after, plans = self.fixture()
        configs = {'before': {}, 'after': {}}
        self.assertEqual(d.compare(before, after, plans, configs)['strict_objects'], 2424)
        after['0.o'] = {'section .text#0': b'changed'}
        with self.assertRaises(ValueError):
            d.compare(before, after, plans, configs)
        after['extra.o'] = {}
        with self.assertRaises(ValueError):
            d.compare(before, after, plans, configs)

    def test_alias_midbody_third_crossed_slot_and_unknown_target_reject(self):
        before, after, plans = self.fixture()
        unit = dp.SCOPES['TU13']['object']
        name = next(iter(plans['TU13']['symbols']))
        for target, addend in [(('sym', name), 0), (('sym', name), 4),
                               (('local', '.text#0', 16), 0), (('unknown', 0), 0)]:
            changed = copy.deepcopy(after)
            changed[unit]['relocations for .text#0'] = ((0, 1, addend, target),)
            with self.subTest(target=target), self.assertRaises(ValueError):
                d.compare(before, changed, plans, {'before': {}, 'after': {}})

    def test_plan_pins_wrong_scope_and_missing_source_reject(self):
        before, after, plans = self.fixture()
        pins = {s: dp.plan_hash(p) for s, p in plans.items()}
        spec = {'base': 'b'*40, 'units': dict.fromkeys(before),
                'before': plans['TU13']['bindings']['source_trees']['before'],
                'after': plans['TU13']['bindings']['source_trees']['after'],
                'toolchain': {'compiler': 'f'*64}, 'generators': {'generator': '1'*64}}
        class NoSources:
            def read(self, revision, path):
                raise ValueError('unqualified source')
        for changed in ({**pins, 'TU13': '0'*64}, {'TU13': pins['TU13']}, pins):
            with self.subTest(pins=changed), self.assertRaises(ValueError):
                d.plans_check(plans, changed, spec, NoSources(), 'c'*40)


class ProvenanceControls(unittest.TestCase):
    def test_tracked_header_wrong_hash_and_path_escape_reject(self):
        class Sources:
            def read(self, revision, name):
                if revision == 'a'*40 and name == 'include/header.hpp':
                    return b'header'
                raise ValueError('missing local immutable source')
        text = 'object: #deps 1\n    include/header.hpp\n'
        hashes = {'include/header.hpp': d.sha(b'header')}
        d.dependency_check(text, hashes, 'before', 'a'*40, Sources(), {})
        for name, digest in [('include/header.hpp', '0'*64), ('../outside', '0'*64),
                             ('/foreign/include/header.hpp', d.sha(b'header'))]:
            with self.subTest(name=name), self.assertRaises(ValueError):
                d.dependency_check('object:\n    '+name+'\n', {name: digest},
                                   'before', 'a'*40, Sources(), {})
        with self.assertRaises(ValueError):
            d.dependency_check(text, {}, 'before', 'a'*40, Sources(), {})

    def test_wrong_private_dol_and_missing_operator_command_qualification_reject(self):
        trust = {'spec_bytes': d.canonical({'units': dict.fromkeys(str(i) for i in range(2426))}),
                 'tip': 'b'*40, 'host': 'test', 'plan_pins': {}, 'contract_sha256': '0'*64,
                 'verifier_tools': d.verifier_tools(), 'command_tokens': {}}
        with self.assertRaises(ValueError):
            d.verify(b'{}', Path('.'), trust, None, b'not-original-dol')
        trust['command_tokens'] = dict.fromkeys(('before', 'after', 'TU13', 'TU14'), [['compiler']])
        with self.assertRaises(ValueError):
            d.verify(b'{}', Path('.'), trust, None, b'not-original-dol')


class AbsoluteAlias(SemanticInventory):
    def test_absolute_alias_present_in_both_phases_still_rejects(self):
        before, after, plans = self.fixture()
        name = next(iter(plans['TU13']['symbol_map']))
        for phase, tree in [('before', before), ('after', after)]:
            address = plans['TU13']['symbol_map'][name][phase][0] + 4
            tree['0.o'] = {'named symbols': (('absolute_alias', 16, 0, 'special0xfff1', address, 0),),
                           'relocations for .data#0': ((0, 1, 0, ('sym', 'absolute_alias')),)}
            with self.assertRaises(ValueError):
                d.census(tree, plans, {}, phase)

    def test_config_only_alias_present_in_both_phases_still_rejects(self):
        before, after, plans = self.fixture()
        name = next(iter(plans['TU13']['symbol_map']))
        for phase, tree in [('before', before), ('after', after)]:
            tree['0.o'] = {'relocations for .data#0': ((0, 10, 0, ('sym', 'config_alias')),)}
            address = plans['TU13']['symbol_map'][name][phase][0] + 4
            with self.assertRaises(ValueError):
                d.census(tree, plans, {'config_alias': [('.text', address)]}, phase)


if __name__ == '__main__':
    unittest.main()
