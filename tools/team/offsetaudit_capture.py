#!/usr/bin/env python3
"""Explicit native AST producer for offsetaudit; never a verdict classifier.

Based on the enemies Sol E source-pinned capture prototype. No compiler runs
without --run. Transient full ASTs are hashed, reduced, then deleted. Original
and production-encoded token correspondence is checked, never guessed.
"""
from pathlib import Path
import argparse,bisect,concurrent.futures,hashlib,importlib.util,json,shutil,subprocess,sys,time

ROOT=LEGACY=OUT=None
INPUT=CTX=AUD=None
PRODUCER_SHA=ENCODER_SHA=AUDIT_SHA=COMPILER=None
TIMEOUT=120
CONTEXT_PINS={}

def verify_source_pins():
 for name,expected in INPUT['files'].items():
  path=ROOT/name
  if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest()!=expected:
   raise ValueError('source pin drift: '+name)

def verify_dependencies(dependencies):
 for name,value in dependencies.items():
  p=Path(name)
  if not p.is_file() or hashlib.sha256(p.read_bytes()).hexdigest()!=value['sha256']:
   raise ValueError('dependency drift: '+name)
  original=value.get('original')
  if original and hashlib.sha256((ROOT/original['path']).read_bytes()).hexdigest()!=original['sha256']:
   raise ValueError('original dependency drift: '+original['path'])

BYFILE={}
MAPS={}
def tokenmap(name):
 if name in MAPS:return MAPS[name]
 raw=(ROOT/name).read_bytes();encoded=(LEGACY/name).read_bytes();head,body=encoded.split(b'\n',3)[:3],encoded.split(b'\n',3)[3];prefix=len(encoded)-len(body)
 orig=AUD.lex(raw.decode('latin1'));enc=AUD.lex(body.decode('latin1'))
 if len(orig)!=len(enc):MAPS[name]=None;return None
 allowed={'wchar_t':'PetariChar16','wcslen':'petari_utf16_wcslen','wcscpy':'petari_utf16_wcscpy','wcsncpy':'petari_utf16_wcsncpy','wcscmp':'petari_utf16_wcscmp','wcschr':'petari_utf16_wcschr','swprintf':'petari_utf16_swprintf','vswprintf':'petari_utf16_vswprintf','snprintf':'petari_msl_snprintf','vsnprintf':'petari_msl_vsnprintf','sprintf':'petari_msl_sprintf','vsprintf':'petari_msl_vsprintf'}
 for x,y in zip(orig,enc):
  if allowed.get(x.text,x.text)!=y.text:MAPS[name]=None;return None
 result=([x.start+prefix for x in enc],[(x,y.start+prefix,y.end+prefix) for x,y in zip(orig,enc)])
 MAPS[name]=result;return result

def original_offset(name,offset,end=False):
 m=tokenmap(name)
 if m is None:return None
 starts,pairs=m;i=(bisect.bisect_left(starts,offset) if end else bisect.bisect_right(starts,offset))-1
 if i<0:return None
 orig,begin,finish=pairs[i]
 if offset>finish:return None
 if end or offset==finish:return orig.end
 return orig.start+min(offset-begin,len(orig.text))

def name_for(path):
 if not path:return None
 for prefix in [str(LEGACY)+'/',str(ROOT)+'/']:
  if path.startswith(prefix):return path[len(prefix):]
 return None

INTEREST={'ParmVarDecl','FieldDecl','VarDecl','EnumConstantDecl','FunctionDecl','CXXMethodDecl','CXXConstructorDecl','CXXDestructorDecl','CXXRecordDecl','RecordDecl','TypedefDecl','TypeAliasDecl','CXXStaticCastExpr','CXXReinterpretCastExpr','CXXConstCastExpr','CXXDynamicCastExpr','CStyleCastExpr','CXXFunctionalCastExpr','ImplicitCastExpr','BinaryOperator','CompoundAssignOperator','UnaryOperator','ArraySubscriptExpr','MemberExpr','DeclRefExpr','CallExpr','CXXMemberCallExpr','CXXOperatorCallExpr','CXXConstructExpr','IntegerLiteral','FloatingLiteral','UnaryExprOrTypeTraitExpr','ParenExpr','InitListExpr','CXXCtorInitializer'}

