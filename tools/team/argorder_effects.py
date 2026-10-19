"""Bounded priority evidence for argorder; no sequencing/equivalence certificates."""
import collections
import hashlib
from pathlib import Path
import re
import struct

def bodies(text,name):
    from argorder import lex,pairs
    ts,_,_=lex(text);match,_=pairs(ts);out=[]
    for i,t in enumerate(ts[:-1]):
        if t.text!=name or ts[i+1].text!='('or i+1 not in match:continue
        j=match[i+1]+1
        while j<len(ts)and ts[j].text in('const','NO_INLINE','inline','override','final'):j+=1
        if j<len(ts)and ts[j].text=='{'and j in match:
            out.append(''.join(x.text for x in ts[j+1:match[j]]))
    return out


def class_text(text,name):
    from argorder import lex,pairs
    ts,_,_=lex(text);match,_=pairs(ts)
    for i,t in enumerate(ts[:-1]):
        if t.text not in('class','struct')or ts[i+1].text!=name:continue
        j=i+2
        while j<len(ts)and ts[j].text not in('{',';'):j+=1
        if j<len(ts)and ts[j].text=='{'and j in match:return text[ts[j].end:ts[match[j]].start]
    return ''


RNG_TYPES={'JPARandom','JPABaseEmitter','JPAEmitterWorkData','JMath::TRandom_fast_'}
RNG_METHODS={'JPARandom':{'get_rndm_u','get_rndm_f','get_rndm_zp','get_rndm_zh','get_rndm_ss'},
             'JPABaseEmitter':{'get_r_f','get_r_zp','get_r_zh','get_r_ss'},
             'JMath::TRandom_fast_':{'rand','getRand','getRandF'}}


def receiver_type(expression,bindings,fields):
    bits=re.split(r'->|\.',re.sub(r'\s+','',expression or''))
    if not bits or bits[0]not in bindings:return None
    typ=bindings[bits[0]]
    for member in bits[1:]:
        typ=fields.get((typ,member))
        if typ is None:return None
    return typ


def typed_bindings(text):
    """Only simple named declarations; conflicting same-name types refuse attribution."""
    from argorder import lex
    tokens,_,directives=lex(text)
    if directives:return {}  # Do not turn an inactive conditional declaration into a type proof.
    clean=[' ']*len(text)
    for token in tokens:
        if token.kind!='literal':clean[token.start:token.end]=text[token.start:token.end]
    text=''.join(clean)
    found=collections.defaultdict(set)
    pat=r'(?<![\w:])((?:[A-Za-z_]\w*::)*[A-Za-z_]\w*)\s*(?:const\s*)?([*&]+\s*|\s+)([A-Za-z_]\w*)\s*(?=[=;,){])'
    for m in re.finditer(pat,text):
        if m[1]not in {'return','case','throw','delete','new'}:found[m[3]].add(m[1])
    return {name:next(iter(types))for name,types in found.items()if len(types)==1 and next(iter(types))in RNG_TYPES}


