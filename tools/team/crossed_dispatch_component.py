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
    """Resolve address records regardless of type/size; labels are not dropped."""
    import re
    result={}
    for line in text.splitlines():
        if ' = .' not in line:continue
        m=re.fullmatch(r'(\S+) = (\.[\w.]+):0x([0-9a-fA-F]+);(?: // (.*))?',line)
        require(m is not None,'unsupported address map record')
        name,section,value,attributes=m.groups();attributes=attributes or ''
        typ=re.search(r'(?:^| )type:(\w+)',attributes);size=re.search(r'(?:^| )size:0x([0-9a-fA-F]+)',attributes)
        row=[section,int(value,16),int(size[1],16) if size else None,typ[1] if typ else 'unclassified']
        if name not in result:result[name]=row
        elif type(result[name]) is dict:result[name]['occurrences'].append(row)
        else:result[name]={'occurrences':[result[name],row]}  # Preserve, never choose one bare-name owner.
    return result


def relocation_width(typ):
    widths={1:4,2:4,3:2,4:2,5:2,6:2,7:4,8:4,9:4,10:4,11:4,12:4,13:4,26:4,32:4,109:4}
    require(type(typ) is int and typ in widths,'unqualified relocation write width')
    return widths[typ]


def require_summary_extents(summary):
    for key,section in summary.items():
        if not key.startswith('section '):continue
        require(len(section)==5 and type(section[2]) is int and section[2]>=0,'malformed section shape')
        require(isinstance(section[4],bytes) and len(section[4])==(0 if section[0]==8 else section[2]),'declared/payload section extent mismatch')
    for row in summary['named symbols']:
        require(len(row)==6 and type(row[4]) is int and type(row[5]) is int and row[4]>=0 and row[5]>=0,'malformed symbol extent')
        if row[3]=='UND':continue
        if row[3].startswith('special'):
            require(row[3] in ('special0xfff1','special0xfff2'),'unsupported special symbol section')
            continue
        require('section '+row[3] in summary,'invalid named symbol section')
        require(row[4]+row[5]<=summary['section '+row[3]][2],'symbol extends outside section')


def required_context(summary,spec,measured,phase):
    require(type(measured) is dict and set(measured)==set(spec['required_context']),'mandatory family context inventory')
    for name,kind in spec['required_context'].items():
        row=record(summary,name)
        require(set(measured[name])=={'before','after'} and row==measured[name][phase],'family context tuple not independently measured')
        require(row[1]&15==kind and row[3]!='UND' and not row[3].startswith('special'),'mandatory provider undefined/wrong type')
        require(row[5]>0,'mandatory provider extent absent')
        require(measured[name]['before']==measured[name]['after'],'unapproved family context delta')


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
            require(0<=offset and offset+relocation_width(typ)<=summary['section '+section][2],'full relocation write exceeds source section')
            require(target[0] in ('sym','local'),'unsupported relocation target kind')
            if target[0]=='sym':
                require(len(target)==2,'malformed named reference')
                named=_defined(summary,target[1]);loc=None
                if named:
                    require(not named[3].startswith('bad'),'bad target section')
                    if named[3] not in ('UND','special0xfff2'):loc=(named[3],named[4]+addend)
                cfg=config.get(target[1]) if named is None or named[3]=='UND' else None
                candidates=cfg['occurrences'] if type(cfg) is dict else ([cfg] if cfg else [])
                absolute_values=[r[1]+addend for r in candidates]
            else:
                require(len(target)==3 and type(target[2]) is int,'malformed local reference')
                loc=(target[1],target[2]+addend);absolute_values=[loc[1]] if target[1]=='special0xfff1' else []
                require(target[1] in ('UND','special0xfff1','special0xfff2') or 'section '+target[1] in summary,'bad local target section')
            for expected,r in spans:
                by_name=target[0]=='sym' and target[1]==expected['symbol']
                by_location=loc is not None and loc[0]==r[3] and r[4]<=loc[1]<r[4]+r[5]
                by_va=any(expected[phase+'_va']<=absolute<expected[phase+'_va']+8 for absolute in absolute_values)
                by_abs=loc is not None and loc[0]=='special0xfff1' and expected[phase+'_va']<=loc[1]<expected[phase+'_va']+8
                if by_name or by_location or by_va or by_abs:hits.append([section,offset,typ,addend,list(target),expected['symbol']])
    return sorted(hits,key=canonical)

def compare_summaries(before,after,envelope,envelope_sha,spec,config_before,config_after):
    """Only operator-pinned measured symbol/slot tuples; never inferred allowances."""
    require(digest(envelope)==envelope_sha,'independent measured envelope pin')
    require(set(envelope)=={'schema','family','base','candidate','raw_objects','symbols','owning_slots','route_done','context'},'exact measured envelope fields')
    require(type(envelope['schema']) is int and envelope['schema']==1,'envelope schema')
    require(envelope['family']==spec['family'] and envelope['base']==spec['base'] and envelope['candidate']==spec['candidate'],'family/source envelope mismatch')
    require(set(envelope['raw_objects'])=={'before','after'} and all(type(h) is str and len(h)==64 and set(h)<=set('0123456789abcdef') for h in envelope['raw_objects'].values()),'actual object digest inventory')
    names={r['symbol'] for r in spec['targets']}
    require(set(envelope['symbols'])==names and set(envelope['owning_slots'])==names,'exact family envelope inventory')
    require_summary_extents(before);require_summary_extents(after)
    norm=[copy.deepcopy(before),copy.deepcopy(after)]
    for phase,summary in [('before',before),('after',after)]:
        required_context(summary,spec,envelope['context'],phase)
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
      'full_reference_closure':False,'original_ownership':False,'raw_metadata_qualified':False,'full_provider_ownership':False,'changed_execute_count':len(names)}

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


def qualification_admission():
    raise ValueError('actual ELF qualification disabled pending corrected raw/context adapter independent review')


def qualify_files(before,after,envelope,pin,spec,map_before,map_after):
    # No real objects are read through this API until exact corrected source review.
    qualification_admission()


def snapshot_component(path,pin):
    """One immutable byte read feeds BOTH digest and raw/semantic parsers.

    Source-only inspection helper; invoking it on real objects needs allocation.
    """
    import os
    import stat
    from crossed_dispatch_raw import read_snapshot
    fd=os.open(path,os.O_RDONLY|os.O_NOFOLLOW)
    try:
        before=os.fstat(fd)
        require(stat.S_ISREG(before.st_mode) and before.st_size<=16*1024**2,'bounded regular object')
        chunks=[];remaining=before.st_size
        while remaining:
            block=os.read(fd,min(remaining,65536));require(bool(block),'object shortened during snapshot')
            chunks.append(block);remaining-=len(block)
        require(os.read(fd,1)==b'','object grew during snapshot')
        after=os.fstat(fd)
        require((before.st_dev,before.st_ino,before.st_size,before.st_mtime_ns,before.st_ctime_ns)==
                (after.st_dev,after.st_ino,after.st_size,after.st_mtime_ns,after.st_ctime_ns),'object mutated during snapshot')
    finally:os.close(fd)
    raw=b''.join(chunks)
    require(hashlib.sha256(raw).hexdigest()==pin,'snapshot raw object pin')
    return read_snapshot(raw)