def reduce_ast(tree):
 state={'file':None};facts=[];declarations={};constructor_references={};mapping_errors=set()
 def loc(raw):
  if not isinstance(raw,dict):return None
  if 'spellingLoc' in raw or 'expansionLoc' in raw:
   spelling=loc(raw.get('spellingLoc'));expansion=loc(raw.get('expansionLoc'));chosen=expansion or spelling
   return dict(chosen,macro={'spelling':spelling,'expansion':expansion}) if chosen else None
  if 'file' in raw:state['file']=raw['file']
  if 'offset' not in raw:return None
  return {'file':name_for(state['file']),'offset':raw['offset'],'tokLen':raw.get('tokLen',0),'line':raw.get('presumedLine',raw.get('line')),'col':raw.get('col')}
 def compact(n,depth=0):
  if not isinstance(n,dict):return None
  d={k:n[k] for k in ['id','kind','name','type','castKind','opcode','value','valueCategory','isArrow','referencedMemberDecl','referencedDecl','anyInit','path','isInvalid','isImplicit'] if k in n}
  if depth<3 and n.get('inner'):d['children']=[compact(x,depth+1) for x in n['inner'] if isinstance(x,dict)]
  return d
 def walk(n,parents):
  if not isinstance(n,dict):return
  kind=n.get('kind');ownloc=None;begin=None;end=None;parententry=None
  for key,value in n.items():
   if key=='loc':ownloc=loc(value)
   elif key=='range':begin=loc(value.get('begin'));end=loc(value.get('end'))
   elif isinstance(value,list):
    if key in ['inner','array_filler']:
     for child in value:walk(child,parents+([{'kind':kind,'name':n.get('name'),'id':n.get('id'),'type':n.get('type')} ] if kind else []))
    else:
     for child in value:
      if isinstance(child,dict) and 'kind' in child:walk(child,parents)
   elif isinstance(value,dict) and 'kind' in value:
    if key=='anyInit' and value.get('id'):
     constructor_references.setdefault(value['id'],[]).append(parents[-1] if parents else None)
    walk(value,parents)
  if kind not in INTEREST:return
  if kind.endswith('Decl') and n.get('id') and n.get('name'):
   # anyInit/referencedDecl previews carry an ID/type, not declaration ownership.
   # A later location-less preview must not erase the full header declaration.
   if ownloc or not declarations.get(n['id'],{}).get('location'):
    declarations[n['id']]={'id':n['id'],'kind':kind,'name':n['name'],'type':n.get('type'),'location':ownloc,'range':{'begin':begin,'end':end},'bases':n.get('bases'),'parent':parents[-1] if parents else None,'isInvalid':n.get('isInvalid',False)}
  if not begin or not end or begin['file']!=end['file'] or begin['file'] not in BYFILE:return
  name=begin['file'];start=original_offset(name,begin['offset']);finish=original_offset(name,end['offset']+end['tokLen'],True)
  if start is None or finish is None:
   mapping_errors.add(name);return
  rows=BYFILE[name]
  overlap=[r['id'] for r in rows if r['byte_offset']<finish and r['end_byte_offset']>start]
  if not overlap:return
  fact=compact(n);fact.update(file=name,start=start,end=finish,rows=overlap,parents=parents[-4:]);
  raw_source=(ROOT/name).read_bytes()
  validation=None
  if kind=='MemberExpr' and n.get('name'):
   token_start=original_offset(name,end['offset'])
   validation=token_start is not None and raw_source[token_start:finish].decode('latin1')==n['name']
  elif kind in {'CXXStaticCastExpr','CXXReinterpretCastExpr','CXXConstCastExpr','CXXDynamicCastExpr','CStyleCastExpr'}:
   keyword={'CXXStaticCastExpr':'static_cast','CXXReinterpretCastExpr':'reinterpret_cast','CXXConstCastExpr':'const_cast','CXXDynamicCastExpr':'dynamic_cast','CStyleCastExpr':'('}[kind]
   validation=raw_source[start:finish].decode('latin1').startswith(keyword)
  fact['token_validation']=validation
  if begin.get('macro') or end.get('macro'):fact['macro_provenance']={'begin':begin.get('macro'),'end':end.get('macro')}
  facts.append(fact)
 walk(tree,[])
 for key,parents in constructor_references.items():
  if key in declarations:declarations[key]['constructor_references']=parents
 # Keep exact referenced declaration facts needed to interpret the selected nodes.
 refs=set()
 def references(n):
  if isinstance(n,dict):
   for key,value in n.items():
    if key=='referencedMemberDecl' and isinstance(value,str):refs.add(value)
    elif key=='referencedDecl' and isinstance(value,dict):refs.add(value.get('id'))
    elif key=='typeAliasDeclId' and isinstance(value,str):refs.add(value)
    elif key=='anyInit' and isinstance(value,dict):refs.add(value.get('id'))
    elif key in {'children','type','parents'}:references(value)
  elif isinstance(n,list):
   for x in n:references(x)
 for fact in facts:
  references(fact)
  refs.update(p.get('id') for p in fact.get('parents',[]) if p.get('id'))
 return facts,{key:value for key,value in declarations.items() if key in refs},sorted(mapping_errors)


