"""Invented controls only: no retained ELF, original DOL, compiler or runtime.

Declarative v1 cases are instantiated against fresh byte-backed fixtures. Extra
controls mutate backing bytes as well as refreshed pins; metadata-only tests are
reported separately. No control grants an original owner or API admission.
"""
import copy
import json
import struct
import unittest
from pathlib import Path
from unittest import mock
import nervemap_ownership_proposal as checker
from ownership_proposal_fixture import Fixture, walk

DATA = Path(__file__).parent / 'testdata/ownership-proposal'
CASES = json.loads((DATA / 'cases.json').read_bytes())['cases']
SELECTION = json.loads((DATA / 'selection-template.json').read_bytes())


def edit(root, mutation):
    keys = mutation['path'].split('/')[1:]
    parent = root
    for key in keys[:-1]:
        parent = parent[int(key)] if type(parent) is list else parent[key]
    key = keys[-1]
    if mutation['op'] == 'remove':
        if type(parent) is list:
            parent.pop(int(key))
        else:
            del parent[key]
    elif type(parent) is list:
        if key == '-':
            parent.append(copy.deepcopy(mutation['value']))
        elif mutation['op'] == 'add':
            parent.insert(int(key), copy.deepcopy(mutation['value']))
        else:
            parent[int(key)] = copy.deepcopy(mutation['value'])
    else:
        parent[key] = copy.deepcopy(mutation['value'])


def selection(f):
    result = copy.deepcopy(SELECTION)
    rec = f.relation['reconstruction']
    for provider in result['providers'] + [result['selected_provider']]:
        provider['definition'] = copy.deepcopy(rec['singleton'])
        provider['section'] = copy.deepcopy(rec['singleton_section'])
        f.put(provider['original_input_object'], f.elf)
    for node in walk(result):
        if type(node) is dict and set(node) == {'path', 'sha256'}:
            f.put(node, f.blobs.get(node['path'], b'{}'))
    f.relation['original_selection'] = result
    return result


def end8(f):
    f.symbols = f.symbols.replace(b'type:object size:0x4', b'type:object size:0x8', 1)
    f.refresh_symbols()
    f.relation['mapped_record']['declared_size'] = 8
    f.relation['extents']['mapped_span']['size'] = 8
    f.relation['extents']['unassigned_ranges'] = [dict(range=dict(address='0x81002004', size=4),
                state='UNKNOWN', reason='No original contribution extent established')]


def review(f):
    covered = {k: v for k, v in f.envelope.items() if k != 'review'}
    value = dict(status='reviewed_design_only', scope='singleton_data_ownership_disabled_design', reviewer='invented reviewer',
                 claim_sha256=checker.digest(checker.canonical(covered)),
                 receipt=dict(path='SYNTHETIC/design-review', sha256='0' * 64))
    # Exact enum vocabulary comes from the copied sealed schema.
    f.put_json(value['receipt'], {k: v for k, v in value.items() if k != 'receipt'})
    f.envelope['review'] = value


def instantiate(case):
    f = Fixture()
    if case['base'] == 'selection_fragment':
        target = selection(f)
    else:
        target = f.envelope
    name = case['id']
    if name == 'End_map8_ELF4_unassigned_tail' or name == 'original4B_proof_does_not_own_map8':
        end8(f)
        if name == 'original4B_proof_does_not_own_map8':
            claim = dict(range=dict(address='0x81002000', size=4), witness_role='original_typed_object_record',
                         witness=dict(path='SYNTHETIC/typed-object', sha256='0' * 64),
                         original_image_binding=dict(path='SYNTHETIC/typed-image-binding', sha256='0' * 64))
            for field in ('witness', 'original_image_binding'):
                f.put_json(claim[field], dict(kind='invented identity only; no allocation grant'))
            f.relation['extents']['original_allocation_extent'] = claim
        return f
    if name == 'second_weak_provider_not_closed':
        s = selection(f)
        extra = copy.deepcopy(s['selected_provider']); extra['disposition'] = 'ambiguous'; extra['input_order'] += 1
        s['providers'].append(extra)
        return f
    if name == 'unrelated_same_address_record':
        f.symbols += b'unrelated = .sbss:0x81002000; // type:object size:0x4\n'; f.refresh_symbols()
        return f
    if name == 'second_selected_provider':
        extra = copy.deepcopy(target['selected_provider']); extra['input_order'] += 1
        target['providers'].append(extra)
        return f
    if name == 'review_transplanted_across_arm':
        review(f); f.context['audited_source_commit'] = '3' * 40
        return f
    for mutation in case['mutations']:
        assert mutation['op'] != 'note', 'uninstantiated note case'
        edit(target, mutation)
    return f


