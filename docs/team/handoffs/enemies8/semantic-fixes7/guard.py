#!/usr/bin/env python3
"""Check exact-base report regression and the seven-fix matched-byte attribution."""
import argparse,json,hashlib
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('before',type=Path);p.add_argument('after',type=Path);p.add_argument('--out',type=Path,required=True);a=p.parse_args()
old=json.loads(a.before.read_text());new=json.loads(a.after.read_text())
A={u['name']:u for u in old['units']};B={u['name']:u for u in new['units']};assert A.keys()==B.keys(), 'unit inventory changed'
drops=[];changed=[];anchor_count=0
for n,u in A.items():
 v=B[n]
 for k in ['matched_code','matched_data']:
  x=int(u.get('measures',{}).get(k,0));y=int(v.get('measures',{}).get(k,0))
  if y<x:drops.append([n,k,x,y])
  if y!=x:changed.append([n,k,x,y,y-x])
 # Preserve named body identities; local duplicates are retained by positional occurrence.
 counts={};oldf={};newf={}
 for dest,fs in [(oldf,u.get('functions',[])),(newf,v.get('functions',[]))]:
  counts={}
  for f in fs:
   key=(f['name'],counts.get(f['name'],0));counts[f['name']]=key[1]+1;dest[key]=f
 for key,f in oldf.items():
  if f.get('fuzzy_match_percent')==100:
   anchor_count+=1
   assert key in newf and newf[key].get('fuzzy_match_percent')==100, ('old exact body lost',n,key)
assert not drops,drops
expected={'main/Game/AudioLib/AudSystem':0,'main/Game/Effect/ParticleDrawExecutor':0,'main/Game/Enemy/MogucchiShooter':88,'main/Game/Enemy/Popo':1032,'main/Game/Screen/LayoutManager':340,'main/Game/Screen/SuddenDeathMeter':148}
for n,g in expected.items():assert int(B[n]['measures'].get('matched_code',0))-int(A[n]['measures'].get('matched_code',0))==g,(n,g)
assert all(n in expected for n,k,x,y,d in changed),changed
assert sum(row[4] for row in changed if row[1]=='matched_code')==1608
out={'before_sha256':hashlib.sha256(a.before.read_bytes()).hexdigest(),'after_sha256':hashlib.sha256(a.after.read_bytes()).hexdigest(),'units':len(A),'old_exact_functions_preserved':anchor_count,'drops':drops,'changed_unit_measures':changed,'matched_code_gain':1608,'matched_data_gain':sum(row[4] for row in changed if row[1]=='matched_data')}
a.out.write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out,indent=2))
