import hashlib,json,math
from pathlib import Path

def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def validate_review(out,expected,pid,start,finish):
 out=Path(out).resolve();observation=out/'host-pointer.json';review=out/'visual-review.json'
 if not observation.is_file() or not review.is_file():raise ValueError('missing explicit visual evidence')
 o=json.loads(observation.read_text());r=json.loads(review.read_text())
 if r.get('accepted') is not True:raise ValueError('visual review rejected or pending')
 for field in ('source','binary_sha256','case','mode'):
  if r.get(field)!=expected[field]:raise ValueError('review identity mismatch: '+field)
 for field in ('source','binary_sha256'):
  if o.get(field)!=expected[field]:raise ValueError('observation identity mismatch: '+field)
 when=o.get('observed_monotonic')
 if o.get('observed_pid')!=pid or not isinstance(when,(int,float)) or not math.isfinite(when) or not start<=when<=finish:raise ValueError('stale or wrong live observation')
 if not o.get('ordinary_cua_input') or not o.get('mapping_observed') or not o.get('normal_route_observed'):raise ValueError('missing ordinary normal-route observation')
 if r.get('pointer_observation_sha256')!=digest(observation):raise ValueError('review does not bind live observation')
 paths=[o.get('before_screenshot'),o.get('after_screenshot')]
 if None in paths or len(set(paths))!=2:raise ValueError('two distinct captures required')
 actual={}
 for raw in paths:
  p=Path(raw)
  if p.is_symlink() or not p.is_file() or not p.resolve().is_relative_to(out):raise ValueError('missing or escaped capture')
  actual[raw]=digest(p)
 if o.get('screenshot_sha256')!=actual or r.get('capture_sha256')!=actual:raise ValueError('stale or mismatched capture evidence')
 return True
