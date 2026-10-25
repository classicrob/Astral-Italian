#!/usr/bin/env python3
"""Read-only census of orphan terminal blr labels and four-byte-short functions.

Requires a source body exactly equal to the full original DOL range, plus no
static address/branch/relocation references to the orphan, before proposing a
size correction. Emits metadata proposals only; never edits build inputs.
"""
from __future__ import annotations
import argparse
from collections import defaultdict
import csv
import difflib
import json
from pathlib import Path
import re
import struct
import time

import dispatchcheck as dc
import splitscan as ss
import tuscan as ts


def resolve_target(elf, symbol, addend, catalog):
    name, value, _, section = symbol
    candidates = {s.absolute for s in elf.tuples.get(symbol, []) if s.absolute is not None}
    if len(candidates) == 1:
        return (next(iter(candidates))+addend) & 0xffffffff
    if len(candidates) > 1:
        return None
    if section in elf.origins:
        return (elf.origins[section]+value+addend) & 0xffffffff
    known = catalog.named(name)
    return (known.address+addend) & 0xffffffff if known else None


def branch_destination(word, site):
    opcode = word >> 26
    if opcode not in (16,18):
        return None
    mask,bits = (0xfffc,16) if opcode == 16 else (0x03fffffc,26)
    delta = ss.signed(word & mask,bits)
    return (delta if word & 2 else site+delta) & 0xffffffff


def encoded_reference(kind, word, site, destination):
    """Conservative original-operand bound when an import name has no address."""
    if kind==1:return word==destination
    if kind in (10,11,12,13):return branch_destination(word,site)==destination
    if kind in (3,4):return word&0xffff==destination&0xffff
    if kind==5:return word&0xffff==(destination>>16)&0xffff
    if kind==6:return word&0xffff==((destination+0x8000)>>16)&0xffff
    if kind==109:
        register=(word>>16)&31
        if register not in (0,2,13):return None
        base={0:0,2:dc.SDA2,13:dc.SDA13}[register]
        return (base+ss.signed(word&0xffff,16))&0xffffffff==destination
    return None


def source_proof(path, previous, dol):
    elf = ss.ELF(path)
    matches = [f for f in elf.functions() if f.name == previous.name]
    if len(matches) != 1:
        return dict(path=str(path),sha256=elf.sha256,reason='source symbol missing or ambiguous')
    function = matches[0]
    blob = elf.data[function.section][function.value:function.value+function.size]
    original = dol.bytes(previous.address,function.size)
    return dict(path=str(path),sha256=elf.sha256,name=function.name,source_size=function.size,
        target_size=previous.size,four_longer=function.size==previous.size+4,
        raw_equal=blob==original,source_hex=blob.hex(),original_hex=original.hex() if original else None)


def proposal_symbols(text, candidates):
    lines = text.splitlines(keepends=True)
    for row in candidates:
        previous,orphan = row['previous'],row['orphan']
        found = removed = 0
        for i,line in enumerate(lines):
            if line.startswith(previous['name']+' = '):
                match = re.search(r'\bsize:0x([0-9a-fA-F]+)',line)
                if not match or int(match[1],16)!=previous['size']:
                    raise ValueError('unexpected predecessor size')
                lines[i]=line[:match.start(1)]+format(previous['size']+4,'X')+line[match.end(1):];found+=1
            elif line.startswith(orphan['name']+' = '):
                lines[i]='';removed+=1
        if (found,removed)!=(1,1):
            raise ValueError('metadata identity is missing or duplicated')
    return ''.join(lines)


