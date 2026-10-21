#!/usr/bin/env python3
"""Production-site extraction, canonical native storage + old-offset negative control.
Explicit lifecycle/matrix-generator fixtures do not model full actor gameplay.
"""
import argparse,hashlib,importlib.util,json,os,pathlib,re,subprocess,tempfile
R=pathlib.Path(__file__).resolve().parents[2];NAME=pathlib.Path(__file__).stem.split('_')[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',type=pathlib.Path);p.add_argument('--legacy',type=pathlib.Path);a=p.parse_args();unit='BellyDragonTU05' if NAME=='fire' else 'BellyDragonTU04';source=(R/f'src/Game/Util/{unit}.cpp').read_text()
 if NAME=='camera':site=re.search(r'#ifdef GALAXY_NATIVE\n(    pHost->mPod->mCameraTargetMtx->mMatrix.set\([^\n]+)\n#else\n([^\n]+)\n#endif',source)[0]
 else:
  signature='void BellyDragonUtil::setBodyPartMtx(' if NAME=='body' else 'void BellyDragonPod::appearFireGround(';start=source.index(signature);brace=source.index('{',start);depth=1;end=brace+1
  while depth:
   depth+=(source[end]=='{')-(source[end]=='}');end+=1
  site=source[start:end]
 assert '#ifdef GALAXY_NATIVE' in site
 scalars=''
 if NAME=='fire':
  for name in ['lbl_807D784C','lbl_807D7850']:
   decl=re.search(r'(?:extern "C" )?const f32 '+name+r' = [^;]+;',(R/'src/Game/Util/BellyDragonNativeData.cpp').read_text());assert decl,name;scalars+=decl[0]+'\n'
 template=pathlib.Path(__file__).with_suffix('.cpp').read_text().replace('// __PRODUCTION_SCALARS__',scalars)
 with tempfile.TemporaryDirectory(prefix='smg2-'+NAME+'-storage-') as tmp:
  W=pathlib.Path(tmp);L=a.legacy
  if L is None:
   sp=importlib.util.spec_from_file_location('enc',R/'native/tools/encode_legacy_sources.py');enc=importlib.util.module_from_spec(sp);sp.loader.exec_module(enc);L=W/'legacy';enc.prepare(R,L)
  flags=['clang++','-std=c++17','-O1','-DGALAXY_NATIVE=1','-fno-rtti','-fno-strict-aliasing','-ffp-contract=off','-Wno-register','-Wno-inconsistent-missing-override','-Wno-invalid-offset'+'of','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer']
  for f in [R/'native/include',L/'include',L/'libs/JSystem/include',L/'libs/RVL_SDK/include',L/'libs/nw4r/include',R/'include',R/'libs/JSystem/include',R/'libs/RVL_SDK/include',R/'libs/nw4r/include']:flags+=['-I',str(f)]
  results=[]
  for label,old in [('production',False),('old-Wii-offset-on-native',True)]:
   extracted=re.sub(r'#ifdef GALAXY_NATIVE\n(.*?)\n#else\n(.*?)\n#endif',lambda m:m[2] if old else m[1],site,flags=re.S);cpp=W/'test.cpp';cpp.write_text(template.replace('// __PRODUCTION_SITE__',extracted));binary=W/'test';cmd=flags+[str(cpp),'-o',str(binary)];subprocess.run(cmd,check=True);run=subprocess.run([str(binary)],text=True,capture_output=True,env=dict(os.environ,UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=0'));assert (run.returncode!=0)==old,(label,run.stdout,run.stderr)
   if old:assert 'canonical matrix' in run.stderr,run.stderr
   print(label,run.returncode,run.stdout.strip() or run.stderr.strip());results.append({'variant':label,'exit':run.returncode,'stdout':run.stdout,'stderr':run.stderr,'command':cmd,'extracted_site':extracted,'test_source_sha256':hashlib.sha256(cpp.read_bytes()).hexdigest()})
  if a.output:a.output.write_text(json.dumps({'source':str(R/f'src/Game/Util/{unit}.cpp'),'source_sha256':hashlib.sha256(source.encode()).hexdigest(),'results':results},indent=2)+'\n')
if __name__=='__main__':main()
