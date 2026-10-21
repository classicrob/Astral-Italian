#!/usr/bin/env python3
"""Full production PadRecorder TU with synthetic Wii-format delta recordings.

No retail recordings, game assets or address dumps are used. Inert registration,
cache, aligned allocation and save sinks are isolated test providers. Playback,
PadDataReader, packet decode, status decode and state methods are production code.
"""
import argparse,hashlib,importlib.util,json,pathlib,subprocess,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[2]
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=pathlib.Path);p.add_argument('--compiler',default='clang++');p.add_argument('--baseline-source',type=pathlib.Path);a=p.parse_args()
 sp=importlib.util.spec_from_file_location('encoder',ROOT/'native/tools/encode_legacy_sources.py');enc=importlib.util.module_from_spec(sp);sp.loader.exec_module(enc)
 fixture=ROOT/'native/tests/padrecorder_wire_tests.cpp'
 report={'source_sha256':sha(ROOT/'src/Game/Util/PadRecorder.cpp'),'fixture_sha256':sha(fixture),'encoder_sha256':sha(ROOT/'native/tools/encode_legacy_sources.py'),'scope':'Production PadRecorder and memory reader, synthetic wire input; inert registration/cache/allocation/save sink dependencies; no external harness tape or gameplay proof.','results':[]}
 with tempfile.TemporaryDirectory(prefix='padrecorder-wire-') as directory:
  d=pathlib.Path(directory);legacy=d/'legacy';enc.prepare(ROOT,legacy);source=(legacy/'src/Game/Util/PadRecorder.cpp').read_text()
  common=[a.compiler,'-std=c++17','-DGALAXY_NATIVE=1','-O1','-g','-fno-rtti','-ffp-contract=off','-fno-strict-aliasing','-Wno-register','-Wno-inconsistent-missing-override','-fsanitize=address,undefined','-fno-sanitize-recover=all','-ffunction-sections','-fdata-sections','-Wl,-dead_strip']
  for inc in ['native/include','native/home_menu/include','include','libs/JSystem/include','libs/RVL_SDK/include','libs/nw4r/include','libs/RVLFaceLib/include','libs/libms/include']:
   if not inc.startswith('native/'):common+=['-I',str(legacy/inc)]
   common+=['-I',str(ROOT/inc)]
  def run(text,label,negative=False):
   tu=d/(label+'-source.cpp');tu.write_text(text);harness=d/(label+'.cpp');harness.write_text('#include '+json.dumps(str(tu))+'\n#include '+json.dumps(str(fixture))+'\n');binary=d/label
   command=common+[str(harness),'-o',str(binary)];built=subprocess.run(command,text=True,capture_output=True)
   if built.returncode:raise RuntimeError(label+' failed to compile (not an accepted negative):\n'+built.stdout+built.stderr)
   result=subprocess.run([str(binary)],text=True,capture_output=True,timeout=30)
   report['results'].append({'label':label,'command':command,'exit':result.returncode,'stdout':result.stdout,'stderr':result.stderr,'source_sha256':hashlib.sha256(text.encode()).hexdigest()})
   if negative:assert result.returncode!=0,label+' escaped regression'
   elif result.returncode:raise RuntimeError(result.stdout+result.stderr)
  run(source,'current')
  controls=[
   ('unconverted-packet','parsePacketHeader(&packetNo, &size, &statusNum, PetariNative::readU32BE(mPacketBuffer));','parsePacketHeader(&packetNo, &size, &statusNum, *reinterpret_cast<u32*>(mPacketBuffer));'),
   ('unconverted-output','    decodeRecordedStatus(pOut);','    // omitted status decode'),
   ('converted-history','    decodeRecordedStatus(pOut);','    decodeRecordedStatus(pOut);\n    decodeRecordedStatus(pPrev);'),
   ('unconverted-user-length','    readRecordedScalar(size, mPacketBuffer);','    size = *reinterpret_cast<s32*>(mPacketBuffer);'),
   ('partial-header','    readRecordedScalar(mHeader->_4, buffer + 4);','    // omitted non-size header conversion'),
  ]
  for name,fixed,old in controls:
   assert source.count(fixed)==1,name;run(source.replace(fixed,old),name,True)
  if a.baseline_source:
   # Transform the original source through the actual encoder, not UTF-8 guesswork.
   original=a.baseline_source.read_bytes()
   prefix=''.join(source.splitlines(keepends=True)[:3])
   baseline=prefix+enc.transform(original.decode('utf-8'))
   run(baseline,'old-native',True)
  print(report['results'][0]['stdout'].strip());print('PASS',len(report['results'])-1,'compiled negative controls rejected')
 if a.output:a.output.write_text(json.dumps(report,indent=2)+'\n')
if __name__=='__main__':main()
