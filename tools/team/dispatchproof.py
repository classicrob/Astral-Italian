#!/usr/bin/env python3
"""Proof for a crossed-dispatch fix: the Wii objects change only where the dispatch oracle says they must.

hookproof.sh demands byte-identical objects, which a dispatch fix can never satisfy (a nerve's execute symbol
moves, a vtable relocation retargets).  This tool compares two COMPLETE source-object trees (before / after) and
accepts only:

  1. identical complete object inventories and no unreadable objects;
  2. identical ordered non-debug section contracts (including NOBITS sizes),
     symbol multisets (including unsized symbols), all stored bytes and all
     relocation records;
  3. mapped definitions may reorder only while preserving their entire image at
     each original address; unmapped storage, local/section targets and padding
     retain exact section placement;
  4. a relocation retarget requires an unambiguous BAD->MATCH candidate,
     independently named/chain-anchored targets, and unchanged type/addend.
     Direct and semantic candidates require the original operand at the same
     instruction. Aligned candidates additionally revalidate the masked-word
     equal block and its bounded shift; sinit reordering requires a pure ctor
     and original instance/ctor pair. Inference alone grants no exception.
     MATCH coverage cannot disappear or regress.

`--unit` scopes only --expect-clean, never the object comparison. Compiler
.line/.debug* and .comment bookkeeping is ignored; constructor/destructor and
exception tables are compared in order. --selftest runs repeatable real-object
negative controls in test_dispatchproof.py. This proof is deliberately conservative:
unsupported reorderings require review, rather than weakening the comparator.

Usage: dispatchproof.py --before ROOT --after ROOT [--unit Game/Boss/TwoLegs]... [--expect-clean] [--dol main.dol]
       dispatchproof.py --selftest --after ROOT
Exit 0 and prints `DISPATCH PROOF OK` only if every check passes.
"""
import argparse, bisect, collections, hashlib, json, pathlib, sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import dispatchcheck as dc  # noqa: E402

# Only compiler bookkeeping is ignored. Every other section, including NOBITS,
# .ctors/.dtors and exception tables, participates in the proof.
def ignored(e, sec):
    return not e.s[sec][2]&2 and (e.names[sec] in ('.line','.debug','.comment') or e.names[sec].startswith(('.debug_','.zdebug_')))


def section_ids(e):
    seen, out = collections.Counter(), {}
    for i, name in enumerate(e.names):
        out[i] = (name, seen[name])
        seen[name] += 1
    return out


def section_contract(e):
    ids = section_ids(e)
    return [(ids[i], s[1], s[2], s[3], s[5], s[8], s[9],
             ids.get(s[6]) if s[6] else None, s[7])
            for i, s in enumerate(e.s) if i and s[1] not in (2, 3, 4) and not ignored(e, i)]


def relocation_contract(e):
    ids = section_ids(e)
    return [(ids[i], s[1], s[2], s[5], s[8], s[9], ids.get(s[6]), ids.get(s[7]))
            for i, s in enumerate(e.s) if s[1] in (4, 9)
            and not (0 < s[7] < len(e.s) and ignored(e, s[7]))]


def symbol_contract(e, prog=None, unit=None):
    import struct
    ids, result = section_ids(e), collections.Counter()
    used={si for sec,o,typ,si,ad in e.relocs if not ignored(e,sec)}
    for table in e.s:
        if table[1] != 2:continue
        for idx, off in enumerate(range(0,table[5],16)):
            sym=e.syms[idx];sec=sym['sec']
            if 0<sec<len(e.s) and ignored(e,sec):continue
            other=struct.unpack_from('>B',e.raw,table[4]+off+13)[0]
            a=storage_addr(prog,e,unit,sym) if prog else None
            if prog and sec==0 and idx in used and target_key(prog,e,unit,idx,0)[0]=='A':
                continue
            if a is not None and sym['size'] and sym['type'] in (1,2):
                key=('A',a,sym['size'],sym['bind'],sym['type'],other)
            else:
                name='<anonymous>' if anonymous(sym) else sym['name']
                key=('N',name,sym['bind'],sym['type'],sym['size'],other,ids[sec] if 0<sec<len(e.s) else sec)
            result[key]+=1
    return result


def symbol_addr(prog, e, unit, sym):
    a = prog.sym_addr(e, unit, sym)
    if a is not None:
        return a
    chain=getattr(prog,'_proof_chain_addresses',{}).get(sym['name'])
    if chain is not None:return chain
    anchor=getattr(prog,'vtable_anchor',lambda n:None)(sym['name'])
    if anchor and anchor[1]=='vtable-ctor':return anchor[0]
    return getattr(prog,'_proof_addresses',{}).get((unit,sym['sec'],sym['off'],sym['name']))


def independent_name(prog,name):
    anchor=getattr(prog,'vtable_anchor',lambda n:None)(name)
    return (prog.sy.addr(name) is not None or name in prog._proof_chain_addresses
            or bool(anchor and anchor[1]=='vtable-ctor'))


def storage_addr(prog,e,unit,sym):
    legacy=getattr(prog,'_proof_legacy_addresses',{}).get(sym['name'])
    if legacy is not None:return legacy
    definition=getattr(prog,'_proof_definition_addresses',{}).get((unit,sym['sec'],sym['off'],sym['name']))
    if definition is not None:return definition
    sec=sym['sec']
    if 0<sec<len(e.s) and e.s[sec][1]==8:
        base=getattr(prog,'_proof_storage_bases',{}).get((unit,section_ids(e)[sec]))
        if base is not None:return base+sym['off']
    return symbol_addr(prog,e,unit,sym)


def anonymous(sym):
    return sym['bind'] == 0 and bool(dc.LOCALNAME.fullmatch(sym['name']))