def collect(name):
 dest=OUT/'ast';dest.mkdir(exist_ok=True);key=hashlib.sha256(name.encode()).hexdigest()[:16];report=dest/(key+'.json')
 if report.exists():
  saved=json.loads(report.read_text())
  if saved.get('source_hash')==INPUT['files'][name] and saved.get('producer_sha256')==PRODUCER_SHA and saved.get('source_mapping_tool_sha256')==AUDIT_SHA and saved.get('legacy_encoder_sha256')==ENCODER_SHA and saved.get('compiler')==COMPILER:
   verify_dependencies(saved['dependencies'])
   return saved['summary']
  raise RuntimeError('stale cached AST projection: '+str(report))
 ast=dest/(key+'.full.json');diag=dest/(key+'.diagnostics.txt');dep=dest/(key+'.d');start=time.time()
 command=CTX['command']+['-MMD','-MF',str(dep),'-Xclang','-ast-dump=json',str(LEGACY/name)]
 if name.endswith(('.h','.hpp','.hh','.hxx','.inc','.inl')):command=CTX['command']+['-x','c++','-MMD','-MF',str(dep),'-Xclang','-ast-dump=json',str(LEGACY/name)]
 verify_source_pins()
 with ast.open('w') as output,diag.open('w') as errors:run=subprocess.run(command,cwd=ROOT,stdout=output,stderr=errors,timeout=TIMEOUT)
 verify_source_pins()
 digest=hashlib.sha256(ast.read_bytes()).hexdigest();facts=[];decls={};maps=[];error=None
 try:
  with ast.open() as stream:tree=json.load(stream)
  facts,decls,maps=reduce_ast(tree);del tree
 except (ValueError,RecursionError) as exc:error=str(exc)
 dependencies={}
 if dep.exists():
  paths=dep.read_text().replace('\\\n','').split(': ',1)[-1].split()
  for path in paths:
   f=Path(path)
   if f.is_file():
    dependencies[path]={'sha256':hashlib.sha256(f.read_bytes()).hexdigest()}
    original=name_for(str(f))
    if original and (ROOT/original).is_file():dependencies[path]['original']={'path':original,'sha256':hashlib.sha256((ROOT/original).read_bytes()).hexdigest()}
    if str(f) not in CONTEXT_PINS or CONTEXT_PINS[str(f)]!=dependencies[path]['sha256']:
     raise ValueError('dependency absent/changed since pre-capture snapshot: '+str(f))
 verify_dependencies(dependencies)
 summary={'source':name,'exit':run.returncode,'facts':len(facts),'seconds':round(time.time()-start,2),'ast_sha256':digest,'mapping_errors':maps,'parse_error':error,'evidence_file':str(report.relative_to(OUT))}
 result={'summary':summary,'command':command,'source_base':CTX['source_base'],'source_hash':INPUT['files'][name],'dependencies':dependencies,'producer_sha256':PRODUCER_SHA,'source_mapping_tool_sha256':AUDIT_SHA,'legacy_encoder_sha256':ENCODER_SHA,'compiler':COMPILER,'facts':facts,'referenced_declarations':decls}
 report.write_text(json.dumps(result,separators=(',',':'))+'\n');ast.unlink();dep.unlink(missing_ok=True)
 manifests=OUT/'manifests';manifests.mkdir(exist_ok=True)
 manifest={'schema':1,'format':'offsetaudit_compact_ast_v1','primary_source':name,
           'ast':'../ast/'+report.name,'ast_sha256':hashlib.sha256(report.read_bytes()).hexdigest(),
           'files':INPUT['files'],'producer':{'kind':'native_production_clang_ast',
           'command':command,'producer_sha256':PRODUCER_SHA,'encoder_sha256':ENCODER_SHA,
           'source_mapping_tool_sha256':AUDIT_SHA,'compiler':COMPILER}}
 (manifests/(key+'.json')).write_text(json.dumps(manifest,indent=2)+'\n')
 return summary


