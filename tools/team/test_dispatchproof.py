#!/usr/bin/env python3
"""Real ELF negative controls. No compiler needed; ~5 minutes on the full tree.

Run: python3 tools/team/test_dispatchproof.py --root .
Pass --pointwarp-after TREE for the positive PointWarp control, using an isolated
symbols.txt containing only its two address corrections (no live W1 config).
"""
import argparse
import collections
import copy
import importlib.util
import pathlib
import shutil
import struct
import sys
import tempfile
import unittest

import dispatchproof as dp
import dispatchcheck as dc

ROOT = pathlib.Path(__file__).resolve().parents[2]
DOL = None
POINT = None
UNIT = 'Game/System/WorldMapResultTU19'


def put32(raw, offset, value):
    struct.pack_into('>I', raw, offset, value)


def section_header(raw, idx):
    return dc.u32(raw, 32) + idx * struct.unpack_from('>H', raw, 46)[0]


class ProofControls(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = pathlib.Path(tempfile.mkdtemp(prefix='R-dispatchproof-'))
        cls.tree = cls.tmp / 'after'
        cls.tree.mkdir()
        shutil.copytree(ROOT / 'config', cls.tree / 'config')
        (cls.tree / 'orig').symlink_to(ROOT / 'orig')
        base = ROOT / 'build/SB4E01/src'
        for p in base.rglob('*.o'):
            q = cls.tree / 'build/SB4E01/src' / p.relative_to(base)
            q.parent.mkdir(parents=True, exist_ok=True)
            q.symlink_to(p.resolve())
        cls.pb = dc.Prog(ROOT, DOL)
        cls.rows, _ = cls.pb.run()
        assert len(cls.pb.elfs) > 2000
        cls.e = cls.pb.elfs[UNIT]
        cls.obj = cls.tree / 'build/SB4E01/src' / (UNIT + '.o')
        cls.symbols = (cls.tree / 'config/SB4E01/symbols.txt').read_text()
        cls.pointwarp_raw = (POINT/'build/SB4E01/src'/(UNIT+'.o')).read_bytes() if POINT else None
        cls.symtab = next(s for s in cls.e.s if s[1] == 2)
        cls.text = next(i for i,n in enumerate(cls.e.names) if n == '.text')
        cls.data = cls.e.names.index('.data')
        cls.named = next(i for i,s in enumerate(cls.e.syms) if s['type'] == 2 and s['size'])
        cls.rels = [(s, s[4]+o) for s in cls.e.s if s[1] == 4 and not dp.ignored(cls.e,s[7]) for o in range(0,s[5],12)]
        print(f'CONTROL BASELINE: {len(cls.pb.elfs)} real objects, {len(cls.rows)} oracle rows', flush=True)

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp)

    def setUp(self):
        if self.obj.exists() or self.obj.is_symlink():
            self.obj.unlink()
        self.obj.write_bytes(self.e.raw)
        (self.tree / 'config/SB4E01/symbols.txt').write_text(self.symbols)

    def load_tree(self):
        # Reuse immutable parsed ELFs for unchanged symlinks, while reading the
        # real inventory and parsing every replaced/extra object. This avoids
        # duplicating several million DWARF symbols per control.
        pm=copy.copy(self.pb);pm.root=self.tree
        dc.original_dol(self.tree,DOL or str(ROOT/'orig/SB4E01/sys/main.dol'))
        pm.sy=dc.Symbols(self.tree/'config/SB4E01/symbols.txt')
        pm.bases,pm.ends=dc.load_splits(self.tree/'config/SB4E01/splits.txt')
        pm.elfs={};pm.errors=[]
        for unit,path in dp.tree_objects(self.tree).items():
            try:
                raw=path.read_bytes();old=self.pb.elfs.get(unit)
                pm.elfs[unit]=old if old is not None and raw==old.raw else dc.ELF(path)
            except Exception as ex:pm.errors.append((unit,str(ex)))
        pm.inferred=collections.defaultdict(lambda:collections.defaultdict(list))
        pm.func_info={};pm.exact_stream=[];pm.nonexact=[]
        pm._chains=None;pm.ctor_inst={}
        for name in ('_proof_addresses','_proof_named','_proof_storage_bases','_proof_chain_addresses','_proof_allowed_old'):
            if hasattr(pm,name):delattr(pm,name)
        return pm

    def check(self, raw=None, accept=False, run_oracle=True):
        if raw is not None:
            self.obj.write_bytes(raw)
        pm=self.load_tree()
        # Full oracle run, not mocked verdicts: this also tests disappearing rows.
        rows, _ = pm.run() if run_oracle else (self.rows, {})
        problems, report, flips = dp.compare_trees(self.pb, pm, self.rows, rows, {UNIT}, accept and self.id().endswith('pointwarp_positive'))
        if accept and self.id().endswith('pointwarp_positive'):
            print('POINTWARP REPORT', report, flush=True)
            for row in flips[-4:]:print('FLIPPED', row, flush=True)
            print('DISPATCH PROOF FAILED' if problems else 'DISPATCH PROOF OK', flush=True)
        print(f'{self.id().split(".")[-1]}: {"REJECTED" if problems else "ACCEPTED"}; {problems[:1]}', flush=True)
        self.assertEqual(not problems, accept)

    def test_00_identical(self):
        self.check(accept=True)

    def test_code_byte(self):
        raw = bytearray(self.e.raw); raw[self.e.s[self.text][4]+8] ^= 1
        self.check(raw)

    def test_data_byte(self):
        raw = bytearray(self.e.raw); raw[self.e.s[self.data][4]] ^= 1
        self.check(raw)

    def test_uncovered_padding_byte(self):
        # Padding lies outside every sized function; old unit_image omitted it.
        covered = {o for s in self.e.syms if s['sec']==self.text and s['size'] and s['type'] in (1,2)
                   for o in range(s['off'],s['off']+s['size'])}
        off = next(o for o in range(self.e.s[self.text][5]) if o not in covered)
        raw=bytearray(self.e.raw);raw[self.e.s[self.text][4]+off]^=1
        self.check(raw)

    def test_section_size(self):
        raw=bytearray(self.e.raw);i=self.e.names.index('.sbss')
        put32(raw,section_header(raw,i)+20,self.e.s[i][5]+4)
        self.check(raw)

    def test_removed_duplicate_padding_byte(self):
        import copy
        u='Game/Demo/DemoSoundKeeper';fixture=ROOT.parent/'dispatch-misc/build/SB4E01/src'/f'{u}.o'
        if not fixture.exists():self.skipTest('stable dispatch-misc fixture not available')
        afterraw=fixture.read_bytes();original=self.pb.elfs[u]
        cfg=self.tmp/'demo-symbols.txt'
        cfg.write_text(self.symbols.replace('start__15DemoSoundKeeperFv = .text:0x80139FA0;',
            'executeType__42DemoSheetKeeperInfoHolder<13DemoSoundInfo>FPC13DemoSoundInfo = .text:0x80139FA0;'))
        for dirty in (False,True):
            raw=bytearray(original.raw)
            if dirty:raw[original.s[1][4]+4]^=1
            path=self.tmp/'demo-before.o';path.write_bytes(raw)
            before=copy.copy(self.pb);before.elfs=dict(self.pb.elfs);before.elfs[u]=dc.ELF(path)
            path=self.tmp/'demo-after.o';path.write_bytes(afterraw)
            after=copy.copy(self.pb);after.elfs=dict(self.pb.elfs);after.elfs[u]=dc.ELF(path);after.sy=dc.Symbols(cfg)
            dp.prepare_addresses(before,after)
            ib,ia=dp.unit_image(before,u),dp.unit_image(after,u)
            pk=('.text',('P',0x80139fa4),12)
            permissions=dp.emission_permissions(before,after,u,ib,ia,{(0x80139fa0,4)})
            self.assertEqual(pk in permissions,not dirty)
            if dirty:
                problems=[]
                self.assertIsNone(dp.compare_units(u,ib,ia,set(),problems,permissions))
                self.assertIn('image keys differ',problems[0])
        print('removed duplicate with changed DOL padding: REJECTED',flush=True)

    def test_new_conflicting_weak_variant(self):
        sym=next(s for s in self.e.syms if s['bind']==2 and s['type']==2 and s['size'] and self.pb.sym_addr(self.e,UNIT,s) is not None)
        raw=bytearray(self.e.raw);raw[self.e.s[sym['sec']][4]+sym['off']]^=1
        self.check(raw)

    def test_undefined_symbol_binding(self):
        raw=bytearray(self.e.raw)
        used={si for sec,o,t,si,ad in self.e.relocs if not dp.ignored(self.e,sec)}
        idx=next(i for i,s in enumerate(self.e.syms) if i in used and s['sec']==0 and s['bind']==1 and self.pb.sy.addr(s['name']) is not None)
        off=self.symtab[4]+idx*16+12;raw[off]=(2<<4)|(raw[off]&15)
        self.check(raw)

    def test_undefined_symbol_type(self):
        raw=bytearray(self.e.raw)
        used={si for sec,o,t,si,ad in self.e.relocs if not dp.ignored(self.e,sec)}
        idx=next(i for i,s in enumerate(self.e.syms) if i in used and s['sec']==0 and s['type']==0 and self.pb.sy.addr(s['name']) is not None)
        off=self.symtab[4]+idx*16+12;raw[off]=(raw[off]&240)|2
        self.check(raw)

    def test_symbol_coverage_loss(self):
        raw=bytearray(self.e.raw)
        off=self.symtab[4]+self.named*16
        put32(raw,off+4,0);put32(raw,off+8,0)
        struct.pack_into('>H',raw,off+14,0)
        self.check(raw)

    def test_rename_to_other_address(self):
        sym=self.e.syms[self.named];old=self.pb.sy.addr(sym['name']);self.assertIsNotNone(old)
        lines=self.symbols.splitlines(True)
        import re
        for i,line in enumerate(lines):
            if line.startswith(sym['name']+' ='):
                lines[i]=re.sub(r':0x[0-9A-Fa-f]+;',':0x%08X;'%(old+4),line,count=1)
                break
        (self.tree/'config/SB4E01/symbols.txt').write_text(''.join(lines))
        self.check()

    def test_allocated_debug_section(self):
        raw=bytearray(self.e.raw);idx=self.e.names.index('.debug')
        put32(raw,section_header(raw,idx)+8,self.e.s[idx][2]|2)
        self.check(raw)

    def test_relocation_section_metadata(self):
        raw=bytearray(self.e.raw);idx=next(i for i,s in enumerate(self.e.s) if s[1]==4)
        put32(raw,section_header(raw,idx)+32,self.e.s[idx][8]*2)
        self.check(raw)

    def test_symbol_size(self):
        raw=bytearray(self.e.raw);s=self.e.syms[self.named]
        put32(raw,self.symtab[4]+self.named*16+8,s['size']+4)
        self.check(raw)

    def test_symbol_add(self):
        # Append one valid symbol record; relocate only the symtab's file offset.
        raw=bytearray(self.e.raw);start=len(raw)
        raw.extend(self.e.raw[self.symtab[4]:self.symtab[4]+self.symtab[5]])
        raw.extend(self.e.raw[self.symtab[4]+self.named*16:self.symtab[4]+(self.named+1)*16])
        i=next(i for i,s in enumerate(self.e.s) if s[1]==2)
        put32(raw,section_header(raw,i)+16,start);put32(raw,section_header(raw,i)+20,self.symtab[5]+16)
        self.check(raw)

    def test_symbol_remove(self):
        # Remove an unused STT_FILE record and repair all relocation indices.
        used={si for _,_,_,si,_ in self.e.relocs}
        idx=next(i for i,s in enumerate(self.e.syms) if i and i not in used and s['name'] and s['sec']>=len(self.e.s))
        raw=bytearray(self.e.raw);start=len(raw)
        table=self.e.raw[self.symtab[4]:self.symtab[4]+self.symtab[5]]
        raw.extend(table[:idx*16]+table[(idx+1)*16:])
        stidx=next(i for i,s in enumerate(self.e.s) if s[1]==2)
        put32(raw,section_header(raw,stidx)+16,start);put32(raw,section_header(raw,stidx)+20,self.symtab[5]-16)
        if idx<self.symtab[7]:put32(raw,section_header(raw,stidx)+28,self.symtab[7]-1)
        for sh in self.e.s:
            if sh[1]!=4:continue
            for loc in range(sh[4],sh[4]+sh[5],12):
                info=dc.u32(raw,loc+4)
                if info>>8>idx:put32(raw,loc+4,info-256)
        self.check(raw)

    def test_symbol_rename(self):
        raw=bytearray(self.e.raw);st=self.e.s[self.symtab[6]]
        idx=dc.u32(raw,self.symtab[4]+self.named*16)
        raw[st[4]+idx]=ord('Z')
        self.check(raw)

    def retarget(self, typ, prefix=None):
        rows=[r for r in self.rows if r['unit']==UNIT and r['layer'] in ('code','data') and r['verdict']=='MATCH'
              and (prefix is None or r.get('target','').startswith(prefix))]
        for sh,loc in self.rels:
            off,info,ad=struct.unpack_from('>IIi',self.e.raw,loc)
            if info&255!=typ:continue
            target=self.e.syms[info>>8]
            if not any(r.get('target')==target['name'] and (r['offset']==off or r['offset']==off-next((s['off'] for s in self.e.syms if s['name']==r.get('function')),0)) for r in rows):continue
            alt=next(i for i,s in enumerate(self.e.syms) if s['name']!=target['name'] and s['type']==target['type'] and self.pb.sym_addr(self.e,UNIT,s) not in (None,self.pb.sym_addr(self.e,UNIT,target)))
            raw=bytearray(self.e.raw);put32(raw,loc+4,(alt<<8)|typ)
            return raw
        self.fail('no MATCH relocation control candidate')

    def test_match_data_retarget(self):self.check(self.retarget(dc.R_ADDR32))
    def test_match_code_branch_retarget(self):self.check(self.retarget(dc.R_REL24))
    def test_match_instance_retarget(self):self.check(self.retarget(dc.R_SDA21,'sInstance__'))

    def test_vtable_swap(self):
        candidates=[]
        for sh,loc in self.rels:
            off,info,ad=struct.unpack_from('>IIi',self.e.raw,loc)
            if sh[7]==self.data and info&255==dc.R_ADDR32 and self.e.syms[info>>8]['name'].startswith('execute__'):
                # Select slots that are currently MATCH, rather than PointWarp's BAD slots.
                if any(r['layer']=='data' and r['unit']==UNIT and r['offset']==off and r['verdict']=='MATCH' for r in self.rows):candidates.append(loc)
        # PointWarp has only one matching nerve slot; use ordinary virtual slots.
        if len(candidates)<2:
            candidates=[loc for sh,loc in self.rels if sh[7]==self.data and dc.u32(self.e.raw,loc+4)&255==dc.R_ADDR32
                        and any(r['layer']=='data' and r['unit']==UNIT and r['offset']==dc.u32(self.e.raw,loc) and r['verdict']=='MATCH' for r in self.rows)]
        first=candidates[0];second=next(loc for loc in candidates if dc.u32(self.e.raw,loc+4)!=dc.u32(self.e.raw,first+4))
        raw=bytearray(self.e.raw);x=raw[first+4:first+12];raw[first+4:first+12]=raw[second+4:second+12];raw[second+4:second+12]=x
        self.check(raw)

    def pointwarp(self):
        if self.pointwarp_raw is None:self.skipTest('pass --pointwarp-after TREE for the compiled fixed-pair control')
        raw=self.pointwarp_raw
        # Only the two execute names change addresses in de4289f1.
        text=self.symbols
        names=[s['name'] for s in self.e.syms if s['name'].startswith('execute__Q231NrvWorldMapResultStatePointWarp') and any(v in s['name'] for v in ('WarpStart','WarpEnd'))]
        self.assertEqual(len(names),2)
        lines=text.splitlines(True);idx=[i for i,l in enumerate(lines) if any(l.startswith(n+' =') for n in names)]
        suffix=[lines[i].split(' =',1)[1] for i in idx]
        for i,v in zip(idx,reversed(suffix)):lines[i]=lines[i].split(' =',1)[0]+' ='+v
        (self.tree/'config/SB4E01/symbols.txt').write_text(''.join(lines))
        return bytearray(raw)

    def test_pointwarp_positive(self):self.check(self.pointwarp(),accept=True)

    def test_fix_plus_unrelated_change(self):
        raw=self.pointwarp();raw[self.e.s[self.data][4]]^=1
        self.check(raw)

    def test_missing_object(self):
        self.obj.unlink()
        self.check()
        self.obj.write_bytes(self.e.raw)

    def test_extra_object(self):
        extra=self.obj.with_name('R_extra.o');extra.write_bytes(self.e.raw)
        try:self.check()
        finally:extra.unlink()

    def test_unreadable_object(self):self.check(b'not an ELF')

    def test_section_symbol_target(self):
        # STT_SECTION addends cannot disappear into a section-only identity.
        raw=bytearray(self.e.raw)
        loc=next(loc for sh,loc in self.rels if self.e.names[sh[7]]=='.ctors')
        old=self.e.syms[dc.u32(raw,loc+4)>>8]
        idx=next(i for i,s in enumerate(self.e.syms) if s['type']==3 and s['sec']==old['sec'])
        put32(raw,loc+4,(idx<<8)|dc.R_ADDR32)
        put32(raw,loc+8,old['off']+4)
        self.check(raw)

    def mismatched_permission(self, other_unit=False):
        # Exercise the comparator with real images, but permission for another
        # container of the same numerical address (or another object's site).
        import copy
        raw=self.retarget(dc.R_ADDR32);self.obj.write_bytes(raw)
        pm=copy.copy(self.pb);pm.elfs=dict(self.pb.elfs);pm.elfs[UNIT]=dc.ELF(self.obj)
        ib,ia=dp.unit_image(self.pb,UNIT),dp.unit_image(pm,UNIT)
        k=next(k for k in ib if ib[k][1]!=ia[k][1])
        old=next(iter(ib[k][1]-ia[k][1]));new=next(iter(ia[k][1]-ib[k][1]))
        o,typ,ad,target=old;site=k[1][1]+o
        wrongk=(k[0],k[1],k[2]+4)
        permission=(UNIT+'/other' if other_unit else UNIT,k if other_unit else wrongk,o,typ,ad,site,target,new[3])
        problems=[]
        n=dp.compare_units(UNIT,ib,ia,{permission},problems)
        self.assertIsNone(n);self.assertTrue(problems)
        print(self.id().split('.')[-1]+': REJECTED',flush=True)

    def test_same_address_other_container_rejected(self):self.mismatched_permission()
    def test_same_address_other_unit_rejected(self):self.mismatched_permission(True)

    def test_duplicate_relocation(self):
        raw=bytearray(self.e.raw);sh,loc=self.rels[0];start=len(raw)
        raw.extend(self.e.raw[sh[4]:sh[4]+sh[5]]);raw.extend(self.e.raw[loc:loc+12])
        idx=self.e.s.index(sh);put32(raw,section_header(raw,idx)+16,start);put32(raw,section_header(raw,idx)+20,sh[5]+12)
        self.check(raw)


