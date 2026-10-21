#!/usr/bin/env python3
"""Prepare/run full dispatch proof components. NEVER imports/lands/emits generic cert."""
import argparse,base64,copy,fcntl,hashlib,json,os,shutil,sys,tarfile,time
from pathlib import Path
import dispatch_preflight as p
import dispatch_fullcheck as check
import dispatch_full_budget as budget
HERE=Path(__file__).resolve().parent

def hashbytes(data):return hashlib.sha256(data).hexdigest()
def canonical(x):return json.dumps(x,sort_keys=True,separators=(',',':')).encode()
def record(path):return {'path':str(path),'sha256':p.sha(path)}
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--repo',type=Path,required=True);ap.add_argument('--spec',type=Path,required=True);ap.add_argument('--qualified',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);ap.add_argument('--dol',type=Path,required=True);ap.add_argument('--execute-contract-sha256');args=ap.parse_args()
 repo=args.repo.resolve();out=args.output.resolve();q=args.qualified.resolve();spec=json.loads(args.spec.read_bytes())
 assert p.sha(args.spec)==p.SPEC and spec['base']==p.BASE and len(spec['units'])==2426
 assert p.git(repo,'rev-parse','HEAD').decode().strip()==p.TIP and p.git(repo,'rev-parse','main').decode().strip()==p.BASE
 assert not p.git(repo,'status','--porcelain','--untracked-files=no')
 qualified=json.loads((q/'execution/result.json').read_text());qplan=json.loads((q/'plan.json').read_text())
 assert qualified['status']=='THREE_TU_COMPONENT_QUALIFICATION_ONLY_NOT_FULL_PROOF'
 assert p.sha(q/'execution/result.json')=='2ff50b4ad70f9f837d6420365a60387d24fee5bb0a49315d1ce37cc1a53182ed'
 context=dict(qplan['input_files'])
 for name in ('dispatch_fullproof.py','dispatch_fullcheck.py','dispatch_preflight.py','dispatch_package.py','dispatch_artifacts.py','dispatch_budget.py','dispatch_full_budget.py'):context[str(HERE/name)]=p.sha(HERE/name)
 for exe in (Path(sys.executable).resolve(),Path(shutil.which('ninja')).resolve()):context[str(exe)]=p.sha(exe)
 for path,h in context.items():assert p.sha(path)==h,'tool/context drift '+path
 decoder={name:p.git(repo,'show',p.DECODER+':tools/team/'+name+'.py') for name in ('dispatchproof','dispatchownership','objsame')}
 # Source-only preparation reads already qualified decoder and ELF summaries.
 import tempfile
 with tempfile.TemporaryDirectory(prefix='solP-dispatch-prepare-') as tmp:
  lib=Path(tmp)
  for name,data in decoder.items():(lib/(name+'.py')).write_bytes(data)
  sys.path.insert(0,str(lib));ob=p.load('objsame',lib/'objsame.py');dp=p.load('dispatchproof',lib/'dispatchproof.py')
  plans={}
  for scope in ('TU13','TU14'):
   unit=dp.SCOPES[scope]['object'];source=dp.SCOPES[scope]['source'];name=Path(unit).name
   summaries={phase:ob.summarize(q/'execution'/phase/'build/SB4E01'/unit) for phase in ('before','after')}
   bindings={'base':p.BASE,'tip':p.TIP,'source_trees':{phase:spec[phase] for phase in ('before','after')},'source_files':{phase:{source:hashbytes(p.git(repo,'show',rev+':'+source))} for phase,rev in [('before',p.BASE),('after',p.TIP)]},'config_files':{phase:{'config/SB4E01/symbols.txt':hashbytes(p.git(repo,'show',rev+':config/SB4E01/symbols.txt'))} for phase,rev in [('before',p.BASE),('after',p.TIP)]},'toolchain':spec['toolchain'],'generators':spec['generators'],'graphs':{phase:qualified['phases'][phase]['graph'] for phase in ('before','after')},'dol_sha256':p.DOL}
   symbols={name:{phase:list(next(r[1:] for r in summaries[phase]['named symbols'] if r[0]==name)) for phase in ('before','after')} for name in dp.SCOPES[scope]['symbols']}
   references=dp.reference_closure({unit:summaries['after']},symbols)
   plans[scope]={'schema':1,'scope':scope,'bindings':bindings,'units':sorted(spec['units']),'symbols':symbols,'references':references,'symbol_map':{name:dict(zip(('before','after'),map(list,pair))) for name,pair in dp.SCOPES[scope]['symbols'].items()}}
   dp.validate_plan(plans[scope],dp.plan_hash(plans[scope]))
 pins={scope:hashbytes(canonical(plan)) for scope,plan in plans.items()}
 perturb={scope:{'source':'src/Game/System/WorldMapResult'+scope+'.cpp','append':'\nextern "C" int solP_dispatch_source_control_'+scope.lower()+'() { return 191; }\n'} for scope in ('TU13','TU14')}
 for scope,ctl in perturb.items():
  src=p.git(repo,'show',p.TIP+':'+ctl['source']);assert ctl['append'].split('()')[0].encode() not in src
  ctl['source_sha256']=hashbytes(src);ctl['perturbed_source_sha256']=hashbytes(src+ctl['append'].encode())
 contract={'schema':1,'type':'DISPATCH_FULL_COMPONENT_RECIPE_NOT_IMPORT_APPROVAL','execution_enabled':False,'blocking_gate':'Accepted v7 disposable component only; generic compiler launch gate UNIMPLEMENTED/UNQUALIFIED, full caller/package/import review pending','accepted_disposable_component':{'source':'7c9ab833','plan_sha256':'566fe968de003b6a1df0b05c51e0d3052500a502eedd750610d536996abdd160','release_receipt_sha256':'e5f81c3bf24f96670ccfe18b7c350d118eea16aaf499edebee3bd3c6c1d8abf9','tests':20,'actual_cases':6,'scope':'disposable only; no generic/full qualification'},'generic_ownership_status':budget.GENERIC_OWNERSHIP_STATUS,'scan_limits':{'entries':budget.SCAN_ENTRIES,'seconds':budget.SCAN_SECONDS,'semantics':'sampled filesystem syscall latency, finite entry/time checks; not hard real-time quota'},'required_live_budget':{'minimum_free_bytes':50*1024**3,'maximum_owned_growth_bytes':4*1024**3,'sample_interval_seconds':0.25,'scope':'child waits, Python controls and packaging; retain original baseline across packaging'},'before':p.BASE,'after':p.TIP,'canonical_spec_sha256':p.SPEC,'plans':pins,'qualified_result_sha256':p.sha(q/'execution/result.json'),'qualified_objects':{phase:qualified['phases'][phase]['objects'] for phase in ('before','after')},'context':context,'decoder_commit':p.DECODER,'decoder_files':{n:hashbytes(data) for n,data in decoder.items()},'real_ELF_controls_per_scope':13,'config_controls':8,'source_controls':perturb,'work_seconds':2400,'cleanup_grace_seconds':4,'jobs':4,'primary_archive_members':4855,'secondary_archive_members':3,'control_member_paths':{'TU13':['control/'+n+'.o' for n in ('original','perturbed','restored')],'TU14':['control-tu14/'+n+'.o' for n in ('original','perturbed','restored')]},'import_status':'REQUIRES_L_SEPARATE_REVIEWED_ADAPTER_AND_PORT_ALLOCATION','certificate_type':'DISPATCH_PROOF_V1'}
 pin=hashbytes(canonical(contract))
 if not args.execute_contract_sha256:
  out.mkdir(parents=True,exist_ok=False);p.dump(out/'contract.json',contract);(out/'contract.sha256').write_text(pin+'\n')
  for scope,plan in plans.items():p.dump(out/(scope+'-plan.json'),plan)
  print(json.dumps({'contract_sha256':pin,'plans':pins,'status':'PREPARED_ONLY_NO_BUILD_OR_IMPORT'},indent=2));return
 assert contract['execution_enabled'] is True,'full execution blocked pending independently qualified live-budget termination controls'
 assert args.execute_contract_sha256==pin and json.loads((out/'contract.json').read_bytes())==json.loads(canonical(contract))
 for scope,plan in plans.items():assert json.loads((out/(scope+'-plan.json')).read_bytes())==json.loads(canonical(plan))
 assert os.getpriority(os.PRIO_PROCESS,0)>=19 and shutil.disk_usage(out).free>=50*1024**3
 assert p.sha(args.dol)==p.DOL
 lock=(Path.home()/'MacioGalaxy2-team/slots/native.lock').open('a');fcntl.lockf(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
 work=out/'execution';work.mkdir(exist_ok=False)
 graph=Path.home()/'MacioGalaxy2-team/scratch-port/solP/source_graph.py';macro=Path.home()/'MacioGalaxy2-team/scratch-port/solP/dtk-source/assets/macros.inc'
 with budget.FullBudgetJob(work,contract['work_seconds'],out,contract['required_live_budget']['minimum_free_bytes'],contract['required_live_budget']['maximum_owned_growth_bytes'],contract['required_live_budget']['sample_interval_seconds']) as job:
  run=job.run
  def deps_manifest(tree,log,dest):
   dependencies={}
   for line in (work/log).read_text().splitlines():
    if line.startswith('    '):
     name=line.strip();path=Path(name) if Path(name).is_absolute() else tree/name
     assert path.is_file(),'missing actual header dependency '+name
     dependencies[name]=p.sha(path)
   assert dependencies,'empty dependency list'
   p.dump(work/dest,dependencies);return p.sha(work/dest)
  def state(phase):
   (out/'STATE').write_text(json.dumps({'host':'host_db276ab2vt','source':[p.BASE,p.TIP],'spec':p.SPEC,'contract':pin,'phase':phase,'owner_pid':os.getpid(),'output':str(work),'resume':'Preserve failure; new output and fresh pair required; no fallback/partial acceptance.'},separators=(',',':'))+'\n')
  state('BEFORE_CLEAN_SOURCE_PREPARATION')
  lib=work/'decoder';lib.mkdir()
  for name,data in decoder.items():(lib/(name+'.py')).write_bytes(data)
  sys.path.insert(0,str(lib));ob=p.load('objsame',lib/'objsame.py');dp=p.load('dispatchproof',lib/'dispatchproof.py');owner=p.load('dispatchownership',lib/'dispatchownership.py')
  result={'status':'RUNNING_NO_CERTIFICATE','contract_sha256':pin,'phases':{},'controls':{}}
  try:
   trees={};summaries={};captured_specs={};observed_graphs={}
   for phase,rev in [('before',p.BASE),('after',p.TIP)]:
    state(phase+'_CLEAN_BUILD');assert shutil.disk_usage(out).free>=50*1024**3
    tree=work/phase;trees[phase]=tree
    run(['git','clone','--shared','--no-checkout',str(repo),str(tree)])
    run(['git','checkout','--detach',rev],cwd=tree)
    inc=tree/'build/SB4E01/include';inc.mkdir(parents=True);shutil.copy2(macro,inc/'macros.inc')
    for rel,h in spec['toolchain'].items():
     target=tree/rel;origin=Path.home()/'MacioGalaxy2'/rel
     if rel=='build/SB4E01/include/macros.inc':origin=macro
     assert p.sha(origin)==h
     if not target.exists():target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(origin,target)
     assert p.sha(target)==h
    run(['git','update-ref','refs/heads/main',p.BASE],cwd=tree)
    run([sys.executable,'tools/team/remoteproof.py','spec','--output',str(work/(phase+'-captured-spec.json'))],cwd=tree,log=phase+'-spec.log')
    actual=json.loads((work/(phase+'-captured-spec.json')).read_text());expected=copy.deepcopy(spec)
    if phase=='before':expected['after']=expected['before']
    assert actual==expected,'independent actual phase contract mismatch'
    captured_specs[phase]=actual
    run([sys.executable,str(graph)],cwd=tree,log=phase+'-graph.log')
    assert p.sha(tree/'build.ninja')==qualified['phases'][phase]['graph'],'actual graph differs from reviewed qualified graph'
    observed_graphs[phase]=p.sha(tree/'build.ninja')
    run(['ninja','-j',str(contract['jobs']),'-k','1','all_source'],cwd=tree,log=phase+'-build.log')
    paths={str(x.relative_to(tree/'build/SB4E01')):x for x in (tree/'build/SB4E01/src').rglob('*.o')}
    assert set(paths)==set(spec['units']) and len(paths)==2426
    for rel,h in contract['qualified_objects'][phase].items():assert p.sha(tree/rel)==h,'qualified scoped raw object drift'
    summaries[phase]={unit:ob.summarize(path) for unit,path in paths.items()}
    p.dump(work/(phase+'-objects.json'),{unit:p.sha(path) for unit,path in paths.items()})
    run(['ninja','-t','deps'],cwd=tree,log=phase+'-deps.txt')
    run(['ninja','-t','commands','all_source'],cwd=tree,log=phase+'-commands.txt')
    headers_sha=deps_manifest(tree,phase+'-deps.txt',phase+'-dependency-hashes.json')
    run(['git','status','--porcelain','--untracked-files=no'],cwd=tree,log=phase+'-clean-status.txt')
    assert not (work/(phase+'-clean-status.txt')).read_bytes().strip()
    result['phases'][phase]={'exit_code':0,'clean':True,'objects':2426,'source':rev,'graph_sha256':p.sha(tree/'build.ninja'),'manifest_sha256':p.sha(work/(phase+'-objects.json')),'dependency_hashes_sha256':headers_sha}
   state('FULL_COMPARISON_OWNERSHIP_REFERENCE_CLOSURE')
   observed_bindings={}
   for scope,plan in plans.items():
    source=dp.SCOPES[scope]['source']
    observed_bindings[scope]={'base':p.BASE,'tip':p.TIP,'source_trees':{phase:captured_specs[phase]['after'] for phase in ('before','after')},'source_files':{phase:{source:p.sha(trees[phase]/source)} for phase in ('before','after')},'config_files':{phase:{'config/SB4E01/symbols.txt':p.sha(trees[phase]/'config/SB4E01/symbols.txt')} for phase in ('before','after')},'toolchain':captured_specs['after']['toolchain'],'generators':captured_specs['after']['generators'],'graphs':observed_graphs,'dol_sha256':p.sha(args.dol)}
    dp.binding_check(plan,observed_bindings[scope])
   p.dump(work/'observed-bindings.json',observed_bindings)
   config_maps={phase:check.parse_config_symbols((trees[phase]/'config/SB4E01/symbols.txt').read_text()) for phase in ('before','after')}
   comparisons=check.compare_trees(summaries['before'],summaries['after'],plans,pins,observed_bindings,dp,config_maps)
   # Config check covers all four mapped rows together and every other line.
   p.config_envelope((trees['before']/'config/SB4E01/symbols.txt').read_text(),(trees['after']/'config/SB4E01/symbols.txt').read_text(),dp)
   result['ownership']={}
   for scope in ('TU13','TU14'):
    unit=dp.SCOPES[scope]['object'];a=trees['before']/'build/SB4E01'/unit;b=trees['after']/'build/SB4E01'/unit
    baseline_owner=owner.decode_object(a,scope);assert len(baseline_owner['mismatches'])==2
    result['ownership'][scope]={'before':baseline_owner,'after':owner.require_candidate(b,scope),'original':owner.decode_original_dol(args.dol.read_bytes(),scope)}
   p.dump(work/'full-comparison.json',comparisons)
   result['actual_ELF_controls']=[];negative=work/'negative-ELFs';negative.mkdir()
   for scope in ('TU13','TU14'):
    unit=dp.SCOPES[scope]['object'];original=trees['after']/'build/SB4E01'/unit
    for label,raw in check.real_elf_controls(original,scope,ob,dp).items():
     mutant=negative/(scope+'-'+label+'.o');mutant.write_bytes(raw)
     changed=dict(summaries['after']);changed[unit]=ob.summarize(mutant)
     try:check.compare_trees(summaries['before'],changed,plans,pins,observed_bindings,dp,config_maps)
     except ValueError:pass
     else:raise AssertionError('full semantic gate accepted '+scope+'/'+label)
     if label in ('extra-execute-reference','extra-local-midbody-alias') or label.startswith('own-slot'):
      try:check.check_all(changed,plans,dp,config_maps['after'],'after')
      except ValueError:pass
      else:raise AssertionError('reference census accepted '+label)
     result['actual_ELF_controls'].append({'scope':scope,'label':label,'path':str(mutant.relative_to(work)),'sha256':p.sha(mutant),'rejected':True})
   assert len(result['actual_ELF_controls'])==26
   result['actual_config_controls']=[]
   original_maps={phase:(trees[phase]/'config/SB4E01/symbols.txt').read_text() for phase in ('before','after')}
   for plan in plans.values():
    for name,pair in plan['symbol_map'].items():
     for field in (0,1):
      lines=original_maps['after'].splitlines(keepends=True);i=next(i for i,l in enumerate(lines) if l.startswith(name+' = '));values=list(pair['after']);values[field]+=4;lines[i]=name+' = .text:0x%08X; // type:function size:0x%X\n'%tuple(values)
      try:p.config_envelope(original_maps['before'],''.join(lines),dp)
      except AssertionError:pass
      else:raise AssertionError('wrong config address/size accepted')
      result['actual_config_controls'].append({'execute':name,'field':field,'rejected':True})
   assert len(result['actual_config_controls'])==8
   # Actual changed source controls, independently compiled and restored, each TU.
   for scope,ctl in contract['source_controls'].items():
    state(scope+'_SOURCE_PERTURB_RESTORE');tree=trees['after'];source=tree/ctl['source'];unit=dp.SCOPES[scope]['object'];obj=tree/'build/SB4E01'/unit
    control=work/('control' if scope=='TU13' else 'control-tu14');control.mkdir()
    shutil.copy2(obj,control/'original.o');original=source.read_bytes();assert hashbytes(original)==ctl['source_sha256']
    original_obj=p.sha(obj);perturbed_source=original+ctl['append'].encode();assert hashbytes(perturbed_source)==ctl['perturbed_source_sha256']
    try:
     source.write_bytes(perturbed_source);obj.unlink()
     run(['ninja','-j','1',str(obj.relative_to(tree))],cwd=tree,log=scope+'-perturb.log')
     shutil.copy2(obj,control/'perturbed.o');assert ob.compare(control/'original.o',control/'perturbed.o'),'source perturb must change semantics'
     changed=dict(summaries['after']);changed[unit]=ob.summarize(control/'perturbed.o')
     try:check.compare_trees(summaries['before'],changed,plans,pins,observed_bindings,dp,config_maps)
     except ValueError:pass
     else:raise AssertionError('full semantic comparison accepted actual source perturb')
    finally:source.write_bytes(original)
    obj.unlink();run(['ninja','-j','1',str(obj.relative_to(tree))],cwd=tree,log=scope+'-restore.log')
    shutil.copy2(obj,control/'restored.o');assert p.sha(obj)==original_obj==p.sha(control/'original.o') and not ob.compare(control/'original.o',control/'restored.o')
    assert source.read_bytes()==original
    run(['ninja','-t','commands',str(obj.relative_to(tree))],cwd=tree,log=scope+'-control-command.txt')
    run(['ninja','-t','deps',str(obj.relative_to(tree))],cwd=tree,log=scope+'-control-deps.txt')
    headers_sha=deps_manifest(tree,scope+'-control-deps.txt',scope+'-control-dependency-hashes.json')
    result['controls'][scope]={**ctl,'unit':unit,'restored_source_sha256':p.sha(source),'ELFs':{n:p.sha(control/(n+'.o')) for n in ('original','perturbed','restored')},'compiler_command_sha256':p.sha(work/(scope+'-control-command.txt')),'dependencies_sha256':p.sha(work/(scope+'-control-deps.txt')),'dependency_header_hashes_sha256':headers_sha,'compile_exit_codes':[0,0],'semantic_perturb_rejected':True,'restored_raw_identical':True}
   # Source restoration and every consumed tool are checked before any certification.
   run(['git','status','--porcelain','--untracked-files=no'],cwd=trees['after'],log='after-restored-status.txt')
   assert not (work/'after-restored-status.txt').read_bytes().strip()
   for unit,h in json.loads((work/'after-objects.json').read_text()).items():assert p.sha(trees['after']/'build/SB4E01'/unit)==h,'postrestore object drift'
   for path,h in context.items():assert p.sha(path)==h,'tool drift '+path
   job.remaining();result['status']='FULL_PAIR_COMPONENTS_COMPLETE_IMPORT_PENDING';result['seconds']=time.monotonic()-job.start
   p.dump(work/'source-controls.json',result['controls']);state('COMPONENTS_COMPLETE_IMPORT_REVIEW_PENDING_NO_GENERIC_CERT')
  finally:p.dump(work/'result.json',result)
 print(result['status'])
if __name__=='__main__':main()
