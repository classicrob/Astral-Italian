#!/usr/bin/env python3
"""Extract actual setter and Binder::bind condition; test canonical typed storage.

Inert lifecycle fixtures establish actual C++ object lifetimes. The reader is the
exact _0 condition from bind, with only implicit-this qualification made explicit;
this is not a complete collision simulation. Both wrong-setter and wrong-reader
negative controls are required to fail.
"""
import argparse,json,os,pathlib,re,subprocess,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[2]
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',type=pathlib.Path);a=p.parse_args();source=(ROOT/'src/Game/Util/LiveActorUtil.cpp').read_text()
 m=re.search(r'    void setBinderIgnoreMovingCollision\(LiveActor\* pActor\) \{\n#ifdef GALAXY_NATIVE\n([^#]+)#else\n([^#]+)#endif\n    \}',source);assert m,'Actual setter shape changed'
 binder=(ROOT/'src/Game/LiveActor/Binder.cpp').read_text();body=binder[binder.index('const TVec3f Binder::bind('):binder.index('void Binder::moveAlongHittedPlanes(')]
 reader=re.findall(r'if \((_1EC\._0)\) \{\s*moveWithCollisionParts',body);assert len(reader)==1,'Actual bind reader changed'
 read='return binder.'+reader[0]+';';old='void setBinderIgnoreMovingCollision(LiveActor* pActor) {\n'+m[2]+'}'
 template=(ROOT/'native/tests/binder_ignore_storage_tests.cpp').read_text();flags=['clang++','-std=c++17','-O1','-DGALAXY_NATIVE=1','-fno-rtti','-fno-strict-aliasing','-Wno-register','-Wno-inconsistent-missing-override','-Wno-invalid-'+'offsetof','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer']
 for f in ['native/include','include','libs/JSystem/include','libs/RVL_SDK/include','libs/nw4r/include']:flags+=['-I',str(ROOT/f)]
 results=[]
 with tempfile.TemporaryDirectory(prefix='smg2-binder-ignore-') as directory:
  work=pathlib.Path(directory)
  for label,setter,getter in [('production',m[0],read),('old-Wii-mask-on-native',old,read),('old-Wii-reader-mask-on-native',m[0],'return (*reinterpret_cast<const u8*>(&binder._1EC) & 0x80) != 0;')]:
   cpp=work/'test.cpp';cpp.write_text(template.replace('// __PRODUCTION_FUNCTION__',setter).replace('// __PRODUCTION_READER__',getter));binary=work/'test';cmd=flags+[str(cpp),'-o',str(binary)];subprocess.run(cmd,check=True)
   run=subprocess.run([str(binary)],text=True,capture_output=True,env=dict(os.environ,UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=0'))
   if label=='production':assert run.returncode==0,run.stdout+run.stderr
   else:assert run.returncode!=0 and 'native moving-collision reader' in run.stderr,run.stdout+run.stderr
   results.append({'variant':label,'exit':run.returncode,'stdout':run.stdout.strip(),'stderr':run.stderr.strip(),'compile_command':cmd});print(label,run.returncode,run.stdout.strip() or run.stderr.strip())
 if a.output:a.output.write_text(json.dumps({'results':results},indent=2)+'\n')
if __name__=='__main__':main()