class ProposalTests(unittest.TestCase):
    def boundary(self, result):
        self.assertIs(result['production_acceptance'], False)
        self.assertEqual(result['effective_owners'], [])
        self.assertIs(result['actual_original_data_claims'], False)
        for output in [result] + result['relations']:
            self.assertEqual(output['obligations'], checker.OBLIGATIONS)
            self.assertIs(output['production_acceptance'], False)
            self.assertEqual(output['effective_owners'], [])

    def rejected(self, f, reason=None):
        result = f.check(); self.boundary(result)
        self.assertNotEqual(result['status'], 'ASSOCIATION_CANDIDATE_ONLY', result)
        if reason:
            self.assertIn(reason, json.dumps(result))
        return result

    def test_positive_byte_backed(self):
        result = Fixture().check(); self.boundary(result)
        self.assertEqual(result['status'], 'ASSOCIATION_CANDIDATE_ONLY', result)

    def test_non_synthetic_before_reads(self):
        for domain in ('ORIGINAL', '', None):
            f = Fixture(); f.envelope['evidence_domain'] = domain
            evidence = checker.SyntheticEvidence(f.blobs)
            result = checker.check_proposal(f.envelope, evidence)
            self.assertEqual(result['status'], 'INPUT_REFUSED'); self.assertEqual(evidence.reads, [])
            self.boundary(result)

    def test_no_io_during_call(self):
        f = Fixture()
        with mock.patch('builtins.open', side_effect=AssertionError('filesystem')), mock.patch.object(Path, 'read_bytes', side_effect=AssertionError('filesystem')):
            result = f.check()
        self.assertEqual(result['status'], 'ASSOCIATION_CANDIDATE_ONLY')

    def test_strict_schema_types(self):
        for key, value in [('schema_version', True), ('schema_version', 1.0), ('production_enabled', 0)]:
            f = Fixture(); f.envelope[key] = value
            self.assertEqual(self.rejected(f)['status'], 'INPUT_REFUSED')

    def test_duplicate_json_and_floats(self):
        for raw in (b'{"a":1,"a":2}', b'{"a":1.0}', b'{"a":NaN}'):
            with self.assertRaises(ValueError): checker.parse_json(raw)

    def test_missing_blob(self):
        f = Fixture(); del f.blobs[f.context['tool']['path']]; self.rejected(f, 'missing_blob')

    def test_changed_tool(self):
        f = Fixture(); f.put(f.context['tool'], b'invented wrong interpreter'); self.rejected(f, 'baseline_tool_context')

    def test_hash_mismatch(self):
        f = Fixture(); f.blobs[f.context['original_image']['path']] += b'changed'; self.rejected(f, 'blob_hash_mismatch')

    def raw_elf(self, mutation):
        f = Fixture(); mutation(f.elf); f.refresh_object(); f.source_context(); return f

    def test_null_symbol(self):
        self.rejected(self.raw_elf(lambda raw: struct.pack_into('>I', raw, 1024, 32)), 'ELF_reserved_null_symbol')

    def test_null_section(self):
        self.rejected(self.raw_elf(lambda raw: struct.pack_into('>I', raw, 5376, 1)), 'ELF_reserved_null_section')

    def test_symtab_boundary(self):
        self.rejected(self.raw_elf(lambda raw: struct.pack_into('>I', raw, 5376 + 10 * 40 + 28, 0)), 'ELF_symtab_local_boundary')

    def test_extra_ctors_relocation(self):
        # Free section19 adds a duplicate selected .ctors relocation at file4200.
        def mutate(raw):
            struct.pack_into('>10I', raw, 5376 + 19 * 40, 0, 4, 0, 0, 4200, 12, 10, 5, 4, 12)
            struct.pack_into('>IIi', raw, 4200, 0, 101 * 256 + 1, 0)
        self.rejected(self.raw_elf(mutate), 'unlisted_relocation_in_selected_contribution')

    def test_crossing_relocation(self):
        def mutate(raw):
            struct.pack_into('>10I', raw, 5376 + 19 * 40, 0, 4, 0, 0, 4200, 12, 10, 1, 4, 12)
            struct.pack_into('>IIi', raw, 4200, 62, 103 * 256 + 1, 0)
        self.rejected(self.raw_elf(mutate), 'unlisted_relocation_in_selected_contribution')

    def test_unrelated_defined_notype_at_singleton(self):
        def mutate(raw):
            struct.pack_into('>IIIBBH', raw, 1024 + 105 * 16, 224, 0, 0, 0x20, 0, 4)
            raw[3072 + 224:3072 + 224 + 10] = b'unrelated\0'
        self.rejected(self.raw_elf(mutate), 'competing_reconstructed_definition')

    def test_raw_unknown_relocation_width(self):
        def mutate(raw):
            struct.pack_into('>10I', raw, 5376 + 19 * 40, 0, 4, 0, 0, 4200, 12, 10, 1, 4, 12)
            struct.pack_into('>IIi', raw, 4200, 120, 103 * 256 + 255, 0)
        self.rejected(self.raw_elf(mutate), 'unsupported_relocation_width')

    def test_unbound_producing_TU(self):
        f = Fixture(); r = f.relation['reconstruction']; r['source_unit_path'] = 'src/Game/Unprovided.cpp'
        recipe = json.loads(f.blobs[r['build_recipe']['path']]); recipe['source_unit_path'] = r['source_unit_path']
        f.put_json(r['build_recipe'], recipe); self.rejected(f, 'producing_TU_source_pin_missing')

    def image_mutation(self, off, word):
        f = Fixture(); struct.pack_into('>I', f.image, off, word); f.refresh_image(); return f

    def test_unreachable_initializer_pattern(self):
        self.rejected(self.image_mutation(0x120, 0x4e800020), 'singleton_load_role_or_address')

    def test_wrong_receiver(self):
        self.rejected(self.image_mutation(0x120, 0x386d0004), 'singleton_load_role_or_address')

    def test_second_call(self):
        self.rejected(self.image_mutation(0x128, 0x48000019), 'constructor_call_role_receiver_or_census')

    def test_partial_store_overwrites_vptr(self):
        f = Fixture(); struct.pack_into('>II', f.image, 0x14c, 0x98830000, 0x4e800020)
        f.relation['original_initializer']['constructor']['size'] = 20
        f.symbols = f.symbols.replace(b'invented_ctor = .text:0x81001040; // type:function size:0x10', b'invented_ctor = .text:0x81001040; // type:function size:0x14')
        f.refresh_symbols(); f.refresh_image(); self.rejected(f, 'constructor_CFG_or_effect_unclosed')

    def two_span_startup(self):
        f = Fixture()
        struct.pack_into('>4I', f.image, 0x100, 0x7d8802a6, 0x4800007d, 0x7d8803a6, 0x4e800020)
        struct.pack_into('>3I', f.image, 0x180, 0x3da08100, 0x39ad2000, 0x4e800020)
        f.startup['size'] = 16
        f.symbols = f.symbols.replace(b'invented_startup = .text:0x81001000; // type:function size:0xC',
                                     b'invented_startup = .text:0x81001000; // type:function size:0x10')
        f.symbols += b'invented_startup_helper = .text:0x81001080; // type:function size:0xC\n'
        f.refresh_symbols(); f.refresh_image()
        helper = copy.deepcopy(f.startup)
        helper.update(address='0x81001080', size=12, sha256=checker.digest(f.image[0x180:0x18c]))
        pin = f.relation['original_initializer']['register_and_cfg_witness']
        value = json.loads(f.blobs[pin['path']]); value['startup_chain'].append(helper)
        f.put_json(pin, value)
        for role in f.relation['unit_roles']:
            if role['role'] == 'original_initializer_contribution': role['evidence'] = copy.deepcopy(pin)
        return f

    def test_two_span_startup(self):
        result = self.two_span_startup().check(); self.boundary(result)
        self.assertEqual(result['status'], 'ASSOCIATION_CANDIDATE_ONLY', result)

    def test_startup_conditional_skip(self):
        f = self.two_span_startup(); struct.pack_into('>I', f.image, 0x100, 0x4182000c)
        witness = json.loads(f.blobs[f.relation['original_initializer']['register_and_cfg_witness']['path']])
        f.refresh_image(); witness['startup_chain'][0] = f.startup
        for span in witness['startup_chain']: span['image_sha256'] = f.context['original_image']['sha256']
        f.put_json(f.relation['original_initializer']['register_and_cfg_witness'], witness)
        self.rejected(f, 'startup_call_not_linear_dominating')

    def test_startup_postcall_r13_mutation(self):
        f = self.two_span_startup(); struct.pack_into('>I', f.image, 0x108, 0x39a00000)
        witness = json.loads(f.blobs[f.relation['original_initializer']['register_and_cfg_witness']['path']])
        f.refresh_image(); witness['startup_chain'][0] = f.startup
        for span in witness['startup_chain']: span['image_sha256'] = f.context['original_image']['sha256']
        f.put_json(f.relation['original_initializer']['register_and_cfg_witness'], witness)
        self.rejected(f, 'startup_postcall_r13_preservation_unclosed')

    def startup_store(self, instruction, target=0x4000):
        f = Fixture()
        struct.pack_into('>7I', f.image, 0x100, 0x3da08100, 0x39ad2000, 0x3c808100,
                         0x38840000 | target, 0x38a00000, instruction, 0x4e800020)
        f.startup['size'] = 28
        f.symbols = f.symbols.replace(b'invented_startup = .text:0x81001000; // type:function size:0xC',
                                     b'invented_startup = .text:0x81001000; // type:function size:0x1C')
        f.refresh_symbols(); f.refresh_image()
        return f

    def test_startup_memory_positive(self):
        result = self.startup_store(0x60000000).check(); self.boundary(result)
        self.assertEqual(result['status'], 'ASSOCIATION_CANDIDATE_ONLY', result)

    def test_startup_memory_store(self):
        for target in (0x4000, 0x3008, 0x1040):
            self.rejected(self.startup_store(0x90a40000, target), 'startup_memory_effects_unclosed')

    def test_startup_memory_partial_store(self):
        self.rejected(self.startup_store(0x98a40000), 'startup_memory_effects_unclosed')

    def test_initializer_image_store(self):
        f = Fixture()
        struct.pack_into('>6I', f.image, 0x120, 0x386d0000, 0x4800001d, 0x3c808100,
                         0x38844000, 0x90a40000, 0x4e800020)
        f.refresh_image(); self.rejected(f, 'initializer_memory_effects_unclosed')

    def test_constructor_image_store(self):
        f = Fixture()
        struct.pack_into('>5I', f.image, 0x140, 0x3d808100, 0x398c3000, 0x91830000,
                         0x918c0008, 0x4e800020)
        f.relation['original_initializer']['constructor']['size'] = 20
        f.symbols = f.symbols.replace(b'invented_ctor = .text:0x81001040; // type:function size:0x10',
                                     b'invented_ctor = .text:0x81001040; // type:function size:0x14')
        f.refresh_symbols(); f.refresh_image(); self.rejected(f, 'constructor_non_vptr_memory_effects_unclosed')

    def test_BSS_loaded_overlap(self):
        f = Fixture(); struct.pack_into('>II', f.image, 0xd8, 0x81003000, 4)
        f.refresh_image(); self.rejected(f, 'image_BSS_overlaps_loaded_span')

    def test_missing_startup_chain(self):
        f = Fixture(); p = f.relation['original_initializer']['register_and_cfg_witness']; value=json.loads(f.blobs[p['path']]); value['startup_chain']=[]
        f.put_json(p,value); self.rejected(f,'startup_chain_required')

    def test_truncated_caller_owner(self):
        f=Fixture(); f.splits=f.splits.replace(b'end:0x81001100',b'end:0x81001001'); f.put(f.context['full_splits'],f.splits)
        self.rejected(f,'span_full_ownership')

    def test_competing_interior_owner(self):
        f=Fixture(); f.splits+=b'Game/Other.cpp:\n    .text start:0x81001004 end:0x81001008\n'; f.put(f.context['full_splits'],f.splits)
        self.rejected(f,'span_full_ownership')

    def test_review_positive_and_stale(self):
        f=Fixture(); review(f); result=f.check(); self.boundary(result)
        self.assertEqual(result['status'],'ASSOCIATION_CANDIDATE_ONLY',result)
        f.context['allowed_subject_ids'].append('unrelated'); self.rejected(f,'review_context_or_scope_hash')

    def test_selection_candidate_not_original_grant(self):
        f=Fixture(); selection(f); result=f.check(); self.boundary(result)
        self.assertEqual(result['status'],'ASSOCIATION_CANDIDATE_ONLY',result)


def make_case(case):
    def test(self):
        f = instantiate(case); result = f.check(); self.boundary(result)
        expected = case['expected']['association_component']
        self.assertEqual(result['status'], 'INPUT_REFUSED' if expected == 'SCHEMA_REFUSAL' else expected, result)
    test.__doc__ = 'Sealed declarative case ' + case['id']
    return test


for case in CASES:
    setattr(ProposalTests, 'test_sealed_' + case['id'], make_case(case))

if __name__ == '__main__':
    unittest.main()
