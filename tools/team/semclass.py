#!/usr/bin/env python3
"""Fail-closed PPC semantic/ABI classifier. Read-only, bounded proof coverage."""
from __future__ import annotations
import argparse, collections, difflib, hashlib, importlib.util, itertools, json, re, struct, subprocess, sys, time
from pathlib import Path

# Existing audited instruction semantics are dependencies, not a name-based waiver.
_ROOT_TOOLS = Path(__file__).resolve().parent
if not (_ROOT_TOOLS/'rdchecked.py').is_file():
    _ROOT_TOOLS = Path('/Users/roberthaisfield/MacioGalaxy2/tools/team')
sys.path.append(str(_ROOT_TOOLS))
import rdchecked as rd
import semdecode
from semdecode import decode

FP_COMM = {'fadd','fadds','fmul','fmuls'}
FP_MADD_COMM = {'fmadd','fmadds'}
INVERSE = {'bne':'beq','bge':'blt','ble':'bgt','bns':'bso'}
MEM = rd.WIDTH


def parts(line):
    p=line.strip().split(None,1)
    return (p[0], [x.strip() for x in p[1].split(',')]) if len(p)==2 else (p[0] if p else '',[])


def decoded_blocks(lines):
    """A local CFG builder using the frozen extension decoder, never globals."""
    ins=[decode(line) for line in lines]
    if not ins:raise rd.Unproved('empty function')
    starts={0}
    for i,d in enumerate(ins):
        if d['kind']=='term':
            if i+1<len(ins):starts.add(i+1)
            if 'target' in d:
                t=d['target']
                if t%4 or not 0<=t//4<len(ins):raise rd.Unproved('branch outside function')
                starts.add(t//4)
    starts=sorted(starts);ids={s:i for i,s in enumerate(starts)};result=[]
    for bi,start in enumerate(starts):
        body=ins[start:starts[bi+1] if bi+1<len(starts) else len(ins)]
        term=body[-1] if body[-1]['kind']=='term' else None;succ=[]
        if term and 'target' in term:succ.append(ids[term['target']//4])
        if not term or term['op'] not in ('b','blr'):
            if bi+1==len(starts):raise rd.Unproved('unterminated function')
            succ.append(bi+1)
        if any(x<=bi for x in succ):raise rd.Unproved('cyclic CFG')
        result.append((body,tuple(succ)))
    return result


def canonical_graph(lines):
    """Acyclic CFG isomorphism: canonical predicate polarity and edge order.

    Only plain b/beq/bne/blt/bge/bgt/ble/bso/bns are supported. Branch hints
    and arbitrary condition encodings remain unknown. Never changes compares.
    """
    graph=decoded_blocks(lines)
    canon=[]
    for body,succ in graph:
        body=[dict(d) for d in body]
        if body[-1]['op'] in INVERSE:
            if len(succ)!=2: raise rd.Unproved('inverted branch without two successors')
            body[-1]['op']=INVERSE[body[-1]['op']];succ=tuple(reversed(succ))
        # Explicit unconditional branches are represented by their sole edge.
        if body[-1]['op']=='b': body=body[:-1]
        canon.append((body,succ))
    order=[];seen=set()
    def visit(i):
        if i in seen:return
        seen.add(i)
        for s in canon[i][1]:visit(s)
        order.append(i)
    visit(0)
    if len(seen)!=len(canon):raise rd.Unproved('unreachable CFG blocks')
    order.reverse();ids={old:new for new,old in enumerate(order)}
    return [(canon[i][0],tuple(ids[s] for s in canon[i][1])) for i in order]


def checked_graph(left,right,restored=(set(),set()),memory_domain="strict",strict_tokens=(),domain_facts=None):
    """Full value/effect DAG, based on rdchecked's conservative kernel.

    Every load remains an effect node (even dead reads); calls order against ALL
    loads/stores. Only same-valued-base disjoint byte ranges can cross stores.
    FP event order, opcodes, grouping, conversions and precision remain exact.
    Only advisor-authorized add/multiply input commutation is canonicalized.
    """
    try:
        L,R=canonical_graph(left),canonical_graph(right)
        rd.reject_partial_paired_reads(L);rd.reject_partial_paired_reads(R)
        if len(L)!=len(R):raise rd.Unproved('CFG block count')
        for (a,sa),(b,sb) in zip(L,R):
            if sa!=sb:raise rd.Unproved('CFG successors')
            ta=a[-1]['op'] if a and a[-1]['kind']=='term' else None
            tb=b[-1]['op'] if b and b[-1]['kind']=='term' else None
            if ta!=tb:raise rd.Unproved('CFG terminator')
        pool={}
        def val(x):
            if x not in pool:pool[x]=len(pool)
            return pool[x]
        entry={r:val(('entry',r)) for r in rd.REGS}
        live=[set() for _ in L]
        for bi in reversed(range(len(L))):
            for graph in (L,R):
                body,succ=graph[bi];needed=set().union(*(live[s] for s in succ)) if succ else set(rd.OBSERVE)
                for d in reversed(body):needed.difference_update(d['def']);needed.update(d['use'])
                live[bi].update(needed)
        def run(graph,side):
            ends={};effects={};summaries=[];absolute_values=set();strict_values=set();memory_taints=[]
            for bi,(body,succ) in enumerate(graph):
                preds=[p for p in range(bi) if bi in graph[p][1]]
                if bi and not preds:raise rd.Unproved('unreachable block')
                state=dict(entry)
                for r in live[bi]:
                    if preds:
                        incoming=tuple((p,ends[p][r]) for p in preds)
                        state[r]=incoming[0][1] if len({v for _,v in incoming})==1 else val(('phi',bi,r,incoming))
                        if any(v in absolute_values for _,v in incoming):absolute_values.add(state[r])
                        if any(v in strict_values for _,v in incoming):strict_values.add(state[r])
                mem=[]
                if preds:mem.append(('call',None,val(('incoming-effects',tuple((p,effects[p]) for p in preds)))))
                nodes=collections.Counter();fp=val(('fp-entry',bi));term=None
                for d in body:
                    op,kind=d['op'],d['kind']
                    if op=='blr':
                        for r in restored[0 if side=='target' else 1]:state[r]=entry[r]
                    uses=tuple(state[r] for r in d['use'])
                    if kind=='gpr-helper':raise rd.Unproved('helper expansion not proved')
                    if kind=='copy':state[d['def'][0]]=uses[0];continue
                    if op in rd.COMMUTATIVE|FP_COMM and len(uses)==2:uses=tuple(sorted(uses))
                    elif op in FP_MADD_COMM and len(uses)==3:uses=tuple(sorted(uses[:2]))+uses[2:]
                    if kind=='term':term=(op,uses,succ);continue
                    addr=None
                    if kind in ('load','store'):
                        if 'index' in d:
                            # RA=0 denotes architectural zero; RB=0 still reads
                            # the actual r0 value. Indexed EAs are opaque SSA
                            # sums, so dynamic aliases never gain disjointness
                            # merely from different register spellings.
                            inputs=tuple(state[r] for r in (d['base'],d['index']) if r is not None)
                            ea=val(('indexed-EA',d['base'] is None,inputs))
                            if any(v in absolute_values for v in inputs):absolute_values.add(ea)
                            if any(v in strict_values for v in inputs):strict_values.add(ea)
                            addr=(ea,0,d['width'])
                        else:addr=(state[d['base']],d['offset'],d['width'])
                    def strict_address(address):
                        return address is not None and (address[0] in strict_values or any(str(address[1]).startswith(t+'@') or str(address[1])==t for t in strict_tokens))
                    loaded_device=False;loaded_strict=False
                    if kind=='load':
                        for written,device,strict in memory_taints:
                            # Different SSA bases may alias. Only equal-root,
                            # statically disjoint ranges can exclude a tainted
                            # store; overlapping/unknown slots inherit taint.
                            disjoint_slot=(addr is not None and written is not None and addr[0]==written[0] and isinstance(addr[1],int) and isinstance(written[1],int) and (addr[1]+addr[2]<=written[1] or written[1]+written[2]<=addr[1]))
                            if not disjoint_slot:loaded_device|=device;loaded_strict|=strict
                    if kind=='store' and uses and (uses[0] in absolute_values or uses[0] in strict_values):
                        memory_taints.append((addr,uses[0] in absolute_values,uses[0] in strict_values))
                    if addr is not None and domain_facts is not None:
                        if addr[0] in absolute_values:domain_facts.add('device-base')
                        if strict_address(addr):domain_facts.add('symbol-strict')
                    def disjoint(other):
                        if addr is None or other is None:return False
                        b,o,w=addr;b2,o2,w2=other
                        if memory_domain!='ordinary' or b in absolute_values or b2 in absolute_values or strict_address(addr) or strict_address(other):return False
                        return b==b2 and isinstance(o,int) and isinstance(o2,int) and (o+w<=o2 or o2+w2<=o)
                    deps=()
                    if kind in ('load','store','call'):
                        deps=tuple(sorted({v for k,a,v in mem if (kind=='load' and k=='load') or ((kind=='call' or k=='call' or kind=='store' or k=='store') and not disjoint(a))}))
                    const=d.get('const',())
                    if addr is not None:const=(d['offset'],d['width'],d.get('mode',()))
                    sig=val(('node',bi,op,uses,const,deps,fp if kind=='float' or d.get('fp_memory') else None));nodes[sig]+=1
                    if kind in ('load','store','call'):mem.append((kind,addr,sig))
                    if kind in ('float','call') or d.get('fp_memory'):fp=val(('fp-after',sig,fp))
                    for j,r in enumerate(d['def']):
                        if addr is not None and r==d['base'] and op.endswith('u'):
                            state[r]=val(('address-add',addr[0],addr[1]))
                            if addr[0] in absolute_values:absolute_values.add(state[r])
                            if strict_address(addr):strict_values.add(state[r])
                        elif kind=='call' and r not in ('r3','r4','f1'):state[r]=val(('call-clobber',side,sig,r))
                        else:
                            state[r]=val(('result',sig,j))
                            if kind=='pure' and not op.startswith('f') and (not uses or any(u in absolute_values for u in uses)):absolute_values.add(state[r])
                            if kind=='load' and loaded_device:absolute_values.add(state[r])
                            if (kind=='pure' and any(u in strict_values for u in uses)) or (kind=='load' and (strict_address(addr) or loaded_strict)):strict_values.add(state[r])
                needed=set().union(*(live[s] for s in succ)) if succ else set(rd.OBSERVE)
                boundary=tuple((r,state[r]) for r in sorted(needed));effects[bi]=tuple(sorted(v for _,_,v in mem));ends[bi]=state
                summaries.append((nodes,term,boundary,effects[bi],fp))
            return summaries
        if run(L,'target')!=run(R,'source'):raise rd.Unproved('value/effect DAG or live boundary mismatch')
        return True,'full acyclic CFG/value/effect DAG; ABI boundaries; every read; opaque call barriers; ordered FP events'
    except (rd.Unproved,ValueError,KeyError,IndexError,TypeError) as e:return False,str(e)


def frame_info(lines):
    """Typed direct-r1 slots; derived pointers/overlaps are deliberately unknown."""
    slots={};frame=None;escapes=[];issues=[]
    for i,line in enumerate(lines):
        op,a=parts(line)
        if re.search(r'\br1\b',line):
            allocation=i==0 and op=='stwu' and len(a)==2 and a[0]=='r1' and re.fullmatch(r'-?(?:0x[0-9a-f]+|\d+)\(r1\)',a[1])
            release=op=='addi' and len(a)==3 and a[:2]==['r1','r1']
            direct=op in MEM and len(a)==2 and a[0]!='r1' and re.fullmatch(r'-?(?:0x[0-9a-f]+|\d+)\(r1\)',a[1])
            if not (allocation or release or direct):issues.append('stack-address-escape-or-observation')
        if op=='stwu' and a and a[0]=='r1':
            m=re.fullmatch(r'(-?(?:0x[0-9a-f]+|\d+))\(r1\)',a[1]) if len(a)>1 else None
            if i!=0 or not m or int(m[1],0)>=0:issues.append('noncanonical frame allocation')
            else:frame=-int(m[1],0)
            continue
        if op=='addi' and len(a)==3 and a[1]=='r1' and a[0]!='r1':escapes.append((i,int(a[2],0)))
        if op=='mr' and len(a)==2 and a[1]=='r1' and a[0]!='r1':issues.append('derived stack base')
        if len(a)<2:continue
        m=re.fullmatch(r'(-?(?:0x[0-9a-f]+|\d+))\(r1\)',a[1])
        if not m:continue
        if op not in MEM:issues.append('unsupported stack access '+op);continue
        off=int(m[1],0);w=MEM[op];typ='float' if op in ('lfs','lfd','stfs','stfd') else 'integer'
        # Signedness is part of the operation, and is checked by the DAG as well.
        key=(off,w,typ);slots.setdefault(key,[]).append((i,op,a[0]))
    keys=list(slots)
    for i,(o,w,t) in enumerate(keys):
        for p,z,u in keys[i+1:]:
            if o<p+z and p<o+w:issues.append('stack slot width/type overlap')
    return dict(frame=frame,slots=slots,escapes=escapes,issues=issues)


def prepare_frame(lines):
    """Prove a fixed, unescaped straight-line frame before abstracting it.

    Supports only direct typed slots. LR/nonvolatile save pairs are removed only
    after entry-value save, unique private slot, and terminal restore checks.
    No frame-changing proof with branches, unknown instructions, or out-params.
    """
    info=frame_info(lines);frame=info['frame'];restored=set();removed=set();tags=[]
    if info['issues']:raise rd.Unproved('; '.join(info['issues']))
    if frame is None:
        if info['slots']:raise rd.Unproved('unproved incoming-stack slot remap')
        return list(lines),restored,info
    if frame<16 or frame%16:raise rd.Unproved('frame size/alignment')
    if info['escapes']:raise rd.Unproved('unproved stack out-parameter extent')
    for key,accesses in info['slots'].items():
        initialized=False
        for _,op,_ in accesses:
            if op.startswith('st'):initialized=True
            elif not initialized:raise rd.Unproved('uninitialized-stack-read')
    decoded=[decode(s) for s in lines]
    if not decoded or decoded[-1]['op']!='blr':raise rd.Unproved('frame does not terminate in plain return')
    if any(d['kind']=='term' for d in decoded[:-1]):raise rd.Unproved('frame-changing CFG proof unsupported')
    releases=[i for i,d in enumerate(decoded) if d['op']=='addi' and d['args'][:2]==['r1','r1']]
    if len(releases)!=1:raise rd.Unproved('missing/ambiguous frame release')
    release=releases[0]
    if int(decoded[release]['args'][2],0)!=frame:raise rd.Unproved('frame release size mismatch')
    if any(d['kind']!='term' for d in decoded[release+1:]):raise rd.Unproved('instructions after frame release')
    for i,d in enumerate(decoded):
        if 'r1' in d['def'] and i not in (0,release):raise rd.Unproved('unsupported stack pointer definition')
        if 'r1' in d['use'] and d['kind'] not in ('load','store','call','term') and i!=release:raise rd.Unproved('derived/observed stack pointer')
    removed.update([0,release]);lastcall=max([i for i,d in enumerate(decoded) if d['kind']=='call'] or [-1])
    # LR return-address save has exact same slot/width; no inferred callee extent.
    lr_saves=[i for i,d in enumerate(decoded) if d['op']=='mflr']
    if lr_saves:
        if len(lr_saves)!=1:raise rd.Unproved('ambiguous LR save')
        mi=lr_saves[0];reg=decoded[mi]['args'][0]
        candidates=[(key,uses) for key,uses in info['slots'].items() if key==(frame+4,4,'integer')]
        if len(candidates)!=1:raise rd.Unproved('LR linkage slot missing')
        key,uses=candidates[0]
        if len(uses)!=2 or [x[1] for x in uses]!=['stw','lwz']:raise rd.Unproved('LR linkage slot aliased')
        si,_,sv=uses[0];li,_,lv=uses[1]
        if sv!=reg or not mi<si<li or li<=lastcall:raise rd.Unproved('LR save/restore order')
        mts=[i for i,d in enumerate(decoded) if i>li and d['op']=='mtlr' and d['args']==[lv]]
        if len(mts)!=1:raise rd.Unproved('LR terminal restore missing')
        mt=mts[0]
        # No use of saved LR in another expression before it is overwritten.
        for i in range(mi+1,si):
            if reg in decoded[i]['use'] or reg in decoded[i]['def']:raise rd.Unproved('LR temporary has other prologue use')
        for i in range(si+1,len(decoded)):
            if reg in decoded[i]['def']:break
            if reg in decoded[i]['use']:raise rd.Unproved('LR temporary escapes after save')
        for i in range(li+1,mt):
            if lv in decoded[i]['use'] or lv in decoded[i]['def']:raise rd.Unproved('LR restore temporary modified')
        for d in decoded[mt+1:]:
            if lv in d['def']:break
            if lv in d['use']:raise rd.Unproved('LR restored temporary has another live use')
        if any('lr' in d['def'] for d in decoded[mt+1:]):raise rd.Unproved('LR changed after restore')
        removed.update([mi,si,li,mt]);restored.add('lr')
    elif lastcall>=0:raise rd.Unproved('calling frame lacks LR restoration proof')
    # Drop only whole unique entry-register save/terminal-restore slot pairs.
    for key,uses in info['slots'].items():
        if key==(frame+4,4,'integer'):continue
        off,width,typ=key
        if not 8<=off or off+width>frame:raise rd.Unproved('stack slot outside private frame')
        if len(uses)!=2:continue
        (si,so,sr),(li,lo,lr)=uses
        if (so,lo) not in [('stw','lwz'),('stfd','lfd')] or sr!=lr:continue
        if not re.fullmatch(r'(?:r(?:1[4-9]|2\d|3[01])|f(?:1[4-9]|2\d|3[01]))',sr):continue
        if any(sr in d['def'] for d in decoded[:si]):continue
        if li<=lastcall or any(sr in d['use'] or sr in d['def'] for d in decoded[li+1:-1]):continue
        removed.update([si,li]);restored.add(sr)
    # The return value cannot contain any private frame address: all address
    # formation was rejected above, including copied r1 and addi out-parameters.
    kept=[line for i,line in enumerate(lines) if i not in removed]
    return kept,restored,info


def expand_gpr_helpers(lines,context):
    """Expand only original-DOL-validated helper entries in terminal fixed frames."""
    helper_sites={}
    for i,line in enumerate(lines):
        op,a=parts(line)
        if op not in ('bl','b') or len(a)!=1:continue
        name=a[0].removeprefix('CALLEE_')
        h=rd.gpr_helper(name)
        if h:helper_sites[i]=(name,h)
    if not helper_sites:return list(lines),False
    if not context.get('referents_bound'):raise rd.Unproved('helper referents unbound')
    root=Path(context.get('root',Path.cwd()));rd.validate_gpr_runtime(root)
    setups={}
    # Find the actual reaching r11 definition, admitting only straight-line
    # intervening operations that neither use nor clobber the private pointer.
    # Calls clobber r11, and any branch/join needs a separate CFG proof.
    for i in helper_sites:
        if parts(lines[i])[0]!='bl':raise rd.Unproved('unproved tail helper transfer')
        for j in range(i-1,-1,-1):
            d=decode(lines[j])
            if d['kind']=='term' or 'r1' in d['def']:raise rd.Unproved('helper setup crosses control/frame change')
            if 'r11' in d['def']:
                op,a=parts(lines[j])
                if op!='addi' or len(a)!=3 or a[:2]!=['r11','r1']:raise rd.Unproved('helper reaching r11 definition is not frame end')
                setups[i]=j;break
            if 'r11' in d['use']:raise rd.Unproved('helper frame pointer escapes before helper')
        if i not in setups:raise rd.Unproved('helper without reaching r11 setup')
    skip=set(setups.values())
    info=frame_info([s for i,s in enumerate(lines) if i not in skip]);frame=info['frame']
    if frame is None:raise rd.Unproved('helper without fixed frame')
    out=[]
    # Helpers use r11 only as the proven stack-frame end. That pointer may not
    # escape through a later use before an explicit redefinition.
    for i,(name,(direction,start)) in helper_sites.items():
        if int(parts(lines[setups[i]])[1][2],0)!=frame:raise rd.Unproved('helper frame-end displacement mismatch')
        for following in lines[i+1:]:
            d=decode(following)
            if 'r11' in d['def']:break
            if 'r11' in d['use']:raise rd.Unproved('helper stack address has another live use')
    captures=[i for i,s in enumerate(lines) if parts(s)[0]=='mflr']
    if len(captures)!=1 or captures[0]>=min(helper_sites):raise rd.Unproved('helper changes LR before unique entry-LR capture')
    lr_register=parts(lines[captures[0]])[1][0]
    linkage=[]
    for i,line in enumerate(lines):
        op,a=parts(line)
        if op in ('stw','lwz') and len(a)==2 and re.fullmatch(r'-?(?:0x[0-9a-f]+|\d+)\(r1\)',a[1]) and int(a[1][:-4],0)==frame+4:linkage.append((i,op,a[0]))
    if len(linkage)!=2 or linkage[0][1]!='stw' or linkage[0][2]!=lr_register or linkage[0][0]>=min(helper_sites) or linkage[1][1]!='lwz' or linkage[1][0]<=max(helper_sites):raise rd.Unproved('entry-LR linkage store/restore does not enclose all helpers')
    restores=[i for i,s in enumerate(lines) if parts(s)[0]=='mtlr']
    if not restores or max(helper_sites)>=max(restores):raise rd.Unproved('helper link not restored after last helper')
    for i,line in enumerate(lines):
        if i in skip:continue
        if i in helper_sites:
            _,(direction,start)=helper_sites[i]
            out.extend(f'{"stw" if direction=="save" else "lwz"} r{n}, {frame-4*(32-n)}(r1)' for n in range(start,32))
        else:out.append(line)
    return out,True


def normalize_frames(left,right,context):
    empty=(set(),set());tags=[]
    try:
        original_left,original_right=left,right
        left,hl=expand_gpr_helpers(left,context);right,hr=expand_gpr_helpers(right,context)
    except (rd.Unproved,ValueError,KeyError,IndexError) as e:return left,right,tags,str(e),empty
    A,B=frame_info(left),frame_info(right)
    if A['issues'] or B['issues']:return left,right,tags,'; '.join(A['issues']+B['issues']),empty
    # Exact slot lists/frame need no abstraction; their ordinary effects remain.
    if not hl and not hr and A['frame']==B['frame'] and set(A['slots'])==set(B['slots']):return left,right,tags,None,empty
    try:
        L,ls,ai=prepare_frame(left);R,rs,bi=prepare_frame(right)
        if (hl and 'lr' not in ls) or (hr and 'lr' not in rs):raise rd.Unproved('helper LR restoration not established')
        aa,bb=frame_info(L),frame_info(R)
        ka=sorted(aa['slots'],key=lambda k:aa['slots'][k][0][0]);kb=sorted(bb['slots'],key=lambda k:bb['slots'][k][0][0])
        if len(ka)!=len(kb):raise rd.Unproved('unmapped non-padding live stack slots')
        for a,b in zip(ka,kb):
            if a[1:]!=b[1:]:raise rd.Unproved('mapped stack slot size/type mismatch')
            if [x[1] for x in aa['slots'][a]]!=[x[1] for x in bb['slots'][b]]:raise rd.Unproved('mapped stack slot access contract mismatch')
        def rewrite(lines,keys):
            mapped={k[0]:8+16*i for i,k in enumerate(keys)};out=[]
            for line in lines:
                op,a=parts(line)
                if len(a)>=2:
                    m=re.fullmatch(r'(-?(?:0x[0-9a-f]+|\d+))\(r1\)',a[1])
                    if m:a[1]=str(mapped[int(m[1],0)])+'(r1)';line=op+' '+', '.join(a)
                out.append(line)
            return out
        if hl or hr:tags.append('validated-gpr-helper-expansion')
        tags.append('typed-stack-bijection')
        if A['frame']!=B['frame']:
            tags.append('frame-shrunk' if (B['frame'] or 0)<(A['frame'] or 0) else 'frame-grown-private-padding')
        return rewrite(L,ka),rewrite(R,kb),tags,None,(ls,rs)
    except (rd.Unproved,ValueError,KeyError,IndexError) as e:return left,right,tags,str(e),empty


def row_subclasses(a,b):
    if a==b:return []
    oa,aa=parts(a or '');ob,ab=parts(b or '')
    load=lambda o:o.startswith(('lbz','lhz','lha','lwz','lfs','lfd','psq_l'))
    if not a or not b:return ['missing-load'] if load(oa or ob) else ['instruction-count']
    if oa in ('bl','bctrl','blrl') or ob in ('bl','bctrl','blrl'):return ['callee-or-call-order']
    if oa.startswith('f') or ob.startswith('f'):
        if oa==ob and oa in FP_COMM and len(aa)==3 and len(ab)==3 and aa[0]==ab[0] and aa[1:]==ab[:0:-1]:return ['fp-commuted']
        if oa==ob and oa in FP_MADD_COMM and len(aa)==4 and len(ab)==4 and aa[0]==ab[0] and aa[1:3]==ab[2:0:-1] and aa[3]==ab[3]:return ['fp-commuted']
        if oa!=ob:return ['fp-precision-fusion-conversion']
        return ['fp-expression-or-register']
    if oa in MEM or ob in MEM:
        if 'LIT_' in (a or '') or 'LIT_' in (b or ''):
            la=re.findall(r'LIT_\w+?_[0-9a-f]+',a or '');lb=re.findall(r'LIT_\w+?_[0-9a-f]+',b or '')
            if la!=lb:return ['literal-value']
        if oa!=ob:return ['memory-width-signedness']
        ma=re.fullmatch(r'(.+)\((r\d+)\)',aa[1]) if len(aa)>1 else None
        mb=re.fullmatch(r'(.+)\((r\d+)\)',ab[1]) if len(ab)>1 else None
        if ma and mb and ma[1]!=mb[1]:return ['stack-layout' if ma[2]==mb[2]=='r1' else 'memory-offset']
    if oa.startswith(('cmp','cmpl')) or ob.startswith(('cmp','cmpl')):return ['compare-immediate-or-signedness']
    if oa.startswith('b') or ob.startswith('b'):return ['control-flow']
    return ['register-allocation-or-instruction']


def aligned_rows(left,right):
    out=[]
    # All rows are retained; this is diagnostic alignment, never proof input.
    for tag,a,z,b,y in difflib.SequenceMatcher(a=left,b=right,autojunk=False).get_opcodes():
        if tag=='equal':continue
        for i,j in itertools.zip_longest(range(a,z),range(b,y)):
            l=left[i] if i is not None else None;r=right[j] if j is not None else None
            out.append(dict(target_offset=None if i is None else 4*i,source_offset=None if j is None else 4*j,target=l,source=r,subclasses=row_subclasses(l,r)))
    return out


def classify_lines(target:list[str],source:list[str],context=None):
    context=context or {};rows=aligned_rows(target,source);tags=[];findings=[]
    # Referents cannot be certified from their spelling or equal zero ELF fields.
    unresolved=[]
    for side,lines in [('target',target),('source',source)]:
        for i,line in enumerate(lines):
            op,a=parts(line)
            if '@' in line or (op in ('bl','b') and a and a[0].startswith('CALLEE_')) or (op=='bl' and a):
                if not context.get('referents_bound'):unresolved.append(dict(subclass='unresolved-referent',target_offset=i*4 if side=='target' else None,source_offset=i*4 if side=='source' else None,reason='formatted referent lacks binary binding'))
    findings.extend(context.get('findings',[]));findings.extend(unresolved)
    L,R,ft,frame_error,restored=normalize_frames(target,source,context);tags.extend(ft)
    if frame_error:findings.append(dict(subclass='uninitialized-stack-read' if 'uninitialized-stack-read' in frame_error else 'stack-contract-unknown',target_offset=None,source_offset=None,reason=frame_error))
    domain_facts=set()
    if context.get('volatile_members'):
        context=dict(context,memory_domain='strict');domain_facts.add('volatile-member-conservative-fallback')
    ok,reason=checked_graph(L,R,restored,context.get("memory_domain","strict"),context.get('strict_tokens',()),domain_facts) if not findings else (False,'binding/stack obligations unproved')
    # Extra reads are preserved by the DAG, including dead/MMIO reads. Split
    # repeated loads by the intervening effect rather than calling them regalloc.
    def loads(lines):
        out=collections.defaultdict(list)
        for i,line in enumerate(lines):
            op,a=parts(line)
            if op in MEM and not op.startswith('st') and len(a)>1:out[(op,a[1])].append(i)
        return out
    al,bl=loads(target),loads(source)
    if not ok and sum(map(len,al.values()))!=sum(map(len,bl.values())):
        for key in set(al)|set(bl):
            if len(al[key])==len(bl[key]):continue
            side,lines,indices=('target',target,al[key]) if len(al[key])>len(bl[key]) else ('source',source,bl[key])
            for n,i in enumerate(indices):
                previous=indices[n-1] if n else None
                between=lines[previous+1:i] if previous is not None else []
                sub='reload-after-call' if any(parts(s)[0] in ('bl','bctrl','blrl') for s in between) else 'reload-after-store' if any(parts(s)[0].startswith('st') for s in between) else 'missing-load'
                findings.append(dict(subclass=sub,target_offset=4*i if side=='target' else None,source_offset=4*i if side=='source' else None,reason='extra observed read; equality not proved'))
    helper=lambda lines:[re.search(r'_(?:save|rest)gpr_(\d+)',s).group(1) for s in lines if re.search(r'_(?:save|rest)gpr_(\d+)',s)]
    if helper(target)!=helper(source):
        tags.append('savegpr-shift')
        if any(f['subclass'].startswith('reload-') for f in findings):tags.append('savegpr-shift+reload')
    if not ok:
        unsupported=re.search(r'unsupported opcode (\S+)',reason)
        if unsupported:findings.append(dict(subclass='unsupported-opcode:'+unsupported[1],target_offset=None,source_offset=None,reason=reason))
        diagnostic_only={'register-allocation-or-instruction','fp-expression-or-register','instruction-count','control-flow','missing-load'}
        for r in rows:
            for sub in r['subclasses']:
                if sub in diagnostic_only:continue
                fact=dict(subclass=sub,target_offset=r['target_offset'],source_offset=r['source_offset'],reason=reason)
                if sub=='literal-value':
                    fact['target_literal']=re.findall(r'LIT_\w+?_[0-9a-f]+',r['target'] or '')
                    fact['source_literal']=re.findall(r'LIT_\w+?_[0-9a-f]+',r['source'] or '')
                findings.append(fact)
        if not findings:
            findings=[dict(subclass='value-dataflow-unproved',target_offset=r['target_offset'],source_offset=r['source_offset'],reason=reason) for r in rows]
        if not findings:findings=[dict(subclass='unknown-semantics',target_offset=None,source_offset=None,reason=reason)]
    return dict(classification='ABI' if ok else 'SEMANTIC',memory_domain=context.get('memory_domain','strict'),memory_domains_encountered=sorted(domain_facts),abi_subclasses=sorted(({s for r in rows for s in r['subclasses']}|set(tags)) if ok else set()),proof=reason if ok else None,reason=reason,differences=rows,findings=findings,subclasses=sorted({f['subclass'] for f in findings}),tags=sorted(set(tags)),scope='bounded mechanical function proof; unknown is SEMANTIC')


class BinaryUnproved(ValueError):pass


def require(condition,why):
    if not condition:raise BinaryUnproved(why)


def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()


class Image:
    """Own ELF locations; never treats a zero relocation placeholder as address."""
    def __init__(self,path):
        import callaudit
        self.path=Path(path);self.elf=callaudit.ELF(path);self.symbols=collections.defaultdict(list)
        e=self.elf
        for si,s in enumerate(e.s):
            if s[1]!=2:continue
            strings=e.c[s[6]]
            for off in range(0,s[5],s[9]):
                n,a,z,info,_,sec=struct.unpack_from('>IIIBBH',e.c[si],off)
                end=strings.find(b'\0',n);require(end>=0,'unterminated symbol')
                name=strings[n:end].decode();sym=(name,a,z,sec)
                if sym not in self.symbols[name]:self.symbols[name].append(sym)
    def unique(self,name):
        ss=self.symbols.get(name,[]);require(len(ss)==1,'missing/duplicate owning ELF symbol '+name);return ss[0]


class Program:
    def __init__(self,root):
        self.root=Path(root)
        self.dol=(self.root/'orig/SB4E01/sys/main.dol').read_bytes();self.catalog=collections.defaultdict(set);self.bases={}
        for line in (self.root/'config/SB4E01/symbols.txt').read_text().splitlines():
            m=re.match(r'(.+?) = (\.\w+):0x([0-9a-fA-F]+);',line)
            if m:self.catalog[m[1]].add((m[2],int(m[3],16)))
        owner=None
        for line in (self.root/'config/SB4E01/splits.txt').read_text().splitlines():
            if line and not line[0].isspace() and line.endswith(':'):owner=re.sub(r'\.(?:cpp|c)$','',line[:-1])
            m=re.match(r'\s+(\.\w+)\s+start:0x([0-9a-fA-F]+)',line)
            if m and owner:self.bases[(owner,m[1])]=int(m[2],16)


class BinaryContext:
    def __init__(self,root,unit,target,source,program=None):
        self.root=Path(root);self.unit=unit.removeprefix('main/');self.T=Image(target);self.S=Image(source)
        program=program or Program(root)
        self.dol,self.catalog,self.bases=program.dol,program.catalog,program.bases
        self.references=[]
    def read_dol(self,a,z):
        import callaudit
        b=callaudit.read_dol(self.dol,a,z);require(b is not None and len(b)==z,'non-DOL-backed target span');return b
    def target_address(self,sym):
        n,a,z,sec=sym;e=self.T.elf
        if 0<sec<len(e.s):
            base=self.bases.get((self.unit,e.names[sec]));require(base is not None,'missing owning target section range');return e.names[sec],base+a
        values=self.catalog.get(n,set());require(len(values)==1,'ambiguous/missing target external '+n);return next(iter(values))
    def source_address(self,sym):
        n,a,z,sec=sym;e=self.S.elf
        if not 0<sec<len(e.s):
            values=self.catalog.get(n,set());require(len(values)==1,'ambiguous/missing source external '+n);return next(iter(values))
        section=e.names[sec]
        if section in ('.text','.init'):
            ts=self.T.unique(n);tsc,addr=self.target_address(ts);require(tsc==section,'source callee section mismatch');return section,addr
        # Data addresses are locations, not values or name correspondences. Exact
        # own-TU section+offset only; moved pools require a separate audited proof.
        base=self.bases.get((self.unit,section));require(base is not None,'missing source owning section range');return section,base+a
    def source_bytes(self,sym,add,size):
        _,a,_,sec=sym;e=self.S.elf
        require(0<sec<len(e.s) and e.s[sec][1]!=8,'literal not own file-backed source section')
        at=a+add;require(at>=0 and at+size<=len(e.c[sec]),'source literal bounds')
        require(not any(s==sec and at<=o<at+size for (s,o) in e.rel),'relocated bytes are not scalar literal')
        return e.c[sec][at:at+size]
    def target_binding(self,name):
        e=self.T.elf;_,start,size,sec=e.f[name];section,base=self.target_address((name,start,size,sec))
        require(size%4==0 and start+size<=len(e.c[sec]),'invalid target function extent')
        raw=self.read_dol(base,size);refs=[]
        for off in range(0,size,4):
            w=int.from_bytes(e.c[sec][start+off:start+off+4],'big');actual=int.from_bytes(raw[off:off+4],'big');r=e.rel.get((sec,start+off));expected=w
            if r:
                typ,sym,add=r;sc,addr=self.target_address(sym);addr=(addr+add)&0xffffffff
                if typ==10:expected=(w&0xfc000003)|((addr-base-off)&0x3fffffc)
                elif typ==4:expected=(w&0xffff0000)|(addr&0xffff)
                elif typ==5:expected=(w&0xffff0000)|((addr>>16)&0xffff)
                elif typ==6:expected=(w&0xffff0000)|(((addr+0x8000)>>16)&0xffff)
                elif typ==109:
                    ra=(actual>>16)&31;require(ra in (2,13),'unknown SDA base');sda={2:0x807deca0,13:0x807d7320}[ra];delta=addr-sda;require(-32768<=delta<32768,'SDA address outside signed range')
                    expected=(w&0xffe00000)|(actual&0x1f0000)|(delta&0xffff)
                else:raise BinaryUnproved('unsupported target instruction relocation '+str(typ))
                refs.append(dict(offset=off,type=typ,section=sc,address=hex(addr),symbol=sym[0]))
            require(expected==actual,'original DOL instruction/reference mismatch at '+hex(off))
        return dict(address=hex(base),size=size,body_sha256=hashlib.sha256(raw).hexdigest(),references=refs)
    def exact_identity(self,name,left,right):
        # Referenced DATA identity/value is a separate mandatory semdata
        # obligation. This certificate proves instruction identity conditional
        # on that audit, and cannot close the whole function by itself.
        def data_obligations(lines):
            return [re.sub(r'(?:REF|LIT)_(?:rodata|sdata2|sbss2|sdata|sbss|data|bss)_[0-9a-fx]+','DATA_OBLIGATION',line) for line in lines]
        if data_obligations(left)!=data_obligations(right):return False
        A,B=self.T.elf,self.S.elf
        _,a,z,sc=A.f[name];_,b,y,ss=B.f[name]
        if z!=y:return False
        masks={10:0xfc000003,4:0xffff0000,5:0xffff0000,6:0xffff0000,109:0xffe00000}
        for off in range(0,z,4):
            ar=A.rel.get((sc,a+off));br=B.rel.get((ss,b+off))
            if bool(ar)!=bool(br) or (ar and ar[0]!=br[0]):return False
            mask=masks.get(ar[0],0) if ar else 0xffffffff
            if not mask:return False
            aw=int.from_bytes(A.c[sc][a+off:a+off+4],'big');bw=int.from_bytes(B.c[ss][b+off:b+off+4],'big')
            if aw&mask!=bw&mask:return False
        return True
    def normalize(self,name,records,side):
        image=self.T if side=='target' else self.S;e=image.elf;_,start,size,sec=e.f[name];lines=[];offsets=[];refs=[]
        for record in records:
            ins=record.get('instruction')
            if ins is None:continue
            at=int(ins.get('address','0'));off=at-start
            require(off==len(lines)*4 and int(ins['size'])==4,'objdiff instruction extent/order disagreement')
            text=ins.get('formatted','');op,args=parts(text);r=e.rel.get((sec,at))
            if r:
                typ,sym,add=r;literal=None;width=rd.WIDTH.get(op)
                # Own source literal bytes need no historical pool base. Never
                # apply this exception to address formation or writable storage.
                if side=='source' and width and not op.startswith('st') and 0<sym[3]<len(e.s) and e.names[sym[3]] in ('.sdata2','.rodata') and not e.s[sym[3]][2]&1:
                    section=e.names[sym[3]];literal=self.source_bytes(sym,add,width);addr=None
                else:
                    section,addr=self.target_address(sym) if side=='target' else self.source_address(sym);addr=(addr+add)&0xffffffff
                if typ==10:
                    require(op in ('bl','b'),'unsupported relocated control transfer')
                    helper=rd.gpr_helper(sym[0])
                    if helper:rd.validate_gpr_runtime(self.root);token=sym[0]
                    else:token='CALLEE_'+hex(addr)
                    text=op+' '+token
                else:
                    require(typ in (4,5,6,109),'unsupported source instruction relocation')
                    if literal is None and side=='target' and width and not op.startswith('st') and section in ('.sdata2','.rodata'):literal=self.read_dol(addr,width)
                    token='LIT_'+section.replace('.','')+'_'+literal.hex() if literal is not None else 'REF_'+section.replace('.','')+'_'+hex(addr)
                    suffix={4:'@l',5:'@h',6:'@ha',109:'@sda21'}[typ]
                    if typ==109:args[1]=token+suffix
                    else:
                        i=next((i for i,a in enumerate(args) if '@' in a),None);require(i is not None,'unidentified relocation operand')
                        base=re.search(r'\(r\d+\)$',args[i]);args[i]=token+suffix+(base[0] if base else '')
                    text=op+' '+', '.join(args)
                refs.append(dict(offset=off,kind=typ,section=section,address=hex(addr) if addr is not None else None,token=token,symbol=sym[0]))
            if not r and op=='bl':
                require(len(args)==1,'unresolved local BL');dest=int(args[0],0)
                candidates=[f for f in e.f.values() if f[3]==sec and f[1]==dest]
                require(len(candidates)==1,'unresolved/ambiguous unrelocated call')
                section,addr=self.target_address(candidates[0]) if side=='target' else self.source_address(candidates[0]);text='bl CALLEE_'+hex(addr)
            elif not r and args:
                word=int.from_bytes(e.c[sec][at:at+4],'big');major=word>>26
                if major in (16,18):
                    bits=16 if major==16 else 26;disp=word&((1<<bits)-4)
                    if disp&(1<<(bits-1)):disp-=1<<bits
                    dest=disp&0xffffffff if word&2 else (at+disp)&0xffffffff
                    require(not word&2,'absolute unrelocated branch identity unknown')
                    args[-1]=hex(dest-start);text=op+' '+', '.join(args)
            lines.append(text);offsets.append(off)
        require(len(lines)*4==size,'incomplete function instruction stream')
        return lines,offsets,refs


def diagnostic_rows(ctx,symbol,target,source):
    out=[]
    for tr,sr in itertools.zip_longest(target,source,fillvalue={}):
        if 'diff_kind' not in tr and 'diff_kind' not in sr:continue
        ti,si=tr.get('instruction'),sr.get('instruction')
        out.append(dict(target_offset=int(ti.get('address','0'))-ctx.T.elf.f[symbol][1] if ti else None,source_offset=int(si.get('address','0'))-ctx.S.elf.f[symbol][1] if si else None,target=ti.get('formatted') if ti else None,source=si.get('formatted') if si else None,target_diff_kind=tr.get('diff_kind'),source_diff_kind=sr.get('diff_kind'),subclasses=row_subclasses(ti.get('formatted') if ti else None,si.get('formatted') if si else None)))
    return out


def scan_unit(root,u,work,semdata=None,min_match=90,program=None,semdomain=None):
    unit=u['name'].removeprefix('main/');out=dict(unit=unit,functions=[],errors=[],source_only_advisories=[])
    paths=[Path(root)/u.get(k,'') for k in ('target_path','base_path')]
    if not all(p.is_file() for p in paths):out['errors'].append('missing target/source object');return out
    pins={str(p):sha(p) for p in paths};out['object_sha256']=pins;out['audit_pins']=dict(pins)
    try:
        ctx=BinaryContext(root,unit,*paths,program=program)
        domain=semdomain.audit_unit(Path(root),unit,*paths) if semdomain else dict(domain='strict',output_domain='strict-unit',complete=False,evidence=['domain helper missing; strict fallback'],pins={})
        out['domain']=domain;out['audit_pins'].update(domain.get('pins',{}))
        data=semdata.audit_unit(Path(root),unit,*paths) if semdata else None
        if data:out['data_stats']=data.get('stats');out['audit_pins'].update(data.get('pins',{}))
        dest=Path(work)/(unit.replace('/','__')+'.json');dest.parent.mkdir(parents=True,exist_ok=True)
        result=subprocess.run([str(Path(root)/'build/tools/objdiff-cli'),'-L','error','diff','-1',str(paths[0]),'-2',str(paths[1]),'-o',str(dest)],capture_output=True,text=True)
        require(result.returncode==0,'objdiff failed: '+result.stderr[-300:]);j=json.loads(dest.read_text());out['full_objdiff_json']=str(dest)
        left=j['left']['symbols'];right=j['right']['symbols'];reported=set()
        for t in left:
            symbol=t['name'];records=t.get('instructions',[]);percent=t.get('match_percent');dd=data.get('functions',{}).get(symbol) if data else None
            code_selected=bool(records and percent is not None and float(percent)>=min_match)
            if not code_selected and not (dd and (dd.get('findings') or not dd.get('complete'))):continue
            row=dict(function=symbol,volatile_members=domain.get('volatile_members',[]),match_percent=percent,code_selected=code_selected,data_complete=bool(dd and dd.get('complete')),memory_domain=domain.get('domain') if domain.get('complete') else 'strict',domain=domain.get('output_domain','strict-unit'),differences=[],findings=[],subclasses=[],tags=[])
            candidates=[s for s in right if s['name']==symbol and s.get('instructions')]
            if code_selected:
                if len(candidates)!=1:
                    row.update(code_classification='UNKNOWN',classification='SEMANTIC',reason='exact unit-scoped function pairing unavailable')
                    row['findings'].append(dict(subclass='missing-or-ambiguous-pair',target_offset=None,source_offset=None,reason=row['reason']))
                else:
                    s=candidates[0];row['objdiff_differing_rows']=diagnostic_rows(ctx,symbol,records,s['instructions'])
                    try:
                        binding=ctx.target_binding(symbol);L,lo,lr=ctx.normalize(symbol,records,'target');R,ro,rr=ctx.normalize(symbol,s['instructions'],'source')
                        names=[v['name'] for v in domain.get('strict_symbols',[])]
                        strict_tokens={v['token'] for v in lr+rr if any(v['symbol']==n or v['symbol'].startswith(n+'__') for n in names)}
                        proof_context=dict(referents_bound=True,memory_domain='strict' if domain.get('volatile_members') else row['memory_domain'],strict_tokens=sorted(strict_tokens),volatile_members=domain.get('volatile_members',[]),root=str(root))
                        if ctx.exact_identity(symbol,L,R):
                            cr=dict(classification='ABI',proof='exact complete instruction identity; bound code referents/branch coordinates; DATA referents remain mandatory separate obligations',reason='exact-identity',subclasses=[],findings=[],differences=[],tags=[],abi_subclasses=['exact-identity'])
                            row['no_code_difference']=True
                        else:cr=classify_lines(L,R,proof_context);row['no_code_difference']=False
                        row.update(cr,code_classification=cr['classification'],code_reason=cr['reason'],target_binding=binding,referents=dict(target=lr,source=rr))
                    except (BinaryUnproved,rd.Unproved,ValueError,KeyError,IndexError) as e:
                        row.update(code_classification='UNKNOWN',classification='SEMANTIC',reason=str(e))
                        row['findings'].append(dict(subclass='binding-or-semantics-unknown',target_offset=None,source_offset=None,reason=str(e)))
            else:row.update(code_classification='NOT-SELECTED',classification='ABI',reason='code below selection; data audit only')
            if dd:row['findings'].extend(dd.get('findings',[]))
            if not dd or not dd.get('complete'):
                row['findings'].append(dict(subclass='data-unverified',target_offset=None,source_offset=None,reason='all-function data obligations incomplete/missing'))
            if row['findings']:row['classification']='SEMANTIC'
            row['subclasses']=sorted({f['subclass'] for f in row['findings']});out['functions'].append(row);reported.add(symbol)
        # Include data-only target functions with no objdiff instruction records,
        # and keep source-only functions separate without inventing a target ID.
        for symbol,dd in (data or {}).get('functions',{}).items():
            if dd.get('target_size') is None:
                out['source_only_advisories'].append(dict(function=symbol,policy='parent decides whether source-only advisories gate',**dd));continue
            if symbol in reported or (dd.get('complete') and not dd.get('findings')):continue
            ff=list(dd.get('findings',[]))
            if not dd.get('complete'):ff.append(dict(subclass='data-unverified',target_offset=None,source_offset=None,reason='no instruction-bearing objdiff row; data audit incomplete'))
            out['functions'].append(dict(function=symbol,code_selected=False,code_classification='NOT-SELECTED',classification='SEMANTIC',findings=ff,subclasses=sorted({v['subclass'] for v in ff}),data_complete=dd.get('complete',False),differences=[],tags=[],domain=row.get('domain') if out['functions'] else domain.get('output_domain')))
    except Exception as e:out['errors'].append(type(e).__name__+': '+str(e))
    changed=[p for p,h in pins.items() if not Path(p).is_file() or sha(p)!=h]
    if changed:
        out['unstable_objects']=changed
        for row in out['functions']:row['classification']='SEMANTIC';row.setdefault('findings',[]).append(dict(subclass='unstable-input',target_offset=None,source_offset=None,reason='object changed during scan'))
    return out


def load_module(path,name):
    if not path:return None
    spec=importlib.util.spec_from_file_location(name,path);module=importlib.util.module_from_spec(spec);sys.modules[name]=module;spec.loader.exec_module(module);return module


def read_code_proofs(paths,census=None,census_complete=False):
    """Only trusted user-selected files; no data waiver syntax exists."""
    approved={};ignored=[];pins={}
    for path in paths:
        path=Path(path).resolve();pins[str(path)]=sha(path)
        for line_number,line in enumerate(path.read_text().splitlines(),1):
            if not line.strip() or line.lstrip().startswith('#'):continue
            scoped=re.match(r'^@(\S+)\s+(\S+)\s+#\s*proof:\s*(\S.*)$',line.strip())
            bare=re.match(r'^(\S+)\s+#\s*proof:\s*(\S.*)$',line.strip()) if not scoped else None
            record=dict(file=str(path),line=line_number,text=line.strip())
            if scoped:
                unit=re.sub(r'\.(?:cpp|c)$','',scoped[1].removeprefix('main/'));key=(unit,scoped[2])
            elif bare:
                owners=(census or {}).get(bare[1],set())
                if not census_complete or len(owners)!=1:
                    ignored.append(dict(**record,reason='bare name lacks complete unique whole-program unit census'));continue
                key=(next(iter(owners)),bare[1])
            else:ignored.append(dict(**record,reason='no explicit scoped/unique proof contract'));continue
            approved.setdefault(key,[]).append(record)
    return approved,ignored,pins


def bare_proof_names(paths):
    names=set()
    for path in paths:
        for line in Path(path).read_text().splitlines():
            m=re.match(r'^(?!@)(\S+)\s+#\s*proof:\s*\S',line.strip())
            if m:names.add(m[1])
    return names


def whole_program_name_census(root,config,wanted):
    """All configured target ELFs, including units outside the selected lane."""
    census=collections.defaultdict(set);pins={};errors=[]
    if not wanted:return census,True,pins,errors
    for u in config['units']:
        path=Path(root)/u.get('target_path','')
        try:
            raw=path.read_bytes();pins[str(path.resolve())]=hashlib.sha256(raw).hexdigest()
            h=struct.unpack_from('>16sHHIIIIIHHHHHH',raw);require(h[0][:6]==b'\x7fELF\x01\x02','census ELF class')
            sections=[struct.unpack_from('>10I',raw,h[6]+i*h[11]) for i in range(h[12])]
            for s in sections:
                if s[1]!=2:continue
                require(s[9]==16 and s[5]%16==0,'census symbol table bounds');strings=sections[s[6]];blob=raw[strings[4]:strings[4]+strings[5]]
                for off in range(s[4],s[4]+s[5],16):
                    n,a,z,info,_,sec=struct.unpack_from('>IIIBBH',raw,off)
                    if info&15!=2 or not z or not 0<sec<len(sections):continue
                    end=blob.find(b'\0',n);require(end>=0,'census symbol string');name=blob[n:end].decode()
                    if name in wanted:census[name].add(u['name'].removeprefix('main/'))
        except (OSError,ValueError,struct.error,IndexError) as error:errors.append(dict(unit=u['name'],reason=str(error)))
    return census,not errors,pins,errors


def subtract_code_proofs(results,approved):
    matched=0
    for unit in results:
        for row in unit['functions']:
            row['classification_after_code_proofs']=row['classification'];row['code_classification_after_proofs']=row['code_classification']
            records=approved.get((unit['unit'],row['function']))
            if not row.get('code_selected') or row['code_classification'] not in ('SEMANTIC','UNKNOWN') or not records:continue
            row['audited_code_proofs']=records;row['code_classification_after_proofs']='EXEMPT';matched+=1
            # Data/unknown-data NEVER disappears under an old code proof.
            row['classification_after_code_proofs']='ABI' if row.get('data_complete') else 'SEMANTIC'
    return matched


def main():
    ap=argparse.ArgumentParser(description=__doc__,epilog='Audited --code-proofs subtract CODE findings only. Data findings/unknowns never disappear under a code proof; this version accepts ZERO data waivers. Raw and filtered counts are both retained; ambiguous bare names fail closed.')
    ap.add_argument('--root',type=Path,default=Path.cwd());ap.add_argument('--prefix',default='Game/Player/');ap.add_argument('--out',type=Path,required=True);ap.add_argument('--work',type=Path);ap.add_argument('--semdata',type=Path);ap.add_argument('--semdomain',type=Path);ap.add_argument('--fixtures',type=Path);ap.add_argument('--min-match',type=float,default=90);ap.add_argument('--code-proofs',type=Path,action='append',default=[],help='trusted audited code-proof file; @unit symbol # proof: contract; bare names require full unique target-unit census; NEVER waives data');a=ap.parse_args()
    root=a.root.resolve();a.out.parent.mkdir(parents=True,exist_ok=True);work=a.work or a.out.parent/(a.out.stem+'-pairs');work.mkdir(parents=True,exist_ok=True)
    semdata=load_module(a.semdata,'semdata');semdomain=load_module(a.semdomain,'semdomain')
    watched=[root/p for p in ['objdiff.json','config/SB4E01/symbols.txt','config/SB4E01/splits.txt','orig/SB4E01/sys/main.dol','build/tools/objdiff-cli']]+[Path(__file__),Path(rd.__file__),Path(semdecode.__file__),_ROOT_TOOLS/'callaudit.py']+[p for p in [a.semdata,a.semdomain] if p]
    pins={str(p.resolve()):sha(p) for p in watched};config=json.loads((root/'objdiff.json').read_text())
    cases=json.loads(a.fixtures.read_text()).get('cases',[]) if a.fixtures else []
    units=[dict(name='main/'+c['unit'],target_path=c['target_object'],base_path=c['source_object']) for c in cases] if cases else [u for u in config['units'] if u['name'].removeprefix('main/').startswith(a.prefix)]
    started=time.time();results=[];program=Program(root);conflicts=[]
    census,complete,cpins,cerrors=whole_program_name_census(root,config,bare_proof_names(a.code_proofs));pins.update(cpins)
    approved,ignored_proofs,proof_pins=read_code_proofs(a.code_proofs,census,complete);pins.update(proof_pins)
    for i,u in enumerate(units):
        result=scan_unit(root,u,work,semdata,0 if cases else a.min_match,program,semdomain);results.append(result)
        for p,h in result.get('audit_pins',{}).items():
            if p in pins and pins[p]!=h:conflicts.append(p)
            else:pins[p]=h
        if (i+1)%20==0:print(f'{i+1}/{len(units)} units',flush=True)
    # Recheck ALL encountered provider/header/object/config pins at END, not just
    # when the early unit happened to audit them.
    unstable=[p for p,h in pins.items() if not Path(p).is_file() or sha(p)!=h]+conflicts
    if unstable:
        for u in results:
            for row in u['functions']:row['classification']='SEMANTIC';row.setdefault('findings',[]).append(dict(subclass='unstable-input',target_offset=None,source_offset=None,reason='whole-run input pin changed'))
    proof_matches=subtract_code_proofs(results,{} if unstable else approved)
    rows=[r for u in results for r in u['functions']];counts=collections.Counter(r['classification'] for r in rows);dirs=collections.defaultdict(collections.Counter)
    for u in results:
        d='/'.join(u['unit'].split('/')[:2])
        for row in u['functions']:dirs[d][row['classification']]+=1
    payload=dict(schema_version=2,gate_ready=False,adversarial_signoff_required=True,scope='all paired >=90 without reg/label prefilter; ALL-function data prepass; source-only advisories separate',root=str(root),prefix=a.prefix,input_sha256=pins,unstable_inputs=sorted(set(unstable)),semdata_available=semdata is not None,semdomain_available=semdomain is not None,counts=dict(counts),code_counts=dict(collections.Counter(r.get('code_classification','UNKNOWN') for r in rows)),per_directory={d:dict(v) for d,v in dirs.items()},code_counts_after_proofs=dict(collections.Counter(r.get('code_classification_after_proofs','UNKNOWN') for r in rows)),counts_after_code_proofs=dict(collections.Counter(r['classification_after_code_proofs'] for r in rows)),code_proof_matches=proof_matches,ignored_proof_lines=ignored_proofs,proof_census_errors=cerrors,data_waivers_accepted=0,data_counts=dict(complete=sum(u.get('data_stats',{}).get('complete',0) for u in results),open_target=sum(u.get('data_stats',{}).get('semantic',0)-len(u['source_only_advisories']) for u in results),policy='data findings retain independent scope; observed unequal bytes are not confirmed consumer-semantic defects'),unresolved_units=sum(bool(u['errors']) for u in results),source_only_advisories=sum(len(u['source_only_advisories']) for u in results),units=results,elapsed_seconds=round(time.time()-started,3))
    if cases:
        payload['fixture_results']=[]
        for c,u in zip(cases,results):
            chosen=[r for r in u['functions'] if r['function']==c['symbol']];classification=chosen[0]['classification'] if len(chosen)==1 else 'UNRESOLVED'
            payload['fixture_results'].append(dict(id=c['id'],classification=classification,expected=c['expected_class'].upper(),passed=classification==c['expected_class'].upper(),function=c['symbol']))
    a.out.write_text(json.dumps(payload,indent=2)+'\n');print(json.dumps({k:payload[k] for k in ['counts','code_counts','unresolved_units','source_only_advisories','elapsed_seconds']}))


if __name__=='__main__':main()
