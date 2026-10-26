#!/usr/bin/env python3
"""Current pack -> private Python stamp -> actual native production decoder."""
from pathlib import Path
import hashlib,importlib.util,os,subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'native/tools'));import seedpack
spec=importlib.util.spec_from_file_location('identity',ROOT/'native/gx/pipeline_cache_identity.py')
identity=importlib.util.module_from_spec(spec);spec.loader.exec_module(identity)
if __name__=='__main__':
    binary,key_file=sys.argv[1:]
    key=Path(key_file).read_text().strip()
    pack=ROOT/'native/data/pipeline-seeds.v14.xz';before=hashlib.sha256(pack.read_bytes()).hexdigest()
    meta,blobs,_=seedpack.parse(str(pack))
    with tempfile.TemporaryDirectory(prefix='petari-seed-stamp-') as tmp:
        root=Path(tmp);source=root/'source.db';dest=root/'stamped.db';home=root/'home';home.mkdir()
        seedpack.write_database(str(source),meta,blobs,[(i,m[4]) for i,m in enumerate(meta)])
        identity.stamp(source,dest,key)
        env=os.environ.copy();env['HOME']=str(home)
        subprocess.run([binary,'--check-private-seed',str(dest)],env=env,check=True)
    assert hashlib.sha256(pack.read_bytes()).hexdigest()==before
    print('v14 pack bytes unchanged; private stamping/native decode PASS')
