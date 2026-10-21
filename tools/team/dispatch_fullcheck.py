#!/usr/bin/env python3
"""Combined dispatch semantic components, never production import authority."""
import copy,re

def require(ok,message):
 if not ok:raise ValueError('dispatch full: '+message)

def parse_config_symbols(text):
 result={}
 for line in text.splitlines():
  if not line.strip() or line.lstrip().startswith('#'):continue
  match=re.match(r'(.+?) = ([^:;]+):0x([0-9A-Fa-f]+);',line)
  require(match is not None,'unparsed symbol-map record')
  result.setdefault(match[1],[]).append((match[2],int(match[3],16)))
 return result

def check_all(summaries,plans,dp,config_symbols=None,phase=None):
 """Enumerate named/local/effective references into every byte of four spans.

 Undefined named targets resolve through the supplied full object inventory;
 local/section symbols resolve only within their actual owning object. A named
 alias is never silently normalized to an allowed reference. Unknown/midbody
 aliases reject, including a third reference that is equal in both phases.
 """
 scopes=set(plans);require(scopes=={'TU13','TU14'},'two separate scopes required')
 names={name:scope for scope,plan in plans.items() for name in plan['symbols']}
 endpoints={};definitions={}
 for unit,summary in summaries.items():
  for row in summary.get('named symbols',()):
   name,info,other,section,value,size=row
   if section!='UND' and not section.startswith(('special','bad')):
    definitions.setdefault(name,[]).append((unit,section,value,size))
   if name in names and section!='UND':
    require(name not in endpoints,'duplicate execute definition')
    endpoints[name]=(unit,section,value,size)
 require(set(endpoints)==set(names),'missing execute definition')
 expected=[]
 for plan in plans.values():expected.extend(tuple(r) for r in plan['references'])
 observed=[]
 for unit,summary in summaries.items():
  for key,rows in summary.items():
   if not key.startswith('relocations for '):continue
   section=key[len('relocations for '):]
   for offset,typ,addend,target in rows:
    hits=set()
    if target[0]=='sym':
     name=target[1]
     if name in names:hits.add(name)
     if config_symbols is not None:
      require(phase in ('before','after'),'symbol-map phase missing')
      for cfgsection,address in config_symbols.get(name,()):
       for plan in plans.values():
        for execute,mapping in plan['symbol_map'].items():
         start,width=mapping[phase]
         if cfgsection=='.text' and start<=address+addend<start+width:hits.add(execute)
     candidates=definitions.get(name,())
    elif target[0]=='local':candidates=[(unit,target[1],target[2],0)]
    else:raise ValueError('unknown relocation target kind')
    for owner,sec,val,size in candidates:
     for name,(own,esection,start,width) in endpoints.items():
      if owner==own and sec==esection and start<=val+addend<start+width:hits.add(name)
    for name in hits:
     # Exact raw named zero-addend slot is mandatory, not merely a resolved alias.
     row=(unit,section,offset,typ,addend,*target)
     require(target==('sym',name) and addend==0,'effective alias/midbody execute reference: '+repr(row))
     observed.append(row)
 require(sorted(observed)==sorted(expected),'extra/missing/crossed owning reference closure')
 return sorted(observed)

