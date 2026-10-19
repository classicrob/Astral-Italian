#!/usr/bin/env python3
"""Read-only registered-function direct-BL identity advisory; no score exclusions."""
import argparse,collections,concurrent.futures,hashlib,json,re,struct,subprocess,sys
from pathlib import Path
def require(ok, message):
 if not ok:raise ValueError(message)

class ELF:
 def __init__(self,p):
  self.raw=Path(p).read_bytes();h=struct.unpack_from('>16sHHIIIIIHHHHHH',self.raw);require(h[0][:6]==b'\x7fELF\x01\x02','expected big-endian ELF32');self.s=[struct.unpack_from('>IIIIIIIIII',self.raw,h[6]+i*h[11]) for i in range(h[12])];self.c=[b'' if s[1]==8 else self.raw[s[4]:s[4]+s[5]] for s in self.s];sn=self.c[h[13]];self.names=[sn[s[0]:sn.find(b'\0',s[0])].decode() for s in self.s];tabs={};self.f={};self.rel={}
  for i,s in enumerate(self.s):
   if s[1]!=2:continue
   strings=self.c[s[6]];tab=[]
   for off in range(0,s[5],s[9]):
    n,a,z,t,_,sec=struct.unpack_from('>IIIBBH',self.c[i],off);name=strings[n:strings.find(b'\0',n)].decode();sym=(name,a,z,sec);tab.append(sym)
    if t&15==2 and z and 0<sec<len(self.s):require(name not in self.f,'duplicate ELF function '+name);self.f[name]=sym
   tabs[i]=tab
  for i,s in enumerate(self.s):
   if s[1]!=4:continue
   for off in range(0,s[5],s[9]):
    a,info,add=struct.unpack_from('>IIi',self.c[i],off);key=(s[7],a&~3);require(key not in self.rel,'multiple relocations on instruction');self.rel[key]=(info&255,tabs[s[6]][info>>8],add)


def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def digest(v):return hashlib.sha256(json.dumps(v,sort_keys=True,separators=(',',':')).encode()).hexdigest()
class Catalog(dict):pass

def catalog(path):
 out=Catalog();all_records=[]
 for line in path.read_text().splitlines():
  m=re.match(r'(.+?) = \.text:0x([0-9A-Fa-f]+);.*type:function.*size:0x([0-9A-Fa-f]+)',line)
  if m:
   n=m[1];f={'address':int(m[2],16),'size':int(m[3],16)};all_records.append((n,f))
   if n not in out:out[n]=f
 groups=collections.defaultdict(list)
 for n,f in all_records:groups[n].append(f)
 out.duplicates={n:fs for n,fs in groups.items()if len({(f['address'],f['size'])for f in fs})>1}
 out.all_records=all_records;out.exact=collections.defaultdict(list)
 for n,f in all_records:
  identity={'callee':n,'addend':0}
  if identity not in out.exact[f['address']]:out.exact[f['address']].append(identity)
 return out

def read_dol(dol,address,size):
 for i in range(18):
  off=struct.unpack_from('>I',dol,i*4)[0];base=struct.unpack_from('>I',dol,0x48+i*4)[0];extent=struct.unpack_from('>I',dol,0x90+i*4)[0]
  if base<=address and address+size<=base+extent:return dol[off+address-base:off+address-base+size]
 return None

def branch_destination(w,site):
 if w>>26!=18 or not w&1:return None
 disp=w&0x3fffffc
 if disp&0x2000000:disp-=0x4000000
 return disp&0xffffffff if w&2 else (site+disp)&0xffffffff

def actual_identities(address,cat):
 exact=cat.exact.get(address,[])
 if exact:return exact
 return [{'callee':n,'addend':address-f['address']}for n,f in cat.all_records if f['address']<address<f['address']+f['size']]

