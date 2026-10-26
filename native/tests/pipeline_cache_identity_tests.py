#!/usr/bin/env python3
"""Source-only compatibility/seed tests; every SQLite file is under a private root."""
from pathlib import Path
import hashlib,importlib.util,sqlite3,tempfile,shutil,json,unittest,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
script=ROOT/'native/gx/pipeline_cache_identity.py'
spec=importlib.util.spec_from_file_location('cache_identity',script)
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)

def backend(path):
    value=dict(aurora_revision='a'*40,dawn_ref='b'*40,dawn_package='fixture-package',dawn_library_sha256='c'*64,architecture='arm64',provider='package')
    path.write_text(json.dumps(value));return value

class CompatibilityTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory(prefix='petari-cache-compat-');self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name)/'checkout';self.root.mkdir()
        (self.root/'native/gx').mkdir(parents=True)
        shutil.copyfile(ROOT/'native/gx/pipeline_cache_compatibility.json',self.root/'native/gx/pipeline_cache_compatibility.json')
        self.backend=Path(self.tmp.name)/'backend.json';backend(self.backend)
    def key(self):return module.identity(self.root,backend=self.backend)
    def edit(self,name,value):
        path=self.root/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(value)
    def test_unrelated_source_and_cmake_and_seed_hit(self):
        first=self.key()
        for name in ('native/gx/diagnostics.cpp','native/gx/pipeline_worker.inc','native/gx/CMakeLists.txt','native/app/CMakeLists.txt','native/cmake/GraphicsPresentation.cmake','native/data/pipeline-seeds/pack-v99.bin'):
            self.edit(name,'arbitrary unrelated change')
            self.assertEqual(self.key(),first,name)
        other=Path(self.tmp.name)/'other';shutil.copytree(self.root,other)
        self.assertEqual(module.identity(other,backend=self.backend),first)
    def test_descriptor_and_layout_and_generator_miss(self):
        path=self.root/'native/gx/pipeline_cache_compatibility.json';original=json.loads(path.read_text());first=self.key()
        for field in ('descriptor_abi','vertex_layout_abi','generator_abi','uniform_abi','recipe_schema'):
            changed=original.copy();changed[field]+=1;path.write_text(json.dumps(changed))
            self.assertNotEqual(self.key(),first,field)
        path.write_text(json.dumps(original))
    def test_backend_is_effective_and_separate(self):
        original=backend(self.backend);first=module.manifest(self.root,self.backend)
        for field in original:
            if field=='provider':continue
            changed=original.copy();changed[field]=('d'*len(changed[field]) if field.endswith('sha256') or field.endswith('ref') or field.endswith('revision') else 'changed')
            self.backend.write_text(json.dumps(changed));new=module.manifest(self.root,self.backend)
            self.assertEqual(new['recipe_key'],first['recipe_key'])
            self.assertNotEqual(new['backend_key'],first['backend_key'])
    def test_manifest_unknown_missing_duplicate_invalid(self):
        path=self.root/'native/gx/pipeline_cache_compatibility.json';original=path.read_text();value=json.loads(original)
        for mutation in ({**value,'unexpected':1},{k:v for k,v in value.items() if k!='descriptor_abi'},{**value,'generator_abi':True},{**value,'gx_config_size':-1},{**value,'protocol':'unknown'}):
            path.write_text(json.dumps(mutation))
            with self.assertRaises(ValueError):self.key()
        path.write_text(original[:-2]+',"generator_abi":1}\n')
        with self.assertRaises(ValueError):self.key()
        path.write_text(original)
        with self.assertRaises(ValueError):module.identity(self.root)
    def test_stamp_integrity_and_no_pack_rewrite(self):
        source=Path(self.tmp.name)/'source.db';dest=Path(self.tmp.name)/'bundle/db';key=self.key()
        clear=b'\x05\0\0\0\x01\x01\x01\0'
        with sqlite3.connect(source) as db:
            db.execute('CREATE TABLE aurora_schema(value INTEGER)');db.execute('INSERT INTO aurora_schema VALUES(1)')
            db.execute('CREATE TABLE pipeline_cache(type INTEGER,hash INTEGER,config_version INTEGER,config_size INTEGER,config BLOB,first_frame_used INTEGER,PRIMARY KEY(type,hash))')
            db.execute('INSERT INTO pipeline_cache VALUES(0,-42,5,8,?,12)',(clear,))
        before=hashlib.sha256(source.read_bytes()).hexdigest();module.stamp(source,dest,key)
        self.assertEqual(hashlib.sha256(source.read_bytes()).hexdigest(),before)
        with sqlite3.connect(dest) as db:
            self.assertEqual(db.execute('SELECT * FROM petari_cache_compatibility').fetchall(),[('petari-pipeline-cache-v2','recipe',key)])
            self.assertEqual(db.execute('SELECT digest FROM petari_recipe_integrity').fetchone()[0],module.recipe_digest(key,0,-42,5,clear))
            self.assertEqual(db.execute('SELECT config,first_frame_used FROM pipeline_cache').fetchall(),[(clear,12)])
        with self.assertRaises(ValueError):module.stamp(dest,Path(self.tmp.name)/'foreign.db','d'*64)
        with sqlite3.connect(source) as db:db.execute('CREATE TABLE petari_cache_build(build_key TEXT)')
        with self.assertRaises(ValueError):module.stamp(source,Path(self.tmp.name)/'old.db',key)
    def test_generated_header(self):
        out=Path(self.tmp.name)/'generated/pipeline_cache_build_identity.hpp'
        subprocess.run([sys.executable,'-B',str(script),'identity',str(self.root),'unused',str(out),str(self.backend)],check=True)
        self.assertIn(self.key(),out.read_text())
        self.assertEqual(json.loads(out.with_suffix('.json').read_text())['recipe_key'],self.key())

if __name__=='__main__':unittest.main()
