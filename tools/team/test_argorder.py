import unittest
import hashlib,struct,tempfile
from pathlib import Path
from argorder import scan_text, lex, pairs, templates, resolve_order, symbol_owners

class LexicalTests(unittest.TestCase):
    def rows(self,s):return scan_text(s)['rows']
    def test_mixed_jpa_rng(self):
        r=self.rows('void JPA::init(){ JPAGetYZRotateMtx(e->get_r_zp()*s, e->get_r_ss(), m); }')[0]
        self.assertEqual(r['enclosing_function'],'JPA::init');self.assertEqual(r['effectful_arguments'],2)
        self.assertEqual([a['call_names']for a in r['arguments']],[['get_r_zp'],['get_r_ss'],[]])
    def test_triple_constructor(self):
        r=self.rows('void f(){ JGeometry::TVec3<float> v(rng.next(), rng.next(), rng.next()); }')[0]
        self.assertTrue(r['constructor']);self.assertEqual(r['constructor_type_hint'],'TVec3')
        self.assertEqual([a['call_count']for a in r['arguments']],[1,1,1])
    def test_braced_constructor(self):
        r=self.rows('void f(){ Vec v{a(), b(), c()}; }')[0]
        self.assertTrue(r['constructor']);self.assertEqual(len(r['arguments']),3)
    def test_nested_only_one_arg(self):
        self.assertEqual(self.rows('void f(){ outer(wrap(a(), b()), 0); }')[0]['callee'],'wrap')
        self.assertEqual(len(self.rows('void f(){ outer(wrap(a(), b()), 0); }')),1)
    def test_nested_two_args_counts(self):
        rs=self.rows('void f(){ outer(wrap(a()), b()); }');r=next(r for r in rs if r['callee']=='outer')
        self.assertEqual([a['call_count']for a in r['arguments']],[2,1])
    def test_opaque_literals_comments(self):
        text='void f(){ out(/*fake(x(),y())*/ a("bad(), x++"), b(R"xyz(c(), d++)xyz")); }'
        r=self.rows(text)[0];self.assertEqual([a['call_names']for a in r['arguments']],[['a'],['b']])
        self.assertEqual(r['effect_count'],2)
    def test_macro_definition_and_invocation(self):
        rs=self.rows('#define BOTH(a,b) a \\\n + b\nvoid f(){ BOTH(a(),b()); }')
        self.assertEqual(len(rs),1);self.assertIn('macro',rs[0]['uncertainty'][0])
    def test_mutations_not_comparisons(self):
        self.assertFalse(self.rows('void f(){ out(x==y, z>=v, t!=u, a<=b); }'))
        r=self.rows('void f(){ out(++x, y--, z+=3, a=b); }')[0]
        self.assertEqual([a['mutation_ops']for a in r['arguments']],[['++'],['--'],['+='],['=']])
    def test_nested_template_commas(self):
        r=self.rows('void f(){ obj.set<Thing<A,B>, C>(a(), other<X,Y>(b(), c())); }')[0]
        self.assertEqual(r['callee'],'set');self.assertEqual(len(r['arguments']),2)
        self.assertEqual(r['arguments'][1]['call_count'],3)
    def test_shift_not_template(self):
        r=self.rows('void f(){ out(a()<<2, b()>>1); }')[0];self.assertEqual(len(r['arguments']),2)
    def test_comparison_not_template(self):
        r=self.rows('void f(){ out(a() < x, b() > y); }')[0];self.assertEqual(len(r['arguments']),2)
    def test_function_scope_control_lambda(self):
        r=self.rows('void X::go(){if(x){ out(a(),b()); } auto l=[](){ out(c(),d()); }; }')
        self.assertEqual(len(r),2);self.assertTrue(all(x['enclosing_function']=='X::go'for x in r))
    def test_prototypes_not_invocations(self):
        self.assertFalse(self.rows('void f(int x=a(), int y=b());'))
    def test_constructor_initializer(self):
        rs=self.rows('A::A(): member(a(), b()) { out(c(), d()); }')
        self.assertEqual({r['callee']for r in rs},{'member','out'})
        self.assertTrue(all(r['enclosing_function']=='A::A'for r in rs))
    def test_initializer_braces(self):
        rs=self.rows('A::A(): member{a(), b()} { out(c(), d()); }')
        self.assertEqual({r['callee']for r in rs},{'member','out'})
    def test_unbalanced_closed_fail(self):
        r=scan_text('void f(){ out(a(),b()); bad(')
        self.assertTrue(r['diagnostics']);self.assertFalse(r['rows'])
    def test_lines_raw_and_escaped(self):
        r=self.rows('void f(){\n // blah\n out(a("\\\""),b(\'x\'));\n}')[0];self.assertEqual(r['line'],3)
    def test_assignment_inside_braces_one_argument(self):
        r=self.rows('void f(){ out(Vec{a(), b()}, z++); }')[0]
        self.assertEqual(len(r['arguments']),2);self.assertEqual(r['arguments'][1]['mutation_count'],1)
    def test_indirect_parenthesized(self):
        r=self.rows('void f(){ (*fp)(a(), b()); }')[0]
        self.assertEqual(r['callee'],'(*fp)');self.assertIn('indirect',r['uncertainty'][0])
    def test_indirect_indexed(self):
        r=self.rows('void f(){ handlers[i](a(), b()); }')[0]
        self.assertEqual(r['effectful_arguments'],2);self.assertIn('indirect',r['uncertainty'][0])
    def test_nested_indirect(self):
        r=self.rows('void f(){ out((*fp)(), b()); }')[0]
        self.assertEqual(r['arguments'][0]['calls'][0]['indirect'],True)
    def test_spliced_line_comment(self):
        self.assertFalse(self.rows('void f(){ // splice\\\n out(a(),b());\n }'))
    def test_header_macro_names(self):
        r=scan_text('void f(){ lower(a(),b()); }',macro_names=['lower'])['rows'][0]
        self.assertIn('macro',r['uncertainty'][0])
    def test_unevaluated_uncertain(self):
        r=self.rows('void f(){ out(sizeof(a()),b()); }')[0]
        self.assertTrue(any('unevaluated'in x for x in r['uncertainty']))
    def test_deferred_lambda_uncertain(self):
        r=self.rows('void f(){ out([&]{ a(); },b()); }')[0]
        self.assertTrue(any('lambda'in x for x in r['uncertainty']))
    def test_new_constructor(self):
        r=self.rows('void f(){ auto p=new Vec(a(),b(),c()); }')[0]
        self.assertTrue(r['constructor']);self.assertEqual(r['constructor_type_hint'],'Vec')
    def test_namespace_scope_decoding(self):
        self.assertEqual(symbol_owners('calc__14JPAFieldRandomFv'),['JPAFieldRandom'])
        self.assertEqual(symbol_owners('f__Q23Foo3BarFv'),['Foo','Bar'])