def calls(e,name,cat,dol,target=False):
 _,start,size,sec=e.f[name];out=[];unresolved=[];indirect=[]
 if e.s[sec][1]==8:return [],[{'reason':'NOBITS function refused'}],[]
 if size%4:return [],[{'reason':'unaligned PPC function size'}],[]
 for off in range(0,size,4):
  w=struct.unpack_from('>I',e.c[sec],start+off)[0]
  if w>>26==19 and w&1 and (w>>1)&1023 in (16,528):indirect.append({'offset':off,'word':f'{w:08x}','reason':'linked indirect branch not compared'})
  if w>>26!=18 or not w&1:continue
  row={'offset':off,'word':f'{w:08x}'};rel=e.rel.get((sec,start+off));identity=None
  if rel:
   typ,sym,add=rel;row['relocation_type']=typ;row['callee']=sym[0];row['addend']=add
   if typ==10 and sym[0]:identity=(sym[0],add)
   else:unresolved.append({'offset':off,'reason':'unsupported/unnamed BL relocation','type':typ})
  else:
   # Section-local BL can be resolved only by a uniquely defined ELF function extent.
   dest=branch_destination(w,start+off);defined=[(n,f)for n,f in e.f.items()if f[3]==sec and f[1]<=dest<f[1]+f[2]]
   if len(defined)==1:
    n,f=defined[0];identity=(n,dest-f[1]);row.update(callee=n,addend=identity[1],resolution='unique section-local ELF function')
   else:unresolved.append({'offset':off,'reason':'unrelocated BL unresolved/ambiguous in ELF','section_offset':dest,'matches':[n for n,f in defined]})
  if identity:row['identity']=list(identity)
  if target:
   f=cat.get(name)
   if name in cat.duplicates:
    unresolved.append({'offset':off,'reason':'registered caller address ambiguous; actual DOL lookup refused','candidates':cat.duplicates[name]})
   elif f:
    site=f['address']+off;raw=read_dol(dol,site,4)
    if raw is None:unresolved.append({'offset':off,'reason':'actual target instruction not DOL-backed'})
    else:
     actual=struct.unpack('>I',raw)[0];dest=branch_destination(actual,site);row.update(actual_DOL_site=f'0x{site:08X}',actual_DOL_word=f'{actual:08x}')
     if dest is None:unresolved.append({'offset':off,'reason':'DOL instruction is not direct linked branch'})
     else:
      ids=actual_identities(dest,cat);row.update(actual_DOL_destination=f'0x{dest:08X}',actual_DOL_identities=ids)
      if len(ids)!=1:unresolved.append({'offset':off,'reason':'actual DOL destination missing/ambiguous mapped identity','matches':ids})
      if identity and identity[0]in cat and identity[0]not in cat.duplicates:
       expected=(cat[identity[0]]['address']+identity[1])&0xffffffff;row['relocation_matches_actual_destination']=expected==dest
       if expected!=dest:unresolved.append({'offset':off,'reason':'relocation/DOL destination disagreement','expected':f'0x{expected:08X}'})
  elif identity and identity[0]in cat and identity[0]not in cat.duplicates:row['mapped_destination_hint']=f"0x{(cat[identity[0]]['address']+identity[1])&0xffffffff:08X}"
  out.append(row)
 return out,unresolved,indirect

def compare(target,source,name,cat,dol):
 tc,tu,ti=calls(target,name,cat,dol,True);sc,su,si=calls(source,name,cat,dol,False)
 tseq=[tuple(x['identity'])if 'identity'in x else('UNRESOLVED',x['offset'])for x in tc];sseq=[tuple(x['identity'])if 'identity'in x else('UNRESOLVED',x['offset'])for x in sc]
 ct,cs=collections.Counter(tseq),collections.Counter(sseq);known=all('identity'in x for x in tc+sc)
 row={'function':name,'registered_address':f"0x{cat[name]['address']:08X}"if name not in cat.duplicates else None,'ambiguous_registered_addresses':cat.duplicates.get(name,[]),'target_size':target.f[name][2],'source_size':source.f[name][2],'ordered_identity_comparison_complete':known,'ordered_direct_calls_equal':known and tseq==sseq,'direct_call_multisets_equal':known and ct==cs,'target_only':[{'callee':k[0],'addend':k[1],'count':v}for k,v in (ct-cs).items()],'source_only':[{'callee':k[0],'addend':k[1],'count':v}for k,v in (cs-ct).items()],'target_calls':tc,'source_calls':sc,'unresolved':{'target':tu,'source':su},'indirect_calls_outside_scope':{'target':ti,'source':si}}
 for side,e in [('target',target),('source',source)]:
  _,a,z,s=e.f[name];row[side+'_body_sha256']=hashlib.sha256(e.c[s][a:a+z]).hexdigest()
 return row

def source_inputs(root,u):
 p=u.get('metadata',{}).get('source_path');paths=set()
 if p and(root/p).is_file():
  paths.add(p);own='include/'+str(Path(p).with_suffix('.hpp')).removeprefix('src/')
  if(root/own).is_file():paths.add(own)
  for inc in re.findall(r'^\s*#include\s*["<]([^">]+)[">]',(root/p).read_text(),re.M):
   for rel in ('include/'+inc,'src/'+inc,str(Path(p).parent/inc)):
    if(root/rel).is_file():paths.add(rel);break
 return {p:sha(root/p)for p in sorted(paths)}

