import importlib.util,json,os,sys,tempfile,unittest
from pathlib import Path
spec=importlib.util.spec_from_file_location('semclass',Path(__file__).with_name('semclass.py'));m=importlib.util.module_from_spec(spec);sys.modules[spec.name]=m;spec.loader.exec_module(m)

class KernelTests(unittest.TestCase):
    def check(self,a,b,want,sub=None,context=None):
        if context is None and any('CALLEE_' in x for x in a+b):context={'referents_bound':True}
        result=m.classify_lines(a,b,context)
        self.assertEqual(result['classification'],want,result)
        if sub:self.assertIn(sub,result['subclasses'],result)
        return result
    def test_register_rename(self):self.check(['lwz r11,0xec(r3)','mr r3,r11','blr'],['lwz r12,0xec(r3)','mr r3,r12','blr'],'ABI')
    def test_inconsistent_register_role(self):self.check(['lwz r11,0xec(r3)','lwz r12,0xf0(r3)','subf r3,r11,r12','blr'],['lwz r12,0xec(r3)','lwz r11,0xf0(r3)','subf r3,r11,r12','blr'],'SEMANTIC')
    def test_fp_add_commuted(self):self.check(['fadds f1,f2,f3','blr'],['fadds f1,f3,f2','blr'],'ABI')
    def test_fp_mul_commuted(self):self.check(['fmuls f1,f2,f3','blr'],['fmuls f1,f3,f2','blr'],'ABI')
    def test_fma_multiplicands_commuted(self):self.check(['fmadds f1,f2,f3,f4','blr'],['fmadds f1,f3,f2,f4','blr'],'ABI')
    def test_fma_addend_not_commuted(self):self.check(['fmadds f1,f2,f3,f4','blr'],['fmadds f1,f4,f3,f2','blr'],'SEMANTIC')
    def test_reassociated(self):self.check(['fmuls f5,f2,f3','fmuls f1,f5,f4','blr'],['fmuls f5,f3,f4','fmuls f1,f2,f5','blr'],'SEMANTIC')
    def test_fused_vs_separate(self):self.check(['fmadds f1,f2,f3,f4','blr'],['fmuls f5,f2,f3','fadds f1,f5,f4','blr'],'SEMANTIC')
    def test_frsp_dropped(self):self.check(['frsp f1,f2','blr'],['fmr f1,f2','blr'],'SEMANTIC')
    def test_frsp_moved(self):self.check(['fmul f4,f2,f3','frsp f1,f4','blr'],['frsp f4,f2','fmul f1,f4,f3','blr'],'SEMANTIC')
    def test_conversion(self):self.check(['fctiwz f1,f2','blr'],['fctiw f1,f2','blr'],'SEMANTIC')
    def test_reciprocal(self):self.check(['fres f1,f2','blr'],['fdivs f1,f3,f2','blr'],'SEMANTIC')
    def test_float_events_ordered(self):self.check(['fadds f4,f2,f3','fmuls f1,f5,f6','blr'],['fmuls f1,f5,f6','fadds f4,f2,f3','blr'],'SEMANTIC')
    def test_field_offset(self):self.check(['lwz r3,0xec(r4)','blr'],['lwz r3,0xf0(r4)','blr'],'SEMANTIC','memory-offset')
    def test_width_signedness(self):self.check(['lha r3,8(r4)','blr'],['lhz r3,8(r4)','blr'],'SEMANTIC','memory-width-signedness')
    def test_compare_immediate(self):self.check(['cmpwi r3,1','blr'],['cmpwi r3,2','blr'],'SEMANTIC','compare-immediate-or-signedness')
    def test_repeated_unknown_reads_are_distinct(self):self.check(['lwz r3,0(r4)','lwz r5,0(r4)','subf r3,r3,r5','blr'],['lwz r5,0(r4)','lwz r3,0(r4)','subf r3,r3,r5','blr'],'SEMANTIC')
    def test_two_ordered_reads_regalloc_positive(self):self.check(['lwz r11,0(r4)','lwz r12,0(r4)','subf r3,r11,r12','blr'],['lwz r12,0(r4)','lwz r11,0(r4)','subf r3,r12,r11','blr'],'ABI')
    def test_missing_dead_load(self):self.check(['lwz r11,8(r3)','li r3,0','blr'],['li r3,0','blr'],'SEMANTIC','missing-load')
    def test_calls_require_binding(self):self.check(['bl getter','blr'],['bl getter','blr'],'SEMANTIC','unresolved-referent')
    def test_canonical_name_is_not_a_binding_by_itself(self):self.assertEqual(m.classify_lines(['bl CALLEE_0x8000','blr'],['bl CALLEE_0x8000','blr'])['classification'],'SEMANTIC')
    def test_changed_callee(self):self.check(['bl CALLEE_0x8001','blr'],['bl CALLEE_0x8002','blr'],'SEMANTIC','callee-or-call-order')
    def test_call_argument_order(self):self.check(['fmr f3,f1','fmr f1,f2','fmr f2,f3','bl CALLEE_0x8000','blr'],['bl CALLEE_0x8000','blr'],'SEMANTIC')
    def test_load_never_crosses_call(self):self.check(['lwz r31,8(r30)','bl CALLEE_0x8000','mr r3,r31','blr'],['bl CALLEE_0x8000','lwz r31,8(r30)','mr r3,r31','blr'],'SEMANTIC')
    def test_reload_after_call(self):self.check(['lwz r31,8(r30)','bl CALLEE_0x8000','lwz r31,8(r30)','mr r3,r31','blr'],['lwz r31,8(r30)','bl CALLEE_0x8000','mr r3,r31','blr'],'SEMANTIC','reload-after-call')
    def test_disjoint_store_schedule(self):self.check(['lwz r11,8(r30)','stw r4,16(r30)','mr r3,r11','blr'],['stw r4,16(r30)','lwz r11,8(r30)','mr r3,r11','blr'],'ABI',context={'memory_domain':'ordinary'})
    def test_overlapping_store_schedule(self):self.check(['lwz r11,8(r30)','stw r4,8(r30)','mr r3,r11','blr'],['stw r4,8(r30)','lwz r11,8(r30)','mr r3,r11','blr'],'SEMANTIC')
    def test_different_base_store_schedule(self):self.check(['lwz r11,8(r30)','stw r4,16(r31)','mr r3,r11','blr'],['stw r4,16(r31)','lwz r11,8(r30)','mr r3,r11','blr'],'SEMANTIC')
    def test_copy_of_stack_unknown(self):self.check(['mr r31,r1','lwz r3,8(r31)','blr'],['mr r31,r1','lwz r3,8(r31)','blr'],'SEMANTIC','stack-contract-unknown')
    def test_mixed_stack_width(self):self.check(['stw r3,8(r1)','lhz r3,8(r1)','blr'],['stw r3,8(r1)','lhz r3,8(r1)','blr'],'SEMANTIC','stack-contract-unknown')
    def test_stack_float_int_pun_unknown(self):self.check(['stfs f1,8(r1)','lwz r3,8(r1)','blr'],['stfs f1,8(r1)','lwz r3,8(r1)','blr'],'SEMANTIC','stack-contract-unknown')
    def test_stack_outparam_frame_unknown(self):self.check(['stwu r1,-32(r1)','addi r4,r1,8','bl CALLEE_0x8000','addi r1,r1,32','blr'],['stwu r1,-48(r1)','addi r4,r1,8','bl CALLEE_0x8000','addi r1,r1,48','blr'],'SEMANTIC','stack-contract-unknown')
    def test_typed_stack_slot_bijection(self):self.check(['stwu r1,-32(r1)','stw r3,8(r1)','lwz r3,8(r1)','addi r1,r1,32','blr'],['stwu r1,-48(r1)','stw r3,16(r1)','lwz r3,16(r1)','addi r1,r1,48','blr'],'ABI')
    def test_frame_shrunk_flag(self):
        r=self.check(['stwu r1,-48(r1)','stw r3,8(r1)','lwz r3,8(r1)','addi r1,r1,48','blr'],['stwu r1,-32(r1)','stw r3,8(r1)','lwz r3,8(r1)','addi r1,r1,32','blr'],'ABI')
        self.assertIn('frame-shrunk',r['tags'])
    def test_frame_padding_growth(self):self.check(['stwu r1,-16(r1)','li r3,1','addi r1,r1,16','blr'],['stwu r1,-32(r1)','li r3,1','addi r1,r1,32','blr'],'ABI')
    def test_frame_and_preserved_register_shift(self):self.check(['stwu r1,-32(r1)','stw r31,28(r1)','mr r31,r3','addi r3,r31,1','lwz r31,28(r1)','addi r1,r1,32','blr'],['stwu r1,-48(r1)','stw r30,44(r1)','mr r30,r3','addi r3,r30,1','lwz r30,44(r1)','addi r1,r1,48','blr'],'ABI')
    def test_stack_mapping_wrong_value(self):self.check(['stwu r1,-32(r1)','stw r3,8(r1)','lwz r3,8(r1)','addi r1,r1,32','blr'],['stwu r1,-48(r1)','stw r4,16(r1)','lwz r3,16(r1)','addi r1,r1,48','blr'],'SEMANTIC')
    def test_frame_slot_size_mismatch(self):self.check(['stwu r1,-32(r1)','stw r3,8(r1)','lwz r3,8(r1)','addi r1,r1,32','blr'],['stwu r1,-48(r1)','sth r3,8(r1)','lhz r3,8(r1)','addi r1,r1,48','blr'],'SEMANTIC','stack-contract-unknown')
    def test_stored_frame_address_escape(self):
        a=['stwu r1,-32(r1)','mflr r0','stw r0,36(r1)','stw r1,0(r3)','li r0,5','stw r0,8(r1)','bl CALLEE_0x8000','lwz r3,8(r1)','lwz r0,36(r1)','mtlr r0','addi r1,r1,32','blr']
        b=[s.replace('-32','-48').replace('36(r1)','52(r1)').replace('8(r1)','16(r1)').replace('r1,32','r1,48') for s in a]
        self.check(a,b,'SEMANTIC','stack-contract-unknown')
    def test_uninitialized_remapped_stack_read(self):self.check(['stwu r1,-32(r1)','lwz r3,8(r1)','addi r1,r1,32','blr'],['stwu r1,-48(r1)','lwz r3,16(r1)','addi r1,r1,48','blr'],'SEMANTIC','uninitialized-stack-read')
    def test_immediate_device_stores_never_reorder(self):self.check(['lis r3,0xcc00','sth r4,2(r3)','sth r5,4(r3)','blr'],['lis r3,0xcc00','sth r5,4(r3)','sth r4,2(r3)','blr'],'SEMANTIC',context={'memory_domain':'ordinary'})
    def test_indexed_device_store_origin_kept(self):self.check(['lis r6,0xcc00','add r6,r6,r3','sth r4,0(r6)','sth r5,2(r6)','blr'],['lis r6,0xcc00','add r6,r6,r3','sth r5,2(r6)','sth r4,0(r6)','blr'],'SEMANTIC',context={'memory_domain':'ordinary'})
    def test_strict_pointer_stores_never_reorder(self):self.check(['sth r4,2(r3)','sth r5,4(r3)','blr'],['sth r5,4(r3)','sth r4,2(r3)','blr'],'SEMANTIC',context={'memory_domain':'strict'})
    def test_volatile_global_pointer_taints_indexed_base(self):
        a=['lwz r6,REF_sbss_0x80700000@sda21','add r6,r6,r3','sth r4,0(r6)','sth r5,2(r6)','blr'];b=a[:2]+[a[3],a[2]]+a[4:]
        self.check(a,b,'SEMANTIC',context={'memory_domain':'ordinary','referents_bound':True,'strict_tokens':['REF_sbss_0x80700000']})
    def test_nonvolatile_global_pointer_disjoint_stores(self):
        a=['lwz r6,REF_sbss_0x80700000@sda21','sth r4,0(r6)','sth r5,2(r6)','blr'];b=[a[0],a[2],a[1],a[3]]
        self.check(a,b,'ABI',context={'memory_domain':'ordinary','referents_bound':True})
    def test_helper_index_shift_one_call(self):
        def body(frame,start):
            return [f'stwu r1,-{frame}(r1)','mflr r0',f'stw r0,{frame+4}(r1)',f'addi r11,r1,{frame}',f'bl _savegpr_{start}','mr r31,r3','lwz r3,4(r31)','bl CALLEE_0x8000',f'addi r11,r1,{frame}',f'bl _restgpr_{start}',f'lwz r0,{frame+4}(r1)','mtlr r0',f'addi r1,r1,{frame}','blr']
        r=self.check(body(48,27),body(32,28),'ABI',context={'referents_bound':True})
        self.assertIn('savegpr-shift',r['tags']);self.assertIn('validated-gpr-helper-expansion',r['tags'])
    def test_unsupported_opcode_reported(self):self.check(['foo r3,r4','blr'],['foo r3,r5','blr'],'SEMANTIC','unsupported-opcode:foo')
    def test_literal_value_subclass(self):self.check(['lfs f1,LIT_sdata2_3f800000@sda21','blr'],['lfs f1,LIT_sdata2_40000000@sda21','blr'],'SEMANTIC','literal-value',{'referents_bound':True})
    def test_unknown_domain_stores_never_reorder(self):self.check(['sth r4,2(r3)','sth r5,4(r3)','blr'],['sth r5,4(r3)','sth r4,2(r3)','blr'],'SEMANTIC')
    def test_frameless_incoming_slots_cannot_remap(self):self.check(['lwz r3,8(r1)','blr'],['lwz r3,12(r1)','blr'],'SEMANTIC','stack-contract-unknown')
    def test_member_addressing_cannot_hide_volatile_range(self):self.check(['addi r6,r3,8','sth r4,0(r6)','sth r5,4(r6)','blr'],['addi r6,r3,8','sth r5,4(r6)','sth r4,0(r6)','blr'],'SEMANTIC',context={'memory_domain':'ordinary','volatile_members':[{'offset':8,'width':2}]})
    def test_device_spill_preserves_origin(self):
        a=['lis r6,0xcc00','stw r6,8(r1)','lwz r7,8(r1)','sth r4,0(r7)','sth r5,2(r7)','blr'];b=a[:3]+[a[4],a[3]]+a[5:]
        self.check(a,b,'SEMANTIC',context={'memory_domain':'ordinary'})
    def test_device_partial_overlap_spill_preserves_origin(self):
        a=['lis r6,0xcc00','stw r6,8(r1)','lhz r7,10(r1)','sth r4,0(r7)','sth r5,2(r7)','blr'];b=a[:3]+[a[4],a[3]]+a[5:]
        self.check(a,b,'SEMANTIC',context={'memory_domain':'ordinary'})
    def test_helper_must_not_precede_entry_LR_save(self):
        normal=['stwu r1,-32(r1)','mflr r0','stw r0,36(r1)','addi r11,r1,32','bl _savegpr_29','li r3,1','addi r11,r1,32','bl _restgpr_29','lwz r0,36(r1)','mtlr r0','addi r1,r1,32','blr']
        bad=normal[:1]+normal[3:5]+normal[1:3]+normal[5:]
        self.check(normal,bad,'SEMANTIC',context={'referents_bound':True})
    def test_helper_delayed_setup_reaching_definition(self):
        def body(frame,start):
            return [f'stwu r1,-{frame}(r1)','mflr r0',f'stw r0,{frame+4}(r1)',f'addi r11,r1,{frame}',f'bl _savegpr_{start}','mr r31,r3','lwz r3,4(r31)','bl CALLEE_0x8000',f'addi r11,r1,{frame}','mr r3,r31',f'bl _restgpr_{start}',f'lwz r0,{frame+4}(r1)','mtlr r0',f'addi r1,r1,{frame}','blr']
        self.check(body(48,27),body(32,28),'ABI',context={'referents_bound':True})
    def test_helper_setup_clobbered_by_call(self):
        a=['stwu r1,-32(r1)','mflr r0','stw r0,36(r1)','addi r11,r1,32','bl _savegpr_29','li r3,1','addi r11,r1,32','bl _restgpr_29','lwz r0,36(r1)','mtlr r0','addi r1,r1,32','blr']
        b=a[:7]+['bl CALLEE_0x8000']+a[7:]
        self.check(a,b,'SEMANTIC',context={'referents_bound':True})
    def test_helper_setup_pointer_escape(self):
        a=['stwu r1,-32(r1)','mflr r0','stw r0,36(r1)','addi r11,r1,32','bl _savegpr_29','li r3,1','addi r11,r1,32','bl _restgpr_29','lwz r0,36(r1)','mtlr r0','addi r1,r1,32','blr']
        b=a[:7]+['stw r11,0(r3)']+a[7:]
        self.check(a,b,'SEMANTIC',context={'referents_bound':True})
    def test_helper_setup_wrong_frame_end(self):
        a=['stwu r1,-32(r1)','mflr r0','stw r0,36(r1)','addi r11,r1,32','bl _savegpr_29','li r3,1','addi r11,r1,32','bl _restgpr_29','lwz r0,36(r1)','mtlr r0','addi r1,r1,32','blr']
        self.check(a,[s.replace('r11,r1,32','r11,r1,28')for s in a],'SEMANTIC',context={'referents_bound':True})
    def test_zero_base_not_stack(self):self.check(['lwz r3,8(r0)','blr'],['lwz r3,12(r0)','blr'],'SEMANTIC')
    def test_sda_not_stack(self):self.check(['lwz r3,8(r13)','blr'],['lwz r3,12(r13)','blr'],'SEMANTIC','memory-offset')
    def test_inverse_wrong_successors(self):self.check(['cmpwi r3,0','beq 16','li r3,1','b 20','li r3,2','blr'],['cmpwi r3,0','bne 16','li r3,1','b 20','li r3,2','blr'],'SEMANTIC')
    def test_inverse_swapped_successors(self):self.check(['cmpwi r3,0','beq 16','li r3,1','b 20','li r3,2','blr'],['cmpwi r3,0','bne 16','li r3,2','b 20','li r3,1','blr'],'ABI')
    def test_signed_compare_inverse_not_equal(self):self.check(['cmpw r3,r4','blt 16','li r3,1','b 20','li r3,2','blr'],['cmplw r3,r4','bge 16','li r3,2','b 20','li r3,1','blr'],'SEMANTIC')
    def test_loop_unknown(self):self.check(['addi r3,r3,1','b 0'],['addi r3,r3,1','b 0'],'SEMANTIC')
    def test_unknown_opcode(self):self.check(['foo r3,r4','blr'],['foo r3,r4','blr'],'SEMANTIC')
    def test_literal_unknown_even_same(self):self.check(['lfs f1,@7@sda21','blr'],['lfs f1,@7@sda21','blr'],'SEMANTIC','unresolved-referent')
    def test_data_only_finding(self):self.check(['li r3,1','blr'],['li r3,1','blr'],'SEMANTIC','data',{'findings':[{'subclass':'data','target_offset':None,'source_offset':None,'reason':'different indexed payload'}]})
    def test_all_changes_retained(self):
        x=self.check(['lwz r3,8(r4)','lfs f1,12(r4)','cmpwi r3,2','blr'],['lwz r3,16(r4)','lfd f1,12(r4)','cmpwi r3,3','blr'],'SEMANTIC')
        self.assertIn('memory-offset',x['subclasses']);self.assertIn('memory-width-signedness',x['subclasses']);self.assertIn('compare-immediate-or-signedness',x['subclasses'])
    def test_partial_paired_write_rejected(self):self.check(['fmr f1,f2','psq_st f1,0(r3),0,qr0','blr'],['psq_st f2,0(r3),0,qr0','blr'],'SEMANTIC')

