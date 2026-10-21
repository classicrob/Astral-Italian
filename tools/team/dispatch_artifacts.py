"""Confined copied-artifact validation; data only, no execution authority."""
import hashlib,re
from pathlib import Path,PurePosixPath

def require(ok,message):
 if not ok:raise ValueError('dispatch artifact: '+message)

def relative_name(name):
 require(type(name) is str,'path type')
 rel=PurePosixPath(name)
 require(not rel.is_absolute() and '..' not in rel.parts and rel.parts and str(rel)==name,'noncanonical/traversal path')
 return rel

def confined_file(root,name):
 root=Path(root).resolve();rel=relative_name(name);path=root/rel
 cursor=root
 for part in rel.parts:
  cursor=cursor/part;require(not cursor.is_symlink(),'symlink artifact component')
 require(path.is_file() and path.resolve().is_relative_to(root),'missing/nonregular/outside artifact')
 return path

def negative_artifacts(root,rows,count=26):
 require(type(rows) is list and len(rows)==count,'exact negative artifact count')
 require({r.get('scope') for r in rows}=={'TU13','TU14'},'both scopes required')
 seen=set();paths=set()
 for row in rows:
  require(set(row)=={'scope','label','path','sha256','rejected'},'negative artifact fields')
  require(row['rejected'] is True,'control did not reject')
  require(type(row['label']) is str and row['label'],'control label')
  key=(row['scope'],row['label']);require(key not in seen and row['path'] not in paths,'duplicate control/path');seen.add(key);paths.add(row['path'])
  require(type(row['sha256']) is str and re.fullmatch('[0-9a-f]{64}',row['sha256']) is not None,'digest format')
  path=confined_file(root,row['path']);require(hashlib.sha256(path.read_bytes()).hexdigest()==row['sha256'],'stale artifact hash')
 require(all(sum(r['scope']==scope for r in rows)==count//2 for scope in ('TU13','TU14')),'separate equal scoped counts')
 return True
