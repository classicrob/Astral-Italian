import json,subprocess,sys,time
from pathlib import Path
case=Path(sys.argv[1]);label=sys.argv[2];base=Path.home()/'toolchains/sol-t-validation/worldmap51';dest=case/('host-'+label+'.json');assert not dest.exists()
subprocess.run(['open','-n','-a','/opt/homebrew/Cellar/python@3.15/3.15.0/Frameworks/Python.framework/Versions/3.15/Resources/Python.app','--args',str(base/'host_capture.py'),str(case),label],check=True)
end=time.monotonic()+10
while not dest.exists():
 if time.monotonic()>end:raise SystemExit('capture receipt timeout')
 time.sleep(.1)
r=json.loads(dest.read_text());print(json.dumps(r));assert r['returncode']==0