def prepare_addresses(pb, pa):
    # Independent chain anchors are supplied by oracle v5. Consistent nearby
    # symbols also anchor otherwise unnamed storage in that SAME section.
    for prog in (pb, pa):
        prog._proof_addresses = {}
        prog._proof_definition_addresses = {}
        prog._proof_allowed_old=set()
        prog._proof_named={}
        prog._proof_chain_addresses={}
        prog._proof_legacy_addresses={}
        prog._proof_variants=set()
        prog._proof_image_bytes={}
        tried=set()
        for unit,e in prog.elfs.items():
            for idx,sym in enumerate(e.syms):
                name=sym['name']
                if name.startswith('sInstance__') and 'Nrv' in name and name not in tried and prog.sy.addr(name) is None:
                    tried.add(name)
                    addr,how=prog.target(e,unit,idx,0)
                    if how=='chain':prog._proof_chain_addresses[name]=addr
        for unit, e in prog.elfs.items():
            offsets = collections.defaultdict(set)
            for sym in e.syms:
                if sym['size'] and 0 < sym['sec'] < len(e.s):
                    a = symbol_addr(prog, e, unit, sym)
                    if a is not None:
                        offsets[sym['sec']].add(a - sym['off'])
            for sym in e.syms:
                sec = sym['sec']
                if not sym['size'] or not 0 < sec < len(e.s) or ignored(e, sec):
                    continue
                if prog.sym_addr(e, unit, sym) is not None or anonymous(sym):
                    continue
                choices = offsets[sec]
                base = prog.bases.get((unit, e.names[sec]))
                # Section offsets are authoritative only when owned by a split,
                # or unanimously anchored by the section's named BSS instances.
                a = None
                if e.s[sec][1] == 8 and len(choices) == 1:
                    a = next(iter(choices)) + sym['off']
                elif base is not None and e.names[sec] not in ('.text',):
                    a = base + sym['off']
                if a is not None:
                    prog._proof_addresses[unit, sec, sym['off'], sym['name']] = a
    # A newly named definition may already exist as an identical weak copy. It
    # gains an identity only when its complete relocated body independently
    # equals the DOL at that address, never just from a coincidental call operand.
    for prog, other in ((pb, pa), (pa, pb)):
        for unit, e in prog.elfs.items():
            for sym in e.syms:
                if not sym['size'] or sym['type'] not in (1,2) or prog.sym_addr(e,unit,sym) is not None:
                    continue
                a = other.sy.addr(sym['name'])
                if a is not None and exact_body(prog,e,unit,sym,a):
                    prog._proof_addresses[unit,sym['sec'],sym['off'],sym['name']] = a

    # BSS is zero-initialized storage; its layout is independent of the names
    # used to bind references. A phase with consistent independent anchors fixes
    # the shared section placement, even if the other phase names one cell wrong.
    bases={}
    for unit in set(pb.elfs)&set(pa.elfs):
        for prog in (pb,pa):
            e=prog.elfs[unit];candidates=collections.defaultdict(set)
            for sym in e.syms:
                if sym['size'] and 0<sym['sec']<len(e.s) and e.s[sym['sec']][1]==8:
                    a=prog.sym_addr(e,unit,sym)
                    if a is not None:candidates[section_ids(e)[sym['sec']]].add(a-sym['off'])
            for sid,values in candidates.items():
                if len(values)==1:bases[unit,sid]=next(iter(values))
    for prog in (pb,pa):
        prog._proof_storage_bases=bases
        for unit,e in prog.elfs.items():
            for sym in e.syms:
                sec=sym['sec']
                base=bases.get((unit,section_ids(e).get(sec))) if 0<sec<len(e.s) and e.s[sec][1]==8 else None
                if base is not None and sym['size'] and prog.sym_addr(e,unit,sym) is None:
                    prog._proof_addresses[unit,sec,sym['off'],sym['name']]=base+sym['off']
    for prog in (pb,pa):
        names=collections.defaultdict(set)
        for unit,e in prog.elfs.items():
            for sym in e.syms:
                if sym['bind'] and sym['sec'] and sym['size']:
                    a=symbol_addr(prog,e,unit,sym)
                    if a is not None:names[sym['name']].add(a)
        prog._proof_named={n:next(iter(v)) for n,v in names.items() if len(v)==1}

    # Definition placement and a reference's named binding are different facts.
    # A corrected name map can identify unchanged, independently DOL-exact
    # storage that the old map mislabeled. Never rewrite target_key's binding.
    ranges=[{(a,z,sc,typ) for a,z,sc,typ in p.sy.by_name.values()} for p in (pb,pa)]
    common=ranges[0]&ranges[1]
    for prog,other in ((pb,pa),(pa,pb)):
        for unit,e in prog.elfs.items():
            oe=other.elfs.get(unit)
            if oe is None:continue
            counterparts=collections.defaultdict(list)
            for s in oe.syms:
                if s['size'] and s['type'] in (1,2):counterparts[s['name']].append(s)
            for sym in e.syms:
                if not sym['size'] or sym['type'] not in (1,2) or not 0<sym['sec']<len(e.s):continue
                addr=other.sy.addr(sym['name'])
                own=symbol_addr(prog,e,unit,sym)
                if addr is None or addr==own:continue
                if own is not None and exact_body(prog,e,unit,sym,own):continue
                typ='function' if sym['type']==2 else 'object'
                if (addr,sym['size'],e.names[sym['sec']],typ) not in common:continue
                cs=counterparts[sym['name']]
                if len(cs)!=1:continue
                cs=cs[0]
                if any(sym[k]!=cs[k] for k in ('size','type','bind')):continue
                if not 0<cs['sec']<len(oe.s) or e.names[sym['sec']]!=oe.names[cs['sec']]:continue
                if e.content(sym['sec'],sym['off'],sym['size'])!=oe.content(cs['sec'],cs['off'],cs['size']):continue
                if exact_body(prog,e,unit,sym,addr) and exact_body(other,oe,unit,cs,addr):
                    prog._proof_definition_addresses[unit,sym['sec'],sym['off'],sym['name']]=addr

    # Removing a wrong global name must not relocate its unchanged weak copies.
    # This is comparison bookkeeping only: L targets are NEVER DOL evidence.
    for prog,other in ((pb,pa),(pa,pb)):
        for name,record in prog.sy.by_name.items():
            if name in other.sy.by_name or record not in other.sy.by_name.values():continue
            addr,size,sec,typ=record
            if typ!='function':continue
            def legacy_records(p):
                out=[];definitions=[]
                for unit,e in p.elfs.items():
                    ids=section_ids(e)
                    for i,s in enumerate(e.syms):
                        if s['name']!=name:continue
                        out.append((unit,'symbol',tuple(s.items())))
                        if s['sec']:
                            if not 0<s['sec']<len(e.s) or s['bind']!=2 or s['type']!=2 or s['size']!=size:return None,None
                            definitions.append((unit,e,s))
                            out.append((unit,'body',e.content(s['sec'],s['off'],size),
                                tuple((o-s['off'],t,ad,e.syms[si]['name']) for sc,o,t,si,ad in e.relocs if sc==s['sec'] and s['off']<=o<s['off']+size)))
                        for sc,o,t,si,ad in e.relocs:
                            if si==i and not ignored(e,sc):out.append((unit,'use',ids[sc],o,t,ad,e.content(sc,o&~3,4)))
                return out,definitions
            old,defs=legacy_records(prog);new,otherdefs=legacy_records(other)
            if old and old==new and defs and all(not exact_body(prog,e,u,s,addr) for u,e,s in defs):
                pb._proof_legacy_addresses[name]=pa._proof_legacy_addresses[name]=addr

    # Keep pre-existing, unchanged non-exact duplicate variants distinct from
    # their address's exact image. Their bytes/relocations still compare in full.
    for unit in set(pb.elfs)&set(pa.elfs):
        for prog,other in ((pb,pa),(pa,pb)):
            e,oe=prog.elfs[unit],other.elfs[unit];groups=collections.defaultdict(list)
            for s in e.syms:
                if s['size'] and s['type'] in (1,2) and 0<s['sec']<len(e.s):
                    a=storage_addr(prog,e,unit,s)
                    if a is not None:groups[e.names[s['sec']],a,s['size']].append(s)
            for (sec,addr,size),syms in groups.items():
                if len({(s['sec'],s['off']) for s in syms})<2:continue
                for s in syms:
                    if exact_body(prog,e,unit,s,addr):continue
                    matches=[x for x in oe.syms if x==s]
                    if len(matches)!=1:continue
                    x=matches[0]
                    if storage_addr(other,oe,unit,x)!=addr or exact_body(other,oe,unit,x,addr):continue
                    def body(p,el,sy):
                        return (el.content(sy['sec'],sy['off'],size),collections.Counter((o-sy['off'],t,ad,target_key(p,el,unit,si,ad))
                            for sc,o,t,si,ad in el.relocs if sc==sy['sec'] and sy['off']<=o<sy['off']+size))
                    if body(prog,e,s)==body(other,oe,x):
                        key=(unit,s['sec'],s['off'],size)
                        prog._proof_variants.add(key);other._proof_variants.add(key)