class BinaryAssociationTests(unittest.TestCase):
    def row(self,text='void f(){ out(a(), b()); }'):return scan_text(text)['rows'][0]
    def call(self,n,off,dst=0x80001000):
        return {'callee':n,'offset':off,'addend':0,'mapped_destination_hint':hex(dst),'actual_DOL_destination':hex(dst)}
    def orders(self,row,source,target,sw=None,tw=None):
        return resolve_order(row,source,target,sw or [0x48000001]*8,tw or [0x48000001]*8)
    def test_verified_reverse_calls_and_addresses(self):
        sc=[self.call('b',0),self.call('a',4,0x80002000),self.call('out',8)]
        tc=[self.call('a',0,0x80002000),self.call('b',4),self.call('out',8)]
        r=self.orders(self.row(),sc,tc);self.assertEqual(r['status'],'resolved-direct-effect-call-order')
        self.assertFalse(r['argument_call_order_equal']);self.assertEqual([e['argument']for e in r['source']['events']],[1,0])
    def test_repeated_unknown(self):
        r=self.orders(self.row('void f(){ out(a(),a()); }'),[],[])
        self.assertEqual(r['status'],'unknown');self.assertIn('repeated',r['reason'])
    def test_inline_missing_unknown(self):
        cs=[self.call('a',0),self.call('out',8)]
        r=self.orders(self.row(),cs,cs);self.assertIn('inline/indirect',r['reason'])
    def test_indirect_unknown(self):
        cs=[self.call('a',0),self.call('b',4),self.call('out',12)]
        r=self.orders(self.row(),cs,cs,[0x48000001,0x48000001,0x4e800421,0x48000001])
        self.assertEqual(r['status'],'unknown');self.assertIn('control flow',r['reason'])
    def test_branch_unknown(self):
        cs=[self.call('a',0),self.call('b',8),self.call('out',12)]
        self.assertEqual(self.orders(self.row(),cs,cs,[0x48000001,0x41820008,0x48000001,0x48000001])['status'],'unknown')
    def test_mutation_unknown(self):
        r=self.orders(self.row('void f(){ out(a(),b++); }'),[],[]);self.assertIn('mutation',r['reason'])
    def test_source_occurrence_elsewhere_unknown(self):
        r=self.orders(self.row('void f(){ a(); out(a(),b()); }'),[],[]);self.assertIn('elsewhere',r['reason'])
    def test_overloaded_symbols_unknown(self):
        cs=[self.call('a__1XFv',0),self.call('a__1YFv',4),self.call('b',8),self.call('out',12)]
        r=self.orders(self.row(),cs,cs);self.assertEqual(r['status'],'unknown')
    def test_callee_addend_not_guessed(self):
        cs=[self.call('a',0),self.call('b',4),self.call('out',8)];cs[0]['addend']=4
        self.assertEqual(self.orders(self.row(),cs,cs)['status'],'unknown')
    def test_no_address_unknown(self):
        cs=[self.call('a',0),self.call('b',4),self.call('out',8)];cs[0].pop('mapped_destination_hint');cs[0].pop('actual_DOL_destination')
        self.assertIn('address unresolved',self.orders(self.row(),cs,cs)['reason'])

