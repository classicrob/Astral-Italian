import tempfile
from pathlib import Path
import unittest
from argorder import scan_text
from argorder_effects import EffectAudit,rank,summarize_words,typed_bindings,receiver_type

class PriorityTests(unittest.TestCase):
    def row(self,s):return scan_text(s)['rows'][0]
    def test_function_body_is_not_an_indirect_constructor(self):
        self.assertFalse(scan_text('void f(){int x=1,y=2; int a=3,b=4;}')['rows'])
    def test_control_and_else_blocks_not_calls(self):
        self.assertFalse(scan_text('void f(){if(ok){a=1,b=2;}else{c=3,d=4;}}')['rows'])
    def test_local_struct_body_not_constructor(self):
        self.assertFalse(scan_text('void f(){struct X{int a=1,b=2;};}')['rows'])
    def test_two_mutations_high(self):
        r=rank(self.row('void f(){ out(++x,y--); }'),lambda c:{'kind':'UNKNOWN'})
        self.assertEqual(r['level'],'HIGH');self.assertEqual(r['known_effect_argument_count'],2)
    def test_one_mutation_slot_not_high(self):
        r=rank(self.row('void f(){ out((++x,++y),getter()); }'),lambda c:{'kind':'UNKNOWN'})
        self.assertEqual(r['level'],'MEDIUM')
    def test_all_calls_proved_pure_low(self):
        row=self.row('void f(){ out(getA(),getB()); }');row['argument_order']={'status':'resolved-direct-effect-call-order'}
        r=rank(row,lambda c:{'kind':'PURE_LEAF','evidence':'fixture bounded direct body'})
        self.assertEqual(r['level'],'LOW')
    def test_pure_bodies_but_unknown_target_association_not_low(self):
        r=rank(self.row('void f(){ out(getA(),getB()); }'),lambda c:{'kind':'PURE_LEAF'})
        self.assertEqual(r['level'],'MEDIUM')
    def test_const_name_without_body_not_low(self):
        r=rank(self.row('void f(){ out(getA(),getB()); }'),lambda c:{'kind':'UNKNOWN','reason':'const annotation only'})
        self.assertEqual(r['level'],'MEDIUM')
    def test_unknown_nested_call_blocks_low(self):
        r=rank(self.row('void f(){ out(getA(),wrap(getB())); }'),lambda c:{'kind':'UNKNOWN'if c['name']=='wrap'else'PURE_LEAF'})
        self.assertEqual(r['level'],'MEDIUM')
    def test_two_write_summaries_high(self):
        r=rank(self.row('void f(){ out(constA(),constB()); }'),lambda c:{'kind':'KNOWN_WRITE','writes':['member/global']})
        self.assertEqual(r['level'],'HIGH')
    def test_shared_state_not_required_or_claimed(self):
        r=rank(self.row('void f(){ out(x++,y++); }'),lambda c:{'kind':'UNKNOWN'})
        self.assertIn('need not share storage',r['scope'])
    def test_unevaluated_mutation_not_high(self):
        r=rank(self.row('void f(){ out(sizeof(++x),y++); }'),lambda c:{'kind':'UNKNOWN'})
        self.assertEqual(r['level'],'UNKNOWN')
    def test_macro_can_drop_args_not_high(self):
        r=rank(self.row('#define OUT(a,b) a\nvoid f(){ OUT(++x,y++); }'),lambda c:{'kind':'UNKNOWN'})
        self.assertEqual(r['level'],'UNKNOWN')
    def test_deferred_lambda_not_high(self):
        r=rank(self.row('void f(){ out([&]{++x;},y++); }'),lambda c:{'kind':'UNKNOWN'})
        self.assertEqual(r['level'],'UNKNOWN')
    def test_repeated_proven_rng_high_even_order_unknown(self):
        row=self.row('void f(){ out(rng(),rng(),rng()); }')
        self.assertEqual(rank(row,lambda c:{'kind':'KNOWN_WRITE','contract':'checked RNG state update'})['level'],'HIGH')
        self.assertEqual(row['argument_order']['status'],'unknown')

