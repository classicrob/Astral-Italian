#!/usr/bin/env python3
"""Bounded THREE-TU qualification only. No full-tree/certificate/landing authority."""
import argparse,copy,fcntl,hashlib,importlib.util,json,os,re,shutil,signal,struct,subprocess,sys,time
from pathlib import Path
BASE='33e71ee880b426c0c7da03a724377200e874c417'
TIP='22d4c005612f83dc70a2a187c2b1a426ada66296'
DECODER='39bf295bc07c74e4407fa44322086cc31035db8b'
SPEC='3b0975d48ca4354862814f13de004121fb16fbc1b90f87fb8e9f78a65155f8d9'
DOL='abea6b0c498c4af90b17cfa08d54a01781186f097bee3e0761dbfb4838cbc281'
UNITS=['WorldMapResultTU13','WorldMapResultTU14','GrandGalaxyMapSequence']
# Name->before/after ELF value,size. Independent scopes, no body/data masking.
EXPECT={
'TU13':{'Start':((0x370,8),(0x390,8)),'End':((0x390,8),(0x370,8))},
'TU14':{'Wait':((0x240,12),(0x230,8)),'ZoomOut':((0x230,8),(0x240,12))}}
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def dump(p,x):Path(p).write_text(json.dumps(x,indent=2,sort_keys=True)+'\n')
def git(repo,*a):return subprocess.check_output(['git','-C',str(repo),*a])
def load(name,path):
 s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);sys.modules[name]=m;s.loader.exec_module(m);return m
def envelope(ob,before,after,scope,dp):
 a,b=ob.summarize(before),ob.summarize(after)
 for name in dp.SCOPES[scope]['symbols']:
  identity=next(k for k in EXPECT[scope] if ('Map'+k+'CFP' in name or 'End'+k+'CFP' in name))
  old,new=EXPECT[scope][identity]
  for summary,expected in ((a,old),(b,new)):
   rows=[r for r in summary['named symbols'] if r[0]==name]
   assert len(rows)==1 and rows[0][1:]==(18,0,'.text#0',*expected),(scope,name,rows)
  b['named symbols']=tuple(sorted((r[0],18,0,'.text#0',*old) if r[0]==name else r for r in b['named symbols']))
 assert a==b,'outside approved execute symbol value/size envelope: '+scope
 return True

def config_envelope(before,after,dp):
 maps={name:pair for scope in dp.SCOPES.values() for name,pair in scope['symbols'].items()}
 def filtered(text,phase):
  keep=[];found=set()
  for line in text.splitlines(keepends=True):
   name=line.split(' = ',1)[0]
   if name not in maps:keep.append(line);continue
   assert name not in found;found.add(name)
   match=re.fullmatch(re.escape(name)+r' = \.text:0x([0-9A-Fa-f]+); // type:function size:0x([0-9A-Fa-f]+)\n?',line)
   assert match and tuple(int(x,16) for x in match.groups())==maps[name][phase],name
  assert found==set(maps)
  return keep
 assert filtered(before,0)==filtered(after,1),'unapproved config remainder'
 return True