def exact_body(prog, e, unit, sym, addr):
    sec, off, size = sym['sec'], sym['off'], sym['size']
    if not 0 < sec < len(e.s) or e.s[sec][1] == 8:
        return False
    body, dol = bytearray(e.content(sec,off,size)), prog.dol.bytes(addr,size)
    if dol is None or len(body) != size:
        return False
    want = bytearray(dol)
    seen = set()
    if not hasattr(e,'_proof_reloc_index'):
        grouped=collections.defaultdict(list)
        for rs,o,typ,si,ad in e.relocs:grouped[rs].append((o,typ,si,ad))
        e._proof_reloc_index={sc:(sorted(v),sorted(r[0] for r in v)) for sc,v in grouped.items()}
    records,offsets=e._proof_reloc_index.get(sec,([],[]))
    for o,typ,si,ad in records[bisect.bisect_left(offsets,off):bisect.bisect_left(offsets,off+size)]:
        n = o-off
        if n in seen:
            return False
        seen.add(n)
        target = target_key(prog,e,unit,si,ad)
        if target[0] != 'A':
            return False
        if typ == dc.R_ADDR32:
            if n+4>size or (dc.u32(dol,n) != target[1]&0xffffffff and (unit,addr+n,target) not in getattr(prog,'_proof_allowed_old',set())):
                return False
            body[n:n+4] = want[n:n+4] = b'\0'*4
        elif typ in dc.MASK:
            at = n & ~3 if typ in (dc.R_ADDR16,dc.R_ADDR16_LO,dc.R_ADDR16_HI,dc.R_ADDR16_HA) else n
            if at+4>size:
                return False
            op=prog.dol_operand(typ,dc.u32(dol,at),addr+at)
            if op is None:
                return False
            expected=target[1]&0xffffffff
            if op[0]=='hi':expected=((expected+0x8000)>>16 if typ==dc.R_ADDR16_HA else expected>>16)&0xffff
            elif op[0]=='lo':expected &= 0xffff
            if op[1] != expected and (unit,addr+at,target) not in getattr(prog,'_proof_allowed_old',set()):
                return False
            mask=~dc.MASK[typ]&0xffffffff
            import struct
            struct.pack_into('>I',body,at,dc.u32(body,at)&mask)
            struct.pack_into('>I',want,at,dc.u32(want,at)&mask)
        else:
            return False
    return body==want


_COVERAGE_CACHE = {}


def exact_coverage(prog):
    result=collections.defaultdict(list)
    config=hashlib.sha256((prog.root/'config/SB4E01/symbols.txt').read_bytes()).digest()
    named=hashlib.sha256(repr(sorted(prog._proof_named.items())).encode()).digest()
    legacy=tuple(sorted(getattr(prog,'_proof_legacy_addresses',{}).items()))
    aliases=collections.defaultdict(list)
    for key,addr in prog._proof_addresses.items():aliases[key[0]].append((key[1:],addr))
    for key,addr in getattr(prog,'_proof_definition_addresses',{}).items():aliases[key[0]].append((('definition',)+key[1:],addr))
    permitted=collections.defaultdict(list)
    for unit,site,target in getattr(prog,'_proof_allowed_old',set()):permitted[unit].append((site,target))
    for unit,e in prog.elfs.items():
        key=(unit,hashlib.sha256(e.raw).digest(),config,named,legacy,tuple(sorted(aliases[unit],key=repr)),tuple(sorted(permitted[unit])))
        entries=_COVERAGE_CACHE.get(key)
        if entries is None:
            entries=[]
            for sym in e.syms:
                if sym['size'] and sym['type'] in (1,2):
                    a=storage_addr(prog,e,unit,sym)
                    if a is not None and exact_body(prog,e,unit,sym,a):entries.append((a,sym['size'],sym['name']))
            _COVERAGE_CACHE[key]=entries
        for a,size,name in entries:result[a,size].append((unit,name))
    return result


def target_key(prog, e, unit, si, ad):
    """Never use learned addresses as target identity: inference can be circular.

    Local/section targets retain their exact placement and addend. Equal bodies
    are not interchangeable (two BSS objects both have empty bodies).
    """
    t = e.syms[si]
    legacy=getattr(prog,'_proof_legacy_addresses',{}).get(t['name'])
    if legacy is not None:return ('L',t['name'],legacy+ad,t['bind'],t['type'],t['size'])
    if not hasattr(e,'_proof_other'):
        e._proof_other=[e.raw[sh[4]+o+13] for sh in e.s if sh[1]==2 for o in range(0,sh[5],16)]
    def addressed(a):return ('A',a,t['bind'],t['type'],t['size'],e._proof_other[si])
    addr = symbol_addr(prog, e, unit, t)
    if addr is not None:
        return addressed(addr+ad)
    if t['name'].startswith('sInstance__'):
        a,how=prog.target(e,unit,si,ad)
        if a is not None and how == 'chain':
            return addressed(a)
    a=getattr(prog,'_proof_named',{}).get(t['name'])
    if a is not None and t['bind']:
        return addressed(a+ad)
    sec = t['sec']
    if 0 < sec < len(e.s):
        sid = section_ids(e)[sec]
        return ('S', sid, t['off'] + ad) if t['type'] == 3 else ('N', '<anonymous>' if anonymous(t) else t['name'], sid, t['off'], ad)
    return ('N', t['name'], sec, t['off'], ad)


def unit_image(prog, unit):
    """Partition ALL non-debug section bytes into anchored symbols and offset gaps.

    Mapped definitions may reorder, but their full bytes and relocation multiset
    must survive at the same original address. Unmapped storage stays at its
    exact section offset. Overlapping independent anchors fail closed.
    """
    e = prog.elfs[unit]
    ids, img = section_ids(e), {}
    if not hasattr(prog,'_proof_image_bytes'):prog._proof_image_bytes={}
    bodies={};prog._proof_image_bytes[unit]=bodies
    for sec, sh in enumerate(e.s):
        if not sec or ignored(e, sec) or sh[1] in (2, 3, 4):
            continue
        anchors = {}
        for sym in e.syms:
            if sym['sec'] != sec or not sym['size'] or sym['type'] not in (1, 2):
                continue
            a = storage_addr(prog,e,unit,sym)
            if a is not None:
                block = (sym['off'], sym['size'])
                if block in anchors and anchors[block] != a:
                    raise ValueError(f'{unit}: aliases at one offset have different original addresses')
                anchors[block] = a
        cursor, blocks, previous = 0, [], None
        for (off, size), addr in sorted(anchors.items()):
            if off < cursor or off + size > sh[5]:
                raise ValueError(f'{unit}: overlapping/out-of-section symbol at {ids[sec]}+{off:#x}')
            if off > cursor:
                blocks.append((cursor, off - cursor, ('P',previous) if previous is not None else ('O',cursor)))
            variant=(unit,sec,off,size) in getattr(prog,'_proof_variants',set())
            blocks.append((off, size, ('V',addr,ids[sec],off) if variant else ('A', addr)))
            cursor = off + size
            previous = addr+size
        if cursor < sh[5]:
            blocks.append((cursor, sh[5]-cursor, ('P',previous) if previous is not None else ('O',cursor)))
        if not blocks:
            blocks = [(0, sh[5], ('O', 0))]
        for off, size, key in blocks:
            body = b'' if sh[1] == 8 else e.content(sec, off, size)
            if sh[1] != 8 and len(body) != size:
                raise ValueError(f'{unit}: truncated section {ids[sec]}')
            rels = collections.Counter((o - off, typ, ad, target_key(prog, e, unit, si, ad))
                                      for rs, o, typ, si, ad in e.relocs if rs == sec and off <= o < off + size)
            k = (e.names[sec] if key[0] in ('A','P','V') else ids[sec], key, size)
            if k in img and img[k] != (hashlib.sha256(body).hexdigest(),rels):
                raise ValueError(f'{unit}: conflicting duplicate original-address image key {k}')
            img[k] = (hashlib.sha256(body).hexdigest(), rels)
            bodies[k]=body
        # Relocations outside storage must not silently vanish.
        for rs, o, typ, si, ad in e.relocs:
            if rs == sec and not 0 <= o < sh[5]:
                raise ValueError(f'{unit}: relocation outside section {ids[sec]}')
    return img


