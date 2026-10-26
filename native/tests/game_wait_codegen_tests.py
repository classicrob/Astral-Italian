#!/usr/bin/env python3
"""Audit optimized Release LLVM: OFF reaches original work without any hook/clock."""
import re,sys
from pathlib import Path
def verify(text):
    def body(name):
        found=re.search(r'^define[^\n]*@'+name+r'\([^\n]*\{\n(.*?)^}',text,re.M|re.S)
        if not found:raise ValueError('missing Release probe '+name)
        return found.group(1)
    raw=body('gw_probe_guarded')
    if 'optnone' in text:raise ValueError('Release optimization required')
    chunks=re.split(r'^([\w.]+):[^\n]*\n',raw,flags=re.M)
    blocks={'entry':chunks[0]}
    blocks.update({chunks[i]:chunks[i+1] for i in range(1,len(chunks),2)})
    entry=blocks['entry']
    if not re.search(r'load atomic.*enabled',entry):raise ValueError('cached flag not in entry')
    if re.search(r'\bcall\b|\binvoke\b',entry):raise ValueError('work before cached flag branch')
    targets=re.findall(r'label %([\w.]+)',entry)
    if len(targets)!=2:raise ValueError('missing initial OFF/ON branch')
    def fast(label,seen=()):
        if label in seen or label not in blocks:return False
        block=blocks[label]
        if 'hooks' in block:return False
        calls=re.findall(r'(?:call|invoke)[^\n]+',block)
        if any('@gw_work(' not in c for c in calls):return False
        if calls and 'ret i32' in block:return len(calls)==1
        nexts=re.findall(r'label %([\w.]+)',block)
        return len(nexts)==1 and fast(nexts[0],seen+(label,))
    if not any(fast(t) for t in targets):raise ValueError('OFF path contains observer work or lost original call')
    metadata=body('gw_probe_metadata')
    # No original work is allowed before testing the flag; argument evaluation
    # must live strictly in the enabled branch.
    first_branch=metadata.find('br i1')
    if first_branch<0 or '@gw_work' in metadata[:first_branch]:raise ValueError('OFF evaluates metadata')
    return True
if __name__=='__main__':
    if len(sys.argv)!=2:raise SystemExit('usage: game_wait_codegen_tests.py RELEASE.ll')
    verify(Path(sys.argv[1]).read_text());print('Release cached-flag OFF path / metadata erasure PASS')