def summarize_words(words,relocs=None):
    """Small direct body effect census. Never infer purity from const/name alone.

    Writes need a proven incoming argument/global base. Unknown loads/instructions
    kill pointer provenance. Pure leaves have no stores/calls/control flow and only
    whitelisted integer arithmetic/address formation; memory reads remain unknown.
    """
    relocs=relocs or{}
    origin={i:('arg',i,0)for i in range(3,11)};origin.update({1:('frame',0,0),2:('sda',2),13:('sda',13)})
    writes=[];unknown_stores=[];pure=True;why=[]
    def kill(r):origin.pop(r,None)
    for k,w in enumerate(words):
        off=k*4;op=w>>26;rt=(w>>21)&31;ra=(w>>16)&31;rb=(w>>11)&31;xo=(w>>1)&1023;imm=(w&65535)-(65536 if w&32768 else 0)
        if w==0x4e800020 and k==len(words)-1:continue
        if op in (36,37,38,39,44,45,52,53,54,55):
            pure=False;rel=relocs.get(off);base=origin.get(ra)
            if rel and rel[0]==109 and((ra==0 and all(origin.get(r,())[0:1]==('sda',)for r in(2,13)))or(ra in(2,13)and origin.get(ra,())[0:1]==('sda',))):role={'kind':'global-storage-write','symbol':rel[1][0],'addend':rel[2]}
            elif base and base[0]=='sda':role={'kind':'global-storage-write','base':base[1],'displacement':imm}
            elif base and base[0]=='arg':role={'kind':'incoming-argument-storage-write','argument_register':base[1],'displacement':base[2]+imm}
            elif rel and rel[0]==4 and base and base[0]=='symbol-hi-addi'and base[1]==rel[1]:role={'kind':'symbol-storage-write','symbol':rel[1][0],'offset_from_symbol':rel[2]}
            elif base and base[0]=='symbol-address':role={'kind':'symbol-storage-write','symbol':base[1][0],'offset_from_symbol':base[2]+imm}
            elif base and base[0]=='constant'and(base[1]+imm)&0xffffffff>=0x80000000:role={'kind':'absolute-memory-write','address':hex((base[1]+imm)&0xffffffff)}
            elif base and base[0]=='frame':role=None
            else:role={'kind':'unresolved-store-base'}
            if role:
                record={'offset':off,'word':f'{w:08x}',**role}
                (unknown_stores if role['kind']=='unresolved-store-base'else writes).append(record)
            if op in(37,39,45,53,55):
                if base and base[0]=='frame':origin[ra]=base
                else:kill(ra)
            continue
        if op in(32,33,34,35,40,41,42,43,48,49,50,51):
            pure=False;why.append('memory getter/literal needs additional volatility/ownership proof')
            if op<48:kill(rt)
            if op in(33,35,41,43,49,51):kill(ra)
            continue
        rel=relocs.get(off)
        if op==15 and ra==0 and rel and rel[0]in(5,6):
            if rt not in(0,*range(3,13)):pure=False;why.append('nonvolatile/register-base mutation')
            origin[rt]=('symbol-hi-addi'if rel[0]==6 else'symbol-hi-ori',rel[1],rel[2]);continue
        if op in(14,24)and rel and rel[0]==4:
            src=ra if op==14 else rt;dest=rt if op==14 else ra;base=origin.get(src)
            expected='symbol-hi-addi'if op==14 else'symbol-hi-ori'
            if dest not in(0,*range(3,13)):pure=False;why.append('nonvolatile/register-base mutation')
            if base and base[0]==expected and base[1]==rel[1]and base[2]==rel[2]:origin[dest]=('symbol-address',rel[1],rel[2])
            else:kill(dest)
            continue
        if op==14:
            if rt not in(0,*range(3,13)):pure=False;why.append('nonvolatile/register-base mutation')
            b=origin.get(ra)if ra else('constant',0)
            if b and b[0]=='constant':origin[rt]=('constant',(b[1]+imm)&0xffffffff)
            elif b and b[0]=='symbol-address':origin[rt]=(b[0],b[1],b[2]+imm)
            elif b and b[0]in('arg','frame'):origin[rt]=(b[0],b[1],b[2]+imm)
            elif b:origin[rt]=b
            else:kill(rt)
            continue
        if op==15 and ra==0:
            if rt not in(0,*range(3,13)):pure=False;why.append('nonvolatile/register-base mutation')
            origin[rt]=('constant',(imm<<16)&0xffffffff);continue
        if op==24:
            if ra not in(0,*range(3,13)):pure=False;why.append('nonvolatile/register-base mutation')
            b=origin.get(rt)
            if b and b[0]=='constant':origin[ra]=('constant',b[1]|(w&65535))
            elif w&65535==0 and b:origin[ra]=b
            else:kill(ra)
            continue
        if op==31 and xo==444:
            if ra not in(0,*range(3,13)):pure=False;why.append('nonvolatile/register-base mutation')
            if rt==rb and rt in origin:origin[ra]=origin[rt]
            else:kill(ra)
            continue
        if op in(16,18)or op==19:
            pure=False;why.append('call/control-flow/special instruction')
            if (op==18 and w&1)or(op==19 and ((w>>1)&1023)in(16,528)and w&1):
                for r in range(0,13):
                    if r not in(1,2):kill(r)
                continue
            # Only a straight-line prefix is summarized; do not count a skipped/dead store.
            break
        # Ordinary arithmetic allowed only on ABI-volatile result/scratch GPRs.
        if op in(20,21,23,25,26,27,28,29):
            if ra not in(0,*range(3,13)):pure=False;why.append('nonvolatile/register-base mutation')
            kill(ra);continue
        if op in(7,8,12,13):
            if rt not in(0,*range(3,13)):pure=False;why.append('nonvolatile/register-base mutation')
            kill(rt);continue
        if op==31 and xo in(266,40,104,26,954,922,24,536,824,316,28,60,124):
            dest=ra if xo in(26,954,922,24,536,824,316,28,60,124)else rt
            if dest not in(0,*range(3,13)):pure=False;why.append('nonvolatile/register-base mutation')
            kill(dest);continue
        if op==31 and xo==339:
            pure=False;why.append('special-register read');kill(rt);continue
        pure=False;why.append('unsupported instruction/provenance')
        origin={}
    # A leaf must end in BLR, not fall through or contain empty/unbounded data.
    pure=pure and bool(words)and words[-1]==0x4e800020
    return {'kind':'KNOWN_WRITE'if writes else'PURE_LEAF'if pure else'UNKNOWN',
            'writes':writes,'unresolved_stores':unknown_stores,
            'reason':'no memory loads/stores/calls/special operations; bounded address/integer leaf'if pure else'; '.join(sorted(set(why)))or'no proved external write; stores/unsupported behavior remain',
            'word_count':len(words)}


