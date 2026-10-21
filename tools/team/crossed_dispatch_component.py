"""Isolated crossed-dispatch component primitives. No compiler/cert/import API.

Callers are three fixed family modules; this does not alter dispatchproof.SCOPES.
Fresh measured envelopes and independent original ownership qualification remain
mandatory. Synthetic models are not actual ELF or original-image acceptance.
"""
import copy
import hashlib
import json
from pathlib import Path
import struct
import objsame

DOL_SHA='abea6b0c498c4af90b17cfa08d54a01781186f097bee3e0761dbfb4838cbc281'

def require(ok,message):
    if not ok:raise ValueError('crossed component: '+message)

def canonical(value):return json.dumps(value,sort_keys=True,separators=(',',':')).encode()
def digest(value):return hashlib.sha256(canonical(value)).hexdigest()
def record(summary,name):
    rows=[list(row) for row in summary['named symbols'] if row[0]==name]
    require(len(rows)==1,'missing/duplicate symbol '+name)
    return rows[0]

def parse_map(text):
    import re
    result={}
    for line in text.splitlines():
        m=re.fullmatch(r'(\S+) = (\.\w+):0x([0-9a-fA-F]+); // type:(function|object) size:0x([0-9a-fA-F]+)',line)
        if m:
            name,section,value,kind,size=m.groups()
            require(name not in result,'duplicate configured name')
            result[name]=[section,int(value,16),int(size,16),kind]
    return result

def map_check(before,after,spec):
    """Exact full map remainder including line ordering; only fixed named edits."""
    require(hashlib.sha256(before.encode()).hexdigest()==spec['config_sha256']['before'] and hashlib.sha256(after.encode()).hexdigest()==spec['config_sha256']['after'],'exact source-bound full map bytes')
    a=parse_map(before);b=parse_map(after)
    edits={r['symbol'] for r in spec['targets']}
    bridge=spec.get('config_bridge')
    if bridge:edits.update((bridge['before_name'],bridge['after_name']))
    for row in spec['targets']:
        for phase,tree in [('before',a),('after',b)]:
            require(tree.get(row['symbol'])==['.text',row[phase+'_va'],8,'function'],'mapped execute address/width')
    if bridge:
        require(a.get(bridge['before_name'])==['.sbss',bridge['va'],4,'object'],'old WaitNext map bridge')
        require(b.get(bridge['after_name'])==['.sbss',bridge['va'],4,'object'],'new WaitNext map bridge')
    def remainder(text):return [line for line in text.splitlines(keepends=True) if line.split(' = ',1)[0] not in edits]
    require(remainder(before)==remainder(after),'unapproved map bytes/order changed')
    # Changed lines themselves must match exact source-bound plan bytes/order.
    for phase,text in [('before',before),('after',after)]:
        observed=[line for line in text.splitlines(keepends=True) if line.split(' = ',1)[0] in edits]
        require(observed==spec['map_edit_lines'][phase],'mapped edit bytes/order drift')

def _defined(summary,name):
    rows=[row for row in summary['named symbols'] if row[0]==name]
    require(len(rows)<=1,'ambiguous named relocation target')
    return rows[0] if rows else None

def closure(summary,spec,phase,config):
    """Enumerate named/local/ABS/config aliases into the changed execute spans.

    Component unit only. A complete all-object tree/final-link closure is separate.
    """
    spans=[]
    for target in spec['targets']:
        r=record(summary,target['symbol']);spans.append((target,r))
    hits=[]
    for key,rows in summary.items():
        if not key.startswith('relocations for '):continue
        section=key[len('relocations for '):]
        require('section '+section in summary,'bad relocation source section')
        for row in rows:
            offset,typ,addend,target=row
            require(type(offset) is int and type(typ) is int and type(addend) is int,'malformed relocation scalar')
            require(0<=offset<summary['section '+section][2],'bad source relocation extent')
            require(target[0] in ('sym','local'),'unsupported relocation target kind')
            if target[0]=='sym':
                require(len(target)==2,'malformed named reference')
                named=_defined(summary,target[1]);loc=None
                if named:
                    require(not named[3].startswith('bad'),'bad target section')
                    if named[3] not in ('UND','special0xfff2'):loc=(named[3],named[4]+addend)
                cfg=config.get(target[1]);absolute=cfg[1]+addend if cfg else None
            else:
                require(len(target)==3 and type(target[2]) is int,'malformed local reference')
                loc=(target[1],target[2]+addend);absolute=loc[1] if target[1]=='special0xfff1' else None
                require(target[1]=='UND' or target[1].startswith('special') or 'section '+target[1] in summary,'bad local target section')
            for expected,r in spans:
                by_name=target[0]=='sym' and target[1]==expected['symbol']
                by_location=loc is not None and loc[0]==r[3] and r[4]<=loc[1]<r[4]+r[5]
                by_va=absolute is not None and expected[phase+'_va']<=absolute<expected[phase+'_va']+8
                by_abs=loc is not None and loc[0]=='special0xfff1' and expected[phase+'_va']<=loc[1]<expected[phase+'_va']+8
                if by_name or by_location or by_va or by_abs:hits.append([section,offset,typ,addend,list(target),expected['symbol']])
    return sorted(hits,key=canonical)