def compare_trees(before,after,plans,pins,bindings,dp,config_maps=None):
 require(set(plans)==set(pins)==set(bindings)=={'TU13','TU14'},'two independent plans/pins/bindings')
 units=set(before);require(units==set(after) and len(units)==2426,'full2426inventory required')
 require(config_maps is not None and set(config_maps)=={'before','after'},'both actual config symbol maps required')
 scoped=set()
 for scope,plan in plans.items():
  dp.validate_plan(plan,pins[scope]);dp.binding_check(plan,bindings[scope])
  require(set(plan['units'])==units,'plan unit inventory')
  scoped.add(dp.SCOPES[scope]['object'])
 for phase in ('before','after'):
  for plan in plans.values():
   for name,mapping in plan['symbol_map'].items():
    require(config_maps[phase].get(name)==[('.text',mapping[phase][0])],'actual config execute address/uniqueness')
 census={phase:check_all(tree,plans,dp,config_maps[phase],phase) for phase,tree in [('before',before),('after',after)]}
 for unit in sorted(units):
  a,b=copy.deepcopy(before[unit]),copy.deepcopy(after[unit])
  for scope,plan in plans.items():
   if unit!=dp.SCOPES[scope]['object']:continue
   for phase,summary in [('before',a),('after',b)]:
    for name,record in plan['symbols'].items():
     actual=[r for r in summary['named symbols'] if r[0]==name]
     require(len(actual)==1 and list(actual[0][1:])==record[phase],'actual execute symbol outside pinned envelope')
   b['named symbols']=tuple(sorted((r[0],*plan['symbols'][r[0]]['before']) if r[0] in plan['symbols'] else r for r in b['named symbols']))
  require(a==b,'unapproved object remainder: '+unit)
 return {'status':'FULL_SEMANTIC_COMPONENTS_ONLY_NOT_IMPORT_ACCEPTANCE','strict_objects':len(units-scoped),'scoped_objects':sorted(scoped),'effective_references':census,'production_acceptance':False}

def real_elf_controls(path,scope,ob,dp):
 """Thirteen isolated real-ELF changes per scope; source object never modified."""
 import struct
 raw=path.read_bytes();shoff=struct.unpack_from('>I',raw,32)[0];step,count=struct.unpack_from('>HH',raw,46)
 headers=[struct.unpack_from('>10I',raw,shoff+i*step) for i in range(count)]
 sections,symbols=ob.read_object(path);keys=ob.section_keys(sections);names=list(dp.SCOPES[scope]['symbols']);output={}
 def change(label,pos,fmt,value):
  data=bytearray(raw);struct.pack_into(fmt,data,pos,value);output[label]=bytes(data)
 ti=keys.index('.text#0');change('first-text',headers[ti][4],'>B',raw[headers[ti][4]]^1)
 di=keys.index('.data#0');change('outside-data',headers[di][4],'>B',raw[headers[di][4]]^1)
 symtab=next(h for h in headers if h[1]==2);indices=[i for i,s in enumerate(symbols) if s['name'] in names];require(len(indices)==2,'execute symbol indices')
 for i in indices:
  for label,field in [('value',4),('size',8)]:
   pos=symtab[4]+i*16+field;change('execute-'+str(i)+'-'+label,pos,'>I',struct.unpack_from('>I',raw,pos)[0]+4)
 other=next(i for i,s in enumerate(symbols) if s['name'] and s['name'] not in names and (s['info']&15)==2 and s['shndx']==ti)
 pos=symtab[4]+other*16+8;change('outside-symbol-size',pos,'>I',symbols[other]['size']+4)
 owned={'TU13':{0x48,0x68},'TU14':{0x58,0x68}}[scope]
 for h in headers:
  if h[1]!=4 or keys[h[7]]!='.data#0':continue
  for pos in range(h[4],h[4]+h[5],12):
   if struct.unpack_from('>I',raw,pos)[0] not in owned:continue
   change('own-slot-addend-'+str(pos),pos+8,'>i',4)
   info=struct.unpack_from('>I',raw,pos+4)[0];old=info>>8;require(old in indices,'owning slot names')
   target=next(i for i in indices if i!=old);change('own-slot-crossed-'+str(pos),pos+4,'>I',(target<<8)|(info&255))
 third=next((h[4] for h in headers if h[1]==4 and h[5]>=12 and keys[h[7]]=='.text#0'),None);require(third is not None,'non-owning text relocation required')
 info=struct.unpack_from('>I',raw,third+4)[0];change('extra-execute-reference',third+4,'>I',(indices[0]<<8)|(info&255))
 local=next(i for i,s in enumerate(symbols) if s['shndx']==ti and ((s['info']&15)==3 or not s['name'] or ob.LOCAL_LABEL.match(s['name'])))
 alias=bytearray(raw);struct.pack_into('>I',alias,third+4,(local<<8)|(info&255));struct.pack_into('>i',alias,third+8,symbols[indices[0]]['value']+4-symbols[local]['value']);output['extra-local-midbody-alias']=bytes(alias)
 require(len(output)==13,'exact thirteen real ELF controls')
 return output
