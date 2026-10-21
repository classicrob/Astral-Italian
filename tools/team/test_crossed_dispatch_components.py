"""Pure synthetic summary/source tests; no compiler/process/DOL payload reads."""
import copy
import json
from pathlib import Path
import unittest
import crossed_dispatch_component as c
import crossed_routeopen as route
import crossed_pointwarp as point
import crossed_createtico as tico

MODULES=(route,point,tico)

def fixture(module):
    s=module.spec();minimum=min(t['before_va'] for t in s['targets'])
    if s['family']=='RouteOpen':minimum=0x804f16a0
    text_size=max(t['before_va']-minimum+8 for t in s['targets'])+16
    raw=bytearray(text_size);method=None
    if s['family']=='RouteOpen':raw[0x7c:0x80]=bytes.fromhex('388d0000')
    summaries=[];envelope={'schema':1,'family':s['family'],'base':s['base'],'candidate':s['candidate'],
      'raw_objects':{'before':'0'*64,'after':'1'*64},'symbols':{},'owning_slots':{},'route_done':None,'context':{}}
    configs=[]
    for phase in ('before','after'):
        symbols=[];relocs=[]
        for i,t in enumerate(s['targets']):
            ex=[t['symbol'],0x22,0,'.text#0',t[phase+'_va']-minimum,8]
            vt=[t['vtable_symbol'],0x21,0,'.data#0',i*16,16]
            symbols.extend((tuple(ex),tuple(vt)));row=('.data#0',i*16+8,1,0,['sym',t['symbol']],t['symbol'])
            envelope['symbols'].setdefault(t['symbol'],{})[phase]=ex
            envelope['owning_slots'].setdefault(t['symbol'],{})[phase]=list(row)
            relocs.append((i*16+8,1,0,('sym',t['symbol'])))
        summary={'section .text#0':(1,6,len(raw),4,bytes(raw)),
                 'section .data#0':(1,3,len(s['targets'])*16,4,bytes(len(s['targets'])*16)),
                 'named symbols':None,'relocations for .data#0':tuple(relocs)}
        if s['family']=='RouteOpen':
            summary['section .sbss#0']=(8,3,8,4,b'')
            symbols.extend([(s['done']['method_symbol'],0x12,0,'.text#0',0,0xb8),
                 (s['done']['before_singleton'],0x21,0,'.sbss#0',4,4),
                 (s['done']['after_singleton'],0x21,0,'.sbss#0',0,4)])
            summary['relocations for .text#0']=((0x7c,109,0,('sym',s['done'][phase+'_singleton'])),)
        # Synthetic full context, not observed ELF/provider ownership.
        text=bytearray(summary['section .text#0'][4]);data=bytearray(summary['section .data#0'][4]);sbss=8
        for name,kind in s['required_context'].items():
            existing=next((r for r in symbols if r[0]==name),None)
            if existing is None:
                if name.startswith('__vt__'):
                    value=len(data);data.extend(bytes(16));existing=(name,0x21,0,'.data#0',value,16)
                elif name.startswith('sInstance__'):
                    existing=(name,0x21,0,'.sbss#0',sbss,4);sbss+=4
                else:
                    width=72 if 'WhiteWait' in name and name.startswith('execute') else 16
                    value=len(text);text.extend(bytes(width));existing=(name,0x22,0,'.text#0',value,width)
                symbols.append(existing)
            envelope['context'].setdefault(name,{})[phase]=list(existing)
        summary['section .text#0']=(1,6,len(text),4,bytes(text))
        summary['section .data#0']=(1,3,len(data),4,bytes(data))
        summary['section .sbss#0']=(8,3,sbss,4,b'')
        summary['named symbols']=tuple(sorted(symbols));summaries.append(summary)
        configs.append({t['symbol']:['.text',t[phase+'_va'],8,'function'] for t in s['targets']})
    if s['family']=='RouteOpen':
        envelope['route_done']={'section':'.text#0','offset':0x7c,'words':{'before':'388d0000','after':'388d0000'},
        'relocations':{phase:[0x7c,109,0,['sym',s['done'][phase+'_singleton']]] for phase in ('before','after')}}
    return s,summaries,envelope,configs

