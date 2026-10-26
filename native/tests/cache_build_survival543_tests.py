#!/usr/bin/env python3
"""Proof scorer negatives only; no app, GPU, personal HOME or user-cache access."""
from pathlib import Path
import copy,hashlib,importlib.util,json,sqlite3,struct,tempfile,unittest,zlib
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('proof',ROOT/'tools/port/cache_build_survival543.py')
p=importlib.util.module_from_spec(spec);spec.loader.exec_module(p)

class ProofTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory(prefix='petari-survival-score-');self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name)
        self.e=dict(recipe='r',backend='b',device='d',artifact='a',recipe_accepted=20,recipe_rejected=0,
            counters=dict(hits=30,misses=0,rejected=0,stored=0),observed_artifact_path='/private/dawn_cache.v2.a.db')
        self.before={'dawn_cache.v2.a.db':dict(kind='artifact',entries={'full-key':{'bytes':'unchanged'}})}
        self.images={'frame':dict(width=8,height=8,pixels_sha256='pixel-key',file_sha256='compression-can-differ')}
    def judge(self,e=None,before=None,after=None,images=None):
        return p.judge('reuse',e or self.e,self.before if before is None else before,self.before if after is None else after,self.e,self.images,self.images if images is None else images)
    def test_valid_warm_requires_all_predicates(self):self.assertTrue(all(self.judge().values()))
    def test_new_store_blocks(self):
        e=copy.deepcopy(self.e);e['counters']['stored']=1;self.assertFalse(all(self.judge(e).values()))
    def test_new_miss_blocks_even_without_store(self):
        e=copy.deepcopy(self.e);e['counters']['misses']=1;self.assertFalse(all(self.judge(e).values()))
    def test_missing_hits_blocks(self):
        e=copy.deepcopy(self.e);e['counters']['hits']=0;self.assertFalse(all(self.judge(e).values()))
    def test_no_accepted_recipes_or_corruption_blocks(self):
        for field,value in [('recipe_accepted',0),('recipe_rejected',1)]:
            e=copy.deepcopy(self.e);e[field]=value;self.assertFalse(all(self.judge(e).values()))
        e=copy.deepcopy(self.e);e['counters']['rejected']=1;self.assertFalse(all(self.judge(e).values()))
    def test_every_context_component_bound(self):
        for key in ('recipe','backend','device','artifact'):
            e=copy.deepcopy(self.e);e[key]+='different';self.assertFalse(all(self.judge(e).values()),key)
    def test_overwritten_entry_blocks(self):
        changed=copy.deepcopy(self.before);changed['dawn_cache.v2.a.db']['entries']['full-key']['bytes']='different'
        self.assertFalse(all(self.judge(after=changed).values()))
    def test_missing_and_different_pixels_block(self):
        with self.assertRaises(RuntimeError):self.judge(images={})
        changed=copy.deepcopy(self.images);changed['frame']['pixels_sha256']='other'
        self.assertFalse(all(self.judge(images=changed).values()))
    def test_negative_requires_namespace_miss_and_new_store(self):
        neg=copy.deepcopy(self.e);neg.update(recipe='new-r',artifact='new-a',observed_artifact_path='/private/dawn_cache.v2.new-a.db');neg['counters'].update(misses=20,stored=20)
        after=copy.deepcopy(self.before);after['dawn_cache.v2.new-a.db']=dict(kind='artifact',entries={'new-key':{}})
        self.assertTrue(all(p.judge('negative',neg,self.before,after,self.e).values()))
        for field in ('misses','stored'):
            bad=copy.deepcopy(neg);bad['counters'][field]=0;self.assertFalse(all(p.judge('negative',bad,self.before,after,self.e).values()))
        del after['dawn_cache.v2.a.db'];self.assertFalse(all(p.judge('negative',neg,self.before,after,self.e).values()))
    def log(self):
        return ('[pipeline compatibility] recipe=r backend=b device=d artifact=a\n'
            '[pipeline artifacts] hits=30 misses=0 rejected=0 stored=0\n'
            '[pipeline recipes] type=1 accepted=20 rejected=0 ABI=r\n'
            '[gx frontend pre-present] version=3 result=ready keys=54 completed=54 pending=0 failed=0 manifest_errors=0 elapsed_ms=88 cap_ms=7000 presented_frames=3\n'
            '[gx boot splash frames] presented=3 elapsed_ms=100\n'
            '[pipeline artifacts open] path=/private/dawn_cache.v2.a.db namespace=a legacy=not-adopted\n')
    def test_required_native_reports_and_no_duplicate_counter(self):
        self.assertEqual(p.log_evidence(self.log())['counters']['stored'],0)
        for line in self.log().splitlines(True):
            with self.assertRaises(RuntimeError):p.log_evidence(self.log().replace(line,''))
        with self.assertRaises(RuntimeError):p.log_evidence(self.log()+'[pipeline artifacts] hits=30 misses=0 rejected=0 stored=0\n')
    def png(self,name,change=False):
        def chunk(kind,body):return struct.pack('>I',len(body))+kind+body+struct.pack('>I',zlib.crc32(kind+body)&0xffffffff)
        rows=bytearray()
        for y in range(8):rows+=b'\0'+bytes((i+y+(1 if change else 0))%256 for i in range(24))
        data=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',8,8,8,2,0,0,0))+chunk(b'IDAT',zlib.compress(rows))+chunk(b'IEND',b'')
        path=self.root/name;path.write_bytes(data);return path
    def test_strict_png_pixels_crc_and_truncation(self):
        a,b=self.png('a.png'),self.png('b.png',True)
        self.assertNotEqual(p.image(a)['pixels_sha256'],p.image(b)['pixels_sha256'])
        bad=self.root/'bad.png';data=bytearray(a.read_bytes());data[-1]^=1;bad.write_bytes(data)
        with self.assertRaises(RuntimeError):p.image(bad)
        bad.write_bytes(a.read_bytes()[:-3])
        with self.assertRaises(RuntimeError):p.image(bad)
    def test_snapshot_retains_full_binary_key_and_rejects_index_collision(self):
        path=self.root/'dawn_cache.v2.a.db';key=b'Dawn\x00full-key';index=hashlib.sha256(key).hexdigest()
        with sqlite3.connect(path) as db:
            db.executescript('CREATE TABLE petari_cache_compatibility(protocol,kind,identity);CREATE TABLE artifacts(key_hash,full_key,context,version,value,size,encoding,digest);')
            db.execute('INSERT INTO petari_cache_compatibility VALUES(?,?,?)',(p.PROTOCOL,'artifact','a'))
            db.execute('INSERT INTO artifacts VALUES(?,?,?,2,?,4,0,?)',(index,key,'a',b'data','checked-by-native-loader'))
        snap=p.cache_snapshot(self.root);self.assertEqual(snap[path.name]['entries'][index]['full_key_hex'],key.hex())
        with sqlite3.connect(path) as db:db.execute("UPDATE artifacts SET full_key=x'00'")
        with self.assertRaises(RuntimeError):p.cache_snapshot(self.root)
    def test_cpu_time_parsing(self):
        self.assertEqual(p.cpu_seconds('01:02.50'),62.5);self.assertEqual(p.cpu_seconds('1-01:02:03'),90123)

if __name__=='__main__':unittest.main()
