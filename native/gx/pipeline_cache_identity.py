#!/usr/bin/env python3
"""Versioned cache compatibility, never app-build identity; only build-local seed writes."""
from pathlib import Path
import hashlib
import json
import shutil
import sqlite3
import struct

FIELDS={'protocol','recipe_schema','generator_abi','descriptor_abi','vertex_layout_abi','uniform_abi','byte_order','gx_config_version','gx_config_size','clear_config_version','clear_config_size','field_layout'}
BACKEND_FIELDS={'aurora_revision','dawn_ref','dawn_package','dawn_library_sha256','architecture','provider'}

def canonical(value):return json.dumps(value,sort_keys=True,separators=(',',':'))
def checksum(value):return hashlib.sha256(value.encode()).hexdigest()
def exact_object(path,fields):
    def pairs(items):
        out={}
        for key,value in items:
            if key in out:raise ValueError('duplicate compatibility field: '+key)
            out[key]=value
        return out
    value=json.loads(Path(path).read_text(),object_pairs_hook=pairs)
    if not isinstance(value,dict) or set(value)!=fields:raise ValueError('unknown/missing compatibility fields')
    return value

def manifest(root,backend):
    recipe=exact_object(Path(root)/'native/gx/pipeline_cache_compatibility.json',FIELDS)
    if recipe['protocol']!='petari-pipeline-cache-v2' or recipe['byte_order']!='little':raise ValueError('unsupported cache protocol/byte order')
    if any(type(value) is not int or value<=0 for key,value in recipe.items() if key not in ('protocol','byte_order','field_layout')):raise ValueError('invalid ABI version/size')
    if not isinstance(recipe['field_layout'],str) or not recipe['field_layout']:raise ValueError('missing layout ledger')
    runtime=exact_object(backend,BACKEND_FIELDS)
    if any(not isinstance(v,str) or not v for v in runtime.values()):raise ValueError('missing effective backend identity')
    if runtime['provider']!='package':raise ValueError('unverified Dawn provider; persistent cache requires a configured package')
    for key in ('aurora_revision','dawn_ref','dawn_library_sha256'):
        if len(runtime[key])!=(64 if key.endswith('sha256') else 40) or any(c not in '0123456789abcdef' for c in runtime[key]):raise ValueError('invalid backend digest/ref')
    return dict(protocol=recipe['protocol'],recipe=recipe,recipe_key=checksum(canonical(recipe)),backend=runtime,backend_key=checksum(canonical(runtime)))

def identity(root,aurora=None,backend=None):
    if backend is None:raise ValueError('effective backend receipt required; source-tree hashing is forbidden')
    return manifest(root,backend)['recipe_key']

def recipe_digest(key,kind,hash_value,version,blob):
    return hashlib.sha256(b'PCC2recipe\0'+key.encode()+b'\0'+struct.pack('<IQII',kind,hash_value&((1<<64)-1),version,len(blob))+blob).hexdigest()

def stamp(source,dest,key):
    source,dest=Path(source),Path(dest)
    if source.resolve()==dest.resolve():raise ValueError('seed stamping requires a distinct build-local destination')
    if len(key)!=64 or any(c not in '0123456789abcdef' for c in key):raise ValueError('invalid recipe identity')
    dest.parent.mkdir(parents=True,exist_ok=True)
    shutil.copyfile(source,dest)
    with sqlite3.connect(dest) as db:
        exists=db.execute("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='petari_cache_compatibility'").fetchone()[0]
        if exists and db.execute('SELECT protocol,kind,identity FROM petari_cache_compatibility').fetchall()!=[('petari-pipeline-cache-v2','recipe',key)]:raise ValueError('refusing to relabel a foreign-compatible seed')
        if db.execute("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='petari_cache_build'").fetchone()[0]:raise ValueError('legacy build-stamped seeds must be regenerated from the original pack')
        db.execute('CREATE TABLE IF NOT EXISTS petari_cache_compatibility(protocol TEXT NOT NULL,kind TEXT NOT NULL,identity TEXT NOT NULL)')
        if not exists:db.execute('INSERT INTO petari_cache_compatibility VALUES(?,?,?)',('petari-pipeline-cache-v2','recipe',key))
        db.execute('CREATE TABLE IF NOT EXISTS petari_recipe_integrity(type INTEGER NOT NULL,hash INTEGER NOT NULL,digest TEXT NOT NULL,PRIMARY KEY(type,hash))')
        for kind,h,version,size,blob in db.execute('SELECT type,hash,config_version,config_size,config FROM pipeline_cache'):
            if type(kind) is not int or kind not in (0,1) or type(h) is not int or type(version) is not int or type(size) is not int or not isinstance(blob,bytes) or size!=len(blob):raise ValueError('malformed seed recipe')
            db.execute('INSERT OR REPLACE INTO petari_recipe_integrity VALUES(?,?,?)',(kind,h,recipe_digest(key,kind,h,version,blob)))

if __name__=='__main__':
    import sys
    if sys.argv[1]=='identity':
        if len(sys.argv)!=6:raise SystemExit('identity ROOT AURORA HEADER EFFECTIVE_BACKEND_JSON')
        root,aurora,out,backend=map(Path,sys.argv[2:]);m=manifest(root,backend);out.parent.mkdir(parents=True,exist_ok=True)
        defines={'PETARI_PIPELINE_BUILD_KEY':m['recipe_key'],'PETARI_PIPELINE_RECIPE_KEY':m['recipe_key'],'PETARI_PIPELINE_BACKEND_KEY':m['backend_key'],'PETARI_PIPELINE_COMPATIBILITY_JSON':canonical(m['recipe'])}
        text='#pragma once\n'+''.join('#define '+k+' '+json.dumps(v)+'\n' for k,v in defines.items())
        for k,v in m['recipe'].items():
            if type(v) is int:text+='#define PETARI_CACHE_'+k.upper()+' '+str(v)+'\n'
        if not out.exists() or out.read_text()!=text:out.write_text(text)
        out.with_suffix('.key').write_text(m['recipe_key']+'\n');out.with_suffix('.json').write_text(json.dumps(m,indent=2)+'\n')
    elif sys.argv[1]=='stamp':stamp(sys.argv[2],sys.argv[3],Path(sys.argv[4]).read_text().strip())
    else:raise SystemExit('identity ROOT AURORA HEADER BACKEND_JSON | stamp SOURCE DEST KEYFILE')