def compare(module,fixture0):
    s,pair,e,maps=fixture0
    return module.compare(*pair,e,c.digest(e),*maps)

class FamilyModels(unittest.TestCase):
    def test_each_exact_component(self):
        for module in MODULES:
            with self.subTest(module=module.__name__):
                result=compare(module,fixture(module));self.assertFalse(result['production_acceptance']);self.assertFalse(result['full_reference_closure'])
    def test_each_owning_target_crossed(self):
        for module in MODULES:
            original=fixture(module)
            for i,t in enumerate(original[0]['targets']):
                f=copy.deepcopy(original);row=list(f[1][1]['relocations for .data#0'][i]);row[3]=('sym','unapprovedTarget')
                rows=list(f[1][1]['relocations for .data#0']);rows[i]=tuple(row);f[1][1]['relocations for .data#0']=tuple(rows)
                with self.assertRaises(ValueError):compare(module,f)
    def test_strict_remainder_and_first_text(self):
        for module in MODULES:
            for key in ('section .text#0','section .data#0'):
                f=fixture(module);section=list(f[1][1][key]);payload=bytearray(section[4]);payload[0]^=1;section[4]=bytes(payload);f[1][1][key]=tuple(section)
                with self.assertRaises(ValueError):compare(module,f)
            f=fixture(module);f[1][1]['section .rodata#0']=(1,2,4,4,b'X'*4)
            with self.assertRaises(ValueError):compare(module,f)
    def test_symbol_width_visibility_binding_delta(self):
        for module in MODULES:
            for field,value in ((1,0x12),(2,2),(3,'.text#1'),(4,123),(5,12)):
                f=fixture(module);name=f[0]['targets'][0]['symbol'];rows=list(f[1][1]['named symbols'])
                idx=next(i for i,r in enumerate(rows) if r[0]==name);row=list(rows[idx]);row[field]=value;rows[idx]=tuple(row);f[1][1]['named symbols']=tuple(rows)
                # Even repinning the producer envelope cannot authorize new fields/offsets.
                f[2]['symbols'][name]['after']=row
                with self.assertRaises(ValueError):compare(module,f)
    def test_extra_named_local_abs_config_alias_and_midbody(self):
        for module in MODULES:
            for kind in ('named','local','abs','config','midbody'):
                f=fixture(module);t=f[0]['targets'][0];s=f[1][1];record=c.record(s,t['symbol'])
                if kind=='named':target=('sym',t['symbol'])
                elif kind=='local':target=('local',record[3],record[4])
                elif kind=='midbody':target=('local',record[3],record[4]+4)
                elif kind=='abs':target=('local','special0xfff1',t['after_va'])
                else:
                    target=('sym','externalAlias');f[3][1]['externalAlias']=['.text',t['after_va'],8,'function']
                s['relocations for .data#0']+=((0,1,0,target),)
                with self.assertRaises(ValueError):compare(module,f)
    def test_duplicate_missing_vtable_or_symbol_reject(self):
        for module in MODULES:
            f=fixture(module);rows=f[1][1]['named symbols'];f[1][1]['named symbols']=rows+(rows[0],)
            with self.assertRaises(ValueError):compare(module,f)
            f=fixture(module);name=f[0]['targets'][0]['vtable_symbol'];f[1][1]['named symbols']=tuple(r for r in f[1][1]['named symbols'] if r[0]!=name)
            with self.assertRaises(ValueError):compare(module,f)
    def test_old_source_and_wrong_family_pin(self):
        for module in MODULES:
            f=fixture(module);f[1][1]=copy.deepcopy(f[1][0])
            with self.assertRaises(ValueError):compare(module,f)
            f=fixture(module)
            with self.assertRaises(ValueError):module.compare(*f[1],f[2],'0'*64,*f[3])
            f[2]['candidate']='0'*40
            with self.assertRaises(ValueError):compare(module,f)
    def test_route_done_controls(self):
        for change in ('wrong_target','wrong_site','wrong_width','wrong_opcode','duplicate_reloc'):
            f=fixture(route);s=f[1][1]
            if change=='wrong_target':s['relocations for .text#0']=((0x7c,109,0,('sym',f[0]['done']['before_singleton'])),)
            elif change=='wrong_site':f[2]['route_done']['offset']=0x78
            elif change=='wrong_width':f[2]['route_done']['words']['after']='388d'
            elif change=='wrong_opcode':
                row=list(s['section .text#0']);raw=bytearray(row[4]);raw[0x7c:0x80]=bytes.fromhex('38630000');row[4]=bytes(raw);s['section .text#0']=tuple(row);f[2]['route_done']['words']['after']='38630000'
            else:s['relocations for .text#0']*=2
            with self.assertRaises(ValueError):compare(route,f)
    def test_whitewait_or_extra_field_not_sanctioned(self):
        f=fixture(tico);f[1][1]['named symbols']+=(('WhiteWait',0x22,0,'.text#0',0,72),)
        with self.assertRaises(ValueError):compare(tico,f)
        f=fixture(point);f[2]['route_done']={}
        with self.assertRaises(ValueError):compare(point,f)
    def test_no_production_admission(self):
        for module in MODULES:
            self.assertFalse(module.production_status()['execution_enabled']);self.assertFalse(module.production_status()['certificate'])
    def test_label_alias_and_unclassified_address_records(self):
        for module in MODULES:
            f=fixture(module);t=f[0]['targets'][0]
            text='Alias = .text:0x%08X; // type:label\n'%t['after_va']
            parsed=c.parse_map(text);self.assertEqual(parsed['Alias'][1],t['after_va']);self.assertIsNone(parsed['Alias'][2])
            ambiguous=c.parse_map(text+'Alias = .text:0x1000; // type:label\n')
            self.assertEqual(len(ambiguous['Alias']['occurrences']),2)
            f[3][1].update(ambiguous);f[1][1]['relocations for .data#0']+=((0,1,0,('sym','Alias')),)
            with self.assertRaises(ValueError):compare(module,f)
    def test_required_family_context_and_full_extents(self):
        for module in MODULES:
            f=fixture(module);name=next(n for n in f[0]['required_context'] if n.startswith('sInstance'))
            for phase in (0,1):f[1][phase]['named symbols']=tuple(r for r in f[1][phase]['named symbols'] if r[0]!=name)
            with self.assertRaises(ValueError):compare(module,f)
            f=fixture(module);last=max(row[1] for row in f[2]['owning_slots'].values() for row in [row['after']])
            for phase in (0,1):
                row=f[1][phase]['section .data#0'];f[1][phase]['section .data#0']=(row[0],row[1],last+1,row[3],row[4][:last+1])
            with self.assertRaises(ValueError):compare(module,f)
        f=fixture(route);name=f[0]['done']['after_singleton']
        for phase in (0,1):f[1][phase]['named symbols']=tuple(r for r in f[1][phase]['named symbols'] if r[0]!=name)
        with self.assertRaises(ValueError):compare(route,f)
        f=fixture(tico);name=next(n for n in f[0]['required_context'] if 'WhiteWait' in n and n.startswith('execute'))
        for phase in (0,1):f[1][phase]['named symbols']=tuple(r for r in f[1][phase]['named symbols'] if r[0]!=name)
        with self.assertRaises(ValueError):compare(tico,f)
    def test_actual_qualification_refuses_before_object_access(self):
        for module in MODULES:
            with self.assertRaisesRegex(ValueError,'qualification disabled'):
                module.qualify_files('/NOT_OPENED','/NOT_OPENED',{},'', '', '')

    def test_original_slot_decoder_helpers_only_synthetic(self):
        self.assertEqual(c.branch(0x48000008,0x1000,False),0x1008)
        self.assertEqual(c.branch(0x4bfffff9,0x1000,True),0xff8)
        with self.assertRaises(ValueError):c.branch(0x48000009,0x1000,False)
        with self.assertRaises(ValueError):c.original_slots(bytes(256),point.spec())

if __name__=='__main__':unittest.main()
