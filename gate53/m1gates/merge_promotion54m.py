#!/usr/bin/env python3
"""Combine disjoint, actual promotion cases after a harness-only interrupted attempt.

merge_promotion54m.py NEW_OUT_DIR RUN_RESULTS_JSON [RUN_RESULTS_JSON ...]
Requires all 15 unique cases, the frozen perf copy identity, and each original
per-case receipt byte-for-byte. Never converts a failed case to a pass.
"""
import argparse,json,sys
from pathlib import Path
from common import ROOT,FIXTURE,prep,sha,write
from promotion54m import inventory

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('out',type=Path);p.add_argument('inputs',type=Path,nargs='+');a=p.parse_args()
 out=a.out.expanduser().resolve();assert out.is_relative_to(ROOT/'campaigns') and not out.exists()
 app,meta=prep();expected={(n,mode) for n in ('yoshi','flip','fluffy','unlocked-stage-adapter','worldmap','return') for mode in ('fresh','warm')}|{(n,'functional') for n in ('cappy-fluffy','cappy-panel','cappy-spin-after-throw')}
 canonical=json.loads((FIXTURE.parent/'receipt.json').read_text());assert inventory(FIXTURE)=={k:canonical[k] for k in ('files','xattrs')}
 live=Path(canonical['source']);live_now=inventory(live);preservation=[]
 combined=[];seen=set();evidence=[]
 for source in a.inputs:
  source=source.expanduser().resolve();assert source.is_relative_to(ROOT/'runs')
  before=json.loads((source.parent/'save-source-drift.json').read_text())['now']
  assert live_now==before,('live save changed since run',source)
  preservation.append({'source_results':str(source),'live_save':str(live),'before_file':str(source.parent/'save-source-drift.json'),'live_save_unchanged':True})
  for record in json.loads(source.read_text()):
   key=(record['case'],record['mode']);assert key in expected and key not in seen,('duplicate or unknown case',key)
   receipt=source.parent/(key[0]+'-'+key[1])/'receipt.json'
   assert json.loads(receipt.read_text())==record,('source result differs from original receipt',receipt)
   assert record['binary_sha256']==meta['perf_exe_sha256'] and record['source']==meta['source'] and record['renderer_key']==meta['renderer_key'] and record['nice']==0
   assert record['passed']==all(record['checks'].values())
   seen.add(key);combined.append(record);evidence.append({'case':key[0],'mode':key[1],'source_results':str(source),'source_results_sha256':sha(source),'receipt':str(receipt),'receipt_sha256':sha(receipt)})
 assert seen==expected,('missing cases',sorted(expected-seen))
 out.mkdir(parents=True)
 write(out/'results.json',combined)
 write(out/'EVIDENCE.json',{'status':'MEASURED_COMPOSITE','cases':len(combined),'passed_cases':sum(r['passed'] for r in combined),'nice':0,'frozen_exe_sha256':meta['source_exe_sha256'],'perf_exe_sha256':meta['perf_exe_sha256'],'source':meta['source'],'inputs':evidence,'preservation':preservation,'canonical_fixture_unchanged':True,'note':'Disjoint actual runs combined after initial harness failures; failed case verdicts are preserved. Not a complete freeze receipt.'})
 print(str(sum(r['passed'] for r in combined))+'/15 actual promotion cases pass; evidence '+str(out));return 0 if all(r['passed'] for r in combined) else 1
if __name__=='__main__':sys.exit(main())
