"""Synthetic-only PPC/DOL/source controls; no game binary or source payload."""
import copy
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).parent))
import nervemap as n


def q(name):
    parts = name.split('::')
    return ('Q' + str(len(parts)) if len(parts) > 1 else '') + ''.join(str(len(p)) + p for p in parts)


def sym(cls, meth, suffix=''):
    return meth + '__' + q(cls) + suffix


def li(reg, base, imm):
    return (14 << 26) | reg << 21 | base << 16 | (imm & 0xffff)


def jump(pc, target, link=False):
    return (18 << 26) | ((target - pc) & 0x3fffffc) | int(link)


class Fixture:
    """Invented addresses/instructions, minimal semantic shape of the bug class."""
    A, B = 'NrvThing::Ready', 'NrvThing::Done'

    def __init__(self, root):
        self.root = Path(root)
        self.data = bytearray(0x100 + 0x500 + 0x200)
        for p, v in ((0, 0x100), (0x48, 0x80001000), (0x90, 0x500),
                     (7*4, 0x600), (0x48+7*4, 0x80002000), (0x90+7*4, 0x200)):
            struct.pack_into('>I', self.data, p, v)
        self.records = []
        self.emit('__init_registers', 0x80001000, [0x3da08000, 0x61ad3000, 0x4e800020])
        self.emit('__sinit_Thing', 0x80001040, [li(3, 13, 0), jump(0x80001044, 0x80001080, True),
                                             li(3, 13, 4), jump(0x8000104c, 0x800010a0, True), 0x4e800020])
        for cls, ctor, vt, execute in ((self.A, 0x80001080, 0x80002000, 0x80001100),
                                       (self.B, 0x800010a0, 0x80002010, 0x80001120)):
            self.emit(sym(cls, '__ct', 'Fv'), ctor, [0x3c808000, li(4, 4, vt & 0xffff), 0x90830000, 0x4e800020])
            self.record(sym(cls, '__vt'), '.data', vt, 16)
            self.write(vt, [0, 0, execute, 0])
        self.record(sym(self.A, 'sInstance'), '.sbss', 0x80003000, 4)
        self.record(sym(self.B, 'sInstance'), '.sbss', 0x80003004, 4)
        self.emit(sym(self.A, 'execute', 'CFP5Spine'), 0x80001100, [0x80640000, jump(0x80001104, 0x80001300)])
        self.emit(sym(self.B, 'execute', 'CFP5Spine'), 0x80001120, [0x80640000, jump(0x80001124, 0x80001320)])
        self.emit(sym('Spine', 'update', 'Fv'), 0x80001140,
                  [0x7c7f1b78, 0x807f0004, 0x7fe4fb78, 0x81830000, 0x818c0008, 0x7d8903a6, 0x4e800421, 0x4e800020])
        self.emit(sym('Thing', 'init', 'Fv'), 0x80001200,
                  [li(4, 13, 0), li(5, 0, 0), jump(0x80001208, 0x80001400, True), 0x4e800020])
        self.emit(sym('Thing', 'appear', 'Fv'), 0x80001220,
                  [li(4, 13, 4), li(5, 0, 0), jump(0x80001228, 0x80001400, True), 0x4e800020])
        self.emit(sym('Thing', 'exeOne', 'Fv'), 0x80001300, [0x4e800020])
        self.emit(sym('Thing', 'exeTwo', 'Fv'), 0x80001320, [0x4e800020])
        self.emit(sym('NerveExecutor', 'initNerve', 'FPC5Nervel'), 0x80001400, [0x90830008, 0x4e800020])
        self.emit(sym('NerveExecutor', 'setNerve', 'FPC5Nerve'), 0x80001420, [0x90830008, 0x4e800020])
        self.write(0x80002100, [0x80001040])
        self.source = '''#include "Game/LiveActor/Nerve.hpp"
#include "Game/SyntheticApi.hpp"
class Thing : public NerveExecutor { public: void init(); void appear(); void exeOne(); void exeTwo(); };
namespace NrvThing {
 NEW_NERVE(Ready, Thing, One);
 NEW_NERVE(Done, Thing, Two);
}
void Thing::init() { initNerve(GET_NERVE(Thing, Ready), 0); }
void Thing::appear() { initNerve(GET_NERVE(Thing, Done), 0); }
'''
        self.header = '''#pragma once
#define GET_NERVE(cls, nerve) (&Nrv##cls::nerve::sInstance)
#define NEW_NERVE(name, host, state) \\
 class name : public Nerve { public: \\
 virtual void execute(Spine* p) const { reinterpret_cast<host*>(p->mExecutor)->exe##state(); } \\
 static name sInstance; }; name name::sInstance;
'''
        self.splits = '''Game/Thing.cpp:
 .text start:0x80001000 end:0x80001500
 .data start:0x80002000 end:0x80002040
 .ctors start:0x80002100 end:0x80002104
 .sbss start:0x80003000 end:0x80003008
'''

    def write(self, addr, words):
        off = addr - 0x80001000 + 0x100 if addr < 0x80002000 else addr - 0x80002000 + 0x600
        struct.pack_into('>' + 'I' * len(words), self.data, off, *words)

    def record(self, name, section, addr, size):
        self.records.append(dict(name=name, section=section, address=addr, size=size))

    def emit(self, name, addr, words):
        self.record(name, '.init' if name == '__init_registers' else '.text', addr, len(words)*4)
        self.write(addr, words)

    def save(self):
        paths = {'src/Game/Thing.cpp': self.source, 'include/Game/LiveActor/Nerve.hpp': self.header,
                 'config/splits.txt': self.splits,
                 'include/Game/SyntheticApi.hpp': '''class Nerve;
class NerveExecutor { public:
void initNerve(const Nerve*, long); void setNerve(const Nerve*);
void* executor; const Nerve* current; const Nerve* next;
};''',
                 'src/Game/SyntheticApi.cpp': '''#include "Game/SyntheticApi.hpp"
void NerveExecutor::initNerve(const Nerve* p, long) { next = p; }
void NerveExecutor::setNerve(const Nerve* p) { next = p; }
'''}
        for name, data in paths.items():
            p = self.root / name
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text(data)
        (self.root / 'test.dol').write_bytes(self.data)
        (self.root / 'config/symbols.txt').write_text('\n'.join(
            f"{r['name']} = {r['section']}:0x{r['address']:08x}; // type:{'function' if r['section'] in ('.text','.init') else 'object'} size:0x{r['size']:X}"
            for r in self.records) + '\n')
        # Explicit invented review input: hashes alone are NOT a semantic proof.
        contracts = []
        image = n.Dol(self.root / 'test.dol')
        for api in ('initNerve', 'setNerve'):
            provider = next(r for r in self.records if r['name'].startswith(api + '__13NerveExecutor'))
            def extent(record):
                return dict(symbol=record['name'], section=record['section'], address=n.address(record['address']),
                            size=record['size'], unit='Game/Thing.cpp', sha256=n.sha(image.read(record['address'], record['size'])))
            callsites = []
            for record in self.records:
                if record['section'] not in ('.text', '.init'):
                    continue
                for pc in range(record['address'], record['address'] + record['size'], 4):
                    word = image.word(pc)
                    if word >> 26 == 18 and n.branch(word, pc) == provider['address']:
                        callsites.append(dict(caller=extent(record), pc=n.address(pc),
                                              instruction_sha256=n.sha(image.read(pc, 4)), call_kind='linked' if word & 1 else 'tail',
                                              register_requirements={'r5': 0} if api == 'initNerve' else {}))
            source = [dict(path=p, sha256=n.sha((self.root / p).read_bytes()), role=role)
                      for p, role in [('include/Game/SyntheticApi.hpp', 'declaration'),
                                      ('include/Game/SyntheticApi.hpp', 'header'),
                                      ('src/Game/SyntheticApi.cpp', 'definition')]]
            claim = dict(kind='member', api=api, this_register=3, nerve_register=4, receiver_adjustment=0,
                         behavior='selected_input_reaches_nerve_storage_unchanged', dependencies_closed=True,
                         dol_sha256=n.sha(self.data), source=source, dependencies=[], provider=extent(provider),
                         preconditions=['Valid member receiver at the pinned callsite; normal returning path.'],
                         allowed_callsites=callsites)
            receipt = dict(status='accepted', claim_sha256=n.canonical_sha(claim), reviewer='INVENTED-SYNTHETIC-REVIEW',
                           scope='nerve_argument_storage_transport', dependencies_closed=True)
            receipt_path = 'review-' + api + '.json'
            (self.root / receipt_path).write_text(json.dumps(receipt))
            contracts.append(dict(claim=claim, review=dict(path=receipt_path, sha256=n.sha((self.root / receipt_path).read_bytes()))))
        self.contract_path = self.root / 'contracts.json'
        self.contract_path.write_text(json.dumps(dict(schema=1, contracts=contracts)))

    def run(self, inventory=False, contracts=True):
        self.save()
        return n.audit(self.root, self.root/'test.dol', self.root/'config/symbols.txt', self.root/'config/splits.txt',
                       inventory=inventory, contracts_path=self.contract_path if contracts else None)


