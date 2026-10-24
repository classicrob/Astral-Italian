#!/usr/bin/env python3
"""PORT-authorized main lineage plus fresh candidate Wii/hash/oracle proof."""
import argparse,hashlib,json,re,subprocess,sys
from pathlib import Path
from common import ROOT,HOME,REPO,sha,write
MAIN='71353f07a00371cc50282a7bbc54b5dc68a8f1b7'

def evaluate(directory,server_log):
 directory=Path(directory).resolve();server_log=Path(server_log).resolve()
 assert directory.is_relative_to(ROOT/'candidates') and server_log.is_relative_to(ROOT/'candidates')
 build=json.loads((directory/'BUILD.json').read_text());source=build['source']
 assert re.fullmatch('[0-9a-f]{40}',source) and server_log.name=='wii-proof-'+source[:8]+'.log'
 def git(*args):return subprocess.run(['git',*args],cwd=REPO,capture_output=True,text=True)
 report=HOME/'MacioGalaxy2-team/main-report'/(MAIN+'.json');main_report=json.loads(report.read_text())
 diff=git('diff','--name-only',MAIN,source,'--','src','include','libs')
 land=git('show',MAIN+':tools/team/land.sh');allow=git('show',source+':tools/team/dispatchcheck-allow.txt')
 assert land.returncode==allow.returncode==0
 status=dict(re.findall(r'^(rc|dispatch_rc)=(-?\d+)\s*$',server_log.read_text(),re.M))
 checks={'main_on_origin_main':git('merge-base','--is-ancestor',MAIN,'origin/main').returncode==0,
  'candidate_descends_from_main':git('merge-base','--is-ancestor',MAIN,source).returncode==0,
  'land_report_present':main_report.get('version')==2 and bool(main_report.get('units')),
  'land_full_gate_witness':all(marker in land.stdout for marker in ('tools/team/nj.sh','dispatchcheck.py','--strict','if [ "$gate_mode" = full ]; then','main-report')),
  'zero_wii_source_diff':diff.returncode==0 and not diff.stdout.strip(),
  'candidate_binary_identity':sha(directory/'Petari.app/Contents/MacOS/Petari')==build['binary_sha256'],
  'fresh_candidate_wii_build_hash':status.get('rc')=='0','strict_dispatch_oracle':status.get('dispatch_rc')=='0'}
 return {'passed':all(checks.values()),'proof_kind':'main-lineage-and-fresh-candidate-wii','build_kind':'server-candidate','source':source,'source_exe_sha256':build['binary_sha256'],'checks':checks,
  'authority':'PORT ruling 2026-10-24: main71353 lineage + zero Wii source diff + fresh candidate Wii build/hash and strict dispatch; preserve old a23609 comparison failure.',
  'main_lineage':{'commit':MAIN,'report':str(report),'report_sha256':sha(report),'report_units':len(main_report.get('units',[])),'land_script_sha256':hashlib.sha256(land.stdout.encode()).hexdigest()},
  'source_diff':{'command':['git','diff','--name-only',MAIN,source,'--','src','include','libs'],'exit':diff.returncode,'files':diff.stdout.splitlines()},
  'fresh_candidate':{'server_log':str(server_log),'server_log_sha256':sha(server_log),'build_hash_exit':int(status['rc']) if 'rc' in status else None,'strict_dispatch_exit':int(status['dispatch_rc']) if 'dispatch_rc' in status else None,'build_command_reported_by_PORT':'tools/team/nj.sh (full Wii build + hash check)','dispatch_command_reported_by_PORT':'dispatchcheck.py --strict with checked-in allow list','allow_list_sha256':hashlib.sha256(allow.stdout.encode()).hexdigest(),'source_run_provenance':'PORT report in thr_p6fp7zhizj identifies this log as fresh candidate '+source[:8]},
  'reason':'Main71353 landed through full Wii/hash/oracle checks; candidate has zero Wii-source changes; fresh candidate Wii/hash and strict dispatch both exit0.',
  'limits':['Server status log contains exit codes, not full build stdout; command/source provenance comes from PORT\'s explicit run report.','Old unchanged-since-a23609ca comparison remains false and is retained separately.'],'frozen54_acceptance':False}

def validate(path,directory):
 actual=json.loads(Path(path).read_text());expected=evaluate(directory,actual['fresh_candidate']['server_log'])
 assert actual==expected and actual['passed'] is True
 return actual

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('candidate_dir',type=Path);p.add_argument('out',type=Path);p.add_argument('--server-log',type=Path,required=True);a=p.parse_args()
 assert a.out.resolve().is_relative_to(ROOT/'campaigns') and not a.out.exists()
 proof=evaluate(a.candidate_dir,a.server_log);write(a.out,proof);print(json.dumps({'passed':proof['passed'],'checks':proof['checks'],'receipt':str(a.out)},indent=2));return 0 if proof['passed'] else 1
if __name__=='__main__':sys.exit(main())