def main(argv=None):
 global ROOT,LEGACY,OUT,INPUT,CTX,AUD,BYFILE,MAPS,PRODUCER_SHA,ENCODER_SHA,AUDIT_SHA,COMPILER,TIMEOUT,CONTEXT_PINS
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--plan',required=True,help='schema1 root/inventory/command/optional sources; explicit compiler argv, no shell')
 parser.add_argument('--out',required=True,type=Path)
 parser.add_argument('--jobs',type=int,default=1)
 parser.add_argument('--timeout',type=int,default=120)
 parser.add_argument('--run',action='store_true',help='explicitly authorize the plan compiler probes; otherwise dry-run')
 parser.add_argument('--source',action='append',help='limit to exact files from the pinned inventory')
 args=parser.parse_args(argv)
 if args.jobs<1:parser.error('--jobs must be positive')
 plan_path=Path(args.plan).resolve();plan=json.loads(plan_path.read_text())
 if plan.get('schema')!=1:parser.error('expected schema1 plan')
 ROOT=Path(plan['root']).resolve();OUT=args.out.resolve();OUT.mkdir(parents=True,exist_ok=True)
 inventory=Path(plan['inventory']);inventory=inventory if inventory.is_absolute() else plan_path.parent/inventory
 INPUT=json.loads(inventory.read_text());BYFILE={};MAPS={};TIMEOUT=args.timeout
 for row in INPUT['rows']:BYFILE.setdefault(row['file'],[]).append(row)
 for rows in BYFILE.values():rows.sort(key=lambda r:r['byte_offset'])
 sources=args.source or plan.get('sources') or sorted(name for name in BYFILE if name.startswith('src/') and name.endswith(('.cpp','.c','.cc','.cxx')))
 if any(name not in INPUT['files'] for name in sources):parser.error('source outside pinned inventory')
 command=plan['command']
 if not isinstance(command,list) or '-fsyntax-only' not in command:parser.error('command must be explicit syntax-only argv')
 if not args.run:
  print(json.dumps({'mode':'dry-run','compiler_executed':False,'sources':sources,'command':command}));return 0
 verify_source_pins()
 spec=importlib.util.spec_from_file_location('offsetaudit_native_capture_inventory',Path(__file__).with_name('offsetaudit.py'));AUD=importlib.util.module_from_spec(spec);sys.modules[spec.name]=AUD;spec.loader.exec_module(AUD)
 AUDIT_SHA=hashlib.sha256(Path(__file__).with_name('offsetaudit.py').read_bytes()).hexdigest()
 encoder=ROOT/'native/tools/encode_legacy_sources.py';spec=importlib.util.spec_from_file_location('offsetaudit_native_capture_encoder',encoder);enc=importlib.util.module_from_spec(spec);spec.loader.exec_module(enc)
 LEGACY=OUT/'legacy';enc.prepare(ROOT,LEGACY)
 CONTEXT_PINS={}
 for directory in [ROOT/'src',ROOT/'include',ROOT/'libs',ROOT/'native',LEGACY]:
  for path in directory.rglob('*'):
   if path.is_file() and path.suffix in {'.c','.cpp','.cc','.cxx','.h','.hpp','.hh','.hxx','.inc','.inl'}:
    CONTEXT_PINS[str(path)]=hashlib.sha256(path.read_bytes()).hexdigest()
 # Use production legacy include precedence, not raw wchar_t/UTF8 assumptions.
 expanded=[arg.replace('{root}',str(ROOT)).replace('{legacy}',str(LEGACY)) for arg in command]
 CTX={'command':expanded,'source_base':INPUT.get('source_base',plan.get('source_base'))}
 PRODUCER_SHA=hashlib.sha256(Path(__file__).read_bytes()).hexdigest();ENCODER_SHA=hashlib.sha256(encoder.read_bytes()).hexdigest()
 version=subprocess.run([plan.get('compiler','clang++'),'--version'],capture_output=True,text=True,timeout=args.timeout)
 COMPILER={'command':expanded,'version':version.stdout+version.stderr}
 executable=shutil.which(plan.get('compiler','clang++'))
 if executable:
  executable=Path(executable).resolve();COMPILER.update(executable=str(executable),executable_sha256=hashlib.sha256(executable.read_bytes()).hexdigest())
 (OUT/'producer.json').write_text(json.dumps({'schema':1,'source_base':CTX['source_base'],'source_pins':INPUT['files'],'producer_sha256':PRODUCER_SHA,'encoder_sha256':ENCODER_SHA,'compiler':COMPILER,'plan':plan},indent=2)+'\n')
 result=[]
 with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
  for row in pool.map(collect,sources):
   result.append(row)
   if len(result)%10==0 or row['exit']:print(json.dumps({'done':len(result),'total':len(sources),**row}),flush=True)
 (OUT/'ast-summary.json').write_text(json.dumps(result,indent=2)+'\n')
 print(json.dumps({'sources':len(result),'passing':sum(x['exit']==0 for x in result),'unmapped_files':sorted({f for r in result for f in r['mapping_errors']}),'not_a_verdict':True}))
 return 0 if all(r['exit']==0 and not r['parse_error'] for r in result) else 1

if __name__=='__main__':
 sys.exit(main())