def compare_units(unit, ib, ia, flipped_sites, problems, removable=frozenset(), addable=frozenset(), binding_trades=frozenset(), detail_bodies=None):
    failed=False
    if set(ib)-set(ia)-set(removable) or set(ia)-set(ib)-set(addable):
        suffix=f': -{sorted(set(ib)-set(ia)-set(removable),key=repr)} +{sorted(set(ia)-set(ib)-set(addable),key=repr)}' if detail_bodies else ''
        problems.append(f'{unit}: section/symbol image keys differ{suffix}')
        if not detail_bodies:return None
        failed=True
    retargets = 0
    for k in set(ib)&set(ia):
        hb,rb=ib[k]
        ha,ra=ia[k]
        if hb != ha:
            suffix=''
            if detail_bodies:
                b,a=detail_bodies[0][k],detail_bodies[1][k];runs=[];start=None
                for i in range(max(len(b),len(a))+1):
                    diff=i<max(len(b),len(a)) and b[i:i+1]!=a[i:i+1]
                    if diff and start is None:start=i
                    if not diff and start is not None:runs.append((hex(start),b[start:i].hex(),a[start:i].hex()));start=None
                suffix=f': (offset, before, after) {runs}'
            problems.append(f'{unit}: bytes at {k} differ{suffix}')
            if not detail_bodies:return None
            failed=True
        if rb == ra:
            continue
        removed, added = rb - ra, ra - rb
        # One old and one new relocation, with the SAME type and addend. Never
        # bless removal/insertion, duplicate records, or another container/unit.
        for old, count in removed.items():
            o, typ, ad, target = old
            candidates = [r for r in added if r[:3] == old[:3] and added[r] == count]
            site = k[1][1] + (o & ~3 if typ in (dc.R_ADDR16,dc.R_ADDR16_LO,dc.R_ADDR16_HI,dc.R_ADDR16_HA) else o) if k[1][0] == 'A' else None
            new = candidates[0] if len(candidates) == 1 else None
            allowed = (unit, k, o, typ, ad, site, target, new[3] if new else None)
            trade=False
            if new is not None and target[0]==new[3][0]=='A' and len(target)==len(new[3])==6:
                t=new[3]
                oldrecord=('A',target[1]-ad,target[4],target[2],target[3],target[5])
                newrecord=('A',t[1]-ad,t[4],t[2],t[3],t[5])
                trade=(oldrecord,newrecord) in binding_trades
            if count != 1 or new is None or (allowed not in flipped_sites and not trade):
                suffix=f': -{old} x{count} +{candidates}' if detail_bodies else ''
                problems.append(f'{unit}: relocation {k}+{o:#x} changed without an independent BAD->MATCH proof{suffix}')
                if not detail_bodies:return None
                failed=True
                if new is not None:
                    added.subtract({new:count});added+=collections.Counter()
                continue
            added.subtract({new: 1})
            added += collections.Counter()
            retargets += not trade
        if added:
            problems.append(f'{unit}: extra relocation records at {k}'+(f': {list(added.elements())}' if detail_bodies else ''))
            if not detail_bodies:return None
            failed=True
    return None if failed else retargets


def row_key(r):
    return (r['layer'], r['unit'], r.get('container') or r.get('function') or '', r['offset'], r.get('target', ''))


def bad(r):
    return r['verdict'] in ('CROSSED', 'WRONG', 'CONFLICT') or r['verdict'].startswith('ADVISORY')


def tree_objects(root):
    base = pathlib.Path(root) / 'build/SB4E01/src'
    return {str(p.relative_to(base).with_suffix('')): p for p in sorted(base.rglob('*.o'))}


def digest(p):
    return hashlib.sha1(pathlib.Path(p).read_bytes()).hexdigest()


def oracle_key(r):
    # Names/targets can change in a valid fix; the original physical site cannot.
    if r['layer']=='align':return (r['layer'],r['unit'],r.get('function'),r['offset'])
    if isinstance(r.get('site'),int):
        return (r['layer'],r['unit'],r['site'])
    return (r['layer'],r['unit'],r.get('container') or r.get('function') or '',r['offset'],r.get('section'))


def row_relocation(prog, row):
    """Identify the actual ELF record behind a row; ambiguity grants no exception."""
    unit = row['unit']
    if unit not in prog.elfs:
        return None
    e = prog.elfs[unit]
    candidates = []
    name = row.get('container') or row.get('function')
    for sym in e.syms:
        if sym['name'] != name or not 0 < sym['sec'] < len(e.s):
            continue
        off = row['offset'] if row['layer'] == 'data' else sym['off'] + row['offset']
        for sec, o, typ, si, ad in e.relocs:
            if sec == sym['sec'] and (o & ~3 if row['layer'] != 'data' and typ in (dc.R_ADDR16,dc.R_ADDR16_LO,dc.R_ADDR16_HI,dc.R_ADDR16_HA) else o) == off and e.syms[si]['name'] == row.get('target'):
                addr = symbol_addr(prog, e, unit, sym)
                if addr is not None and sym['size']:
                    block = (e.names[sec], ('A', addr), sym['size'])
                    candidates.append((block, o - sym['off'], typ, ad, target_key(prog, e, unit, si, ad)))
    return candidates[0] if len(candidates) == 1 else None


def paired_high_permissions(pb,pa,direct_lows):
    """An unchanged HA field may accompany ONLY its approved adjacent LO fix."""
    permitted=set()
    for unit,block,off,typ,ad,site,old,new in direct_lows:
        if typ!=dc.R_ADDR16_LO or off<6 or off%4!=2:continue
        parts=[]
        for prog,target in ((pb,old),(pa,new)):
            e=prog.elfs[unit]
            fs=[f for f in e.syms if f['type']==2 and 0<f['sec']<len(e.s) and f['size']==block[2] and e.names[f['sec']]==block[0]
                and prog.sym_addr(e,unit,f)==block[1][1]]
            if len(fs)!=1:break
            f=fs[0]
            lo=[r for r in e.relocs if r[0]==f['sec'] and r[1]==f['off']+off]
            hi=[r for r in e.relocs if r[0]==f['sec'] and r[1]==f['off']+off-4]
            if len(lo)!=1 or len(hi)!=1:break
            lo,hi=lo[0],hi[0]
            if lo[2]!=dc.R_ADDR16_LO or hi[2]!=dc.R_ADDR16_HA or hi[3:]!=lo[3:] or lo[4]!=ad:break
            if target_key(prog,e,unit,lo[3],ad)!=target:break
            at=f['off']+off-2
            sw_hi=dc.u32(e.content(f['sec'],at-4,4),0);sw_lo=dc.u32(e.content(f['sec'],at,4),0)
            if sw_hi>>26!=15 or (sw_hi>>16)&31 or sw_lo>>26 not in (14,*range(32,56)):break
            if (sw_hi>>21)&31!=(sw_lo>>16)&31:break
            dw_hi=prog.dol.word(site-4);dw_lo=prog.dol.word(site)
            if dw_hi is None or dw_lo is None:break
            if sw_hi&0xffff0000!=dw_hi&0xffff0000 or sw_lo&0xffff0000!=dw_lo&0xffff0000:break
            parts.append((sw_hi,sw_lo,dw_hi,dw_lo))
        if len(parts)!=2 or parts[0]!=parts[1]:continue
        dw_hi,dw_lo=parts[0][2:]
        before=((old[1]+0x8000)>>16)&0xffff;after=((new[1]+0x8000)>>16)&0xffff
        actual=(((dw_hi&0xffff)<<16)+dc.s16(dw_lo&0xffff))&0xffffffff
        if before!=after or after!=dw_hi&0xffff or actual!=new[1] or actual==old[1]:continue
        permitted.add((unit,block,off-4,dc.R_ADDR16_HA,ad,site-4,old,new))
    return permitted


