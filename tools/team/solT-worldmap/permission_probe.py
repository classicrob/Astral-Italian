import contextlib,runpy,sys
from pathlib import Path
p=Path.home()/'toolchains/sol-t-validation/worldmap51'
sys.argv=[str(p/'host_input.py'),'query']
with (p/'launchservices-preflight.json').open('w') as out:
 with contextlib.redirect_stdout(out):runpy.run_path(str(p/'host_input.py'),run_name='__main__')