def rank(row,summaries):
    """HIGH needs two distinct known-effect slots. LOW requires every call proved pure."""
    blocked=any(any(word in u for word in ['macro','unevaluated','deferred lambda','unbalanced'])for u in row['uncertainty'])
    slots=[]
    for arg in row['arguments']:
        known=[];unknown=[];pure=[]
        if arg['mutation_count'] and not blocked:known.append({'kind':'source-mutation','operators':arg['mutation_ops']})
        for call in arg['calls']:
            s=summaries(call)
            if s['kind']=='KNOWN_WRITE'and not blocked:known.append(s)
            elif s['kind']=='PURE_LEAF':pure.append(s)
            else:unknown.append(s)
        slots.append({'argument':arg['index'],'known_effects':known,'pure_calls':pure,'unknown_calls':unknown})
    count=sum(bool(s['known_effects'])for s in slots)
    allpure=row['argument_order'].get('status')=='resolved-direct-effect-call-order'and not blocked and not any(a['mutation_count']for a in row['arguments'])and all(not s['known_effects']and not s['unknown_calls']for s in slots)
    level='HIGH'if count>=2 else'LOW'if allpure else'UNKNOWN'if blocked else'MEDIUM'
    reason=f'{count} distinct argument slots have established mutation/RNG/direct-storage-write effects'if level=='HIGH'else'every argument call has a checked pure-leaf summary; no mutations'if level=='LOW'else'macro/unevaluated/deferred or malformed context prevents effect attribution'if blocked else'fewer than two established-effect slots; other calls/effects remain unresolved'
    return {'level':level,'known_effect_argument_count':count,'reason':reason,'arguments':slots,
            'scope':'Priority advisory only. Known effects need not share storage; LOW is not a sequencing/equivalence proof.'}