def aligned_site(pb,pa,b,a,block,off,typ):
    """Revalidate v8's equal-block alignment; return original PC, never source PC."""
    import difflib
    if typ not in (dc.R_SDA21,dc.R_REL24):return None
    di=b.get('dol_offset')
    if not isinstance(di,int) or di!=a.get('dol_offset') or di%4 or abs(di-off)>32:return None
    bodies=[];pcs=[]
    for prog,row in ((pb,b),(pa,a)):
        e=prog.elfs[row['unit']]
        fs=[f for f in e.syms if f['name']==row['function'] and f['type']==2 and f['size']]
        if len(fs)!=1:return None
        f=fs[0];addr=prog.sym_addr(e,row['unit'],f)
        if addr!=block[1][1] or f['size']!=block[2] or row['offset']!=off:return None
        size=prog.sy.by_name.get(f['name'],(0,f['size']))[1]
        dol=prog.dol.bytes(addr,size)
        src=e.content(f['sec'],f['off'],f['size'])
        if dol is None or len(src)%4 or len(dol)%4 or abs(len(src)-len(dol))>64 or not 0<=di<len(dol):return None
        rels={}
        for sec,o,t,si,ad in e.relocs:
            if sec!=f['sec'] or not f['off']<=o<f['off']+f['size']:continue
            n=(o-f['off'])&~3
            if n in rels:return None
            rels[n]=t
        if rels.get(off)!=typ:return None
        mask=~dc.MASK[typ]&0xffffffff
        if dc.u32(src,off)&mask!=dc.u32(dol,di)&mask:return None
        ws=[dc.u32(src,n)&~dc.MASK.get(rels.get(n),0)&0xffffffff for n in range(0,len(src),4)]
        kinds=set(rels.values());wd=[]
        for n in range(0,len(dol),4):
            w=dc.u32(dol,n);op=w>>26
            if op==18 and dc.R_REL24 in kinds:w &= ~dc.MASK[dc.R_REL24]&0xffffffff
            elif op in (14,*range(32,56)) and dc.R_SDA21 in kinds and (w>>16)&31 in (2,13):w &= ~dc.MASK[dc.R_SDA21]&0xffffffff
            wd.append(w)
        pairs=[(i1,i2,j1) for tag,i1,i2,j1,j2 in difflib.SequenceMatcher(None,ws,wd,autojunk=False).get_opcodes()
               if tag=='equal' and i1<=off//4<i2 and j1+off//4-i1==di//4]
        if len(pairs)!=1 or row.get('site')!=addr+di:return None
        bodies.append(src);pcs.append(addr+di)
    return pcs[0] if bodies[0]==bodies[1] and pcs[0]==pcs[1] else None


def sinit_permissions(pb, pa, before, after):
    """Prove an independent pure initialization pair, never a site=None PC match.

    Only addi r3,SDA or lis/addi r3 followed by bl is supported. Intervening
    stack stores and immediate arithmetic on other registers preserve r3. Both changed operands stay pinned to this pair.
    """
    unit=before['unit']; eb,ea=pb.elfs[unit],pa.elfs[unit]
    def function(e,row):
        fs=[f for f in e.syms if f['name']==row['function'] and f['type']==2 and f['size']]
        return fs[0] if len(fs)==1 else None
    fb,fa=function(eb,before),function(ea,after)
    if not fb or not fa or fb['size']!=fa['size']:return set()
    if eb.content(fb['sec'],fb['off'],fb['size'])!=ea.content(fa['sec'],fa['off'],fa['size']):return set()
    def pattern(words, call):
        # Return the load instruction(s) for a straight-line r3 -> bl pair.
        if call<4 or words[call] & 0xfc000003 != 0x48000001:return None
        end=call-4
        while end>=0:
            w=words[end];op=w>>26;rt=(w>>21)&31;ra=(w>>16)&31
            if (op==36 and ra==1) or (op in (14,15) and rt!=3):end-=4
            else:break
        if end<0:return None
        w=words[end]
        if w & 0xffff0000==0x38600000:return [end]  # unlinked SDA21 addi r3,0
        if w>>26==14 and (w>>21)&31==3 and (w>>16)&31 in (2,13):return [end]
        if w & 0xffff0000==0x38630000 and end>=4 and words[end-4]&0xffff0000==0x3c600000:return [end-4,end]
        return None
    def source(prog,e,f,row):
        body=e.content(f['sec'],f['off'],f['size'])
        words={n:dc.u32(body,n) for n in range(0,len(body),4)}
        loads=pattern(words,row['offset'])
        if loads is None:return None
        offsets=loads+[row['offset']];records=[]
        for off in offsets:
            rs=[r for r in e.relocs if r[0]==f['sec'] and r[1] in (f['off']+off,f['off']+off+2)]
            if len(rs)!=1:return None
            sec,o,typ,si,ad=rs[0];name=e.syms[si]['name']
            a,how=prog.target(e,unit,si,ad)
            if ad or a is None:return None
            if off==row['offset']:
                if typ!=dc.R_REL24 or how!='sym' or not name.startswith('__ct__') or 'Nrv' not in name:return None
            elif how not in ('sym','chain') or not name.startswith('sInstance__') or 'Nrv' not in name:return None
            elif typ != (dc.R_SDA21 if len(loads)==1 else dc.R_ADDR16_HA if off==loads[0] else dc.R_ADDR16_LO):return None
            block=(e.names[sec],('A',prog.sym_addr(e,unit,f)),f['size'])
            if block[1][1] is None:return None
            records.append((block,o-f['off'],typ,ad,target_key(prog,e,unit,si,ad)))
        if len(loads)==2 and records[0][4][1]!=records[1][4][1]:return None
        return records
    br,ar=source(pb,eb,fb,before),source(pa,ea,fa,after)
    if not br or not ar or len(br)!=len(ar) or any(b[:4]!=a[:4] for b,a in zip(br,ar)):return set()
    y,k=ar[0][4][1],ar[-1][4][1]
    if pa.dol_sinit_pairs().get(y)!=k:return set()
    # Independently constrain the original ctor, not the oracle's heuristic scan.
    kb=pa.dol.bytes(k,16)
    if kb is None:return set()
    kw=[dc.u32(kb,i) for i in range(0,16,4)]
    if [kw[0]&0xffff0000,kw[1]&0xffff0000,kw[2],kw[3]]!=[0x3c800000,0x38840000,0x90830000,0x4e800020]:return set()
    ctors=[(u,e,f) for u,e in pa.elfs.items() for f in e.syms
           if f['type']==2 and f['size']==16 and pa.sym_addr(e,u,f)==k and f['name'].startswith('__ct__')]
    if not ctors or not all(exact_body(pa,e,u,f,k) for u,e,f in ctors):return set()
    # Locate and decode a real original sinit pair, with the same strict pattern.
    found=False
    for name,(addr,size,sec,typ) in pa.sy.by_name.items():
        if not name.startswith('__sinit_') or not size or sec!='.text':continue
        body=pa.dol.bytes(addr,size)
        if body is None:continue
        words={n:dc.u32(body,n) for n in range(0,size,4)}
        for off in words:
            loads=pattern(words,off)
            if loads is None:continue
            call=pa.dol_operand(dc.R_REL24,words[off],addr+off)
            if not call or call!=('addr',k):continue
            if len(loads)==1:
                w=words[loads[0]];ra=(w>>16)&31
                if ra not in (2,13):continue
                inst=((dc.SDA2 if ra==2 else dc.SDA13)+dc.s16(w&0xffff))&0xffffffff
            else:inst=((words[loads[0]]&0xffff)<<16)+dc.s16(words[loads[1]]&0xffff)
            if inst==y:found=True;break
        if found:break
    if not found:return set()
    allowed=set()
    for b,a in zip(br,ar):
        if b[4]==a[4]:continue
        block,off,typ,ad=b[:4]
        site=block[1][1]+(off&~3 if typ in (dc.R_ADDR16_HA,dc.R_ADDR16_LO) else off)
        allowed.add((unit,block,off,typ,ad,site,b[4],a[4]))
    return allowed


def emission_permissions(prog, other, unit, img, otherimg, coverage):
    e=prog.elfs[unit]
    allowed=set()
    def locations(p):
        x=p.elfs[unit];out=collections.defaultdict(set)
        for sym in x.syms:
            if sym['size'] and sym['type'] in (1,2) and 0<sym['sec']<len(x.s):
                a=storage_addr(p,x,unit,sym)
                if a is not None:out[(x.names[sym['sec']],('A',a),sym['size'])].add((sym['sec'],sym['off'],sym['size']))
        return out
    here,there=locations(prog),locations(other)
    for sym in e.syms:
        if not sym['size'] or sym['type'] not in (1,2):continue
        a=storage_addr(prog,e,unit,sym)
        k=(e.names[sym['sec']],('A',a),sym['size']) if a is not None else None
        if k in img and len(here[k])>len(there[k]) and (a,sym['size']) in coverage and exact_body(prog,e,unit,sym,a):
            allowed.add(k)
            # Alignment following a removed exact duplicate may disappear only
            # when EVERY byte equals DOL padding at the exact same address.
            pk=next((p for p in img if p[0]==k[0] and p[1]==('P',a+sym['size'])),None)
            if pk is not None and pk not in otherimg and not img[pk][1]:
                dol=prog.dol.bytes(a+sym['size'],pk[2])
                if dol is not None and hashlib.sha256(dol).hexdigest()==img[pk][0]:allowed.add(pk)
    return allowed


def exact_binding_trades(pb,pa,unit):
    eb,ea=pb.elfs[unit],pa.elfs[unit]
    cb,ca=symbol_contract(eb,pb,unit),symbol_contract(ea,pa,unit)
    missing,extra=cb-ca,ca-cb
    trades=set()
    # A strong/weak emission may trade places only over the same independently
    # exact image. Binding changes on unanchored or non-exact storage still fail.
    for record,count in list(missing.items()):
        if record[0]!='A' or count!=1 or record[3] not in (1,2):continue
        counterpart=record[:3]+(3-record[3],)+record[4:]
        if extra[counterpart]!=1:continue
        _,addr,size,bind,typ,other=record
        copies=[]
        for p,e in ((pb,eb),(pa,ea)):
            ss=[s for s in e.syms if s['size']==size and s['type']==typ and storage_addr(p,e,unit,s)==addr]
            copies.append(bool(ss) and all(exact_body(p,e,unit,s,addr) for s in ss))
        if all(copies):trades.add((record,counterpart))
    return trades


def check_symbol_changes(pb,pa,unit,ib,ia,removed,added):
    eb,ea=pb.elfs[unit],pa.elfs[unit]
    cb,ca=symbol_contract(eb,pb,unit),symbol_contract(ea,pa,unit)
    missing,extra=cb-ca,ca-cb
    for record,counterpart in exact_binding_trades(pb,pa,unit):
        missing.subtract({record:1});extra.subtract({counterpart:1})
    for counter,permissions in ((+missing,removed),(+extra,added)):
        for record,count in counter.items():
            if record[0]!='A' or count!=1:return False
            _,addr,size,bind,typ,other=record
            if typ not in (1,2) or not any(k[1]==('A',addr) and k[2]==size for k in permissions):return False
    before={s['name']:s for s in eb.syms if s['size'] and s['type'] in (1,2) and not anonymous(s) and not ignored(eb,s['sec'])}
    after={s['name']:s for s in ea.syms if s['size'] and s['type'] in (1,2) and not anonymous(s) and not ignored(ea,s['sec'])}
    for name in set(before)&set(after):
        b,a=before[name],after[name]
        if b['size']!=a['size'] and symbol_addr(pb,eb,unit,b)==symbol_addr(pa,ea,unit,a):return False
    # A compiler record cannot be renamed arbitrarily: independently mapped
    # names must exist in that phase's symbol map (or an original chain anchor).
    for prog,e,own,other in ((pb,eb,before,after),(pa,ea,after,before)):
        for name in set(own)-set(other):
            sym=own[name]
            if prog.sy.addr(name) is None and (pa if prog is pb else pb).sy.addr(name) is None:
                return False
    return True


def config_ranges_preserved(pb,pa):
    """Renames retain ranges; first naming may split one unnamed label range.

    The new definition must already occupy the same independently split-owned
    address and size in both objects, and independently equal the original DOL.
    The remaining label retains the suffix bytes of the original range.
    """
    rb={v for v in pb.sy.by_name.values()};ra={v for v in pa.sy.by_name.values()}
    removed,added=rb-ra,ra-rb
    for old in list(removed):
        addr,size,sec,typ=old
        oldnames=[n for n,v in pb.sy.by_name.items() if v==old]
        if not oldnames or any(n!=f'lbl_{addr:08X}' for n in oldnames):continue
        candidates=[]
        for name,new in pa.sy.by_name.items():
            a,z,sc,t=new
            if new not in added or a!=addr or not 0<z<size or (sc,t)!=(sec,typ) or name.startswith('lbl_'):continue
            rest=(addr+z,size-z,sec,typ)
            if rest not in added or pa.sy.by_name.get(f'lbl_{addr+z:08X}')!=rest:continue
            valid=True
            for prog in (pb,pa):
                copies=[]
                for unit,e in prog.elfs.items():
                    for sym in e.syms:
                        if sym['name']!=name or sym['size']!=z or not 0<sym['sec']<len(e.s):continue
                        if e.names[sym['sec']]!=sec or sym['type']!=({'function':2,'object':1}.get(typ)):continue
                        base=prog.bases.get((unit,sec))
                        if base is not None and base+sym['off']==addr and exact_body(prog,e,unit,sym,addr):copies.append((unit,sym))
                if not copies:valid=False;break
            if valid:candidates.append((new,rest))
        if len(candidates)==1:
            new,rest=candidates[0];removed.remove(old);added.remove(new);added.remove(rest)
    return not removed and not added


def config_only_high_permissions(pb,pa,low):
    """Metadata-only HA companion across at most eight harmless instructions.

    This is called only by the preserved config-range case. The general HA
    permission remains adjacent-only. Intermediate integer adds may write
    another register; non-update FP loads/stores do not write any GPR.
    """
    unit,block,off,typ,ad,site,old,new=low
    if typ!=dc.R_ADDR16_LO or off%4!=2 or pb.elfs[unit].raw!=pa.elfs[unit].raw:return set()
    found=set()
    for delta in range(4,37,4):
        parts=[]
        for p,target in ((pb,old),(pa,new)):
            e=p.elfs[unit]
            fs=[s for s in e.syms if s['type']==2 and s['size']==block[2] and 0<s['sec']<len(e.s)
                and e.names[s['sec']]==block[0] and p.sy.addr(s['name'])==block[1][1]]
            if len(fs)!=1 or off<delta+2:break
            f=fs[0];lo=f['off']+off;hi=lo-delta
            lr=[r for r in e.relocs if r[0]==f['sec'] and r[1]==lo]
            hr=[r for r in e.relocs if r[0]==f['sec'] and r[1]==hi]
            if len(lr)!=1 or len(hr)!=1:break
            lr,hr=lr[0],hr[0]
            if lr[2]!=dc.R_ADDR16_LO or hr[2]!=dc.R_ADDR16_HA or hr[3:]!=lr[3:] or lr[4]!=ad:break
            if target_key(p,e,unit,lr[3],ad)!=target:break
            sw_hi=dc.u32(e.content(f['sec'],hi-2,4),0);sw_lo=dc.u32(e.content(f['sec'],lo-2,4),0)
            dw_hi=p.dol.word(site-delta);dw_lo=p.dol.word(site)
            if dw_hi is None or dw_lo is None:break
            reg=(sw_hi>>21)&31
            if sw_hi>>26!=15 or (sw_hi>>16)&31 or sw_lo>>26!=14 or (sw_lo>>16)&31!=reg:break
            if sw_hi&0xffff0000!=dw_hi&0xffff0000 or sw_lo&0xffff0000!=dw_lo&0xffff0000:break
            safe=True
            for at in range(hi+2,lo-2,4):
                sw=dc.u32(e.content(f['sec'],at,4),0);dw=p.dol.word(block[1][1]+at-f['off'])
                rels=[r for r in e.relocs if r[0]==f['sec'] and at<=r[1]<at+4]
                if len(rels)>1 or any(r[2] not in dc.MASK for r in rels):safe=False;break
                mask=~dc.MASK[rels[0][2]]&0xffffffff if rels else 0xffffffff
                if dw is None or sw&mask!=dw&mask:safe=False;break
                op,rt=sw>>26,(sw>>21)&31
                if not (op==14 and rt!=reg or op in (48,50,52,54)):safe=False;break
            if not safe:break
            parts.append((dw_hi,dw_lo))
        if len(parts)!=2 or parts[0]!=parts[1]:continue
        dh,dl=parts[0]
        before=((old[1]+0x8000)>>16)&0xffff;after=((new[1]+0x8000)>>16)&0xffff
        actual=(((dh&0xffff)<<16)+dc.s16(dl&0xffff))&0xffffffff
        if before==after==dh&0xffff and actual==new[1] and actual!=old[1]:
            found.add((unit,block,off-delta,dc.R_ADDR16_HA,ad,site-delta,old,new))
    return found if len(found)==1 else set()


def config_move_storage(pb,pa,high_companions,direct):
    """Correct storage metadata only; references keep each phase's binding.

    A direct original operand must independently prove the new object address.
    Both config ranges survive at the same sizes, all affected objects are byte
    identical, and another unchanged strong BSS anchor confirms section layout.
    This grants no relocation permission: every changed reference still needs
    its own independently checked BAD->MATCH token.
    """
    accepted,extras=set(),set()
    for name in pb.sy.by_name.keys()&pa.sy.by_name.keys():
        old,new=pb.sy.by_name[name],pa.sy.by_name[name]
        before,after=old[0],new[0]
        if before==after or old[2:]!=new[2:] or old[3]!='object':continue
        if pa.sy.by_name.get(f'lbl_{before:08X}')!=old or pb.sy.by_name.get(f'lbl_{after:08X}')!=new:continue
        tokens=[t for t,target in direct.items() if target==name and t[4]==0 and t[6][0]==t[7][0]=='A' and t[6][1]==before and t[7][1]==after]
        proven=False;candidate_highs=set()
        for t in tokens:
            if t[3] in (dc.R_SDA21,dc.R_ADDR32):proven=True
            elif t[3]==dc.R_ADDR16_LO:
                hi=(t[0],t[1],t[2]-4,dc.R_ADDR16_HA,t[4],t[5]-4,t[6],t[7])
                if hi in high_companions:proven=True
                else:
                    pinned=config_only_high_permissions(pb,pa,t)
                    if pinned:proven=True;candidate_highs.update(pinned)
        if not proven:continue
        definitions=[];valid=True
        for unit in set(pb.elfs)|set(pa.elfs):
            eb,ea=pb.elfs.get(unit),pa.elfs.get(unit)
            syms=[s for e in (eb,ea) if e for s in e.syms if s['name']==name]
            if not syms:continue
            if eb is None or ea is None or eb.raw!=ea.raw:valid=False;break
            for s in eb.syms:
                if s['name']!=name or not s['sec']:continue
                sec=s['sec']
                if not 0<sec<len(eb.s) or eb.s[sec][1]!=8 or s['type']!=1 or s['bind']!=1 or not 0<s['size']<=new[1]:valid=False;break
                if eb.names[sec]!=new[2]:valid=False;break
                base=after-s['off'];anchors=[]
                for x in eb.syms:
                    if x['name']==name or x['sec']!=sec or x['bind']!=1 or x['type']!=1 or not x['size']:continue
                    rec=pb.sy.by_name.get(x['name'])
                    if rec and rec==pa.sy.by_name.get(x['name']) and rec[1]>=x['size'] and rec[2:]==(eb.names[sec],'object'):
                        anchors.append(rec[0]-x['off'])
                if not anchors or set(anchors)!={base}:valid=False;break
                definitions.append((unit,sec,s['off'],name))
            if not valid:break
        if valid and definitions:
            for key in definitions:
                pb._proof_definition_addresses[key]=pa._proof_definition_addresses[key]=after
            accepted.add(name)
            extras.update(candidate_highs)
    return accepted,extras


def compare_trees(pb, pa, rows_b, rows_a, units, expect_clean, detail=False):
    ob, oa, problems = pb.elfs, pa.elfs, []
    prepare_addresses(pb,pa)
    if pa.sy.dups-pb.sy.dups:
        problems.append('symbols.txt introduced duplicate names')
    if (pb.root/'config/SB4E01/splits.txt').read_bytes()!=(pa.root/'config/SB4E01/splits.txt').read_bytes():
        problems.append('split ownership changed')
    if pb.dol.raw!=pa.dol.raw:
        problems.append('original DOLs differ')
    files_b, files_a = tree_objects(pb.root), tree_objects(pa.root)
    if not files_b or not files_a:
        problems.append('empty object tree')
    if set(files_b) != set(files_a):
        problems.append(f'object sets differ: -{sorted(set(files_b)-set(files_a))[:5]} +{sorted(set(files_a)-set(files_b))[:5]}')
    for phase, prog, files in (('before', pb, files_b), ('after', pa, files_a)):
        if prog.errors or set(files) != set(prog.elfs):
            problems.append(f'{phase}: unreadable objects: {prog.errors[:3]}')
    # Keep all rows at a site (aliases can overlap); dict overwrite is unsound.
    kb, ka = collections.defaultdict(list), collections.defaultdict(list)
    for rows, dst in ((rows_b, kb), (rows_a, ka)):
        for r in rows:
            dst[oracle_key(r)].append(r)
    flipped, allowed, direct_lows, direct = [], set(), set(), {}
    for k, rs in kb.items():
        after = ka.get(k, [])
        if sum(r['verdict'] == 'MATCH' for r in after) < sum(r['verdict'] == 'MATCH' for r in rs):
            problems.append(f'oracle MATCH row regressed/disappeared: {k}')
        if len(rs) == len(after) == 1 and bad(rs[0]) and after[0]['verdict'] == 'MATCH':
            b, a = rs[0], after[0]
            flipped.append(row_key(b))
            # Candidate rows need independent validation of actual source operands.
            if b['layer']=='sinit':
                allowed.update(sinit_permissions(pb,pa,b,a))
                continue
            if b['layer'] in ('data','code','sem','sem-seq','near','align'):
                if any(not independent_name(prog,row.get('target','')) for prog,row in ((pb,b),(pa,a))):continue
                br,ar=row_relocation(pb,b),row_relocation(pa,a)
                if br and ar and br[:4]==ar[:4] and br[4][0]==ar[4][0]=='A' and br[4][1]!=ar[4][1]:
                    block,off,typ,ad=br[:4]
                    site=block[1][1]+(off&~3 if typ in (dc.R_ADDR16,dc.R_ADDR16_LO,dc.R_ADDR16_HI,dc.R_ADDR16_HA) else off)
                    original_site=aligned_site(pb,pa,b,a,block,off,typ) if b['layer']=='align' else site
                    if original_site is None or original_site!=b.get('site') or original_site!=a.get('site'):continue
                    word=pb.dol.word(original_site)
                    op=pb.dol_operand(typ,word,original_site) if word is not None and typ!=dc.R_ADDR32 else None
                    correct=(word==ar[4][1]) if typ==dc.R_ADDR32 else bool(op and op[0]=='addr' and op[1]==ar[4][1])
                    if typ in (dc.R_ADDR16,dc.R_ADDR16_LO,dc.R_ADDR16_HI,dc.R_ADDR16_HA) and op:
                        val=ar[4][1]
                        want=((val+0x8000)>>16 if typ==dc.R_ADDR16_HA else val>>16)&0xffff if op[0]=='hi' else val&0xffff
                        correct=op[1]==want
                    if correct:
                        token=(b['unit'],block,off,typ,ad,site,br[4],ar[4])
                        allowed.add(token)
                        if b['layer']=='code' and b.get('target')==a.get('target'):direct[token]=b['target']
                        if b['layer']=='code' and typ==dc.R_ADDR16_LO:direct_lows.add(token)

    for k, rs in ka.items():
        if sum(bad(r) for r in rs) > sum(bad(r) for r in kb.get(k, [])):
            problems.append(f'new BAD oracle row: {k}')
    high_companions=paired_high_permissions(pb,pa,direct_lows)
    allowed.update(high_companions)
    _,config_highs=config_move_storage(pb,pa,high_companions,direct)
    allowed.update(config_highs)
    pb._proof_allowed_old={(t[0],t[5],t[6]) for t in allowed}
    pa._proof_allowed_old=set()
    if not config_ranges_preserved(pb,pa):
        problems.append('symbols.txt changed a range without a preserved rename or independently exact first naming')
    cover_b,cover_a=exact_coverage(pb),exact_coverage(pa)
    if set(cover_b)!=set(cover_a):
        problems.append(f'global DOL-exact coverage differs: -{list(set(cover_b)-set(cover_a))[:5]} +{list(set(cover_a)-set(cover_b))[:5]}')
    report = dict(objects=len(files_b), identical=0, retargeted_objects=0, retargeted_sites=0, flipped_rows=len(flipped),
                  still_bad=sum(bad(r) for r in rows_a), bad_before=sum(bad(r) for r in rows_b))
    for unit in sorted(set(ob) & set(oa)):
        eb, ea = ob[unit], oa[unit]
        # Even byte-identical objects can change meaning through symbols.txt.
        # Inspect all objects whose referenced/defined mapped addresses changed.
        def addresses(prog, e):
            return [(symbol_addr(prog,e,unit,s),storage_addr(prog,e,unit,s)) for s in e.syms]
        if eb.raw == ea.raw and addresses(pb, eb) == addresses(pa, ea):
            report['identical'] += 1
            continue
        if eb.raw[:32] != ea.raw[:32] or eb.raw[36:46] != ea.raw[36:46]:
            problems.append(f'{unit}: ELF identity/header differs')
            continue
        if [r[:4]+r[5:] for r in section_contract(eb)] != [r[:4]+r[5:] for r in section_contract(ea)]:
            problems.append(f'{unit}: section contract differs (order/type/flags/size/alignment/link)')
            continue
        # Relocation count is checked in the image; table identity, linkage and
        # alignment must also remain stable, including empty relocation tables.
        if [r[:3]+r[4:] for r in relocation_contract(eb)] != [r[:3]+r[4:] for r in relocation_contract(ea)]:
            problems.append(f'{unit}: relocation section contract differs')
            continue
        # Unsized / unanchored symbols must retain exact placement too.
        def fixed_symbols(prog, e):
            return collections.Counter(('<anonymous>' if anonymous(s) else s['name'], s['sec'], s['off']) for i,s in enumerate(e.syms)
                if not (0 < s['sec'] < len(e.s) and ignored(e, s['sec']))
                and not (s['size'] and s['type'] in (1, 2) and storage_addr(prog, e, unit, s) is not None)
                and not (s['sec']==0 and any(si==i and not ignored(e,sec) for sec,o,t,si,ad in e.relocs) and target_key(prog,e,unit,i,0)[0]=='A'))
        fb,fa=fixed_symbols(pb,eb),fixed_symbols(pa,ea)
        if fb != fa:
            suffix=f': -{list((fb-fa).elements())} +{list((fa-fb).elements())}' if detail else ''
            problems.append(f'{unit}: unanchored/unsized symbol placement differs{suffix}')
            if not detail:continue
        try:
            ib,ia=unit_image(pb,unit),unit_image(pa,unit)
            shared=set(cover_b)&set(cover_a)
            removed=emission_permissions(pb,pa,unit,ib,ia,shared)
            added=emission_permissions(pa,pb,unit,ia,ib,shared)
            if not check_symbol_changes(pb,pa,unit,ib,ia,removed,added):
                suffix=f': -{list((symbol_contract(eb,pb,unit)-symbol_contract(ea,pa,unit)).elements())} +{list((symbol_contract(ea,pa,unit)-symbol_contract(eb,pb,unit)).elements())}' if detail else ''
                problems.append(f'{unit}: symbol metadata/name change is not a preserved image or exact duplicate{suffix}')
                if not detail:continue
            bodies=(pb._proof_image_bytes[unit],pa._proof_image_bytes[unit]) if detail else None
            n=compare_units(unit,ib,ia,allowed,problems,removed,added,exact_binding_trades(pb,pa,unit),bodies)
        except (ValueError, IndexError) as ex:
            problems.append(str(ex))
            continue
        if n is not None:
            report['retargeted_objects' if n else 'identical'] += 1
            report['retargeted_sites'] += n
    if expect_clean:
        left = [row_key(r) for r in rows_a if bad(r) and (r['unit'] in units or not units)]
        if left:
            problems.append(f'{len(left)} BAD rows remain for requested units: {left[:3]}')
    return problems, report, flipped


def selftest(after, dol, pointwarp_after=None):
    import subprocess
    args = [sys.executable, str(pathlib.Path(__file__).with_name('test_dispatchproof.py')), '--root', str(after)]
    if dol:
        args += ['--dol', str(dol)]
    if pointwarp_after:
        args += ['--pointwarp-after', str(pointwarp_after)]
    return subprocess.run(args).returncode == 0


def load_after(root, dol, before):
    """Read every object, sharing only byte-identical immutable ELF parses."""
    import copy
    prog=copy.copy(before);prog.root=pathlib.Path(root)
    prog.dol=dc.Dol(dc.original_dol(root,dol))
    prog.sy=dc.Symbols(prog.root/'config/SB4E01/symbols.txt')
    prog.bases,prog.ends=dc.load_splits(prog.root/'config/SB4E01/splits.txt')
    prog.elfs={};prog.errors=[]
    for unit,path in tree_objects(root).items():
        try:
            raw=path.read_bytes();old=before.elfs.get(unit)
            prog.elfs[unit]=old if old is not None and raw==old.raw else dc.ELF(path)
        except Exception as ex:prog.errors.append((unit,str(ex)))
    prog.inferred=collections.defaultdict(lambda:collections.defaultdict(list))
    prog.func_info={};prog.exact_stream=[];prog.nonexact=[]
    prog._chains=None;prog.ctor_inst={}
    prog.source_wrappers={};prog.source_wrapper_keys={}
    prog.vtable_anchors={};prog.vtable_anchor_reasons={}
    for key in list(vars(prog)):
        if key.startswith('_proof_'):delattr(prog,key)
    return prog


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--before')
    ap.add_argument('--after', required=True)
    ap.add_argument('--unit', action='append', default=[])
    ap.add_argument('--expect-clean', action='store_true')
    ap.add_argument('--selftest', action='store_true')
    ap.add_argument('--pointwarp-after', help='selftest only: tree with the compiled de4289f1 PointWarp object')
    ap.add_argument('--dol', help='original main.dol (default: <root>/orig/SB4E01/sys/main.dol); its sha1 must equal config/SB4E01/build.sha1')
    ap.add_argument('--out')
    ap.add_argument('--report-only', action='store_true', help='list unexplained differences and exit 0; never emit DISPATCH PROOF OK')
    a = ap.parse_args()
    a.dol = a.dol or None
    if a.selftest:
        ok = selftest(a.after, a.dol, a.pointwarp_after)
        print('SELFTEST OK' if ok else 'SELFTEST FAILED')
        sys.exit(0 if ok else 1)
    if not a.before:
        ap.error('--before required')
    pb = dc.Prog(a.before, a.dol)
    pa = load_after(a.after,a.dol,pb)
    rows_b, _ = pb.run()
    rows_a, _ = pa.run()
    problems, report, flipped = compare_trees(pb, pa, rows_b, rows_a, set(a.unit), a.expect_clean, detail=a.report_only)
    print(json.dumps(report))
    for k in flipped:
        print('FLIPPED', *[str(x)[:70] for x in k])
    for p in problems:
        print('PROBLEM', p)
    if a.out:
        payload=dict(report=report, problems=problems, flipped=[list(k) for k in flipped])
        if a.report_only:payload['report_only']=True
        pathlib.Path(a.out).write_text(json.dumps(payload, indent=1))
    if a.report_only:
        print(f'DISPATCH REPORT ONLY: {len(problems)} unexplained differences; no proof granted')
        return
    if problems:
        print('DISPATCH PROOF FAILED')
        sys.exit(1)
    print('DISPATCH PROOF OK')


if __name__ == '__main__':
    main()