def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--repo',type=Path,required=True);ap.add_argument('--spec',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);ap.add_argument('--dol',type=Path,required=True)
 ap.add_argument('--execute-plan-sha256',help='Absent: prepare only. Present: requires separately allocated native.lock slot.')
 args=ap.parse_args(); repo=args.repo.resolve(); out=args.output.resolve(); support=Path.home()/'MacioGalaxy2-team/scratch-port/solP'
 assert sha(args.spec)==SPEC and git(repo,'rev-parse','HEAD').decode().strip()==TIP
 assert git(repo,'rev-parse','main').decode().strip()==BASE
 assert not git(repo,'status','--porcelain','--untracked-files=no')
 tools=Path.home()/'MacioGalaxy2/build'; graph=support/'source_graph.py'; macros=support/'dtk-source/assets/macros.inc'
 spec=json.loads(args.spec.read_text());versions=sorted({u['options']['mw_version'] for u in spec['units'].values()})
 paths=[Path(__file__).resolve(),graph,macros,tools/'tools/wibo',tools/'tools/wibo.real',tools/'tools/sjiswrap.exe',tools/'tools/dtk',tools/'tools/objdiff-cli',tools/'binutils/powerpc-eabi-as']
 for v in versions:paths.extend(p for p in (tools/'compilers'/v).rglob('*') if p.is_file())
 context={str(p):sha(p) for p in sorted(set(paths))}
 decoder_files={n:hashlib.sha256(git(repo,'show',DECODER+':tools/team/'+n+'.py')).hexdigest() for n in ('dispatchownership','dispatchproof','objsame')}
 plan={'schema':1,'status':'PREPARED_NOT_EXECUTED','base':BASE,'tip':TIP,'spec_sha256':SPEC,'decoder_commit':DECODER,'decoder_files':decoder_files,'input_files':context,'expected':EXPECT,'units':UNITS,'dol_sha256':DOL,'hard_seconds':420,'compile_jobs':1,'scope':'six fresh objects; no2426 closure/certificate or native runtime acceptance','source_trees':{'before':spec['before'],'after':spec['after']}}
 canonical=json.dumps(plan,sort_keys=True,separators=(',',':')).encode(); pin=hashlib.sha256(canonical).hexdigest()
 if args.execute_plan_sha256 is None:
  out.mkdir(exist_ok=False,parents=True);dump(out/'plan.json',plan);(out/'plan.sha256').write_text(pin+'\n');print(pin);return
 assert pin==args.execute_plan_sha256,'input/context/recipe drift'
 assert json.loads((out/'plan.json').read_text())==plan,'prepared plan changed'
 assert sha(args.dol)==DOL
 assert os.getpriority(os.PRIO_PROCESS,0)>=19,'nice19 required'
 assert shutil.disk_usage(out).free>=50*1024**3
 lock=(Path.home()/'MacioGalaxy2-team/slots/native.lock').open('a');fcntl.lockf(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
 work=out/'execution';work.mkdir(exist_ok=False);start=time.monotonic()
 def run(argv,cwd=None,log='commands.log'):
  remaining=420-(time.monotonic()-start);assert remaining>0
  with (work/log).open('ab') as f:
   proc=subprocess.Popen(argv,cwd=cwd,stdout=f,stderr=subprocess.STDOUT,start_new_session=True)
   try:
    code=proc.wait(timeout=remaining)
    if code:raise subprocess.CalledProcessError(code,argv)
   except BaseException:
    try:os.killpg(proc.pid,signal.SIGTERM)
    except ProcessLookupError:pass
    try:proc.wait(timeout=3)
    except subprocess.TimeoutExpired:
     os.killpg(proc.pid,signal.SIGKILL);proc.wait()
    raise
 def state(phase):
  (out/'STATE').write_text(json.dumps({'host':'host_db276ab2vt','source':[BASE,TIP],'spec':SPEC,'plan':pin,'phase':phase,'owner_pid':os.getpid(),'output':str(work),'resume':'Preserve failed execution; new prepare/output required, no partial acceptance.'},separators=(',',':'))+'\n')
 state('PREPARE_FRESH_PHASES')
 lib=work/'decoder';lib.mkdir()
 for n,h in decoder_files.items():
  p=lib/(n+'.py');p.write_bytes(git(repo,'show',DECODER+':tools/team/'+n+'.py'));assert sha(p)==h
 sys.path.insert(0,str(lib));ob=load('objsame',lib/'objsame.py');dp=load('dispatchproof',lib/'dispatchproof.py');ownership=load('dispatchownership',lib/'dispatchownership.py')
 for scope in ('TU13','TU14'):
  assert ownership.decode_original_dol(args.dol.read_bytes(),scope)
 results={'status':'RUNNING_NO_PRODUCTION_ACCEPTANCE','plan_sha256':pin,'phases':{},'controls':[]}
 before_map=git(repo,'show',BASE+':config/SB4E01/symbols.txt').decode();after_map=git(repo,'show',TIP+':config/SB4E01/symbols.txt').decode()
 config_envelope(before_map,after_map,dp)
 for scope in dp.SCOPES.values():
  for name,pair in scope['symbols'].items():
   for field in (0,1):
    lines=after_map.splitlines(keepends=True);index=next(i for i,l in enumerate(lines) if l.startswith(name+' = '));values=list(pair[1]);values[field]+=4
    lines[index]=name+' = .text:0x%08X; // type:function size:0x%X\n'%tuple(values)
    try:config_envelope(before_map,''.join(lines),dp)
    except AssertionError:pass
    else:raise AssertionError('wrong config address/size accepted')
    results['controls'].append({'control':'wrong-config-'+name+'-'+str(field),'rejected':True})
 try:
  for phase,rev in [('before',BASE),('after',TIP)]:
   state(phase+'_FRESH_THREE_TUS');tree=work/phase
   run(['git','clone','--shared','--no-checkout',str(repo),str(tree)])
   run(['git','checkout','--detach',rev],cwd=tree)
   inc=tree/'build/SB4E01/include';inc.mkdir(parents=True);shutil.copy2(macros,inc/'macros.inc')
   run([sys.executable,str(graph)],cwd=tree,log=phase+'-graph.log')
   targets=['build/SB4E01/src/Game/System/'+n+'.o' for n in UNITS]
   with (work/(phase+'-commands.txt')).open('wb') as f:subprocess.run(['ninja','-t','commands',*targets],cwd=tree,stdout=f,check=True,timeout=20)
   run(['ninja','-j','1',*targets],cwd=tree,log=phase+'-compile.log')
   with (work/(phase+'-deps.txt')).open('wb') as f:subprocess.run(['ninja','-t','deps'],cwd=tree,stdout=f,check=True,timeout=20)
   assert len(list((tree/'build/SB4E01/src').rglob('*.o')))==3
   dependencies={}
   for line in (work/(phase+'-deps.txt')).read_text().splitlines():
    if line.startswith('    '):
     name=line.strip();path=Path(name) if Path(name).is_absolute() else tree/name
     assert path.is_file(),'missing dependency: '+name
     dependencies[name]=sha(path)
   assert dependencies,'no dependency records'
   dump(work/(phase+'-dependency-hashes.json'),dependencies)
   results['phases'][phase]={'commands':sha(work/(phase+'-commands.txt')),'dependency_hashes':sha(work/(phase+'-dependency-hashes.json')),'objects':{p:sha(tree/p) for p in targets},'graph':sha(tree/'build.ninja'),'deps':sha(work/(phase+'-deps.txt'))}
  for scope in ('TU13','TU14'):
   name='WorldMapResult'+scope+'.o'; a=work/'before/build/SB4E01/src/Game/System'/name;b=work/'after/build/SB4E01/src/Game/System'/name
   assert len(ownership.decode_object(a,scope)['mismatches'])==2
   ownership.require_candidate(b,scope);ownership.decode_original_dol(args.dol.read_bytes(),scope);envelope(ob,a,b,scope,dp)
   # Real-ELF controls: change first text byte, each approved symbol value/size,
   # and each owning data relocation's addend. Never outer-digest-only rejection.
   original=b.read_bytes();shoff=struct.unpack_from('>I',original,32)[0];shents,shnum=struct.unpack_from('>HH',original,46)
   headers=[struct.unpack_from('>10I',original,shoff+i*shents) for i in range(shnum)]
   sections,symbols=ob.read_object(b);keys=ob.section_keys(sections)
   mutations=[]
   textindex=keys.index('.text#0');m=bytearray(original);m[headers[textindex][4]]^=1;mutations.append(('first-text',m))
   syms=next(h for h in headers if h[1]==2)
   names=list(dp.SCOPES[scope]['symbols'])
   for i,s in enumerate(symbols):
    if s['name'] in names:
     for label,off in [('value',4),('size',8)]:
      m=bytearray(original);pos=syms[4]+i*16+off;struct.pack_into('>I',m,pos,struct.unpack_from('>I',m,pos)[0]+4);mutations.append((s['name']+'-'+label,m))
   owned_offsets={'TU13':{0x48,0x68},'TU14':{0x58,0x68}}[scope]
   for h in headers:
    if h[1]==4 and keys[h[7]]=='.data#0':
     for pos in range(h[4],h[4]+h[5],12):
      if struct.unpack_from('>I',original,pos)[0] in owned_offsets:
       m=bytearray(original);struct.pack_into('>i',m,pos+8,4);mutations.append(('own-slot-addend-'+str(pos),m))
   assert len(mutations)==7
   symbol_indices=[i for i,s in enumerate(symbols) if s['name'] in names];assert len(symbol_indices)==2
   for h in headers:
    if h[1]==4 and keys[h[7]]=='.data#0':
     for pos in range(h[4],h[4]+h[5],12):
      if struct.unpack_from('>I',original,pos)[0] in owned_offsets:
       m=bytearray(original);info=struct.unpack_from('>I',m,pos+4)[0];old=info>>8
       assert old in symbol_indices
       new=next(i for i in symbol_indices if i!=old);struct.pack_into('>I',m,pos+4,(new<<8)|(info&255))
       mutations.append(('wrong-owning-target-'+str(pos),m))
   assert len(mutations)==9
   for label,data in mutations:
    mutant=work/(scope+'-mutant.o');mutant.write_bytes(data)
    try:envelope(ob,a,mutant,scope,dp)
    except (AssertionError,ValueError,IndexError):pass
    else:raise AssertionError('mutation accepted: '+label)
    if label.startswith('wrong-owning-target'):
     try:ownership.require_candidate(mutant,scope)
     except ValueError:pass
     else:raise AssertionError('ownership accepted wrong target')
    results['controls'].append({'scope':scope,'control':label,'rejected':True,'sha256':sha(mutant)})
   results[scope]={'ownership':ownership.require_candidate(b,scope),'envelope':'strict remaining bytes/sections/relocations/symbols equal'}
  a=work/'before/build/SB4E01/src/Game/System/GrandGalaxyMapSequence.o';b=work/'after/build/SB4E01/src/Game/System/GrandGalaxyMapSequence.o'
  assert ob.summarize(a)==ob.summarize(b),'typed Sequence changed Wii object'
  for p,h in context.items():assert sha(p)==h,'context drift: '+p
  results['status']='THREE_TU_COMPONENT_QUALIFICATION_ONLY_NOT_FULL_PROOF';results['seconds']=time.monotonic()-start
  state('COMPONENTS_COMPLETE_FULL2426_AND_SOURCE_CONTROLS_PENDING')
 finally:dump(work/'result.json',results)
 print(results['status'])
if __name__=='__main__':main()
