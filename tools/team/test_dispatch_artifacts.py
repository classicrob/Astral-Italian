import hashlib,tempfile,unittest,copy
from pathlib import Path
import dispatch_artifacts as a
class Paths(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name);self.rows=[]
  for scope in ('TU13','TU14'):
   for i in range(13):
    rel=f'dispatch/negative-ELFs/{scope}-{i}.o';p=self.root/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(f'{scope}/{i}'.encode())
    self.rows.append({'scope':scope,'label':str(i),'path':rel,'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'rejected':True})
 def test_all26_copied_paths_and_hashes(self):self.assertTrue(a.negative_artifacts(self.root,self.rows))
 def test_original_execution_relative_path_rejects(self):
  self.rows[0]['path']=self.rows[0]['path'].removeprefix('dispatch/')
  with self.assertRaises(ValueError):a.negative_artifacts(self.root,self.rows)
 def test_missing(self):
  (self.root/self.rows[0]['path']).unlink()
  with self.assertRaises(ValueError):a.negative_artifacts(self.root,self.rows)
 def test_stale(self):
  (self.root/self.rows[0]['path']).write_bytes(b'stale')
  with self.assertRaises(ValueError):a.negative_artifacts(self.root,self.rows)
 def test_traversal_absolute_noncanonical(self):
  for name in ('../outside.o','/tmp/outside.o','dispatch/../outside.o','dispatch//bad.o','./dispatch/bad.o'):
   with self.subTest(name=name):
    rows=copy.deepcopy(self.rows);rows[0]['path']=name
    with self.assertRaises(ValueError):a.negative_artifacts(self.root,rows)
 def test_symlink_file_and_parent(self):
  p=self.root/self.rows[0]['path'];data=p.read_bytes();p.unlink();other=self.root/'other';other.write_bytes(data);p.symlink_to(other)
  with self.assertRaises(ValueError):a.negative_artifacts(self.root,self.rows)
  p.unlink();p.write_bytes(data);directory=p.parent;renamed=directory.with_name('real');directory.rename(renamed);directory.symlink_to(renamed,target_is_directory=True)
  with self.assertRaises(ValueError):a.negative_artifacts(self.root,self.rows)
 def test_duplicate_and_wrong_scope_count(self):
  rows=copy.deepcopy(self.rows);rows[-1]=rows[0]
  with self.assertRaises(ValueError):a.negative_artifacts(self.root,rows)
  rows=copy.deepcopy(self.rows);rows[0]['scope']='TU14'
  with self.assertRaises(ValueError):a.negative_artifacts(self.root,rows)
if __name__=='__main__':unittest.main()