class ExtendedKernelTests(unittest.TestCase):
    check=KernelTests.check
    def test_extrwi_record_alias(self):
        self.check(['extrwi. r3,r4,7,2','blr'],['rlwinm. r3,r4,9,25,31','blr'],'ABI')
    def test_extrwi_changed_extraction(self):
        self.check(['extrwi. r3,r4,7,2','blr'],['extrwi. r3,r4,7,3','blr'],'SEMANTIC')
    def test_extrwi_record_is_observable(self):
        a=['extrwi. r3,r4,7,2','beq 16','li r3,1','b 20','li r3,2','blr']
        b=[a[0].replace('extrwi.','extrwi')]+a[1:]
        self.check(a,b,'SEMANTIC')
    def test_subi_addi_alias(self):
        self.check(['subi r3,r4,7','blr'],['addi r3,r4,-7','blr'],'ABI')
    def test_subi_ra_zero(self):
        self.check(['subi r3,r0,7','blr'],['addi r3,r0,-7','blr'],'ABI')
    def test_subi_wrong_sign(self):
        self.check(['subi r3,r4,7','blr'],['addi r3,r4,7','blr'],'SEMANTIC')
    def test_lwzx_register_rename(self):
        self.check(['lwzx r11,r3,r4','mr r3,r11','blr'],['lwzx r12,r3,r4','mr r3,r12','blr'],'ABI')
    def test_lwzx_wrong_index(self):
        self.check(['lwzx r3,r4,r5','blr'],['lwzx r3,r4,r6','blr'],'SEMANTIC')
    def test_lwzx_r0_index_is_live(self):
        self.check(['lwzx r3,r0,r5','blr'],['lwzx r3,r0,r0','blr'],'SEMANTIC')
    def test_lwzx_ra0_ignores_r0(self):
        a=['li r0,9','lwzx r3,r0,r5','blr'];b=['li r0,8','lwzx r3,r0,r5','blr']
        # Extra pure nodes are conservatively retained; directly inspect the EA
        # using different allocations of a dead incoming r0 value.
        self.assertEqual(m.decode(a[1])['use'],['r5'])
        self.check([a[1],'blr'],[a[1],'blr'],'ABI')
    def test_indexed_access_cannot_cross_potential_store_alias(self):
        a=['lwzx r11,r3,r4','stw r5,16(r3)','mr r3,r11','blr']
        b=[a[1],a[0]]+a[2:]
        self.check(a,b,'SEMANTIC',context={'memory_domain':'ordinary'})
    def test_indexed_device_origin_kept(self):
        a=['lis r6,0xcc00','lwzx r7,r6,r3','sth r4,0(r6)','blr'];b=[a[0],a[2],a[1],a[3]]
        self.check(a,b,'SEMANTIC',context={'memory_domain':'ordinary'})
    def test_bctrl_ctr_target_is_input(self):
        a=['stwu r1,-32(r1)','mflr r0','stw r0,36(r1)','mtctr r11','bctrl','lwz r0,36(r1)','mtlr r0','addi r1,r1,32','blr']
        b=[s.replace('mtctr r11','mtctr r12')for s in a]
        self.check(a,b,'SEMANTIC')
    def test_bctrl_same_call_regalloc(self):
        a=['stwu r1,-32(r1)','mflr r0','stw r0,36(r1)','mr r11,r3','mtctr r11','bctrl','lwz r0,36(r1)','mtlr r0','addi r1,r1,32','blr']
        b=[s.replace('r11','r12')for s in a]
        self.check(a,b,'ABI')
    def test_bctrl_argument_changes(self):
        a=['stwu r1,-32(r1)','mflr r0','stw r0,36(r1)','mtctr r12','li r3,1','bctrl','lwz r0,36(r1)','mtlr r0','addi r1,r1,32','blr']
        self.check(a,[s.replace('li r3,1','li r3,2')for s in a],'SEMANTIC')
    def test_bctrl_load_barrier(self):
        a=['stwu r1,-32(r1)','mflr r0','stw r0,36(r1)','mtctr r12','lwz r31,8(r30)','bctrl','mr r3,r31','lwz r0,36(r1)','mtlr r0','addi r1,r1,32','blr']
        b=a[:4]+[a[5],a[4]]+a[6:]
        self.check(a,b,'SEMANTIC')
    def test_updated_device_base_retains_origin(self):
        a=['lis r6,0xcc00','lwzu r7,4(r6)','sth r4,0(r6)','sth r5,2(r6)','blr'];b=a[:2]+[a[3],a[2]]+a[4:]
        self.check(a,b,'SEMANTIC',context={'memory_domain':'ordinary'})
    def test_update_strict_pointer_retains_origin(self):
        a=['lwz r6,REF_sbss_0x80700000@sda21','lwzu r7,4(r6)','sth r4,0(r6)','sth r5,2(r6)','blr'];b=a[:2]+[a[3],a[2]]+a[4:]
        self.check(a,b,'SEMANTIC',context={'memory_domain':'ordinary','referents_bound':True,'strict_tokens':['REF_sbss_0x80700000']})

