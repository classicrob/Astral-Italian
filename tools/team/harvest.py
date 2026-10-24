#!/usr/bin/env python3
"""harvest.py X [X...] [--dry]   (run from the landing worktree root)
Env: HARVEST_WT (default ~/MacioGalaxy2-wt/decomp-{x}) = helper worktree pattern,
     HARVEST_DIR (default ~/MacioGalaxy2-team/scratch-decomp2/harvest) = harvested-copy store.
3-way merge each Sol worktree's changed files into the lead worktree:
  base = last harvested copy of that file (harvest/<x>/<path>) or the Sol's merge-base with origin/main,
  ours = lead worktree file, theirs = Sol worktree file.
Records the harvested copy so the next harvest only brings new changes. Prints conflicts (left unmerged)."""
import os, subprocess, sys, shutil
HDIR = os.path.expanduser(os.environ.get('HARVEST_DIR', '~/MacioGalaxy2-team/scratch-decomp2/harvest'))
PAT = os.environ.get('HARVEST_WT', '~/MacioGalaxy2-wt/decomp-{x}')
LEAD = os.getcwd()
dry = '--dry' in sys.argv
for x in [a for a in sys.argv[1:] if not a.startswith('--')]:
    wt = os.path.expanduser(PAT.format(x=x))
    base_rev = subprocess.check_output(['git', '-C', wt, 'merge-base', 'HEAD', 'origin/main'], text=True).strip()
    files = subprocess.check_output(['git', '-C', wt, 'diff', '--name-only', base_rev], text=True).split()
    files += subprocess.check_output(['git', '-C', wt, 'ls-files', '--others', '--exclude-standard', 'src', 'include', 'libs'], text=True).split()
    hdir = os.path.join(HDIR, x)
    n = 0
    for f in files:
        theirs = os.path.join(wt, f)
        if not os.path.exists(theirs):
            print(f'[{x}] SKIP deleted {f}'); continue
        hcopy = os.path.join(hdir, f)
        if os.path.exists(hcopy):
            base = open(hcopy, 'rb').read()
        else:
            r = subprocess.run(['git', '-C', wt, 'show', f'{base_rev}:{f}'], capture_output=True)
            base = r.stdout if r.returncode == 0 else b''
        tb = open(theirs, 'rb').read()
        if tb == base:
            continue
        ours = os.path.join(LEAD, f)
        if dry:
            print(f'[{x}] would merge {f}'); n += 1; continue
        os.makedirs(os.path.dirname(ours), exist_ok=True)
        if not os.path.exists(ours):
            open(ours, 'wb').write(b'')
        bf = ours + '.hv-base'; tf = ours + '.hv-theirs'
        open(bf, 'wb').write(base); open(tf, 'wb').write(tb)
        r = subprocess.run(['git', 'merge-file', '-L', 'lead', '-L', 'base', '-L', f'sol-{x}', ours, bf, tf])
        os.unlink(bf); os.unlink(tf)
        if r.returncode != 0:
            print(f'[{x}] CONFLICT {f} ({r.returncode})')
        else:
            n += 1
        os.makedirs(os.path.dirname(hcopy), exist_ok=True)
        open(hcopy, 'wb').write(tb)
    print(f'[{x}] merged {n} files')