def compare_summaries(before,after,envelope,envelope_sha,spec,config_before,config_after):
    """Only operator-pinned measured symbol/slot tuples; never inferred allowances."""
    require(digest(envelope)==envelope_sha,'independent measured envelope pin')
    require(set(envelope)=={'schema','family','base','candidate','raw_objects','symbols','owning_slots','route_done'},'exact measured envelope fields')
    require(type(envelope['schema']) is int and envelope['schema']==1,'envelope schema')
    require(envelope['family']==spec['family'] and envelope['base']==spec['base'] and envelope['candidate']==spec['candidate'],'family/source envelope mismatch')
    require(set(envelope['raw_objects'])=={'before','after'} and all(type(h) is str and len(h)==64 and set(h)<=set('0123456789abcdef') for h in envelope['raw_objects'].values()),'actual object digest inventory')
    names={r['symbol'] for r in spec['targets']}
    require(set(envelope['symbols'])==names and set(envelope['owning_slots'])==names,'exact family envelope inventory')
    norm=[copy.deepcopy(before),copy.deepcopy(after)]
    for phase,summary in [('before',before),('after',after)]:
        expected_refs=[]
        for item in spec['targets']:
            name=item['symbol'];r=record(summary,name)
            require(r==envelope['symbols'][name][phase],'unmeasured execute info/other/section/value/size')
            require(r[1]&15==2 and r[2]==0 and r[3].startswith('.text#') and r[5]==8,'exact FUNC8/visibility/section')
            vt=record(summary,item['vtable_symbol'])
            require(vt[1]&15==1 and vt[3].startswith('.data#') and vt[5]==16,'actual owning vtable identity/extent')
            expected=[vt[3],vt[4]+8,1,0,['sym',name],name]
            require(envelope['owning_slots'][name][phase]==expected,'owning slot is not exact vtable+8/ADDR32/zeroaddend')
            require(r[3]==envelope['symbols'][name]['before'][3],'execute section occurrence changed')
            expected_refs.append(expected)
        refs=closure(summary,spec,phase,config_before if phase=='before' else config_after)
        require(refs==sorted(expected_refs,key=canonical),'unexpected named/local/ABS/midbody/extra reference')
    # Normalize only explicitly measured symbol value fields. Never binding/size/data.
    for item in spec['targets']:
        name=item['symbol'];a=envelope['symbols'][name]['before'];b=envelope['symbols'][name]['after']
        require(a[:4]+a[5:]==b[:4]+b[5:] and a[4]!=b[4],'only named execute location may exchange')
        require(b[4]-a[4]==item['after_va']-item['before_va'],'execute offset delta does not match fixed original map permutation')
        norm[1]['named symbols']=tuple(sorted(tuple(a) if row[0]==name else tuple(row) for row in norm[1]['named symbols']))
    if spec['family']=='RouteOpen':
        route_done(norm,before,after,envelope['route_done'],spec)
    else:require(envelope['route_done'] is None,'unexpected RouteOpen allowance')
    require(norm[0]==norm[1],'strict remainder differs outside exact family envelope')
    return {'status':'COMPONENT_REMAINDER_ONLY','family':spec['family'],'production_acceptance':False,
      'full_reference_closure':False,'original_ownership':False,'changed_execute_count':len(names)}