class RealKnownBugs(unittest.TestCase):
    def test_eight_real_negative_rows(self):
        p=Path(os.environ.get('SEMCLASS_FIXTURES','/nonexistent'))/'fixtures.json'
        if not p.is_file():self.skipTest('external frozen real-bug fixture corpus not supplied')
        for c in json.loads(p.read_text())['cases']:
            with self.subTest(c['id']):
                # Raw formatted data references are NOT self-proving. Object audit
                # is separately mandatory, particularly Jet's identical text.
                r=m.classify_lines(c['target_lines'],c['source_lines'],c.get('context'))
                self.assertEqual(r['classification'],'SEMANTIC',r)
                self.assertTrue(c['full_function_rows'])

class CodeProofPolicyTests(unittest.TestCase):
    def test_scoped_and_ambiguous_bare_fail_closed(self):
        with tempfile.TemporaryDirectory() as td:
            p=Path(td)/'proofs.txt';p.write_text('@main/Game/A f # proof: full body contract\nf # proof: bare contract\n')
            a,ignored,pins=m.read_code_proofs([p],{'f':{'Game/A','Game/B'}},True)
            self.assertIn(('Game/A','f'),a);self.assertEqual(len(a),1);self.assertEqual(len(ignored),1)
    def test_unique_bare_requires_complete_census(self):
        with tempfile.TemporaryDirectory() as td:
            p=Path(td)/'proofs.txt';p.write_text('f # proof: whole function\n')
            self.assertFalse(m.read_code_proofs([p],{'f':{'Game/A'}},False)[0])
            self.assertIn(('Game/A','f'),m.read_code_proofs([p],{'f':{'Game/A'}},True)[0])
    def test_code_proof_cannot_remove_data_finding(self):
        rows=[{'unit':'Game/A','functions':[{'function':'f','code_selected':True,'classification':'SEMANTIC','code_classification':'SEMANTIC','data_complete':False,'findings':[{'subclass':'data'}]}]}]
        self.assertEqual(m.subtract_code_proofs(rows,{('Game/A','f'):[{'text':'audited code only'}]}),1)
        r=rows[0]['functions'][0];self.assertEqual(r['classification'],'SEMANTIC');self.assertEqual(r['classification_after_code_proofs'],'SEMANTIC');self.assertEqual(r['code_classification_after_proofs'],'EXEMPT');self.assertEqual(r['findings'][0]['subclass'],'data')

