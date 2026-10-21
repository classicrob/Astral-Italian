"""Immutable bytes-only ELF32/PPC intake supplement, not qualification authority.

No file/process/compiler API. Every RELA uses its own sh_link symbol table.
Raw local ordinals/metadata and section/RELA identities are retained, never
silently normalized by this intake. Actual family raw-allowance review is open.
"""
import hashlib
import struct
from crossed_dispatch_component import require,relocation_width
import objsame


def read_snapshot(raw):
    require(type(raw) is bytes and 52<=len(raw)<=16*1024**2,'bounded immutable ELF bytes')
    require(raw[:7]==b'\x7fELF\x01\x02\x01','ELF32BE version required')
    require(struct.unpack_from('>HHI',raw,16)==(1,20,1),'relocatable PPC ELF required')
    shoff=struct.unpack_from('>I',raw,32)[0]
    ehsize,_,_,entsize,count,names_index=struct.unpack_from('>6H',raw,40)
    require(ehsize==52 and entsize==40 and 0<count<4096 and 0<names_index<count,'ELF header/table layout')
    require(shoff>=52 and shoff+count*40<=len(raw),'bounded section header table')
    headers=[struct.unpack_from('>10I',raw,shoff+i*40) for i in range(count)]
    require(headers[0]==(0,)*10,'null section header')
    for i,h in enumerate(headers):
        if i==0:continue
        require(h[8]==0 or h[8]&(h[8]-1)==0,'invalid section alignment')
        require(h[1]==8 or h[4]+h[5]<=len(raw),'truncated section payload')
    require(headers[names_index][1]==3,'section name table must be STRTAB')
    def data(index):
        h=headers[index];return b'' if h[1]==8 else raw[h[4]:h[4]+h[5]]
    def string(index,offset):
        require(0<=index<count and headers[index][1]==3,'invalid STRTAB link')
        payload=data(index);require(type(offset) is int and 0<=offset<len(payload),'string offset out of range')
        end=payload.find(b'\0',offset);require(end>=0,'unterminated ELF string')
        return payload[offset:end].decode('latin1')
    sections=[]
    for i,h in enumerate(headers):
        name='' if i==0 else string(names_index,h[0])
        sections.append({'name':name,'type':h[1],'flags':h[2],'size':h[5],'link':h[6],'info':h[7],
                         'entsize':h[9],'align':h[8],'data':data(i)})
    keys=objsame.section_keys(sections);symtabs={};raw_symbols=[]
    for i,h in enumerate(headers):
        if h[1]!=2:continue
        require(h[9]==16 and h[5]%16==0 and h[6]<count and headers[h[6]][1]==3,'SYMTAB layout/link')
        payload=data(i);rows=[];require(h[7]<=len(payload)//16,'SYMTAB first-nonlocal bound')
        for ordinal in range(len(payload)//16):
            no,value,size,info,other,index=struct.unpack_from('>IIIBBH',payload,ordinal*16)
            name=string(h[6],no)
            require(index in (0,0xfff1,0xfff2) or 0<index<count,'invalid symbol section index')
            if index not in (0,0xfff1,0xfff2):require(value+size<=headers[index][5],'full raw symbol extent')
            require((info>>4)==0 if ordinal<h[7] else (info>>4)!=0,'SYMTAB local partition')
            row={'name':name,'value':value,'size':size,'info':info,'other':other,'shndx':index}
            rows.append(row);raw_symbols.append([i,ordinal,no,name,value,size,info,other,index])
        symtabs[i]=rows
    require(bool(symtabs),'SYMTAB required')
    semantic={}
    for i,s in enumerate(sections):
        if s['name'] in objsame.SKIP or s['type'] in (2,3,4) or not s['name']:continue
        semantic['section '+keys[i]]=(s['type'],s['flags'],s['size'],s['align'],s['data'])
    semantic['named symbols']=tuple(sorted((s['name'],s['info'],s['other'],objsame.section_name(sections,s['shndx'],keys),s['value'],s['size'])
      for symbols in symtabs.values() for s in symbols if s['name'] and not objsame.LOCAL_LABEL.match(s['name'])
      and objsame.section_name(sections,s['shndx']) not in objsame.SKIP))
    raw_relocations=[]
    for i,h in enumerate(headers):
        if h[1]!=4:continue
        require(h[9]==12 and h[5]%12==0 and h[6] in symtabs and 0<h[7]<count,'RELA own sh_link/sh_info/layout')
        target_section=sections[h[7]];target_key=keys[h[7]];rows=[];symbols=symtabs[h[6]]
        for ordinal in range(h[5]//12):
            offset,info,addend=struct.unpack_from('>IIi',data(i),ordinal*12);index=info>>8;typ=info&255
            require(index<len(symbols),'RELA symbol ordinal outside linked table')
            # Debug relocation types have no admission authority here; preserve
            # raw metadata while validating their symbol link and start bounds.
            debug=target_section['name'] in objsame.SKIP
            width=1 if debug else relocation_width(typ)
            require(offset+width<=target_section['size'],'full raw relocation write extent')
            symbol=symbols[index]
            raw_relocations.append([i,ordinal,h[6],h[7],offset,info,addend,index,
                 [symbol[k] for k in ('name','info','other','shndx','value','size')]])
            rows.append((offset,typ,addend,objsame.symbol_key(sections,symbol,keys)))
        if target_section['name'] not in objsame.SKIP and sections[i]['name'] not in objsame.SKIP:
            k='relocations for '+target_key;semantic[k]=semantic.get(k,())+tuple(sorted(rows))
    return {'sha256':hashlib.sha256(raw).hexdigest(),'bytes':len(raw),'semantic':semantic,
      'raw_metadata':{'elf_header':raw[:52].hex(),
        'sections':[[i,keys[i],list(h)] for i,h in enumerate(headers)],
        'payload_sha256':[[i,hashlib.sha256(data(i)).hexdigest()] for i in range(count)],
        'symbols':raw_symbols,'relocations':raw_relocations},
      'raw_qualification':False,'actual_admission':'DISABLED_PENDING_INDEPENDENT_RAW_ENVELOPE_REVIEW'}


def strict_raw_metadata(a,b):
    """No automatic raw-symbol/section/RELA allowance or masking. Extra delta HOLD."""
    require(a['raw_metadata']==b['raw_metadata'],'raw metadata differs; no reviewed raw allowance')
    return {'status':'RAW_METADATA_IDENTICAL_ONLY','qualification':False}