def route_done(normalized,before,after,measured,spec):
    """One exact addi+SDA21 site; actual measured tuples required, no body mask."""
    require(type(measured) is dict and set(measured)=={'section','offset','words','relocations'},'measured RouteOpen site missing')
    section=measured['section'];offset=measured['offset'];method=record(before,spec['done']['method_symbol'])
    require(section==method[3] and offset==method[4]+0x7c,'done producer not exact method+0x7c')
    for i,(phase,summary) in enumerate([('before',before),('after',after)]):
        data=summary['section '+section][4];word=bytes.fromhex(measured['words'][phase])
        require(len(word)==4 and data[offset:offset+4]==word,'wrong measured done instruction')
        require(int.from_bytes(word,'big')>>16==0x388d,'done instruction must remain addi r4,r13')
        row=measured['relocations'][phase]
        expected=[offset,109,0,['sym',spec['done'][phase+'_singleton']]]
        require(row==expected,'only exact zeroaddend SDA21 target change')
        key='relocations for '+section;rows=list(summary[key])
        require(sum(list(r[:3])+[list(r[3])]==row for r in rows)==1,'missing/duplicate done relocation')
        if phase=='after':
            a=normalized[1]['section '+section];data=bytearray(a[4]);data[offset:offset+4]=bytes.fromhex(measured['words']['before'])
            normalized[1]['section '+section]=(*a[:4],bytes(data))
            replacement=measured['relocations']['before'];normalized[1][key]=tuple(sorted((replacement[0],replacement[1],replacement[2],tuple(replacement[3])) if list(r[:3])+[list(r[3])]==row else r for r in rows))
    require(measured['relocations']['before']!=measured['relocations']['after'],'done improvement missing')

def dol_span(data,address,size):
    require(len(data)>=0x100 and size>0,'bad DOL extent')
    offsets=struct.unpack_from('>18I',data,0);vas=struct.unpack_from('>18I',data,0x48);sizes=struct.unpack_from('>18I',data,0x90)
    matches=[o+address-v for o,v,n in zip(offsets,vas,sizes) if n and v<=address and address+size<=v+n and o>=0x100]
    require(len(matches)==1 and matches[0]+size<=len(data),'ambiguous/unmapped original span')
    return data[matches[0]:matches[0]+size]

def branch(word,pc,linked):
    require(word>>26==18 and bool(word&1)==linked and not word&2,'wrong relative branch/call')
    d=word&0x03fffffc
    if d&0x02000000:d-=0x04000000
    return (pc+d)&0xffffffff

def original_slots(data,spec):
    """Bounded slot/wrapper check ONLY, not full constructor/weak/lifetime proof."""
    require(hashlib.sha256(data).hexdigest()==DOL_SHA,'private original whole-image pin')
    for r in spec['targets']:
        require(int.from_bytes(dol_span(data,r['slot_va'],4),'big')==r['after_va'],'original owning slot target')
        load,tail=struct.unpack('>II',dol_span(data,r['after_va'],8))
        require(load==0x80640000 and branch(tail,r['after_va']+4,False)==r['method_va'],'original Spine r4/executor/tail target')
    if spec['family']=='RouteOpen':
        word=int.from_bytes(dol_span(data,0x804f171c,4),'big')
        immediate=word&0xffff;immediate=immediate-0x10000 if immediate&0x8000 else immediate
        require(word>>16==0x388d and (0x807d7320+immediate)&0xffffffff==0x807d5f20,'original done r4 SDA target')
        require(branch(int.from_bytes(dol_span(data,0x804f1720,4),'big'),0x804f1720,True)==0x804c4cc0,'original done consuming call')
    return {'status':'ORIGINAL_SLOT_WRAPPER_COMPONENT_ONLY','full_ownership':False,'family':spec['family']}


def qualify_files(before,after,envelope,pin,spec,map_before,map_after):
    require(digest(envelope)==pin,'operator envelope pin required before reading object')
    for phase,path in [('before',before),('after',after)]:
        p=Path(path);require(p.is_file() and not p.is_symlink() and p.stat().st_size<=16*1024**2,'bounded regular object')
        require(hashlib.sha256(p.read_bytes()).hexdigest()==envelope['raw_objects'][phase],'actual raw object pin')
    map_check(map_before,map_after,spec)
    return compare_summaries(objsame.summarize(before),objsame.summarize(after),envelope,pin,spec,parse_map(map_before),parse_map(map_after))