class BoundedBodyTests(unittest.TestCase):
    def test_member_store(self):
        s=summarize_words([0x38000001,0x90030008,0x4e800020]);self.assertEqual(s['kind'],'KNOWN_WRITE')
        self.assertEqual(s['writes'][0]['argument_register'],3)
    def test_global_store(self):
        s=summarize_words([0x38000001,0x900dff00,0x4e800020]);self.assertEqual(s['kind'],'KNOWN_WRITE')
        self.assertEqual(s['writes'][0]['kind'],'global-storage-write')
    def test_relocated_HA_LO_is_symbol_not_zero_address(self):
        symbol=('state',0,0x200,2)
        s=summarize_words([0x3c800000,0x38840000,0x9004010c,0x4e800020],{0:(6,symbol,0),4:(4,symbol,0)})
        self.assertEqual(s['writes'][0]['kind'],'symbol-storage-write')
        self.assertEqual(s['writes'][0]['offset_from_symbol'],0x10c)
        self.assertNotIn('address',s['writes'][0])
    def test_direct_low_store_symbol_not_zero(self):
        symbol=('state',0,4,2)
        s=summarize_words([0x3c800000,0x90040000,0x4e800020],{0:(6,symbol,0),4:(4,symbol,0)})
        self.assertEqual(s['writes'][0]['symbol'],'state')
    def test_unresolved_HA_is_not_a_complete_address(self):
        symbol=('state',0,4,2)
        self.assertNotEqual(summarize_words([0x3c800000,0x9004010c,0x4e800020],{0:(6,symbol,0)})['kind'],'KNOWN_WRITE')
    def test_null_based_store_not_promoted_as_global(self):
        self.assertNotEqual(summarize_words([0x38800000,0x9004010c,0x4e800020])['kind'],'KNOWN_WRITE')
    def test_global_symbolic_sda(self):
        s=summarize_words([0x90000000,0x4e800020],{0:(109,('dirty',0,1,0),0)})
        self.assertEqual(s['writes'][0]['symbol'],'dirty')
    def test_const_member_writer_not_pure(self):
        # A const member may write mutable storage; annotation is irrelevant to this body.
        s=summarize_words([0x80030008,0x38000001,0x90030008,0x4e800020])
        self.assertEqual(s['kind'],'KNOWN_WRITE')
    def test_const_global_writer_not_pure(self):
        self.assertEqual(summarize_words([0x800dff00,0x38000001,0x900dff00,0x4e800020])['kind'],'KNOWN_WRITE')
    def test_stack_store_not_external(self):
        s=summarize_words([0x90010008,0x4e800020]);self.assertEqual(s['kind'],'UNKNOWN');self.assertFalse(s['writes'])
    def test_unknown_pointer_load_not_member_proof(self):
        s=summarize_words([0x80630008,0x90030000,0x4e800020])
        self.assertEqual(s['kind'],'UNKNOWN');self.assertTrue(s['unresolved_stores'])
    def test_volatile_mmio_getter_not_pure(self):
        self.assertEqual(summarize_words([0x3c60cd00,0x80630000,0x4e800020])['kind'],'UNKNOWN')
    def test_ordinary_member_load_conservatively_unknown(self):
        self.assertEqual(summarize_words([0x80630008,0x4e800020])['kind'],'UNKNOWN')
    def test_clock_special_register_not_pure(self):
        self.assertEqual(summarize_words([0x7c6c42e6,0x4e800020])['kind'],'UNKNOWN')
    def test_pure_constant_leaf(self):
        self.assertEqual(summarize_words([0x38600001,0x4e800020])['kind'],'PURE_LEAF')
    def test_pure_address_getter(self):
        self.assertEqual(summarize_words([0x38630008,0x4e800020])['kind'],'PURE_LEAF')
    def test_call_not_pure(self):
        self.assertEqual(summarize_words([0x48000001,0x4e800020])['kind'],'UNKNOWN')
    def test_unknown_opcode_not_pure(self):
        self.assertEqual(summarize_words([0xffffffff,0x4e800020])['kind'],'UNKNOWN')
    def test_address_saved_across_call(self):
        # mr r31,r3 ; bl opaque ; stw r0,8(r31). ABI preserves the address, not mutable contents.
        s=summarize_words([0x7c7f1b78,0x48000001,0x901f0008,0x4e800020]);self.assertEqual(s['kind'],'KNOWN_WRITE')
    def test_volatile_argument_origin_killed_by_call(self):
        self.assertEqual(summarize_words([0x48000001,0x90030008,0x4e800020])['kind'],'UNKNOWN')
    def test_member_offset_composition(self):
        s=summarize_words([0x38630010,0x90030008,0x4e800020]);self.assertEqual(s['writes'][0]['displacement'],24)
    def test_dead_absolute_store_after_branch_not_high(self):
        s=summarize_words([0x48000010,0x3c60cd00,0x90030000,0x4e800020,0x4e800020])
        self.assertNotEqual(s['kind'],'KNOWN_WRITE')
    def test_dead_store_after_early_return_not_high(self):
        s=summarize_words([0x4e800020,0x900dff00,0x4e800020])
        self.assertNotEqual(s['kind'],'KNOWN_WRITE')
    def test_conditionally_reachable_store_left_unknown(self):
        self.assertNotEqual(summarize_words([0x41820008,0x900dff00,0x4e800020])['kind'],'KNOWN_WRITE')
    def test_nonvolatile_register_mutation_not_pure(self):
        self.assertEqual(summarize_words([0x3be00001,0x4e800020])['kind'],'UNKNOWN')

