#!/usr/bin/env python3
"""Read/hash candidate source and tool files only; never invokes a compiler/tool."""
import argparse
import hashlib
import json
from pathlib import Path
import time

CONTEXT_SHA='3c123262017672c6ddf256715645a64d23b613e20d6964a1d3f52c69688918e7'
SKIP={'.git','build','builds','node_modules','.cache','__pycache__'}
TEXT_SUFFIX={'.h','.hpp','.hh','.hxx','.c','.cpp','.cc','.cxx','.inc','.inl','.ipp','.tcc','.def','.modulemap','.apinotes','.tbd',''}

def inventory(context_path,source,output):
    start=time.monotonic();source=Path(source).resolve();output=Path(output).resolve()
    if (Path.home()/'MacioGalaxy2-gamedata') not in output.parents:raise ValueError('private output required')
    raw=Path(context_path).read_bytes()
    if hashlib.sha256(raw).hexdigest()!=CONTEXT_SHA:raise ValueError('canonical context hash differs')
    context=json.loads(raw);args=context['entry']['arguments'];candidates={};records={};byte_count=0
    report=dict(schema_version=1,status='INCOMPLETE',context_sha256=CONTEXT_SHA,operation='read/hash only; no compiler/tool invocation',
                consumed_dependencies='UNOBSERVED; future -MD must join every actual dependency to these pins',
                qualification='BLOCKED',qualified_frames=None,records=records,limits=dict(seconds=60,read_bytes=1073741824))
    def budget():
        if time.monotonic()-start>60:raise ValueError('read-only inventory time budget exceeded')
    def add(path):
        budget();p=Path(path)
        if p.is_file():candidates[str(p)]=p
    def tree(root,all_files=False):
        root=Path(root)
        if not root.is_dir():raise ValueError('missing include/resource root '+str(root))
        for p in root.rglob('*'):
            budget()
            if any(part in SKIP for part in p.relative_to(root).parts):continue
            if p.is_file() and (all_files or p.suffix in TEXT_SUFFIX):add(p)
    try:
        roots=[]
        for i,arg in enumerate(args):
            if arg.startswith('-I') and len(arg)>2:roots.append(Path(arg[2:]))
            elif arg=='-isystem':roots.append(Path(args[i+1]))
        original=Path(context['compile_commands']).parent.parent.parent
        # The repo-root include can name source headers; never recurse its builds/assets.
        for root in sorted(set(roots),key=str):
            if root==original:
                for name in ('src','include','native','libs'):tree(root/name)
            else:tree(root)
        sdk=Path(args[args.index('-isysroot')+1]);tree(sdk/'usr/include',True)
        for framework in (sdk/'System/Library/Frameworks').iterdir():
            for name in ('Headers','Modules'):
                if (framework/name).is_dir():tree(framework/name,True)
        for p in (sdk/'usr/lib').rglob('*.tbd'):add(p)
        for root in context['resource_directories']:tree(root,True)
        tree(source/'native/app/visual')
        for record in context['inputs']:add(record['path'])
        add('/usr/bin/otool')
        for logical,path in sorted(candidates.items()):
            budget();real=path.resolve();size=real.stat().st_size
            if str(real) in records:
                records[str(real)]['aliases'].append(logical);continue
            if byte_count+size>1073741824:raise ValueError('read-only inventory byte budget exceeded')
            h=hashlib.sha256()
            with real.open('rb') as stream:
                for block in iter(lambda:stream.read(1048576),b''):
                    budget();h.update(block)
            byte_count+=size;records[str(real)]=dict(sha256=h.hexdigest(),bytes=size,aliases=[logical])
        for record in context['inputs']:
            if records[str(Path(record['path']).resolve())]['sha256']!=record['sha256']:raise ValueError('actual tool/SDK context drift '+record['path'])
        report['status']='CANDIDATE_INPUT_PINS_ONLY'
    except (OSError,ValueError) as exc:report['error']=str(exc)
    report.update(read_bytes=byte_count,files=len(records),seconds=time.monotonic()-start)
    with output.open('x') as stream:json.dump(report,stream,indent=2);stream.write('\n')
    print(json.dumps(dict(status=report['status'],files=report['files'],read_bytes=byte_count,seconds=report['seconds'],error=report.get('error'),output_sha256=hashlib.sha256(output.read_bytes()).hexdigest())))
    return report

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--context',required=True);p.add_argument('--source',required=True);p.add_argument('--out',required=True)
    a=p.parse_args();inventory(a.context,a.source,a.out)