class SinitControls(unittest.TestCase):
    """Small immutable snapshots of the real TU15 order-independent fix."""
    @classmethod
    def setUpClass(cls):
        cls.unit='Game/System/WorldMapResultTU15'
        fixture=ROOT.parent/'dispatch-land'
        if not (fixture/'build/SB4E01/src'/f'{cls.unit}.o').exists():
            raise unittest.SkipTest('stable world-map fixture unavailable')
        cls.tmp=tempfile.TemporaryDirectory(prefix='R-sinit-')
        cls.dir=pathlib.Path(cls.tmp.name)
        def phase(root,label):
            p=dc.Prog.__new__(dc.Prog);p.root=root
            p.sy=dc.Symbols(root/'config/SB4E01/symbols.txt')
            p.dol=dc.Dol(dc.original_dol(root,DOL))
            p.bases,p.ends=dc.load_splits(root/'config/SB4E01/splits.txt')
            path=cls.dir/(label+'.o');path.write_bytes((root/'build/SB4E01/src'/f'{cls.unit}.o').read_bytes())
            p.elfs={cls.unit:dc.ELF(path)};p.inferred=collections.defaultdict(lambda:collections.defaultdict(list))
            p._chains=None;p.ctor_inst={}
            return p
        cls.before,cls.after=phase(ROOT,'before'),phase(fixture,'after')
        cls.rb=[];cls.ra=[]
        cls.before.run_sinit(cls.rb,collections.Counter());cls.after.run_sinit(cls.ra,collections.Counter())
        cls.rb=next(r for r in cls.rb if r['offset']==20)
        cls.ra=next(r for r in cls.ra if r['offset']==20)
        if cls.rb['verdict']!='CROSSED' or cls.ra['verdict']!='MATCH':raise unittest.SkipTest('TU15 fixture is not the expected fix')

    @classmethod
    def tearDownClass(cls):cls.tmp.cleanup()

    def permissions(self,b=None,a=None):
        b=b or self.before;a=a or self.after
        dp.prepare_addresses(b,a)
        return dp.sinit_permissions(b,a,self.rb,self.ra)

    def test_sinit_positive(self):self.assertEqual(len(self.permissions()),1)

    def changed(self,phase,offset,value):
        p=copy.copy(phase);e=phase.elfs[self.unit];raw=bytearray(e.raw)
        put32(raw,offset,value);path=self.dir/'mutated.o';path.write_bytes(raw)
        p.elfs={self.unit:dc.ELF(path)}
        return p

    def test_sinit_unrelated_code_byte(self):
        e=self.after.elfs[self.unit];f=next(f for f in e.syms if f['name']==self.ra['function'])
        a=self.changed(self.after,e.s[f['sec']][4]+f['off'],0x60000000)
        self.assertFalse(self.permissions(a=a))

    def test_sinit_r3_clobber(self):
        # Both bodies equal, but the load no longer reaches the ctor in r3.
        ps=[]
        for phase in (self.before,self.after):
            e=phase.elfs[self.unit];f=next(f for f in e.syms if f['name']==self.ra['function'])
            ps.append(self.changed(phase,e.s[f['sec']][4]+f['off']+12,0x38600000))
        self.assertFalse(self.permissions(*ps))

    def test_sinit_changed_ctor_body(self):
        e=self.after.elfs[self.unit];f=next(f for f in e.syms if f['name']==self.ra['target'])
        a=self.changed(self.after,e.s[f['sec']][4]+f['off']+8,0x90830004)
        self.assertFalse(self.permissions(a=a))

    def test_sinit_wrong_source_pair(self):
        row=dict(self.ra,offset=16)
        dp.prepare_addresses(self.before,self.after)
        self.assertFalse(dp.sinit_permissions(self.before,self.after,self.rb,row))


if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--root',type=pathlib.Path,default=ROOT);ap.add_argument('--dol');ap.add_argument('--pointwarp-after',type=pathlib.Path,default=POINT);ap.add_argument('--proof-script',type=pathlib.Path)
    args,rest=ap.parse_known_args();ROOT=args.root.resolve();DOL=args.dol;POINT=args.pointwarp_after.resolve() if args.pointwarp_after else None
    if args.proof_script:
        spec=importlib.util.spec_from_file_location('reviewed_proof',args.proof_script);dp=importlib.util.module_from_spec(spec);spec.loader.exec_module(dp)
        if not hasattr(dp,'ignored'):dp.ignored=lambda e,i:e.names[i].startswith(('.line','.debug'))
    unittest.main(argv=[sys.argv[0]]+rest,verbosity=2)