class EntryFixture:
    """Invented names/addresses and three tagged instructions, no game payload."""
    def __init__(self, root, operation='save'):
        self.base, self.root = Fixture(root), Path(root)
        self.function = '__spec_' + operation
        self.entry_prefix = '_spec_' + operation + '_'
        self.op = 'stw' if operation == 'save' else 'lwz'
        self.start, self.size = 0x80001440, 16
        words = [((36 if operation == 'save' else 32) << 26) | reg << 21 | 11 << 16 | ((4 * (reg - 32)) & 0xffff)
                 for reg in range(14, 17)] + [0x4e800020]
        self.base.write(self.start, words)
        self.base.write(0x80001460, [jump(0x80001460, self.start + 8, True), 0x4e800020])
        self.base.splits = self.base.splits.replace('end:0x80001500', 'end:0x80001440') + '''Runtime/runtime.c:
 .text start:0x80001440 end:0x80001480
'''
        self.base.save()
        self.symbol_path, self.split_path = self.root/'config/symbols.txt', self.root/'config/splits.txt'
        with self.symbol_path.open('a') as f:
            f.write(f'{self.function} = .text:0x{self.start:x}; // type:function size:0x10\n')
            for reg in range(14, 17):
                f.write(f'{self.entry_prefix}{reg} = .text:0x{self.start + 4 * (reg - 14):x}; // type:label\n')
            f.write('__spec_caller = .text:0x80001460; // type:function size:0x8\n')
        self.source_path = self.root/'src/Runtime/runtime.c'
        self.source_path.parent.mkdir(parents=True, exist_ok=True)
        source = f'''#define SLOT(reg) {self.entry_prefix} ## reg
#define TAG(reg) entry SLOT(reg)
#define STACK_ANCHOR r11
void SLOT(14)(void);
void SLOT(15)(void);
void SLOT(16)(void);
static asm void {self.function}(void)
{{
 nofralloc
'''
        for reg in range(14, 17):
            source += f' TAG({reg})\n {self.op} r{reg},{4 * (reg - 32)}(STACK_ANCHOR)\n'
        source += ' blr\n}\n'
        self.source_path.write_text(source)
        self.relation = dict(kind='mwcc_runtime_entry_at_function_start', unit='Runtime/runtime.c',
                             source=dict(path='src/Runtime/runtime.c', sha256=''), registers=[14, 15, 16],
                             first_entry=self.entry_prefix + '14',
                             owner=dict(symbol=self.function, address=n.address(self.start), section='.text',
                                        size=self.size, unit='Runtime/runtime.c', sha256=''),
                             dol_sha256='', symbols_sha256='', splits_sha256='',
                             references=[dict(caller=dict(symbol='__spec_caller', address='0x80001460', section='.text',
                                                          size=8, unit='Runtime/runtime.c', sha256=''),
                                              pc='0x80001460', instruction_sha256='', entry_symbol=self.entry_prefix + '16', call_kind='linked')])
        self.refresh_pins()

    def refresh_pins(self):
        self.relation['source']['sha256'] = n.sha(self.source_path.read_bytes())
        self.relation['symbols_sha256'] = n.sha(self.symbol_path.read_bytes())
        self.relation['splits_sha256'] = n.sha(self.split_path.read_bytes())
        self.relation['dol_sha256'] = n.sha((self.root/'test.dol').read_bytes())
        self.relation['owner']['sha256'] = n.sha(n.Dol(self.root/'test.dol').read(self.start, self.size))
        for reference in self.relation['references']:
            caller = reference['caller']
            image = n.Dol(self.root/'test.dol')
            caller['sha256'] = n.sha(image.read(int(caller['address'], 16), caller['size']))
            reference['instruction_sha256'] = n.sha(image.read(int(reference['pc'], 16), 4))

    def candidate(self):
        return n.entry_alias_candidate(self.root, n.Symbols(self.symbol_path), n.Splits(self.split_path),
                                       n.Dol(self.root/'test.dol'), self.relation)


class EntryIntegrationFixture(EntryFixture):
    """Fictional transport/identity/replay reviews; never actual review grants."""
    def __init__(self, root):
        super().__init__(root)
        image = bytearray((self.root/'test.dol').read_bytes())
        struct.pack_into('>III', image, 0x100 + 0x1400 - 0x1000,
                         0x90830008, jump(0x80001404, self.start + 8, True), 0x4e800020)
        (self.root/'test.dol').write_bytes(image)
        self.symbol_path.write_text(self.symbol_path.read_text().replace(
            'initNerve__13NerveExecutorFPC5Nervel = .text:0x80001400; // type:function size:0x8',
            'initNerve__13NerveExecutorFPC5Nervel = .text:0x80001400; // type:function size:0xC'))
        self.api_manifest = json.loads(self.base.contract_path.read_text())
        self.claim = self.api_manifest['contracts'][0]['claim']
        self.claim['provider']['size'] = 12
        self.claim['dependencies'] = [copy.deepcopy(self.relation['owner'])]
        self.claim['source'].append(dict(path='src/Runtime/runtime.c', role='definition', sha256=n.sha(self.source_path.read_bytes())))
        self.relation['references'] = [dict(caller=copy.deepcopy(self.claim['provider']), pc='0x80001404',
                                            instruction_sha256='', entry_symbol=self.entry_prefix + '16', call_kind='linked')]
        self.entry_manifest_path = self.root/'entry-reviews.json'
        self.identity_path, self.replay_path = self.root/'entry-identity-review.json', self.root/'entry-checker-replay.json'
        self.replay_path.write_text(json.dumps(dict(schema=1, status='accepted', reviewer='INVENTED-REPLAY-REVIEW',
                                                   scope='entry_dependency_checker_replay', tool_sha256=n.sha(Path(n.__file__).read_bytes()))))
        self.identity_path.write_text(json.dumps(dict(schema=1, status='accepted', reviewer='INVENTED-IDENTITY-REVIEW',
                                                     scope='runtime_entry_dependency_identity')))
        self.refresh_reviews()

    def refresh_reviews(self, identity=True):
        self.refresh_pins()
        image = n.Dol(self.root/'test.dol')
        self.claim['dol_sha256'] = n.sha(image.data)
        for provider in [self.claim['provider']] + self.claim['dependencies'] + [c['caller'] for c in self.claim['allowed_callsites']]:
            provider['sha256'] = n.sha(image.read(int(provider['address'], 16), provider['size']))
        for call in self.claim['allowed_callsites']:
            call['instruction_sha256'] = n.sha(image.read(int(call['pc'], 16), 4))
        for entry in self.api_manifest['contracts']:
            receipt_path = self.root/entry['review']['path']
            receipt = json.loads(receipt_path.read_text())
            receipt['claim_sha256'] = n.canonical_sha(entry['claim'])
            receipt_path.write_text(json.dumps(receipt))
            entry['review']['sha256'] = n.sha(receipt_path.read_bytes())
        self.base.contract_path.write_text(json.dumps(self.api_manifest))
        if identity:
            receipt = json.loads(self.identity_path.read_text())
            receipt.update(api_claim_sha256=n.canonical_sha(self.claim), relation_sha256=n.canonical_sha(self.relation),
                           tool_sha256=n.sha(Path(n.__file__).read_bytes()),
                           checker_replay=dict(path=self.replay_path.name, sha256=n.sha(self.replay_path.read_bytes())))
            self.identity_path.write_text(json.dumps(receipt))
        self.entry_manifest_path.write_text(json.dumps(dict(schema=1, relations=[dict(relation=self.relation,
            review=dict(path=self.identity_path.name, sha256=n.sha(self.identity_path.read_bytes())))])))

    def preview(self):
        return n.preview_api_contracts_with_entry_dependencies(self.base.contract_path, self.root, n.Symbols(self.symbol_path),
                                                               n.Splits(self.split_path), n.Dol(self.root/'test.dol'), self.entry_manifest_path)

    def dispatch_preview(self):
        return n.preview_dispatch_with_entry_dependencies(self.root, self.root/'test.dol', self.symbol_path, self.split_path,
                                                          self.base.contract_path, self.entry_manifest_path)

    def enabled(self):
        return n.api_contracts(self.base.contract_path, self.root, n.Symbols(self.symbol_path),
                               n.Splits(self.split_path), n.Dol(self.root/'test.dol'), self.entry_manifest_path)

    def enabled_dispatch(self):
        return n.audit(self.root, self.root/'test.dol', self.symbol_path, self.split_path,
                       contracts_path=self.base.contract_path, entry_reviews_path=self.entry_manifest_path)


class NerveMapTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='nervemap-test-')
        self.addCleanup(self.tmp.cleanup)
        self.f = Fixture(self.tmp.name)

    def test_matching_chain(self):
        report = self.f.run()
        self.assertEqual(report['coverage']['MATCH'], 2, json.dumps(report['rows'], indent=2))
        self.assertEqual(report['coverage']['universe'], 2)
        self.assertEqual(report['mismatches'], [])

    def test_crossed_execute_labels_unchanged_bytes(self):
        original = bytes(self.f.data)
        entries = [r for r in self.f.records if r['name'].startswith('execute__')]
        entries[0]['address'], entries[1]['address'] = entries[1]['address'], entries[0]['address']
        r = self.f.run()
        self.assertEqual(bytes(self.f.data), original)
        self.assertEqual(r['coverage']['MISMATCH'], 2, json.dumps(r['rows'], indent=2))
        self.assertTrue(all('EXECUTE_ADDRESS_BINDING' in x['checks'] for x in r['mismatches']))
        self.assertTrue(all(x['relations']['symbol_ownership'] == 'MISMATCH' and x['relations']['source_dispatch'] == 'MATCH'
                            for x in r['rows']))

    def test_crossed_vtable_slots(self):
        self.f.write(0x80002008, [0x80001120])
        self.f.write(0x80002018, [0x80001100])
        r = self.f.run()
        self.assertEqual(r['coverage']['MISMATCH'], 2)
        self.assertIn('SOURCE_METHOD_DISPATCH', r['rows'][0]['checks'])

    def test_source_body_binding_not_suffix(self):
        self.f.source = self.f.source.replace('Ready, Thing, One', 'Ready, Thing, Two')
        r = self.f.run()
        self.assertEqual(r['coverage']['MISMATCH'], 1)
        self.assertIn('SOURCE_METHOD_DISPATCH', r['mismatches'][0]['checks'])

    def test_changed_producer(self):
        self.f.write(0x80001200, [li(4, 13, 4)])
        r = self.f.run()
        done = next(x for x in r['rows'] if x['class_name'] == self.f.B)
        self.assertEqual(done['status'], 'MISMATCH', done)
        self.assertIn('PRODUCER_SINGLETON_BINDING', done['checks'])

    def test_ctor_vtable_identity(self):
        self.f.write(0x80001084, [li(4, 4, 0x2010)])
        r = self.f.run()
        row = next(x for x in r['rows'] if x['class_name'] == self.f.A)
        self.assertEqual(row['status'], 'MISMATCH', row)
        self.assertIn('CONSTRUCTOR_VTABLE_BINDING', row['checks'])

    def test_unknown_ctor_is_not_a_bug(self):
        self.f.write(0x80001080, [0x4e800421])
        r = self.f.run()
        row = next(x for x in r['rows'] if x['class_name'] == self.f.A)
        self.assertEqual(row['status'], 'UNKNOWN')
        self.assertEqual(r['mismatches'], [])

    def test_ambiguous_alias_is_unknown(self):
        row = next(x for x in self.f.records if x['name'] == sym(self.f.A, 'execute', 'CFP5Spine'))
        self.f.records.append(dict(row, name='inherited_shared_execute'))
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 1)
        self.assertEqual(r['mismatches'], [])

    def test_missing_source_ownership_is_unknown_even_when_slot_differs(self):
        self.f.source = self.f.source.replace('NEW_NERVE(Ready, Thing, One);', '')
        self.f.write(0x80002008, [0x80001120])
        r = self.f.run()
        a = next(x for x in r['rows'] if x['class_name'] == self.f.A)
        self.assertEqual(a['status'], 'UNKNOWN')
        self.assertEqual(r['mismatches'], [])

    def test_wrong_end_slot_does_not_replace_execute_slot(self):
        self.f.write(0x8000200c, [0x80001120])
        self.assertEqual(self.f.run()['coverage']['MATCH'], 2)

    def test_inventory_retains_unknown_denominator(self):
        r = self.f.run(inventory=True)
        self.assertEqual(r['coverage']['universe'], 2)
        self.assertEqual(r['coverage']['UNKNOWN'], 2)

    def test_normal_out_of_class_member_and_static_instance(self):
        self.f.source = self.f.source.replace('NEW_NERVE(Ready, Thing, One);', '''
 class Ready : public Nerve { public: void execute(Spine*) const; static Ready sInstance; };
 Ready Ready::sInstance;
 void Ready::execute(Spine* p) const { reinterpret_cast<Thing*>(p->mExecutor)->exeOne(); }
''')
        self.assertEqual(self.f.run()['coverage']['MATCH'], 2)

    def transition(self):
        self.f.write(0x80001100, [0x80640000, li(4, 13, 4), jump(0x80001108, 0x80001420)])
        next(r for r in self.f.records if r['name'] == sym(self.f.A, 'execute', 'CFP5Spine'))['size'] = 12
        self.f.source = self.f.source.replace('NEW_NERVE(Ready, Thing, One);', '''
 class Ready : public Nerve { public: void execute(Spine*) const; static Ready sInstance; };
 Ready Ready::sInstance;
 void Ready::execute(Spine* p) const { reinterpret_cast<Thing*>(p->mExecutor)->setNerve(&NrvThing::Done::sInstance); }
''')

    def test_unequal_transition_and_method_sizes(self):
        self.transition()
        r = self.f.run()
        self.assertEqual(r['coverage']['MATCH'], 2, r['rows'])
        a = next(x for x in r['rows'] if x['class_name'] == self.f.A)
        self.assertEqual(a['chain']['execute_wrapper']['size'], 12)
        self.assertEqual(a['chain']['action']['singleton'], '0x80003004')

    def test_transition_vs_immediate_body_rejected(self):
        self.transition()
        self.f.source = self.f.source.replace('setNerve(&NrvThing::Done::sInstance)', 'exeOne()')
        r = self.f.run()
        self.assertEqual(r['coverage']['MISMATCH'], 1, r['rows'])

    def test_transition_target_change_rejected(self):
        self.transition()
        self.f.source = self.f.source.replace('setNerve(&NrvThing::Done::sInstance)', 'setNerve(&NrvThing::Ready::sInstance)')
        r = self.f.run()
        self.assertEqual(r['coverage']['MISMATCH'], 2, r['rows'])
        a = next(x for x in r['rows'] if x['class_name'] == self.f.A)
        self.assertIn('SOURCE_TRANSITION_DISPATCH', a['checks'])

    def test_wrong_transition_symbol_extent_cannot_pass(self):
        self.transition()
        next(r for r in self.f.records if r['name'] == sym(self.f.A, 'execute', 'CFP5Spine'))['size'] = 8
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 1)
        self.assertEqual(r['coverage']['MATCH'], 1)

    def test_final_constructor_vptr_not_first_store(self):
        self.f.write(0x80001080, [0x3c808000, li(4, 4, 0x2010), 0x90830000,
                                 li(4, 4, -0x10), 0x90830000, 0x4e800020])
        next(r for r in self.f.records if r['name'] == sym(self.f.A, '__ct', 'Fv'))['size'] = 24
        self.assertEqual(self.f.run()['coverage']['MATCH'], 2)

    def test_inline_constructor_store(self):
        self.f.write(0x80001040, [0x3c808000, li(4, 4, 0x2000), 0x908d0000,
                                 li(3, 13, 4), jump(0x80001050, 0x800010a0, True), 0x4e800020])
        next(r for r in self.f.records if r['name'] == '__sinit_Thing')['size'] = 24
        self.assertEqual(self.f.run()['coverage']['MATCH'], 2)

    def test_unverified_consumer_slot_is_unknown(self):
        self.f.write(0x80001150, [0x818c000c])
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertFalse(r['dispatch_consumer']['verified'])

    def test_adjusting_thunk_is_unknown(self):
        self.f.write(0x80001100, [li(3, 3, 4), jump(0x80001104, 0x80001300)])
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 1)
        self.assertEqual(r['mismatches'], [])

    def test_c_wrong_spine_formal_is_unknown(self):
        self.f.header = self.f.header.replace('p->mExecutor', 'otherSpine->mExecutor')
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])

    def test_c_disconnected_cast_and_receiver_is_unknown(self):
        self.f.header = self.f.header.replace(
            'reinterpret_cast<host*>(p->mExecutor)->exe##state();',
            'host* unused = reinterpret_cast<host*>(p->mExecutor); extern host* other; other->exe##state();')
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])

    def test_c_inactive_local_macro_does_not_shadow_header(self):
        override = self.f.header.replace('#pragma once\n', '').replace('exe##state()', 'exeTwo()')
        self.f.source = self.f.source.replace('#include "Game/LiveActor/Nerve.hpp"',
                                             '#include "Game/LiveActor/Nerve.hpp"\n#if 0\n' + override + '#endif\n')
        r = self.f.run()
        self.assertEqual(r['coverage']['MATCH'], 2, r['rows'])
        self.assertEqual(r['mismatches'], [])

    def test_conditional_macro_override_is_unknown(self):
        override = self.f.header.replace('#pragma once\n', '').replace('exe##state()', 'exeTwo()')
        self.f.source = self.f.source.replace('#include "Game/LiveActor/Nerve.hpp"',
                                             '#include "Game/LiveActor/Nerve.hpp"\n#if UNBOUND_FEATURE\n' + override + '#endif\n')
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])

    def test_undefined_getter_is_unknown(self):
        self.f.header = self.f.header.replace('#define GET_NERVE(cls, nerve) (&Nrv##cls::nerve::sInstance)', '')
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])

    def test_getter_actual_definition_changes_binding(self):
        self.f.header = self.f.header.replace('(&Nrv##cls::nerve::sInstance)', '(&NrvThing::Done::sInstance)')
        r = self.f.run()
        self.assertEqual(r['coverage']['MISMATCH'], 1, r['rows'])
        self.assertIn('PRODUCER_SINGLETON_BINDING', r['mismatches'][0]['checks'])

    def test_method_token_macro_is_unknown(self):
        self.f.header += '\n#define exeOne exeTwo\n'
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 1)
        self.assertEqual(r['mismatches'], [])

    def test_c_unreachable_consumer_pattern_is_unknown(self):
        self.f.write(0x80001140, [0x4e800020, 0x7c7f1b78, 0x807f0004, 0x7fe4fb78,
                                 0x81830000, 0x818c0008, 0x7d8903a6, 0x4e800421])
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertFalse(r['dispatch_consumer']['verified'])

    def test_consumer_wrong_spine_receiver_is_unknown(self):
        self.f.write(0x80001140, [0x60000000])
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertFalse(r['dispatch_consumer']['verified'])

    def test_c_partial_constructor_vptr_store_is_unknown(self):
        self.f.write(0x80001080, [0x3c808000, li(4, 4, 0x2000), 0x90830000, 0x98830000, 0x4e800020])
        next(r for r in self.f.records if r['name'] == sym(self.f.A, '__ct', 'Fv'))['size'] = 20
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 1)
        self.assertEqual(r['mismatches'], [])

    def test_partial_inline_initializer_vptr_store_is_unknown(self):
        self.f.write(0x80001040, [0x3c808000, li(4, 4, 0x2000), 0x908d0000, 0x988d0001,
                                 li(3, 13, 4), jump(0x80001054, 0x800010a0, True), 0x4e800020])
        next(r for r in self.f.records if r['name'] == '__sinit_Thing')['size'] = 28
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 1, r['rows'])
        self.assertEqual(r['mismatches'], [])

    def test_c_transition_trailing_side_effect_is_unknown(self):
        self.transition()
        self.f.source = self.f.source.replace('setNerve(&NrvThing::Done::sInstance);',
                                             'setNerve(&NrvThing::Done::sInstance); sideEffect();')
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2, r['rows'])
        self.assertEqual(r['mismatches'], [])

    def test_missing_reviewed_api_contract_is_unknown(self):
        r = self.f.run(contracts=False)
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])

    def test_stale_reviewed_api_source_pin_is_unknown(self):
        self.f.save()
        with (self.f.root / 'include/Game/SyntheticApi.hpp').open('a') as f:
            f.write('\n// changed canonical header\n')
        r = n.audit(self.f.root, self.f.root/'test.dol', self.f.root/'config/symbols.txt',
                    self.f.root/'config/splits.txt', contracts_path=self.f.contract_path)
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])

    def test_unaccepted_api_review_cannot_certify(self):
        self.f.save()
        receipt_path = self.f.root / 'review-initNerve.json'
        receipt = json.loads(receipt_path.read_text())
        receipt['status'] = 'pending'
        receipt_path.write_text(json.dumps(receipt))
        manifest = json.loads(self.f.contract_path.read_text())
        manifest['contracts'][0]['review']['sha256'] = n.sha(receipt_path.read_bytes())
        self.f.contract_path.write_text(json.dumps(manifest))
        r = n.audit(self.f.root, self.f.root/'test.dol', self.f.root/'config/symbols.txt',
                    self.f.root/'config/splits.txt', contracts_path=self.f.contract_path)
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])

    def test_foreign_source_producer_receiver_is_unknown(self):
        self.f.source = self.f.source.replace('initNerve(GET_NERVE(Thing, Ready)', 'other->initNerve(GET_NERVE(Thing, Ready)')
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 1)
        self.assertEqual(r['mismatches'], [])

    def test_nonzero_or_ambiguous_source_base_is_unknown(self):
        self.f.header = self.f.header.replace('public Nerve', 'public Other, public Nerve')
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])

    def test_source_static_instance_wrong_type_is_unknown(self):
        self.f.header = self.f.header.replace('name name::sInstance;', 'Other name::sInstance;')
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])

    def test_source_static_instance_missing_definition_is_unknown(self):
        self.f.header = self.f.header.replace('name name::sInstance;', '')
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])

    def test_unmapped_execute_class_retains_unknown_denominator(self):
        self.f.source += '\nclass Custom : public UnknownBase { public: void execute(Spine* p) const {} };\n'
        r = self.f.run()
        self.assertEqual(r['coverage']['universe'], 3)
        self.assertEqual(r['coverage']['MATCH'], 2)
        self.assertEqual(r['coverage']['UNKNOWN'], 1)

    def test_local_source_api_override_is_unknown(self):
        self.f.source = self.f.source.replace('void init();', 'void initNerve(const Nerve*, long); void init();')
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])

    def test_unbound_source_controller_base_is_unknown(self):
        self.f.source = self.f.source.replace('class Thing : public NerveExecutor', 'class Thing : public Unbound')
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])

    def test_api_contract_whole_dol_pin_is_required(self):
        self.f.save()
        image = self.f.root / 'test.dol'
        data = bytearray(image.read_bytes())
        data[-1] ^= 1
        image.write_bytes(data)
        r = n.audit(self.f.root, image, self.f.root/'config/symbols.txt', self.f.root/'config/splits.txt',
                    contracts_path=self.f.contract_path)
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])

    def catalogue(self):
        self.f.save()
        root = self.f.root
        header = 'include/Game/SyntheticApi.hpp'
        digest = n.sha((root / header).read_bytes())
        pins = dict(source_commit='invented', files={header: digest},
                    symbols_sha256=n.sha((root / 'config/symbols.txt').read_bytes()),
                    splits_sha256=n.sha((root / 'config/splits.txt').read_bytes()))
        (root / 'pins.json').write_text(json.dumps(pins))
        partitions = []
        for family in ('state_registration', 'custom_mario_state'):
            row = dict(id=family + '-1', form=family, kind='state_controller_or_complex',
                       audit_status='UNKNOWN', source_classification='INCLUDED',
                       evidence=dict(path=header, line=1, sha256=digest, conditional_unknown=False))
            p = root / (family + '.json')
            p.write_text(json.dumps(dict(source_commit='invented', rows=[row], row_count=1, scope='synthetic catalogue')))
            partitions.append(dict(path=p.name, sha256=n.sha(p.read_bytes()), row_count=1))
        checkpoint = dict(schema=1, source_commit='invented', partitions=partitions,
                          source_pins=dict(path='pins.json', sha256=n.sha((root / 'pins.json').read_bytes())))
        p = root / 'catalogue.json'
        p.write_text(json.dumps(checkpoint))
        return p

    def test_catalogue_preserves_independent_unknown_denominators(self):
        catalogue = self.catalogue()
        r = n.audit(self.f.root, self.f.root/'test.dol', self.f.root/'config/symbols.txt', self.f.root/'config/splits.txt',
                    inventory=True, catalogue_path=catalogue)
        self.assertEqual(r['coverage']['universe'], 2)
        self.assertEqual(len(r['nonstandard_catalogue']['partitions']), 2)
        for partition in r['nonstandard_catalogue']['partitions']:
            self.assertEqual(partition['coverage']['UNKNOWN'], 1)
            self.assertEqual(partition['coverage']['MATCH'], 0)
            self.assertTrue(partition['rows'][0]['source_pin_current'])

    def test_stale_catalogue_keeps_unknown_occurrences(self):
        catalogue = self.catalogue()
        (self.f.root / 'include/Game/SyntheticApi.hpp').write_text('// changed\n')
        r = n.source_catalogue(catalogue, self.f.root, n.Symbols(self.f.root/'config/symbols.txt'),
                               n.Splits(self.f.root/'config/splits.txt'))
        self.assertEqual(r['status'], 'stale_candidate_catalogue')
        self.assertTrue(all(p['coverage']['UNKNOWN'] == 1 for p in r['partitions']))
        self.assertTrue(all(not p['rows'][0]['source_pin_current'] for p in r['partitions']))

    def test_unreachable_startup_constants_are_unknown(self):
        self.f.write(0x80001000, [0x4e800020, 0x3da08000, 0x61ad3000])
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['startup_bases']['values'], {})

    def test_partial_dol_section_overlap_is_refused(self):
        struct.pack_into('>I', self.f.data, 8 * 4, 0x700)
        struct.pack_into('>I', self.f.data, 0x48 + 8 * 4, 0x80002002)
        struct.pack_into('>I', self.f.data, 0x90 + 8 * 4, 4)
        with self.assertRaisesRegex(ValueError, 'overlapping DOL'):
            self.f.run()

    def test_reviewed_api_scope_cannot_authorize_other_callsite(self):
        self.f.save()
        manifest = json.loads(self.f.contract_path.read_text())
        entry = manifest['contracts'][0]
        entry['claim']['allowed_callsites'] = entry['claim']['allowed_callsites'][:1]
        receipt_path = self.f.root / entry['review']['path']
        receipt = json.loads(receipt_path.read_text())
        receipt['claim_sha256'] = n.canonical_sha(entry['claim'])
        receipt_path.write_text(json.dumps(receipt))
        entry['review']['sha256'] = n.sha(receipt_path.read_bytes())
        self.f.contract_path.write_text(json.dumps(manifest))
        r = n.audit(self.f.root, self.f.root/'test.dol', self.f.root/'config/symbols.txt', self.f.root/'config/splits.txt',
                    contracts_path=self.f.contract_path)
        self.assertEqual(r['coverage']['MATCH'], 1)
        self.assertEqual(r['coverage']['UNKNOWN'], 1)
        self.assertEqual(r['mismatches'], [])

    def resign_invented_manifest(self, manifest):
        """Fresh synthetic review pins isolate dataflow from stale-hash rejection."""
        for entry in manifest['contracts']:
            receipt_path = self.f.root / entry['review']['path']
            receipt = json.loads(receipt_path.read_text())
            receipt['claim_sha256'] = n.canonical_sha(entry['claim'])
            receipt_path.write_text(json.dumps(receipt))
            entry['review']['sha256'] = n.sha(receipt_path.read_bytes())
        self.f.contract_path.write_text(json.dumps(manifest))

    def audit_saved(self):
        return n.audit(self.f.root, self.f.root/'test.dol', self.f.root/'config/symbols.txt', self.f.root/'config/splits.txt',
                       contracts_path=self.f.contract_path)

    def assert_unproved_init_register(self, report, observed):
        self.assertEqual(report['coverage']['UNKNOWN'], 2)
        self.assertEqual(report['mismatches'], [])
        refused = next(r for r in report['api_contracts']['refused'] if 'unproved_callsite_register_requirements' in r['reasons'])
        self.assertNotIn('whole_DOL_pin', refused['reasons'])
        self.assertNotIn('unique_provider_span_or_TU_pin', refused['reasons'])
        self.assertNotIn('review_receipt_pin', refused['reasons'])
        proof = next(p for p in refused['derived_callsite_preconditions'] if p['pc'] != '0x80001228')
        self.assertEqual(proof['requirements'], {'r5': 0})
        self.assertEqual(proof['observed'], {'r5': observed})
        self.assertEqual(proof['entry_initial_constants'], {})
        self.assertFalse(proof['verified'])

    def test_init_zero_state_precondition_is_derived_at_call(self):
        r = self.f.run()
        self.assertEqual(r['coverage']['MATCH'], 2)
        contract = next(c for c in r['api_contracts']['accepted'] if c['claim']['api'] == 'initNerve')
        for proof in contract['derived_callsite_preconditions']:
            self.assertEqual(proof['requirements'], {'r5': 0})
            self.assertEqual(proof['observed'], {'r5': 0})
            self.assertEqual(proof['entry_initial_constants'], {})
            self.assertTrue(proof['verified'])

    def test_init_actual_nonzero_with_manifest_zero_is_unknown(self):
        self.f.write(0x80001204, [li(5, 0, 1)])
        self.assert_unproved_init_register(self.f.run(), 1)

    def test_init_actual_unknown_with_manifest_zero_is_unknown(self):
        self.f.write(0x80001204, [0x80a30000])  # unknown pointer load cannot establish r5=0
        self.assert_unproved_init_register(self.f.run(), None)

    def branch_init(self, alternative):
        self.f.write(0x80001200, [li(4, 13, 0), li(5, 0, 0), 0x41820008,
                                 jump(0x8000120c, 0x80001214), alternative,
                                 jump(0x80001214, 0x80001400, True), 0x4e800020])
        next(r for r in self.f.records if r['name'] == sym('Thing', 'init', 'Fv'))['size'] = 28

    def test_init_one_path_overwrites_zero_is_unknown(self):
        self.branch_init(li(5, 0, 1))
        r = self.f.run()
        self.assert_unproved_init_register(r, None)
        proof = next(p for p in r['api_contracts']['refused'][0]['derived_callsite_preconditions'] if p['pc'] == '0x80001214')
        self.assertEqual(proof['issues'], [])
        self.assertTrue(proof['reachable_call'])

    def test_init_one_path_loads_unknown_is_unknown(self):
        self.branch_init(0x80a30000)
        self.assert_unproved_init_register(self.f.run(), None)

    def test_init_zero_agrees_on_both_reachable_paths(self):
        self.branch_init(li(5, 0, 0))
        r = self.f.run()
        self.assertEqual(r['coverage']['MATCH'], 2)
        proof = next(p for p in r['api_contracts']['accepted'][0]['derived_callsite_preconditions'] if p['pc'] == '0x80001214')
        self.assertEqual(proof['observed'], {'r5': 0})
        self.assertTrue(proof['verified'])

    def test_init_zero_requirement_cannot_be_omitted(self):
        self.f.save()
        manifest = json.loads(self.f.contract_path.read_text())
        for call in manifest['contracts'][0]['claim']['allowed_callsites']:
            call['register_requirements'] = {}
        self.resign_invented_manifest(manifest)
        r = self.audit_saved()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertTrue(any('unsupported_or_missing_machine_preconditions' in item['reasons'] for item in r['api_contracts']['refused']))

    def test_init_shifted_bl_pc_cannot_label_addi_as_call(self):
        self.f.save()
        manifest = json.loads(self.f.contract_path.read_text())
        call = manifest['contracts'][0]['claim']['allowed_callsites'][0]
        call['pc'] = '0x80001204'  # actual r5 ADDI, four bytes before the BL
        call['instruction_sha256'] = n.sha(n.Dol(self.f.root/'test.dol').read(0x80001204, 4))
        self.resign_invented_manifest(manifest)
        r = self.audit_saved()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])
        self.assertTrue(any('unverified_callsite_scope_or_branch' in item['reasons'] for item in r['api_contracts']['refused']))

    def test_init_misaligned_bl_pc_is_rejected(self):
        self.f.save()
        manifest = json.loads(self.f.contract_path.read_text())
        call = manifest['contracts'][0]['claim']['allowed_callsites'][0]
        call['pc'] = '0x8000120a'
        call['instruction_sha256'] = n.sha(n.Dol(self.f.root/'test.dol').read(0x8000120a, 4))
        self.resign_invented_manifest(manifest)
        r = self.audit_saved()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])
        self.assertTrue(any('unverified_callsite_scope_or_branch' in item['reasons'] for item in r['api_contracts']['refused']))

    def test_init_unreachable_zero_bl_is_unknown(self):
        self.f.write(0x80001200, [0x4e800020])
        r = self.f.run()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])
        proof = next(p for item in r['api_contracts']['refused'] for p in item['derived_callsite_preconditions'] if p['pc'] == '0x80001208')
        self.assertFalse(proof['reachable_call'])
        self.assertFalse(proof['verified'])

    def reject_invalid_precondition_list(self, value):
        self.transition()
        self.f.save()
        manifest = json.loads(self.f.contract_path.read_text())
        setter = next(entry for entry in manifest['contracts'] if entry['claim']['api'] == 'setNerve')
        setter['claim']['preconditions'] = value
        self.resign_invented_manifest(manifest)
        r = self.audit_saved()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])
        self.assertEqual([c['claim']['api'] for c in r['api_contracts']['accepted']], ['initNerve'])
        self.assertTrue(any('missing_reviewed_preconditions' in item['reasons'] for item in r['api_contracts']['refused']))

    def test_api_preconditions_string_is_rejected(self):
        self.reject_invalid_precondition_list('not a list')

    def test_api_preconditions_dictionary_is_rejected(self):
        self.reject_invalid_precondition_list({'not a list': True})

    def assert_entry_unknown(self, fixture, reason):
        fixture.refresh_pins()  # Mutants keep fresh pins; isolate the relation proof.
        result = fixture.candidate()
        self.assertEqual(result['status'], 'UNKNOWN', result)
        self.assertFalse(result['production_acceptance'])
        self.assertIn(reason, result['reasons'])

    def test_entry_save_candidate_preserves_all_records_and_offsets(self):
        f = EntryFixture(self.tmp.name)
        result = f.candidate()
        self.assertEqual(result['status'], 'CANDIDATE_BOUND', result)
        self.assertFalse(result['production_acceptance'])
        self.assertEqual([entry['offset'] for entry in result['entries']], [0, 4, 8])
        self.assertEqual([entry['register'] for entry in result['entries']], [14, 15, 16])
        self.assertEqual(len(result['records']), 4)
        self.assertEqual(result['explicit_owner_bytes'], 16)
        self.assertEqual(result['references'][0]['offset'], 8)
        self.assertEqual(result['references'][0]['entry_symbol'], f.entry_prefix + '16')
        self.assertTrue(all(entry['record']['size'] == 0 and not entry['record']['size_explicit'] for entry in result['entries']))

    def test_entry_restore_candidate_binds_complete_body(self):
        f = EntryFixture(self.tmp.name, 'restore')
        result = f.candidate()
        self.assertEqual(result['status'], 'CANDIDATE_BOUND', result)
        self.assertEqual(result['encoded_body_sha256'], f.relation['owner']['sha256'])
        self.assertFalse(result['production_acceptance'])

    def test_entry_different_first_entry_is_unknown(self):
        f = EntryFixture(self.tmp.name)
        f.relation['first_entry'] = f.entry_prefix + '15'
        self.assert_entry_unknown(f, 'incomplete_entry_sequence')

    def test_entry_different_start_label_address_is_unknown(self):
        f = EntryFixture(self.tmp.name)
        text = f.symbol_path.read_text().replace(f'{f.entry_prefix}14 = .text:0x80001440', f'{f.entry_prefix}14 = .text:0x80001444')
        f.symbol_path.write_text(text)
        self.assert_entry_unknown(f, 'entry_type_address_size_or_owner_conflict')

    def test_entry_wrong_internal_label_offset_is_unknown(self):
        f = EntryFixture(self.tmp.name)
        f.symbol_path.write_text(f.symbol_path.read_text().replace(f'{f.entry_prefix}16 = .text:0x80001448',
                                                                  f'{f.entry_prefix}16 = .text:0x80001444'))
        self.assert_entry_unknown(f, 'entry_type_address_size_or_owner_conflict')

    def test_entry_conflicting_explicit_owner_extent_is_unknown(self):
        f = EntryFixture(self.tmp.name)
        f.relation['owner']['size'] = 12
        self.assert_entry_unknown(f, 'owner_type_or_explicit_extent_mismatch')

    def test_entry_owner_extent_is_never_invented(self):
        f = EntryFixture(self.tmp.name)
        f.symbol_path.write_text(f.symbol_path.read_text().replace('type:function size:0x10', 'type:function'))
        self.assert_entry_unknown(f, 'owner_type_or_explicit_extent_mismatch')

    def test_entry_label_cannot_be_given_an_extent(self):
        f = EntryFixture(self.tmp.name)
        f.symbol_path.write_text(f.symbol_path.read_text().replace(f'{f.entry_prefix}14 = .text:0x80001440; // type:label',
                                                                  f'{f.entry_prefix}14 = .text:0x80001440; // type:label size:0x10'))
        self.assert_entry_unknown(f, 'entry_type_address_size_or_owner_conflict')

    def test_entry_conflicting_machine_body_is_unknown_with_fresh_pins(self):
        f = EntryFixture(self.tmp.name)
        image = bytearray((f.root/'test.dol').read_bytes())
        image[0x100 + f.start - 0x80001000 + 3] ^= 4
        (f.root/'test.dol').write_bytes(image)
        self.assert_entry_unknown(f, 'source_body_or_extent_conflict')

    def test_entry_conflicting_source_body_is_unknown_with_fresh_pins(self):
        f = EntryFixture(self.tmp.name)
        f.source_path.write_text(f.source_path.read_text().replace('stw r15,-68', 'lwz r15,-68'))
        self.assert_entry_unknown(f, 'entry_instruction_register_or_slot_mismatch')

    def test_entry_unrelated_same_address_label_is_unknown(self):
        f = EntryFixture(self.tmp.name)
        with f.symbol_path.open('a') as out:
            out.write('unrelated = .text:0x80001440; // type:label\n')
        self.assert_entry_unknown(f, 'unrelated_or_overlapping_symbol_record')

    def test_entry_unrelated_same_address_function_is_unknown(self):
        f = EntryFixture(self.tmp.name)
        with f.symbol_path.open('a') as out:
            out.write('unrelated = .text:0x80001440; // type:function size:0x10\n')
        self.assert_entry_unknown(f, 'unrelated_or_overlapping_symbol_record')

    def test_entry_overlapping_function_is_unknown(self):
        f = EntryFixture(self.tmp.name)
        with f.symbol_path.open('a') as out:
            out.write('overlapping = .text:0x8000143c; // type:function size:0x8\n')
        self.assert_entry_unknown(f, 'unrelated_or_overlapping_symbol_record')

    def test_entry_missing_ownership_is_unknown(self):
        f = EntryFixture(self.tmp.name)
        f.split_path.write_text(f.split_path.read_text().replace('Runtime/runtime.c:', 'Unknown/provider.c:'))
        self.assert_entry_unknown(f, 'missing_or_ambiguous_extent_ownership')

    def test_entry_ambiguous_ownership_is_unknown(self):
        f = EntryFixture(self.tmp.name)
        with f.split_path.open('a') as out:
            out.write('Unknown/provider.c:\n .text start:0x80001440 end:0x80001450\n')
        self.assert_entry_unknown(f, 'missing_or_ambiguous_extent_ownership')

    def test_entry_missing_source_declaration_is_unknown(self):
        f = EntryFixture(self.tmp.name)
        f.source_path.write_text(f.source_path.read_text().replace(f.function, '__different_function'))
        self.assert_entry_unknown(f, 'missing_or_ambiguous_source_function')

    def test_entry_conditional_macro_provenance_is_unknown(self):
        f = EntryFixture(self.tmp.name)
        f.source_path.write_text(f.source_path.read_text().replace('#define SLOT(reg)', '#if UNKNOWN_FEATURE\n#define SLOT(reg)')
                                .replace('#define TAG(reg)', '#endif\n#define TAG(reg)'))
        self.assert_entry_unknown(f, 'unresolved_macro_provenance')

    def test_entry_r11_alias_cannot_be_changed(self):
        f = EntryFixture(self.tmp.name)
        f.source_path.write_text(f.source_path.read_text().replace('STACK_ANCHOR r11', 'STACK_ANCHOR r12'))
        self.assert_entry_unknown(f, 'unsupported_save_restore_base')

    def test_entry_source_register_macro_is_not_guessed(self):
        f = EntryFixture(self.tmp.name)
        f.source_path.write_text('#define r14 r15\n' + f.source_path.read_text())
        self.assert_entry_unknown(f, 'unmodeled_semantic_macro')

    def test_entry_approved_start_does_not_authorize_wrong_internal_target(self):
        f = EntryFixture(self.tmp.name)
        image = bytearray((f.root/'test.dol').read_bytes())
        struct.pack_into('>I', image, 0x100 + 0x1460 - 0x1000, jump(0x80001460, f.start + 4, True))
        (f.root/'test.dol').write_bytes(image)
        self.assert_entry_unknown(f, 'entry_reference_target_or_pin_conflict')

    def test_entry_reference_cannot_select_unproven_internal_label(self):
        f = EntryFixture(self.tmp.name)
        f.relation['references'][0]['entry_symbol'] = f.entry_prefix + '15'
        self.assert_entry_unknown(f, 'entry_reference_target_or_pin_conflict')

    def test_entry_directive_macro_cannot_be_guessed(self):
        f = EntryFixture(self.tmp.name)
        f.source_path.write_text('#define entry something_else\n' + f.source_path.read_text())
        self.assert_entry_unknown(f, 'unmodeled_semantic_macro')

    def test_entry_register_sequence_cannot_be_relabelled(self):
        f = EntryFixture(self.tmp.name)
        f.relation['registers'] = [14, 16, 15]
        self.assert_entry_unknown(f, 'unsupported_gpr_entry_sequence')

    def test_entry_missing_separate_prototype_is_unknown(self):
        f = EntryFixture(self.tmp.name)
        f.source_path.write_text(f.source_path.read_text().replace('void SLOT(15)(void);', ''))
        self.assert_entry_unknown(f, 'missing_or_ambiguous_source_entry_prototype')

    def test_entry_duplicate_separate_prototype_is_unknown(self):
        f = EntryFixture(self.tmp.name)
        f.source_path.write_text(f.source_path.read_text().replace('void SLOT(15)(void);', 'void SLOT(15)(void);\nvoid SLOT(15)(void);'))
        self.assert_entry_unknown(f, 'missing_or_ambiguous_source_entry_prototype')

    def test_entry_bound_candidate_does_not_allow_api_dependency(self):
        f = EntryFixture(self.tmp.name)
        self.assertEqual(f.candidate()['status'], 'CANDIDATE_BOUND')
        self.f = f.base
        manifest = json.loads(self.f.contract_path.read_text())
        manifest['contracts'][0]['claim']['dependencies'].append(f.relation['owner'])
        self.resign_invented_manifest(manifest)
        r = self.audit_saved()
        self.assertEqual(r['coverage']['UNKNOWN'], 2)
        self.assertEqual(r['mismatches'], [])
        self.assertTrue(any('unique_provider_span_or_TU_pin' in refused['reasons'] for refused in r['api_contracts']['refused']))

    def assert_preview_refused(self, fixture):
        report = fixture.preview()
        self.assertFalse(report['production_acceptance'])
        self.assertEqual(report['would_accept'], [], report)
        accepted, enabled = fixture.enabled()
        self.assertEqual(accepted, {}, enabled)
        self.assertEqual(enabled['accepted'], [])
        self.assertEqual(enabled['refused'], report['refused'])
        return report

    def test_entry_enabled_requires_explicit_reviews_and_preserves_records(self):
        f = EntryIntegrationFixture(self.tmp.name)
        before = copy.deepcopy(n.Symbols(f.symbol_path).rows)
        accepted, report = f.enabled()
        self.assertEqual(set(accepted), {0x80001400}, report)
        self.assertEqual(report['accepted'], f.preview()['would_accept'])
        identity = report['accepted'][0]['entry_dependency_reviews']
        self.assertEqual(identity['status'], 'reviewed_dependency_identity')
        self.assertFalse(identity['accepted'][0]['production_acceptance'])
        self.assertFalse(identity['accepted'][0]['candidate']['production_acceptance'])
        self.assertEqual(before, n.Symbols(f.symbol_path).rows)
        strict, _ = n.api_contracts(f.base.contract_path, f.root, n.Symbols(f.symbol_path),
                                   n.Splits(f.split_path), n.Dol(f.root/'test.dol'))
        self.assertEqual(strict, {})
        self.assertEqual(f.enabled_dispatch()['coverage']['MATCH'], 2)
        strict_dispatch = n.audit(f.root, f.root/'test.dol', f.symbol_path, f.split_path,
                                  contracts_path=f.base.contract_path)
        self.assertEqual(strict_dispatch['coverage']['UNKNOWN'], 2)

    def test_entry_enabled_dispatch_keeps_source_class_gate(self):
        f = EntryIntegrationFixture(self.tmp.name)
        p = f.root/'src/Game/Thing.cpp'
        p.write_text(p.read_text().replace('class Thing : public NerveExecutor', 'class Thing : public Unbound'))
        self.assertEqual(len(f.enabled()[0]), 1)
        self.assertEqual(f.enabled_dispatch()['coverage']['UNKNOWN'], 2)

    def test_entry_enabled_dispatch_keeps_static_instance_gate(self):
        f = EntryIntegrationFixture(self.tmp.name)
        p = f.root/'include/Game/LiveActor/Nerve.hpp'
        p.write_text(p.read_text().replace('name name::sInstance;', ''))
        self.assertEqual(len(f.enabled()[0]), 1)
        self.assertEqual(f.enabled_dispatch()['coverage']['UNKNOWN'], 2)

    def test_entry_enabled_dispatch_keeps_source_contradiction(self):
        f = EntryIntegrationFixture(self.tmp.name)
        p = f.root/'src/Game/Thing.cpp'
        self.assertIn('Ready, Thing, One', p.read_text())
        p.write_text(p.read_text().replace('Ready, Thing, One', 'Ready, Thing, Two'))
        self.assertEqual(len(f.enabled()[0]), 1)
        report = f.enabled_dispatch()
        self.assertEqual(report['coverage']['MISMATCH'], 1, report)
        self.assertEqual(len(report['mismatches']), 1)

    def test_entry_enabled_audit_rejects_reviews_without_contracts_or_in_inventory(self):
        f = EntryIntegrationFixture(self.tmp.name)
        for extra in ({}, dict(contracts_path=f.base.contract_path, inventory=True)):
            with self.subTest(extra=extra), self.assertRaisesRegex(ValueError, 'require API contracts'):
                n.audit(f.root, f.root/'test.dol', f.symbol_path, f.split_path,
                        entry_reviews_path=f.entry_manifest_path, **extra)

    def test_entry_enabled_cli_requires_contracts_and_non_inventory_mode(self):
        f = EntryIntegrationFixture(self.tmp.name)
        base = [sys.executable, '-B', n.__file__, '--root', str(f.root), '--dol', str(f.root/'test.dol'),
                '--symbols', str(f.symbol_path), '--splits', str(f.split_path),
                '--entry-reviews', str(f.entry_manifest_path), '--output', str(f.root/'cli.json')]
        for extra in (['--class', 'NrvThing::Ready'], ['--api-contracts', str(f.base.contract_path), '--inventory-only']):
            with self.subTest(extra=extra):
                result = subprocess.run(base + extra, capture_output=True, text=True)
                self.assertEqual(result.returncode, 2)
                self.assertIn('requires --api-contracts and a non-inventory mode', result.stderr)
                self.assertFalse((f.root/'cli.json').exists())
        result = subprocess.run(base + ['--api-contracts', str(f.base.contract_path), '--class', 'NrvThing::Ready'],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(json.loads((f.root/'cli.json').read_text())['coverage']['MATCH'], 1)

    def test_entry_integration_preview_viable_but_production_unchanged(self):
        f = EntryIntegrationFixture(self.tmp.name)
        before = copy.deepcopy(n.Symbols(f.symbol_path).rows)
        preview = f.preview()
        self.assertFalse(preview['production_acceptance'])
        self.assertEqual([c['claim']['api'] for c in preview['would_accept']], ['initNerve'], preview)
        self.assertEqual(len(preview['would_accept'][0]['entry_dependency_reviews']['accepted']), 1)
        self.assertEqual(before, n.Symbols(f.symbol_path).rows)
        strict, _ = n.api_contracts(f.base.contract_path, f.root, n.Symbols(f.symbol_path), n.Splits(f.split_path), n.Dol(f.root/'test.dol'))
        self.assertEqual(strict, {})
        conditional = f.dispatch_preview()
        self.assertEqual(conditional['coverage']['would_MATCH'], 2, conditional)
        self.assertTrue(all(r['status'] == 'WOULD_MATCH' and not r['production_acceptance'] for r in conditional['rows']))

    def test_entry_integration_missing_identity_receipt_cannot_grant(self):
        f = EntryIntegrationFixture(self.tmp.name)
        f.identity_path.unlink()
        self.assert_preview_refused(f)

    def test_entry_integration_pending_identity_cannot_grant(self):
        f = EntryIntegrationFixture(self.tmp.name)
        receipt = json.loads(f.identity_path.read_text()); receipt['status'] = 'pending'
        f.identity_path.write_text(json.dumps(receipt)); f.refresh_reviews()
        self.assert_preview_refused(f)

    def test_entry_integration_missing_replay_cannot_grant(self):
        f = EntryIntegrationFixture(self.tmp.name)
        f.replay_path.unlink()
        self.assert_preview_refused(f)

    def test_entry_integration_pending_replay_cannot_grant(self):
        f = EntryIntegrationFixture(self.tmp.name)
        receipt = json.loads(f.replay_path.read_text()); receipt['status'] = 'pending'
        f.replay_path.write_text(json.dumps(receipt)); f.refresh_reviews()
        self.assert_preview_refused(f)

    def test_entry_integration_other_API_claim_receipt_cannot_grant(self):
        f = EntryIntegrationFixture(self.tmp.name)
        f.claim['preconditions'].append('Changed reviewed domain.')
        f.refresh_reviews(identity=False)
        self.assert_preview_refused(f)

    def test_entry_integration_stale_relation_receipt_cannot_grant(self):
        f = EntryIntegrationFixture(self.tmp.name)
        receipt = json.loads(f.identity_path.read_text()); receipt['relation_sha256'] = '0' * 64
        f.identity_path.write_text(json.dumps(receipt)); f.refresh_reviews(identity=False)
        self.assert_preview_refused(f)

    def test_entry_integration_valid_alias_cannot_replace_semantic_review(self):
        f = EntryIntegrationFixture(self.tmp.name)
        semantic = f.root/'review-initNerve.json'
        receipt = json.loads(semantic.read_text()); receipt['status'] = 'pending'
        semantic.write_text(json.dumps(receipt)); f.refresh_reviews()
        report = self.assert_preview_refused(f)
        refused = next(r for r in report['refused'] if 'missing_or_unaccepted_review' in r['reasons'])
        self.assertEqual(len(refused['entry_dependency_reviews']['accepted']), 1)

    def test_entry_integration_valid_alias_cannot_bypass_register_facts(self):
        f = EntryIntegrationFixture(self.tmp.name)
        image = bytearray((f.root/'test.dol').read_bytes())
        struct.pack_into('>I', image, 0x100 + 0x1224 - 0x1000, li(5, 0, 1))
        (f.root/'test.dol').write_bytes(image); f.refresh_reviews()
        report = self.assert_preview_refused(f)
        refused = next(r for r in report['refused'] if 'unproved_callsite_register_requirements' in r['reasons'])
        self.assertEqual(len(refused['entry_dependency_reviews']['accepted']), 1)

    def test_entry_integration_valid_alias_cannot_bypass_caller_PC(self):
        f = EntryIntegrationFixture(self.tmp.name)
        f.claim['allowed_callsites'][0]['pc'] = '0x80001204'
        f.refresh_reviews()
        self.assert_preview_refused(f)

    def test_entry_integration_valid_alias_cannot_bypass_source_class(self):
        f = EntryIntegrationFixture(self.tmp.name)
        p = f.root/'src/Game/Thing.cpp'
        p.write_text(p.read_text().replace('class Thing : public NerveExecutor', 'class Thing : public Unbound'))
        self.assertEqual(len(f.preview()['would_accept']), 1)
        report = f.dispatch_preview()
        self.assertEqual(report['coverage']['UNKNOWN'], 2)
        self.assertTrue(all('unbound_source_API_receiver_or_override' in row['reasons'] for row in report['rows']))

    def test_entry_integration_valid_alias_cannot_bypass_static_instance(self):
        f = EntryIntegrationFixture(self.tmp.name)
        p = f.root/'include/Game/LiveActor/Nerve.hpp'
        p.write_text(p.read_text().replace('name name::sInstance;', ''))
        self.assertEqual(len(f.preview()['would_accept']), 1)
        report = f.dispatch_preview()
        self.assertEqual(report['coverage']['UNKNOWN'], 2)
        self.assertTrue(all('unknown_or_ambiguous_source_ownership' in row['reasons'] for row in report['rows']))

    def test_entry_integration_other_reference_caller_cannot_grant(self):
        f = EntryIntegrationFixture(self.tmp.name)
        reference = f.relation['references'][0]
        reference.update(caller=dict(symbol='__spec_caller', address='0x80001460', section='.text', size=8,
                                     unit='Runtime/runtime.c', sha256=''), pc='0x80001460')
        f.refresh_reviews()
        self.assertEqual(f.candidate()['status'], 'CANDIDATE_BOUND')
        self.assert_preview_refused(f)

    def test_entry_integration_omitted_second_direct_branch_cannot_grant(self):
        f = EntryIntegrationFixture(self.tmp.name)
        image = bytearray((f.root/'test.dol').read_bytes())
        struct.pack_into('>II', image, 0x100 + 0x1408 - 0x1000, jump(0x80001408, f.start + 4, True), 0x4e800020)
        (f.root/'test.dol').write_bytes(image)
        f.symbol_path.write_text(f.symbol_path.read_text().replace('type:function size:0xC', 'type:function size:0x10'))
        f.claim['provider']['size'] = 16; f.relation['references'][0]['caller']['size'] = 16
        f.refresh_reviews()
        self.assertEqual(f.candidate()['status'], 'CANDIDATE_BOUND')
        report = self.assert_preview_refused(f)
        self.assertTrue(any(r.get('reason') == 'incomplete_direct_entry_reference_census'
                            for item in report['refused'] for r in item['entry_dependency_reviews']['refused']))

    def test_entry_integration_unrelated_cross_section_alias_cannot_grant(self):
        f = EntryIntegrationFixture(self.tmp.name)
        with f.symbol_path.open('a') as out:
            out.write('unrelated_data = .data:0x80001440; // type:object size:0x4\n')
        f.refresh_reviews()
        self.assert_preview_refused(f)

    def api_extent_fixture(self):
        self.f = Fixture(self.tmp.name); self.f.save()
        manifest = json.loads(self.f.contract_path.read_text())
        image = n.Dol(self.f.root/'test.dol')
        manifest['contracts'][0]['claim']['dependencies'] = [dict(symbol=sym('Thing', 'exeOne', 'Fv'), section='.text',
            address='0x80001300', size=4, unit='Game/Thing.cpp', sha256=n.sha(image.read(0x80001300, 4)))]
        self.resign_invented_manifest(manifest)
        self.extent_ranges = dict(provider=(0x80001400, 0x80001408), caller=(0x80001200, 0x80001210), dependency=(0x80001300, 0x80001304))
        text = 'Game/Thing.cpp:\n'
        for lo, hi in list(self.extent_ranges.values()) + [(0x80001220, 0x80001230)]:
            text += f' .text start:0x{lo:08x} end:0x{hi:08x}\n'
        (self.f.root/'config/splits.txt').write_text(text)

    def assert_extent_refused(self, role):
        self.api_extent_fixture()
        lo, hi = self.extent_ranges[role]
        p = self.f.root/'config/splits.txt'
        p.write_text(p.read_text().replace(f'end:0x{hi:08x}', f'end:0x{lo + 1:08x}'))
        accepted, report = n.api_contracts(self.f.contract_path, self.f.root, n.Symbols(self.f.root/'config/symbols.txt'),
                                          n.Splits(p), n.Dol(self.f.root/'test.dol'))
        self.assertEqual(accepted, {})
        self.assertTrue(any(c['role'] == role and not c['full_extent_owned'] for r in report['refused'] for c in r['extent_checks']))

    def test_API_full_extent_positive_control(self):
        self.api_extent_fixture()
        accepted, _ = n.api_contracts(self.f.contract_path, self.f.root, n.Symbols(self.f.root/'config/symbols.txt'),
                                      n.Splits(self.f.root/'config/splits.txt'), n.Dol(self.f.root/'test.dol'))
        self.assertEqual(set(accepted), {0x80001400})

    def test_API_provider_first_byte_ownership_is_insufficient(self):
        self.assert_extent_refused('provider')

    def test_API_caller_first_byte_ownership_is_insufficient(self):
        self.assert_extent_refused('caller')

    def test_API_dependency_first_byte_ownership_is_insufficient(self):
        self.assert_extent_refused('dependency')

    def test_API_competing_byte_overlap_rejects_each_role(self):
        for role in ('provider', 'caller', 'dependency'):
            with self.subTest(role=role):
                self.api_extent_fixture()
                lo, _ = self.extent_ranges[role]
                p = self.f.root/'config/splits.txt'
                with p.open('a') as out:
                    out.write(f'Other.cpp:\n .text start:0x{lo + 1:08x} end:0x{lo + 2:08x}\n')
                accepted, _ = n.api_contracts(self.f.contract_path, self.f.root, n.Symbols(self.f.root/'config/symbols.txt'),
                                              n.Splits(p), n.Dol(self.f.root/'test.dol'))
                self.assertEqual(accepted, {})

    def test_API_touching_boundaries_are_not_overlaps(self):
        self.api_extent_fixture()
        p = self.f.root/'config/splits.txt'
        with p.open('a') as out:
            out.write('Other.cpp:\n .text start:0x800013fc end:0x80001400\n .text start:0x80001408 end:0x80001410\n')
        accepted, _ = n.api_contracts(self.f.contract_path, self.f.root, n.Symbols(self.f.root/'config/symbols.txt'),
                                      n.Splits(p), n.Dol(self.f.root/'test.dol'))
        self.assertEqual(set(accepted), {0x80001400})

    def test_entry_integration_unknown_second_caller_register_cannot_grant(self):
        f = EntryIntegrationFixture(self.tmp.name)
        image = bytearray((f.root/'test.dol').read_bytes())
        struct.pack_into('>I', image, 0x100 + 0x1224 - 0x1000, 0x80a30000)
        (f.root/'test.dol').write_bytes(image); f.refresh_reviews()
        self.assert_preview_refused(f)

    def test_entry_integration_valid_alias_cannot_bypass_stale_source_header(self):
        f = EntryIntegrationFixture(self.tmp.name)
        with (f.root/'include/Game/SyntheticApi.hpp').open('a') as out:
            out.write('\n// altered API source pin\n')
        report = self.assert_preview_refused(f)
        self.assertTrue(any('source_contract_pin' in r['reasons'] for r in report['refused']))

    def test_entry_integration_valid_alias_cannot_bypass_semantic_claim_hash(self):
        f = EntryIntegrationFixture(self.tmp.name)
        p = f.root/'review-initNerve.json'
        receipt = json.loads(p.read_text()); receipt['claim_sha256'] = '0' * 64
        p.write_text(json.dumps(receipt))
        f.api_manifest['contracts'][0]['review']['sha256'] = n.sha(p.read_bytes())
        f.base.contract_path.write_text(json.dumps(f.api_manifest))
        self.assert_preview_refused(f)

    def test_entry_integration_valid_alias_does_not_relax_other_dispatch_aliases(self):
        f = EntryIntegrationFixture(self.tmp.name)
        with f.symbol_path.open('a') as out:
            out.write('unrelated_method_label = .text:0x80001300; // type:label\n')
        f.refresh_reviews()
        self.assertEqual(len(f.preview()['would_accept']), 1)
        report = f.dispatch_preview()
        self.assertEqual(report['coverage']['UNKNOWN'], 1)
        self.assertEqual(report['coverage']['would_MATCH'], 1)

    def test_entry_integration_viable_transport_does_not_hide_source_mismatch(self):
        f = EntryIntegrationFixture(self.tmp.name)
        p = f.root/'src/Game/Thing.cpp'
        p.write_text(p.read_text().replace('Ready, Thing, One', 'Ready, Thing, Two'))
        self.assertEqual(len(f.preview()['would_accept']), 1)
        report = f.dispatch_preview()
        self.assertEqual(report['coverage']['would_MISMATCH'], 1)
        self.assertEqual(len(report['would_mismatches']), 1)
        self.assertTrue(any(row['status'] == 'WOULD_MISMATCH' for row in report['rows']))
        self.assertFalse(report['production_acceptance'])

    def test_API_missing_caller_record_is_refused_without_guessing(self):
        self.api_extent_fixture()
        p = self.f.root/'config/symbols.txt'
        p.write_text('\n'.join(line for line in p.read_text().splitlines() if not line.startswith(sym('Thing', 'init', 'Fv') + ' =')) + '\n')
        accepted, report = n.api_contracts(self.f.contract_path, self.f.root, n.Symbols(p),
                                          n.Splits(self.f.root/'config/splits.txt'), n.Dol(self.f.root/'test.dol'))
        self.assertEqual(accepted, {})
        self.assertTrue(any('ambiguous_precondition_caller' in r['reasons'] for r in report['refused']))


if __name__ == '__main__':
    unittest.main()