class EffectAudit:
    def __init__(self,root,binary=None):
        self.root=root;self.binary=binary;self.contracts={};self.fields={};self.mr=set();self.cache={};self.providers=collections.OrderedDict();self.ranges=[];self.inputs={}
        for uline in (root/'config/SB4E01/splits.txt').read_text().splitlines()if(root/'config/SB4E01/splits.txt').exists()else[]:
            if uline and not uline[0].isspace()and uline.endswith(':'):unit=uline[:-1].removesuffix('.cpp').removesuffix('.c')
            m=re.search(r'\.text\s+start:0x([\dA-Fa-f]+)\s+end:0x([\dA-Fa-f]+)',uline)
            if m:self.ranges.append((int(m[1],16),int(m[2],16),unit))
        self.header_max_mtime_ns=max((p.stat().st_mtime_ns for folder in ['include','libs','src']for p in (root/folder).rglob('*')if p.is_file()and p.suffix in('.h','.hpp','.inc','.inl')),default=0)
        self._contracts()
    def _read(self,rel):
        p=self.root/rel
        if not p.is_file():return''
        raw=p.read_bytes();self.inputs[rel]=hashlib.sha256(raw).hexdigest();return raw.decode(errors='replace')
    def _contracts(self):
        jp='libs/JSystem/include/JSystem/JParticle/JPARandom.hpp';jm='libs/JSystem/include/JSystem/JMath/random.hpp';em='libs/JSystem/include/JSystem/JParticle/JPAEmitter.hpp';mu='src/Game/Util/MathUtil.cpp';holder='include/Game/System/GameSystemObjHolder.hpp'
        a,b,c,m,h=[self._read(p)for p in [jp,jm,em,mu,holder]]
        ja=class_text(a,'JPARandom');jb=class_text(b,'TRandom_fast_');ce=class_text(c,'JPABaseEmitter');cw=class_text(c,'JPAEmitterWorkData')
        expected_jpa={
            'get_rndm_u':'returnmSeed=mSeed*0x19660du+0x3c6ef35fu;',
            'get_rndm_f':'union{u32u;f32f;}a;a.u=((get_rndm_u()>>9)|0x3f800000);returna.f-1.0f;',
            'get_rndm_zp':'f32f=get_rndm_f();return(f+f)-1.0f;',
            'get_rndm_zh':'returnget_rndm_f()-0.5f;',
            'get_rndm_ss':'return(s16)(get_rndm_u()>>16);'}
        deps={'get_rndm_u':None,'get_rndm_f':'get_rndm_u','get_rndm_zp':'get_rndm_f','get_rndm_zh':'get_rndm_f','get_rndm_ss':'get_rndm_u'}
        for n,body in expected_jpa.items():
            if not re.search(r'\bvirtual\s+[^;{]*\b'+n+r'\s*\(',ja)and bodies(ja,n)==[body]and(deps[n]is None or('JPARandom',deps[n])in self.contracts):
                self.contracts[('JPARandom',n)]={'kind':'KNOWN_WRITE','contract':'JPARandom::'+n+' advances mSeed via checked wrapper body','evidence_files':[jp],'normalized_body':body}
        expected_jm={'rand':'returnmSeed=0x19660d*mSeed+0x3c6ef35f;;',
                     'getRandF':'union{f32f;u32s;}out;out.s=(rand()>>9)|0x3f800000;returnout.f-1;',
                     'getRand':'return(rand()>>9)%range;'}
        for n,body in expected_jm.items():
            if not re.search(r'\bvirtual\s+[^;{]*\b'+n+r'\s*\(',jb)and bodies(jb,n)==[body]and(n=='rand'or('JMath::TRandom_fast_','rand')in self.contracts):
                self.contracts[('JMath::TRandom_fast_',n)]={'kind':'KNOWN_WRITE','contract':'JMath::TRandom_fast_::'+n+' advances mSeed via checked wrapper body','evidence_files':[jm],'normalized_body':body}
        for n in RNG_METHODS['JPABaseEmitter']:
            leaf={'get_r_f':'get_rndm_f','get_r_zp':'get_rndm_zp','get_r_zh':'get_rndm_zh','get_r_ss':'get_rndm_ss'}[n]
            if not re.search(r'\bvirtual\s+[^;{]*\b'+n+r'\s*\(',ce)and ('JPARandom',leaf)in self.contracts and bodies(ce,n)==['returnmRndm.'+leaf+'();']:
                self.contracts[('JPABaseEmitter',n)]={'kind':'KNOWN_WRITE','contract':'JPABaseEmitter::'+n+' forwards to JPARandom::'+leaf+' and advances receiver mRndm.mSeed','evidence_files':[em,jp]}
        if re.search(r'JPABaseEmitter\s*\*\s*mpEmtr\s*;',cw):self.fields[('JPAEmitterWorkData','mpEmtr')]='JPABaseEmitter'
        for typ,body in [('JPABaseEmitter',ce),('JPAEmitterWorkData',cw)]:
            if re.search(r'JPARandom\s+mRndm\s*;',body):self.fields[(typ,'mRndm')]='JPARandom'
        mr_base='u32rand=SingletonHolder<GameSystem>::get()->mObjHolder->mRandom.rand();u32value=(rand>>9)|0x3F800000;returnreinterpret_cast<f32&>(value)-1.0f;'
        mr_bodies={mr_base,'returnmin+(max-min)*getRandom();','returngetRandom(static_cast<f32>(min),static_cast<f32>(max));'}
        if ('JMath::TRandom_fast_','rand')in self.contracts and set(bodies(m,'getRandom'))==mr_bodies and re.search(r'JMath::TRandom_fast_\s+mRandom\s*;',h):
            self.mr={'getRandom'}
            if bodies(m,'getRandomDegree')==['returngetRandom(0.0f,360.0f);']:self.mr.add('getRandomDegree')
            vector='pDst->set<f32>(getRandom(-range,range),getRandom(-range,range),getRandom(-range,range));'
            if bodies(m,'getRandomVector')==[vector]:self.mr.add('getRandomVector')
            self.mr_evidence=[mu,jm,holder]
    def stable_contracts(self):
        return all((self.root/p).is_file()and hashlib.sha256((self.root/p).read_bytes()).hexdigest()==h for p,h in self.inputs.items())
    def summary(self,row,call):
        if call.get('macro')or call.get('indirect'):return {'kind':'UNKNOWN','reason':'macro/indirect effect identity unresolved'}
        typ=receiver_type(call.get('receiver'),row.get('typed_receivers',{}),self.fields);key=(typ,call['name'])
        if key in self.contracts:return {**self.contracts[key],'receiver':call.get('receiver'),'resolved_receiver_type':typ}
        if call['name']in self.mr and(call['spelling']=='MR::'+call['name']or(row['unit']=='Game/Util/MathUtil'and call['spelling']==call['name'])):
            return {'kind':'KNOWN_WRITE','contract':'canonical MR RNG uses GameSystem object-holder JMath random seed','evidence_files':self.mr_evidence}
        return self._binary_summary(row,call)
    def _binary_summary(self,row,call):
        if not self.binary or'objects'not in row or'binary_calls'not in row:return {'kind':'UNKNOWN','reason':'no uniquely bound direct callee/provider body'}
        from argorder import short_symbol
        hits={c['callee']for c in row['binary_calls']['source']if short_symbol(c.get('callee',''))==call['name']and not c.get('addend')}
        if len(hits)!=1:return {'kind':'UNKNOWN','reason':'inline/indirect/overloaded callee body unresolved'}
        n=next(iter(hits));key=(row['unit'],n)
        if key in self.cache:return self.cache[key]
        try:
            result=self._provider_summary(row['unit'],n)
        except (ValueError,KeyError,OSError,struct.error)as e:result={'kind':'UNKNOWN','reason':str(e),'callee':n}
        self.cache[key]=result;return result
    def _provider_summary(self,caller,n):
        ca=self.binary.ca;record=self.binary.cat.get(n)
        if not record or n in self.binary.cat.duplicates:raise ValueError('callee target address missing/ambiguous')
        owners=[u for a,z,u in self.ranges if a<=record['address']<z]
        if len(owners)!=1:raise ValueError('owning callee TU missing/ambiguous')
        units=list(dict.fromkeys([caller]+owners));s=None;t=None
        for u in units:
            if u not in self.providers:
                paths=[self.root/f'build/SB4E01/{kind}/{u}.o'for kind in ['src','obj']]
                if not all(p.is_file()for p in paths):continue
                es=[ca.ELF(p)for p in paths];self.providers[u]=(es,paths)
                if len(self.providers)>8:self.providers.popitem(last=False)
            es,paths=self.providers[u]
            if n in es[0].f:s=es[0];sp=paths[0];su=u
            if n in es[1].f:t=es[1];tp=paths[1]
            if s is not None and t is not None:break
        if s is None or t is None:raise ValueError('actual source/target provider body absent')
        a,z,sec=s.f[n][1:];ta,tz,ts=t.f[n][1:];raw=ca.read_dol(self.binary.dol,record['address'],tz)
        if raw is None:raise ValueError('provider original DOL extent unavailable')
        tw=struct.unpack('>'+str(tz//4)+'I',raw);sw=struct.unpack('>'+str(z//4)+'I',s.c[sec][a:a+z])
        sr={off-a:r for(ss,off),r in s.rel.items()if ss==sec and a<=off<a+z}
        tr={off-ta:r for(ss,off),r in t.rel.items()if ss==ts and ta<=off<ta+tz}
        for off in range(0,tz,4):
            w=struct.unpack_from('>I',t.c[ts],ta+off)[0];r=tr.get(off);mask=0xfc000003 if r and r[0]==10 else 0xffe00000 if r and r[0]==109 else 0xffff0000 if r else 0xffffffff
            if w&mask!=tw[off//4]&mask:raise ValueError('provider opcode binding disagrees with original DOL')
        ss=summarize_words(sw,sr);tt=summarize_words(tw,tr)
        kind='KNOWN_WRITE'if ss['kind']=='KNOWN_WRITE'or tt['kind']=='KNOWN_WRITE'else'PURE_LEAF'if ss['kind']==tt['kind']=='PURE_LEAF'else'UNKNOWN'
        freshness={'all_header_mtime_not_newer_than_object':self.header_max_mtime_ns<=sp.stat().st_mtime_ns}
        sourcefiles=[self.root/f'src/{su}{suffix}'for suffix in('.cpp','.c','.cc','.cxx')]
        owned=[p for p in sourcefiles if p.is_file()]
        freshness['owning_source_present_and_not_newer']=len(owned)==1 and owned[0].stat().st_mtime_ns<=sp.stat().st_mtime_ns
        if kind=='PURE_LEAF'and not all(freshness.values()):kind='UNKNOWN'
        for path,e in [(sp,s),(tp,t)]:
            if hashlib.sha256(path.read_bytes()).hexdigest()!=hashlib.sha256(e.raw).hexdigest():raise ValueError('provider object changed during summary')
        return {'kind':kind,'callee':n,'freshness':freshness,'actual_DOL_address':hex(record['address']),'source':ss,'target':tt,
                'source_provider':str(sp),'target_provider':str(tp),'source_object_sha256':hashlib.sha256(s.raw).hexdigest(),'target_object_sha256':hashlib.sha256(t.raw).hexdigest(),
                'scope':'bounded direct body only; opaque calls are not recursively summarized; const annotation is not evidence'}
