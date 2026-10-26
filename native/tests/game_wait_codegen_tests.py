#!/usr/bin/env python3
"""Fail-closed audit of the actual disabled edge in optimized Release LLVM."""
import re,sys
from pathlib import Path
SSA=r'%[\w.]+'

def instructions(block):
    return [line.split(';',1)[0].strip() for line in block.splitlines()
            if line.split(';',1)[0].strip()]

def blocks(raw):
    chunks=re.split(r'^([\w.]+):[^\n]*\n',raw,flags=re.M)
    result={'entry':instructions(chunks[0])}
    result.update({chunks[i]:instructions(chunks[i+1]) for i in range(1,len(chunks),2)})
    return result

def disabled_edge(entry):
    """Evaluate only supported flag tests at enabled=0; reject other entry work."""
    values={};loaded=False
    for index,line in enumerate(entry):
        # Clang reserves an uninitialized Exit stack slot in entry, but starts
        # its lifetime / writes its token only in the enabled block. No token
        # construction, hook lookup, callback or clock occurs on the OFF edge.
        if re.fullmatch(SSA+r' = alloca %struct\.Exit, align \d+',line):continue
        load=re.fullmatch('('+SSA+r') = load atomic i8, ptr @[\w.]*enabled\w* monotonic, align 1(?:, ![\w.]+ !\d+)*',line)
        if load:
            if loaded:raise ValueError('multiple entry flag loads')
            values[load[1]]=0;loaded=True;continue
        trunc=re.fullmatch('('+SSA+r') = trunc(?: nuw)? i8 ('+SSA+r') to i1',line)
        if trunc and trunc[2] in values:
            values[trunc[1]]=values[trunc[2]]&1;continue
        compare=re.fullmatch('('+SSA+r') = icmp (eq|ne) i(?:8|1) ('+SSA+r'), (0|1|false|true)',line)
        if compare and compare[3] in values:
            rhs=int(compare[4] in ('1','true'))
            equal=values[compare[3]]==rhs
            values[compare[1]]=int(equal if compare[2]=='eq' else not equal);continue
        branch=re.fullmatch(r'br i1 ('+SSA+r'), label %([\w.]+), label %([\w.]+)',line)
        if branch and loaded and branch[1] in values and index==len(entry)-1:
            return branch[2] if values[branch[1]] else branch[3]
        raise ValueError('unsupported entry work before cached flag branch: '+line)
    raise ValueError('missing initial OFF/ON branch')

def audit_off_path(function,metadata=False):
    graph=blocks(function);label=disabled_edge(graph['entry']);previous=None
    seen=set();calls=0;aliases={};result=None
    while label in graph and label not in seen:
        seen.add(label);lines=graph[label]
        for index,line in enumerate(lines):
            call=re.fullmatch('('+SSA+r') = (?:tail )?call i32 @gw_work\([^\n]*\)(?: #[0-9]+)?',line)
            if call and not metadata:
                calls+=1;result=call[1];aliases[result]=result
                if calls!=1:raise ValueError('OFF calls original more than once')
                continue
            phi=re.fullmatch('('+SSA+r') = phi i32 (.*)',line)
            if phi and not metadata:
                operands=re.findall(r'\[\s*('+SSA+r'|-?\d+),\s*%([\w.]+)\s*\]',phi[2])
                # Reject incomplete/unknown syntax rather than accepting a
                # partial regex match; only the predecessor actually traversed
                # supplies the return value on the disabled edge.
                reconstructed=', '.join('[ '+v+', %'+p+' ]' for v,p in operands)
                if reconstructed!=phi[2]:raise ValueError('unsupported PHI syntax')
                selected=[v for v,p in operands if p==previous]
                if len(selected)!=1:raise ValueError('ambiguous OFF PHI predecessor')
                aliases[phi[1]]=aliases.get(selected[0],selected[0]);continue
            jump=re.fullmatch(r'br label %([\w.]+)',line)
            if jump and index==len(lines)-1:
                previous,label=label,jump[1];break
            returned=re.fullmatch(r'ret i32 ('+SSA+r')',line)
            if returned and not metadata and index==len(lines)-1:
                if calls!=1 or aliases.get(returned[1],returned[1])!=result:
                    raise ValueError('OFF lost original return value')
                return
            if metadata and line=='ret void' and index==len(lines)-1:return
            # No hook loads, stores, clocks, lifetime calls, metadata evaluation,
            # indirect calls, invoke edges or other work is accepted on OFF.
            raise ValueError('OFF path contains observer/unsupported work: '+line)
        else:raise ValueError('OFF block has no terminator')
    raise ValueError('OFF path missing block or cycles')

def verify(text):
    def body(name):
        found=re.search(r'^define[^\n]*@'+name+r'\([^\n]*\{\n(.*?)^}',text,re.M|re.S)
        if not found:raise ValueError('missing Release probe '+name)
        return found.group(1)
    if 'optnone' in text:raise ValueError('Release optimization required')
    audit_off_path(body('gw_probe_guarded'))
    audit_off_path(body('gw_probe_metadata'),metadata=True)
    return True
if __name__=='__main__':
    if len(sys.argv)!=2:raise SystemExit('usage: game_wait_codegen_tests.py RELEASE.ll')
    verify(Path(sys.argv[1]).read_text());print('Release cached-flag OFF path / metadata erasure PASS')
