#!/usr/bin/env python3
"""Extract the actual native/Wii assignment and test canonical production storage.
Only the enclosing object's variable spelling this->pSource is substituted in
the old negative control; no ABI class model or numeric host layout is invented.
"""
import argparse,json,os,pathlib,re,subprocess,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[2]
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',type=pathlib.Path);a=p.parse_args();source=(ROOT/'src/Game/Util/BellyDragon.cpp').read_text()
 m=re.search(r'#ifdef GALAXY_NATIVE\n\s*(mDemoB->mLinkInfo.mZoneID = mLinkInfo.mZoneID;)\n#else\n\s*(\(\(s16\*\)mDemoB\)\[7\] = \(\(s16\*\)this\)\[7\];)\n#endif',source);assert m,'Actual production statement changed'
 template=(ROOT/'native/tests/belly_zone_storage_tests.cpp').read_text();flags=['clang++','-std=c++17','-O1','-DGALAXY_NATIVE=1','-fno-rtti','-fno-strict-aliasing','-Wno-register','-Wno-inconsistent-missing-override','-Wno-invalid-'+ 'offsetof','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer']
 for f in ['native/include','include','libs/JSystem/include','libs/RVL_SDK/include','libs/nw4r/include']:flags+=['-I',str(ROOT/f)]
 result=[]
 with tempfile.TemporaryDirectory(prefix='smg2-zone-field-') as folder:
  work=pathlib.Path(folder)
  for label,statement in [('production',m[1]),('old-Wii-offset-on-native',m[2].replace('this','pSource'))]:
   cpp=work/'test.cpp';cpp.write_text(template.replace('// __PRODUCTION_ASSIGNMENT__',statement));binary=work/'test';cmd=flags+[str(cpp),'-o',str(binary)];subprocess.run(cmd,check=True)
   run=subprocess.run([str(binary)],text=True,capture_output=True,env=dict(os.environ,UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=0'))
   if label=='production':assert run.returncode==0,run.stdout+run.stderr
   else:assert run.returncode!=0 and 'canonical signed16 zone is copied' in run.stderr,run.stdout+run.stderr
   result.append({'variant':label,'exit':run.returncode,'stdout':run.stdout.strip(),'stderr':run.stderr.strip(),'compile_command':cmd});print(label,run.returncode,run.stdout.strip() or run.stderr.strip())
 if a.output:a.output.write_text(json.dumps({'results':result},indent=2)+'\n')
if __name__=='__main__':main()