class PortableKnownBugGate(unittest.TestCase):
    def test_seven_bound_instruction_controls(self):
        fixture=Path(__file__).with_name('fixtures')/'semclass-known-bugs.json'
        cases=json.loads(fixture.read_text())['cases'];self.assertEqual(len(cases),8)
        for c in cases:
            if c.get('kind')=='object-backed-portable-semdata':continue
            with self.subTest(c['id']):
                r=m.classify_lines(c['target'],c['source'],c['context'])
                self.assertEqual(r['classification'],'SEMANTIC',r)
                self.assertFalse(any(f['subclass']=='unresolved-referent' for f in r['findings']),r)
    def test_Jet_four_wrong_slots_with_identical_indexed_text(self):
        import test_semdata
        c=test_semdata.DataGateTests('test_portable_historical_Jet_four_values_identical_code_bound_DOL')
        try:
            c.test_portable_historical_Jet_four_values_identical_code_bound_DOL()
            fixture=json.loads((Path(__file__).with_name('fixtures')/'semclass-known-bugs.json').read_text())['cases'][-1]
            correct=bytes.fromhex(fixture['literal_facts']['target_region_hex'])
            lfsx=31<<26|1<<21|3<<16|4<<11|535<<1
            c.setup_world(section='.rodata',target=correct,source=correct,target_objects=(('A',0,48),),target_word=0x38600000,instructions=(0x38600000,lfsx,0x4e800020))
            result=c.audit()['functions']['f']
            self.assertFalse(any(v['reason']=='different_data_bytes' for v in result['findings']))
            self.assertFalse(result['complete']) # unknown index range remains open
        finally:c.doCleanups()

if __name__=='__main__':unittest.main()
