#!/usr/bin/env python3
from pathlib import Path
import hashlib,importlib.util,sqlite3,tempfile,shutil,os
script=Path(__file__).resolve().parents[1]/'gx/pipeline_cache_identity.py'
spec=importlib.util.spec_from_file_location('cache_identity',script);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
with tempfile.TemporaryDirectory() as tmp:
    root=Path(tmp)/'first';aurora=Path(tmp)/'aurora'
    for name in ['native/gx/CMakeLists.txt','native/gx/patch.py','native/gx/policy.hpp','native/gx/boot.mm','native/app/CMakeLists.txt','native/cmake/GraphicsPresentation.cmake']:
        f=root/name;f.parent.mkdir(parents=True,exist_ok=True);f.write_text(name+'\n')
    for name in ['lib/gx/pipeline.hpp','lib/gfx/clear.hpp','lib/gfx/pipeline_cache.cpp','lib/webgpu/gpu_cache.cpp']:
        f=aurora/name;f.parent.mkdir(parents=True,exist_ok=True);f.write_text('schema '+name+'\n')
    key=module.identity(root,aurora);other=Path(tmp)/'different/checkout';shutil.copytree(root,other)
    assert module.identity(other,aurora)==key,'clean builds at different paths must match'
    os.utime(other/'native/gx/policy.hpp',(1,1));assert module.identity(other,aurora)==key,'mtime is not identity'
    (other/'native/gx/policy.hpp').write_text('changed semantics\n');assert module.identity(other,aurora)!=key
    (other/'native/gx/policy.hpp').write_bytes((root/'native/gx/policy.hpp').read_bytes())
    assert module.identity(other,aurora)==key
    (other/'native/gx/boot.mm').write_text('changed native splash\n')
    assert module.identity(other,aurora)!=key,'Objective-C++ renderer changes must invalidate caches'
    source=Path(tmp)/'seed.db'
    with sqlite3.connect(source) as db:db.execute('CREATE TABLE pipeline_cache(config BLOB)');db.execute('INSERT INTO pipeline_cache VALUES(?)',(b'untouched',))
    before=hashlib.sha256(source.read_bytes()).hexdigest();destination=Path(tmp)/'bundle/seed.db';module.stamp(source,destination,key)
    assert hashlib.sha256(source.read_bytes()).hexdigest()==before,'seed source must remain untouched'
    with sqlite3.connect(destination) as db:
        assert db.execute('SELECT build_key FROM petari_cache_build').fetchall()==[(key,)]
        assert db.execute('SELECT config FROM pipeline_cache').fetchall()==[(b'untouched',)]
    try:module.stamp(destination,Path(tmp)/'foreign.db','different-build')
    except ValueError:pass
    else:raise AssertionError('foreign seed must not be relabelled')
print('Deterministic identity/path/mtime/content and same/foreign seed stamping PASS')