class ActualDolBindingTests(unittest.TestCase):
    def run_case(self,corrupt_dol=False,wrong_identity=False,stale=False):
        import callaudit as ca
        from argorder import BinaryAudit
        with tempfile.TemporaryDirectory()as td:
            root=Path(td);symbols=root/'symbols.txt'
            symbols.write_text('f = .text:0x80000000; // type:function size:0x10\n'
                               'a = .text:0x80000100; // type:function size:0x4\n'
                               'b = .text:0x80000104; // type:function size:0x4\n'
                               'out = .text:0x80000108; // type:function size:0x4\n')
            cat=ca.catalog(symbols)
            class Fake:
                def __init__(self,names):
                    self.f={'f':('f',0,16,1)};self.s=[(0,0),(0,1)]
                    self.c=[b'',struct.pack('>4I',0x48000001,0x48000001,0x48000001,0x4e800020)]
                    self.raw=self.c[1];self.rel={(1,k*4):(10,(n,0,0,0),0)for k,n in enumerate(names)}
            source,target=Fake(['b','a','out']),Fake(['a','b','out'])
            header=bytearray(256);struct.pack_into('>I',header,0,256);struct.pack_into('>I',header,0x48,0x80000000);struct.pack_into('>I',header,0x90,16)
            words=[0x48000101]*3+[0x4e800020]
            if corrupt_dol:words[0]=0x60000000
            if wrong_identity:words[0]=0x48000105
            dol=bytes(header)+struct.pack('>4I',*words)
            pins={}
            for side,e in [('source',source),('target',target)]:
                p=root/(side+'.o');p.write_bytes(e.raw);pins[side+'_object']=str(p);pins[side+'_object_sha256']=hashlib.sha256(e.raw).hexdigest()
            pins['source_object_mtime_ns']=1
            audit=object.__new__(BinaryAudit);audit.ca=ca;audit.cat=cat;audit.dol=dol
            audit.addresses={n:d['address']for n,d in cat.items()};audit.duplicate_addresses=set()
            audit.unit=lambda u:((source,target),pins,{})
            row=scan_text('void f(){ out(a(),b()); }')['rows'][0];row['unit']='fixture'
            if stale:row['source_mtime_ns']=2
            audit.inspect(row);return row
    def test_original_addresses_bound_and_reverse_order(self):
        r=self.run_case();o=r['argument_order'];self.assertEqual(o['status'],'resolved-direct-effect-call-order')
        self.assertEqual([e['argument']for e in o['source']['events']],[1,0])
        self.assertEqual([e['argument']for e in o['target']['events']],[0,1])
        self.assertEqual(o['target']['events'][0]['callee_address'],'0x80000100')
    def test_original_opcode_mismatch_unknown(self):
        r=self.run_case(corrupt_dol=True);self.assertEqual(r['argument_order']['status'],'unknown')
    def test_actual_callee_mismatch_unknown(self):
        r=self.run_case(wrong_identity=True);self.assertEqual(r['argument_order']['status'],'unknown')
        self.assertIn('unresolved',r['argument_order']['reason'])
    def test_stale_source_unknown(self):
        r=self.run_case(stale=True);self.assertEqual(r['argument_order']['status'],'unknown')
        self.assertIn('newer',r['argument_order']['reason'])

if __name__=='__main__':unittest.main()
