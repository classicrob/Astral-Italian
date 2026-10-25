#!/usr/bin/env python3
"""Deterministic renderer identity and build-local seed stamping (no user DB writes)."""
from pathlib import Path
import hashlib,sys,sqlite3,shutil
import re

def identity(root,aurora):
    root,aurora=Path(root),Path(aurora)
    files=[p for p in (root/'native/gx').rglob('*') if p.is_file() and p.suffix in ('.cpp','.mm','.hpp','.h','.py','.inc','.cmake','.wgsl') and '__pycache__' not in p.parts]
    files += [root/'native/gx/CMakeLists.txt',root/'native/app/CMakeLists.txt',root/'native/cmake/GraphicsPresentation.cmake']
    digest=hashlib.sha256(b'petari-renderer-cache-v1\0aurora-08122911e8621acb7ded6563813b264bec1494b5\0')
    for f in sorted(set(files)):
        digest.update(str(f.relative_to(root)).encode()+b'\0'+f.read_bytes()+b'\0')
    for name in ['lib/gx/pipeline.hpp','lib/gfx/clear.hpp','lib/gfx/pipeline_cache.cpp','lib/webgpu/gpu_cache.cpp']:
        digest.update(name.encode()+b'\0'+(aurora/name).read_bytes()+b'\0')
    packs = list((root/'native/data').glob('pipeline-seeds.v*.xz'))
    if packs:
        selected = max(packs, key=lambda p: int(re.fullmatch(r'pipeline-seeds\.v(\d+)\.xz', p.name)[1]))
        digest.update(str(selected.relative_to(root)).encode()+b'\0'+selected.read_bytes()+b'\0')
    return digest.hexdigest()

def stamp(source,dest,key):
    source,dest=Path(source),Path(dest);dest.parent.mkdir(parents=True,exist_ok=True)
    shutil.copyfile(source,dest)
    with sqlite3.connect(dest) as db:
        db.execute('CREATE TABLE IF NOT EXISTS petari_cache_build(build_key TEXT NOT NULL)')
        rows=db.execute('SELECT build_key FROM petari_cache_build').fetchall()
        if rows and rows!=[(key,)]:raise ValueError('refusing to relabel foreign-build seed')
        if not rows:db.execute('INSERT INTO petari_cache_build VALUES(?)',(key,))

if __name__=='__main__':
    if sys.argv[1]=='identity':
        key=identity(sys.argv[2],sys.argv[3]);out=Path(sys.argv[4]);out.parent.mkdir(parents=True,exist_ok=True)
        text='#pragma once\n#define PETARI_PIPELINE_BUILD_KEY "'+key+'"\n'
        if not out.exists() or out.read_text()!=text:out.write_text(text)
        out.with_suffix('.key').write_text(key+'\n')
    elif sys.argv[1]=='stamp':stamp(sys.argv[2],sys.argv[3],Path(sys.argv[4]).read_text().strip())
    else:raise SystemExit('identity ROOT AURORA HEADER | stamp SOURCE DEST KEYFILE')
