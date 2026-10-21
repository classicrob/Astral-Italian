import json,subprocess,sys,time
from pathlib import Path
case=Path(sys.argv[1]);at=float(sys.argv[2]);label=sys.argv[3];base=Path.home()/'toolchains/sol-t-validation/worldmap51';start=json.loads((case/'command.json').read_text())['start_epoch']
def until(t):
 while time.time()-start<t:time.sleep(.1)
until(at-2)
subprocess.run([sys.executable,str(base/'capture_wait.py'),str(case),label],check=True)
until(at)
subprocess.run([sys.executable,str(base/'host_action.py'),str(case),'click','right'],check=True)