def census(root, base, out, overrides):
    started=time.monotonic();symbols=root/'config/SB4E01/symbols.txt'
    paths=[symbols,root/'config/SB4E01/splits.txt',root/'objdiff.json',root/'orig/SB4E01/sys/main.dol',
           Path(__file__),Path(ss.__file__),Path(dc.__file__),Path(ts.__file__)]
    hashes={str(path):ts.sha(path) for path in paths}
    text=symbols.read_text();catalog=ss.Catalog(text);dol=dc.Dol(root/'orig/SB4E01/sys/main.dol')
    splits=ss.Splits((root/'config/SB4E01/splits.txt').read_text())
    units=json.loads((root/'objdiff.json').read_text())['units'];byunit={u['name']:u for u in units}
    ending=defaultdict(list);small=[]
    for section,items in catalog.sections.items():
        if section not in ss.CODE:continue
        for symbol in items:
            if symbol.kind!='function':continue
            ending[symbol.address+symbol.size].append(symbol)
            if symbol.name.startswith('fn_') and symbol.size==4:small.append(symbol)
    returns=[]
    for symbol in small:
        if dol.word(symbol.address)!=0x4e800020:continue
        before=[s for s in ending[symbol.address] if not s.name.startswith('fn_')]
        owners=splits.owners(symbol.section,symbol.address)
        returns.append(dict(orphan=dict(name=symbol.name,address=ts.hx(symbol.address),size=4),
            address=symbol.address,previous_candidates=[dict(name=s.name,address=ts.hx(s.address),size=s.size) for s in before],
            unit='main/'+owners[0] if len(owners)==1 else '',references=[],source=[],previous_symbols=before))
    byaddress={r['address']:r for r in returns}
    for address,size,offset in dol.secs:
        words=[struct.unpack_from('>I',dol.raw,offset+i)[0] for i in range(0,size-3,4)]
        for i,word in enumerate(words):
            site=address+i*4
            if word in byaddress:byaddress[word]['references'].append(dict(kind='raw_word',site=ts.hx(site)))
            destination=branch_destination(word,site)
            if destination in byaddress:byaddress[destination]['references'].append(dict(kind='direct_branch',site=ts.hx(site),link=bool(word&1)))
            if word >> 26==15 and not (word >> 16)&31:
                register=(word>>21)&31;high=(word&0xffff)<<16
                for j,following in enumerate(words[i+1:i+13],1):
                    if following>>26==14 and (following>>16)&31==register:
                        value=(high+ss.signed(following&0xffff,16))&0xffffffff
                        if value in byaddress:
                            byaddress[value]['references'].append(dict(kind='possible_lis_addi',site=ts.hx(site),low_site=ts.hx(site+j*4)))
    inventory=[];errors=[];relocations=unresolved=unbounded=0;operand_exclusions=[]
    for unit in units:
        path=root/unit['target_path']
        if not path.is_file():errors.append(dict(unit=unit['name'],error='target missing'));continue
        try:elf=ss.ELF(path,True)
        except (ValueError,OSError,struct.error) as error:
            errors.append(dict(unit=unit['name'],error=str(error)));continue
        inventory.append(dict(unit=unit['name'],path=str(path),sha256=elf.sha256))
        for (section,offset),(kind,symbol,addend) in elf.relocations.items():
            relocations+=1;destination=resolve_target(elf,symbol,addend,catalog)
            if destination is None:
                unresolved+=1;origin=elf.origins.get(section)
                site=origin+offset if origin is not None else None
                word=dol.word(site if kind==1 else site&~3) if site is not None else None
                bounded=word is not None
                hits=[]
                for address,row in byaddress.items():
                    possible=encoded_reference(kind,word,site&~3,address) if word is not None else None
                    if possible is None:bounded=False
                    elif possible:
                        row['references'].append(dict(kind='unresolved_operand_possible',unit=unit['name'],
                            site=ts.hx(site),relocation_type=kind,symbol=symbol[0],addend=addend));hits.append(ts.hx(address))
                if not bounded:unbounded+=1
                operand_exclusions.append(dict(unit=unit['name'],symbol=symbol[0],relocation_type=kind,
                    addend=addend,site=ts.hx(site),word=f'{word:08X}' if word is not None else None,
                    bounded=bounded,possible_orphans=hits))
                continue
            if destination in byaddress:
                origin=elf.origins.get(section)
                byaddress[destination]['references'].append(dict(kind='target_relocation',unit=unit['name'],
                    section=elf.names[section],offset=offset,site=ts.hx(origin+offset) if origin is not None else '',
                    relocation_type=kind,symbol=symbol[0],symbol_section=symbol[3],addend=addend,
                    section_local_resolution=symbol[3] in elf.origins))
    accepted=[]
    for row in returns:
        unit=byunit.get(row['unit']);source=overrides.get(row['unit'])
        if source is None and unit and unit.get('base_path'):source=base/unit['base_path']
        for previous in row.pop('previous_symbols'):
            if source and source.is_file():
                try:row['source'].append(source_proof(source,previous,dol))
                except (ValueError,OSError,struct.error) as error:row['source'].append(dict(path=str(source),error=str(error)))
        proofs=[p for p in row['source'] if p.get('four_longer') and p.get('raw_equal')]
        row['proposable']=not row['references'] and not unbounded and len(row['previous_candidates'])==1 and len(proofs)==1
        if row['proposable']:
            row['previous']=dict(name=proofs[0]['name'],size=proofs[0]['target_size'],
                                 address=ts.hx(row['address']-proofs[0]['target_size']))
            accepted.append(row)
    patch=proposal_symbols(text,accepted)
    (out/'symbols.patch').write_text(''.join(difflib.unified_diff(text.splitlines(True),patch.splitlines(True),
        fromfile='a/config/SB4E01/symbols.txt',tofile='b/config/SB4E01/symbols.txt')))
    (out/'inventory.json').write_text(json.dumps(returns,indent=2)+'\n')
    (out/'proposal.json').write_text(json.dumps(accepted,indent=2)+'\n')
    (out/'unresolved-operand-bounds.json').write_text(json.dumps(operand_exclusions,indent=2)+'\n')
    with (out/'candidates.tsv').open('w') as stream:
        writer=csv.writer(stream,delimiter='\t',lineterminator='\n');writer.writerow(['unit','orphan','address','predecessor','new_size','static_refs','raw_source_proof','proposable'])
        for row in returns:
            previous=row.get('previous',{})
            writer.writerow([row['unit'],row['orphan']['name'],row['orphan']['address'],previous.get('name',''),
                previous.get('size',0)+4 if previous else '',len(row['references']),any(p.get('raw_equal') and p.get('four_longer') for p in row['source']),row['proposable']])
    if any(ts.sha(path)!=digest for path,digest in hashes.items()):raise ValueError('pinned inputs changed')
    meta=dict(target_objects=len(inventory),relocations=relocations,unresolved_relocations=unresolved,
        unbounded_relocations=unbounded,
        opaque_four_byte_functions=len(small),opaque_blr_functions=len(returns),proposals=len(accepted),errors=errors,
        seconds=time.monotonic()-started,input_sha256=hashes,objects=inventory,
        limitation='Static references only; possible lis/addi pairs deliberately over-approximate. Unknown dynamic pointer arithmetic cannot be excluded. No code bytes or function entry address changes are proposed.')
    (out/'meta.json').write_text(json.dumps(meta,indent=2)+'\n')
    print(json.dumps({k:v for k,v in meta.items() if k not in ['objects','input_sha256','limitation','errors']}),flush=True)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root',type=Path,default=Path.cwd())
    parser.add_argument('--base-root',type=Path,default=Path.home()/'MacioGalaxy2-wt/decomp3-land')
    parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--source-object',action='append',default=[],metavar='UNIT=PATH')
    args=parser.parse_args();overrides={}
    for value in args.source_object:
        name,path=value.split('=',1);overrides['main/'+ss.norm(name)]=Path(path).resolve()
    if args.out.exists() and any(args.out.iterdir()):parser.error('output directory must be empty')
    args.out.mkdir(parents=True,exist_ok=True)
    census(args.root.resolve(),args.base_root.resolve(),args.out.resolve(),overrides)


if __name__=='__main__':main()