def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--root',type=Path,required=True);ap.add_argument('--prefix',default='Game/Player/');ap.add_argument('--out',type=Path,required=True);ap.add_argument('--jobs',type=int,default=2);a=ap.parse_args();root=a.root.resolve();a.out.parent.mkdir(parents=True,exist_ok=True)
 def head():
  p=subprocess.run(['git','rev-parse','HEAD'],cwd=root,capture_output=True,text=True);return p.stdout.strip()if p.returncode==0 else None
 before=head();meta=['objdiff.json','config/SB4E01/symbols.txt','orig/SB4E01/sys/main.dol'];mpins={p:sha(root/p)for p in meta};cfg=json.loads((root/'objdiff.json').read_text());cat=catalog(root/'config/SB4E01/symbols.txt');dol=(root/meta[2]).read_bytes();units=[u for u in cfg['units']if u['name'].removeprefix('main/').startswith(a.prefix)];results=[]
 def run(u):
  name=u['name'];res={'unit':name,'rows':[],'function_bindings':[],'missing':[],'unpaired':[]};paths=[u.get(k)for k in ('target_path','base_path')]
  absent=[k for k in ('target_path','base_path')if not u.get(k)or not(root/u[k]).is_file()]
  if absent:res['missing']=absent;return res
  pins={p:sha(root/p)for p in paths};src=source_inputs(root,u);res.update(object_sha256=pins,source_inputs_sha256=src,source_input_status='captured-own-and-first-level-inputs'if src else'missing-or-unconfigured-source-path')
  try:
   t,s=ELF(root/paths[0]),ELF(root/paths[1]);registered=sorted(n for n in t.f if n in cat);res['registered_target_functions']=len(registered)
   for n in registered:
    if n not in s.f:res['unpaired'].append({'function':n,'missing':'source function'});continue
    r=compare(t,s,n,cat,dol);r['unit']=name;res['function_bindings'].append({k:r[k]for k in ['function','registered_address','target_size','source_size','target_body_sha256','source_body_sha256','ordered_identity_comparison_complete','ordered_direct_calls_equal','direct_call_multisets_equal']})
    if not r['ordered_direct_calls_equal']or r['unresolved']['target']or r['unresolved']['source']or r['indirect_calls_outside_scope']['target']or r['indirect_calls_outside_scope']['source']:res['rows'].append(r)
   res['source_only_registered_functions']=[n for n in s.f if n in cat and n not in t.f]
  except Exception as e:res['error']=str(e)
  res['unstable_inputs']=[p for p,h in {**pins,**src}.items()if not(root/p).is_file()or sha(root/p)!=h]
  return res
 results=list(concurrent.futures.ThreadPoolExecutor(a.jobs).map(run,units));after=head();unstablemeta=[p for p,h in mpins.items()if sha(root/p)!=h];rows=[r for u in results for r in u['rows']];bindings=[f for u in results for f in u['function_bindings']];stats={'units':len(results),'paired_registered_unit_functions':len(bindings),'ordered_direct_call_mismatches':sum(not f['ordered_direct_calls_equal']for f in bindings),'multiset_mismatches':sum(not f['direct_call_multisets_equal']for f in bindings),'unpaired_functions':sum(len(u['unpaired'])for u in results),'missing_units':sum(bool(u['missing'])for u in results),'error_units':sum('error'in u for u in results),'unstable_units':sum(bool(u.get('unstable_inputs'))for u in results)}
 payload={'advisory_only_no_bug_or_equivalence_claims':True,'root':str(root),'HEAD_before':before,'HEAD_after':after,'HEAD_stable':before is not None and before==after,'metadata_sha256':mpins,'unstable_metadata':unstablemeta,'tool_dependencies_sha256':{str(Path(__file__)):sha(Path(__file__))},'stats':stats,'ambiguous_catalog_names':cat.duplicates,'rows':rows,'units':[{k:v for k,v in u.items()if k!='rows'}for u in results],'limitations':['No report/score/masked100 exclusion; zero-call pairs included in bindings.','Raw ordered direct BL identities and multiplicities only, not call-local arguments/CFG/coverage or equivalence.','Names/addends are not normalized: aliases, save/restore helpers, inlining and harmless reordering are common false positives.','Tail branches, indirect linked calls and unreachable paths are not semantically compared; indirect sites explicitly inventoried.','Registered names come from symbols.txt function records; unit-local registered occurrences may repeat across units, not unique game-function coverage.','Actual DOL destinations are mapped conservatively; ambiguity/missing metadata explicitly unresolved.','Per-unit before/after source/object bytes and metadata pinned; not global atomic snapshot or compiler/header freshness proof.','ELF duplicate-name/parser errors are coverage gaps, never silently skipped; NOBITS bytes not read.']}
 a.out.write_text(json.dumps(payload,indent=2)+'\n');print(json.dumps(stats));return 1 if unstablemeta or not payload['HEAD_stable']or stats['unstable_units']else 0
if __name__=='__main__':sys.exit(main())
