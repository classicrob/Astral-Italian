import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import sqlite3,tempfile,unittest
from contextlib import contextmanager,closing
from pathlib import Path
from cache_gate52 import inspect

KEY='5'*64
class CacheGate(unittest.TestCase):
 def setUp(self):self.tmp=tempfile.TemporaryDirectory();self.p=Path(self.tmp.name)/('pipeline_cache.'+KEY+'.db')
 def tearDown(self):self.tmp.cleanup()
 @contextmanager
 def db(self,key=KEY,table=True):
  d=sqlite3.connect(self.p);d.execute('CREATE TABLE petari_cache_build(build_key TEXT NOT NULL)');d.execute('INSERT INTO petari_cache_build VALUES(?)',(key,))
  if table:
   d.execute('CREATE TABLE aurora_schema(value INTEGER)');d.execute('INSERT INTO aurora_schema VALUES(1)');d.execute('CREATE TABLE pipeline_cache(type INTEGER,hash INTEGER,config_version INTEGER,config_size INTEGER,config BLOB,first_frame_used INTEGER)')
  d.commit()
  try:
   yield d
   d.commit()
  finally:d.close()
 def test_missing(self):
  with self.assertRaises(ValueError):inspect(self.p,KEY)
  self.assertFalse(self.p.exists())
 def test_empty(self):
  with self.db():pass
  self.assertEqual(inspect(self.p,KEY)['rows'],0)
 def test_nonempty(self):
  with self.db() as d:d.execute('INSERT INTO pipeline_cache VALUES(1,1,13,1,?,0)',(b'x',))
  self.assertEqual(inspect(self.p,KEY)['rows'],1)
 def test_wrong_key(self):
  with self.db('f'*64):pass
  with self.assertRaises(ValueError):inspect(self.p,KEY)
 def test_missing_table(self):
  with self.db(table=False):pass
  with self.assertRaises(ValueError):inspect(self.p,KEY)
 def test_committed_wal(self):
  with self.db() as d:
   d.execute('PRAGMA journal_mode=WAL');d.execute('PRAGMA wal_autocheckpoint=0');d.execute('INSERT INTO pipeline_cache VALUES(1,1,13,1,?,0)',(b'x',));d.commit()
   with closing(sqlite3.connect(self.p.as_uri()+'?mode=ro&immutable=1',uri=True)) as stale:self.assertEqual(stale.execute('SELECT count(*) FROM pipeline_cache').fetchone()[0],0)
   self.assertEqual(inspect(self.p,KEY)['rows'],1)
if __name__=='__main__':unittest.main()
