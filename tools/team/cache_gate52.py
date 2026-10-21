import hashlib,sqlite3
from contextlib import closing
from pathlib import Path

COLUMNS=['type','hash','config_version','config_size','config','first_frame_used']
def inspect(path,key):
 path=Path(path)
 if not path.is_file():raise ValueError('missing cache database')
 artifacts={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in (path,Path(str(path)+'-wal')) if p.is_file()}
 with closing(sqlite3.connect(path.as_uri()+'?mode=ro',uri=True)) as db:
  db.execute('PRAGMA query_only=ON');db.execute('BEGIN')
  if db.execute('SELECT build_key FROM petari_cache_build').fetchall()!=[(key,)]:raise ValueError('wrong cache renderer key')
  count=None
  if path.name.startswith('pipeline_cache.'):
   if [r[1] for r in db.execute('PRAGMA table_info(pipeline_cache)')]!=COLUMNS:raise ValueError('missing or wrong pipeline table')
   if db.execute('SELECT value FROM aurora_schema').fetchall()!=[(1,)]:raise ValueError('wrong Aurora schema')
   count=db.execute('SELECT count(*) FROM pipeline_cache').fetchone()[0]
 return {'path':str(path),'rows':count,'sha256':artifacts[path.name],'artifacts':artifacts,'renderer_key':key,'read_method':'SQLite mode=ro query_only transaction; includes committed WAL'}
