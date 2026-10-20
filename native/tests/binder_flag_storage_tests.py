#!/usr/bin/env python3
"""Extract full actual functions; exercise canonical production Binder storage.

The inert constructor/virtual fixtures establish real C++ object lifetimes. They
are not the collision runtime. Negative controls select the verbatim Wii arms
on this native layout and must fail the precise full-byte/field check.
"""
import argparse,json,os,pathlib,re,subprocess,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[2]
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',type=pathlib.Path);p.add_argument('--compiler',default='clang++');a=p.parse_args()
 source=(ROOT/'src/Game/Util/LiveActorUtil.cpp').read_text();functions=[];old=[]
 for name in ['validateExCollisionParts','invalidateExCollisionParts']:
  pattern=r'    void '+name+r'\(LiveActor\* pActor\) \{\n#ifdef GALAXY_NATIVE\n([^#]+)#else\n([^#]+)#endif\n    \}'
  m=re.search(pattern,source);assert m,'Actual production site changed: '+name
  functions.append(m[0]);old.append('void '+name+'(LiveActor* pActor) {\n'+m[2]+'}')
 template=(ROOT/'native/tests/binder_flag_storage_tests.cpp').read_text()
 flags=[a.compiler,'-std=c++17','-O1','-DGALAXY_NATIVE=1','-fno-rtti','-Wno-register','-Wno-inconsistent-missing-override','-Wno-invalid-offsetof','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer']
 for part in ['native/include','include','libs/JSystem/include','libs/RVL_SDK/include','libs/nw4r/include']:flags+=['-I',str(ROOT/part)]
 results=[]
 with tempfile.TemporaryDirectory(prefix='smg2-binder-flags-') as folder:
  work=pathlib.Path(folder)
  for label,bodies in [('production',functions),('old-Wii-mask-on-native',old)]:
   fixture=work/'test.cpp';fixture.write_text(template.replace('// __PRODUCTION_FUNCTIONS__','\n'.join(bodies)));binary=work/'test';cmd=flags+[str(fixture),'-o',str(binary)];subprocess.run(cmd,check=True)
   run=subprocess.run([str(binary)],capture_output=True,text=True,env=dict(os.environ,UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=0'))
   if label=='production':assert run.returncode==0,run.stdout+run.stderr
   else:assert run.returncode!=0 and 'only canonical _2 bit changes' in run.stderr,run.stdout+run.stderr
   results.append({'variant':label,'exit':run.returncode,'stdout':run.stdout.strip(),'stderr':run.stderr.strip(),'compile_command':cmd});print(label,run.returncode,run.stdout.strip() or run.stderr.strip())
 if a.output:a.output.write_text(json.dumps({'results':results},indent=2)+'\n')
if __name__=='__main__':main()
