#!/usr/bin/env python3
"""Package completed intentional-delta components for L's SEPARATE importer review.
No generic hookproof-land.cert, no remoteproof/proofland invocation, no landing.
"""
import argparse,base64,hashlib,json,tarfile,shutil
from pathlib import Path
import dispatch_preflight as p
import dispatch_fullproof as full
import dispatch_artifacts as artifacts

def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--proof',type=Path,required=True);ap.add_argument('--spec',type=Path,required=True);ap.add_argument('--independent-spec',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);ap.add_argument('--reviewed-contract-sha256',required=True);args=ap.parse_args()
 root=args.proof.resolve();work=root/'execution';out=args.output.resolve();contract=json.loads((root/'contract.json').read_text())
 assert contract.get('execution_enabled') is True,'packaging blocked pending qualified full execution/lifecycle contract'
 assert full.hashbytes(full.canonical(contract))==args.reviewed_contract_sha256
 r=json.loads((work/'result.json').read_text());cleanup=json.loads((work/'owned-process-cleanup.json').read_text());spec=json.loads(args.spec.read_text())
 assert r['status']=='FULL_PAIR_COMPONENTS_COMPLETE_IMPORT_PENDING' and r['contract_sha256']==args.reviewed_contract_sha256
 assert cleanup['all_owned_groups_absent'] and not cleanup['remaining_pgids'] and not cleanup['failures'] and cleanup['execution_error'] is None
 assert p.sha(args.spec)==p.sha(args.independent_spec)==p.SPEC and json.loads(args.independent_spec.read_text())==spec
 assert len(r['actual_ELF_controls'])==26 and len(r['actual_config_controls'])==8 and set(r['controls'])=={'TU13','TU14'}
 comparison=json.loads((work/'full-comparison.json').read_text());assert comparison['strict_objects']==2424
 for phase in ('before','after'):
  assert r['phases'][phase]['objects']==2426 and r['phases'][phase]['exit_code']==0 and r['phases'][phase]['clean']
 for name,h in contract['context'].items():assert p.sha(name)==h,'tool/recipe drift'
 out.mkdir(parents=True,exist_ok=False)
 def write(name,data):
  dest=out/name;dest.parent.mkdir(parents=True,exist_ok=True)
  if isinstance(data,bytes):dest.write_bytes(data)
  else:p.dump(dest,data)
  return {'path':name,'sha256':p.sha(dest)}
 def copy(name,source):return write(name,Path(source).read_bytes())
 def archive(name,mapping):
  manifest={key:p.sha(path) for key,path in mapping.items()};dest=out/name
  with tarfile.open(dest,'w:gz') as tar:
   for member,path in sorted(mapping.items()):
    assert path.is_file() and not path.is_symlink()
    info=tarfile.TarInfo(member);info.size=path.stat().st_size;info.mode=0o600;info.mtime=0
    with path.open('rb') as stream:tar.addfile(info,stream)
  encoded=dest.with_suffix(dest.suffix+'.b64')
  with dest.open('rb') as inp,encoded.open('wb') as output:
   while block:=inp.read(3*1024*1024):output.write(base64.b64encode(block))
  return {'path':encoded.name,'sha256':p.sha(dest),'encoded_sha256':p.sha(encoded)},manifest
 mapping={}
 for phase in ('before','after'):
  expected=json.loads((work/(phase+'-objects.json')).read_text());assert set(expected)==set(spec['units'])
  for unit,h in expected.items():
   obj=work/phase/'build/SB4E01'/unit;assert p.sha(obj)==h;mapping[phase+'/'+unit]=obj
 for name in ('original','perturbed','restored'):mapping['control/'+name+'.o']=work/'control'/(name+'.o')
 assert len(mapping)==4855
 primary,files=archive('primary-objects.tar.gz',mapping)
 secondary_paths={'control-tu14/'+n+'.o':work/'control-tu14'/(n+'.o') for n in ('original','perturbed','restored')}
 secondary_archive,secondary_files=archive('secondary-tu14.tar.gz',secondary_paths);assert len(secondary_files)==3
 secondary_manifest=write('dispatch/secondary-tu14-manifest.json',secondary_files)
 secondary=r['controls']['TU14'];assert secondary['restored_source_sha256']==secondary['source_sha256']
 for scope,dirname in [('TU13','control'),('TU14','control-tu14')]:
  ctl=r['controls'][scope]
  assert ctl['semantic_perturb_rejected'] and ctl['restored_raw_identical'] and ctl['compile_exit_codes']==[0,0]
  assert ctl['ELFs']['original']==ctl['ELFs']['restored']==files['after/'+ctl['unit']]
  assert ctl['ELFs']['perturbed']!=ctl['ELFs']['original']
  for name,h in ctl['ELFs'].items():assert p.sha(work/dirname/(name+'.o'))==h
 plans={scope:copy('dispatch/'+scope+'-plan.json',root/(scope+'-plan.json')) for scope in ('TU13','TU14')}
 delta={'schema':1,'plans':plans,'original_ownership':write('dispatch/original-ownership.json',r['ownership']),'strict_remainder':copy('dispatch/full-comparison.json',work/'full-comparison.json'),'effective_reference_closure':write('dispatch/effective-references.json',comparison['effective_references']),'symbol_map':write('dispatch/symbol-map.json',{s:json.loads((root/(s+'-plan.json')).read_text())['symbol_map'] for s in ('TU13','TU14')})}
 # Secondary provenance and source control logs are separately confined/hash-bound.
 secondary_record={**secondary,'archive':secondary_archive,'manifest':secondary_manifest,'context':contract['context'],'logs':{kind:copy('dispatch/TU14-'+kind+'.txt',work/('TU14-'+source)) for kind,source in [('perturb','perturb.log'),('restore','restore.log'),('command','control-command.txt'),('deps','control-deps.txt'),('headers','control-dependency-hashes.json')]}}
 delta['secondary_control']=write('dispatch/secondary-control.json',secondary_record)
 # Metadata paths refer to the PACKAGE root, never the producer execution root.
 packaged_controls=[]
 for row in r['actual_ELF_controls']:
  origin=artifacts.confined_file(work,row['path']);assert row['rejected'] and p.sha(origin)==row['sha256']
  name='dispatch/'+row['path'];copied=copy(name,origin)
  packaged_controls.append({**row,'path':copied['path'],'sha256':copied['sha256']})
 artifacts.negative_artifacts(out,packaged_controls)
 delta['actual_negative_controls']=write('dispatch/actual-negative-controls.json',{'ELF':packaged_controls,'config':r['actual_config_controls']})
 before_log=copy('before.log',work/'before-build.log');after_log=copy('after.log',work/'after-build.log')
 control_log=write('control.log',b'\n'.join((work/(s+'-'+kind+'.log')).read_bytes() for s in ('TU13','TU14') for kind in ('perturb','restore')))
 recipe=write('recipe.json',{'contract':contract,'actual_result':r,'owned_process_cleanup':cleanup,'import_status':'PENDING_INDEPENDENT_L_DISPATCH_IMPORT','source_controls_append_bytes':contract['source_controls']})
 phase_evidence={}
 for phase in ('before','after'):
  phase_evidence[phase]={name:copy('evidence/'+phase+'-'+name,source) for name,source in [('build.ninja',work/phase/'build.ninja'),('source-graph.json',work/phase/'build/source-graph.json'),('commands.txt',work/(phase+'-commands.txt')),('deps.txt',work/(phase+'-deps.txt')),('dependency-hashes.json',work/(phase+'-dependency-hashes.json')),('objects.json',work/(phase+'-objects.json'))]}
 primary_control_evidence={kind:copy('evidence/TU13-'+kind,work/('TU13-'+kind)) for kind in ('control-command.txt','control-deps.txt','control-dependency-hashes.json')}
 units=write('unit-manifest.json',{'spec_units':spec['units'],'phases':r['phases'],'qualified_object_pins':contract['qualified_objects'],'actual_evidence':phase_evidence,'primary_control_evidence':primary_control_evidence})
 spec_compare=write('spec-comparison.json',{'original_spec_sha256':p.sha(args.spec),'independent_spec_sha256':p.sha(args.independent_spec),'parsed_equal':True,'actual_phase_specs':{phase:json.loads((work/(phase+'-captured-spec.json')).read_text()) for phase in ('before','after')}})
 provenance=dict(zip(('before_log','after_log','control_log','recipe','unit_manifest','spec_comparison'),(before_log,after_log,control_log,recipe,units,spec_compare)))
 tagged={'type':'DISPATCH_PROOF_V1','canonical_spec_sha256':p.SPEC,'base':p.BASE,'after_trees':spec['after'],'TU13_plan_sha256':contract['plans']['TU13'],'TU14_plan_sha256':contract['plans']['TU14'],'secondary_control_sha256':delta['secondary_control']['sha256'],'fullcomparison_sha256':delta['strict_remainder']['sha256']}
 certificate=write('dispatch-proof.cert.json',full.canonical(tagged)+b'\n')
 receipt={'schema':1,'type':'DISPATCH_PROOF_V1','status':'COMPLETE_AWAITING_INDEPENDENT_IMPORT','host':'host_db276ab2vt','spec':spec,'restored_trees':spec['after'],'clean':{'before':True,'after':True},'builds':{'before':0,'after':0,'control':0,'restore':0},'archive':primary,'files':files,'certificate':certificate,'provenance':provenance,'control':{k:r['controls']['TU13'][k] for k in ('source','unit','source_sha256','restored_source_sha256')},'dispatch_delta':delta}
 receipt['control']['object']=receipt['control'].pop('unit');write('receipt.json',receipt)
 parts=[];encoded=out/primary['path'];partdir=out/'transfer-parts';partdir.mkdir()
 with encoded.open('rb') as stream:
  index=0
  while block:=stream.read(2*1024*1024):
   name='transfer-parts/part-%04d.b64'%index;path=out/name;path.write_bytes(block);parts.append({'path':name,'sha256':p.sha(path),'bytes':len(block)});index+=1
 write('transfer.json',{'primary_archive':primary,'ordered_parts':parts,'full_encoded_bytes':encoded.stat().st_size,'secondary_archive':secondary_archive,'receipt_sha256':p.sha(out/'receipt.json'),'regular_files':{str(path.relative_to(out)):p.sha(path) for path in out.rglob('*') if path.is_file() and path.name!='transfer.json'}})
 print(json.dumps({'status':receipt['status'],'receipt_sha256':p.sha(out/'receipt.json'),'primary_members':4855,'secondary_members':3,'tagged_certificate_only':True},indent=2))
if __name__=='__main__':main()