class TypedRngTests(unittest.TestCase):
    def test_named_typed_receiver(self):
        b=typed_bindings('void f(JPABaseEmitter* e){ JPARandom r; JMath::TRandom_fast_ j; }')
        self.assertEqual(b['e'],'JPABaseEmitter');self.assertEqual(b['r'],'JPARandom');self.assertEqual(b['j'],'JMath::TRandom_fast_')
    def test_conflicting_shadow_refused(self):
        self.assertNotIn('e',typed_bindings('void f(JPABaseEmitter* e){ Fake* e; }'))
    def test_comment_type_does_not_resolve_receiver(self):
        self.assertNotIn('e',typed_bindings('void f(){ /* JPABaseEmitter* e; */ out(e->get_r_zp(),e->get_r_ss()); }'))
    def test_inactive_type_declaration_refused(self):
        self.assertFalse(typed_bindings('void f(){\n#if 0\n JPABaseEmitter* e;\n#endif\n}'))
    def test_same_rng_name_wrong_type_not_contract(self):
        self.assertFalse(typed_bindings('void f(Fake* e){e->get_r_zp();}'))
    def test_receiver_path_needs_owning_field_contract(self):
        self.assertIsNone(receiver_type('work->unknown',{'work':'JPAEmitterWorkData'},{}))
        self.assertEqual(receiver_type('work->mpEmtr',{'work':'JPAEmitterWorkData'},{('JPAEmitterWorkData','mpEmtr'):'JPABaseEmitter'}),'JPABaseEmitter')
    def test_pure_rng_lookalike_not_trusted(self):
        with tempfile.TemporaryDirectory()as td:
            root=Path(td);p=root/'libs/JSystem/include/JSystem/JParticle/JPARandom.hpp';p.parent.mkdir(parents=True)
            p.write_text('struct JPARandom {u32 get_rndm_u(){return mSeed=mSeed*0x19660du+0x3c6ef35fu;} float get_rndm_zp()const{return 0.0f;}u32 mSeed;};')
            e=EffectAudit(root);self.assertIn(('JPARandom','get_rndm_u'),e.contracts);self.assertNotIn(('JPARandom','get_rndm_zp'),e.contracts)
    def test_missing_contracts_unknown(self):
        with tempfile.TemporaryDirectory()as td:
            e=EffectAudit(Path(td));r=scan_text('void f(JPABaseEmitter* e){out(e->get_r_zp(),e->get_r_ss());}')['rows'][0];r['unit']='fixture'
            self.assertEqual(rank(r,lambda c:e.summary(r,c))['level'],'MEDIUM')

if __name__=='__main__':unittest.main()
