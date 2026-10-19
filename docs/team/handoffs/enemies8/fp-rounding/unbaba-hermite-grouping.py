"""Bounded grouping audit ONLY, not an equivalence certificate or checker extension."""
from pathlib import Path
import sys,json,struct,hashlib
root=Path('/Users/roberthaisfield/MacioGalaxy2-wt/enemies')
sys.path.insert(0,str(root/'tools/team'))
from rdpool import PoolELF
p=Path.home()/'MacioGalaxy2-team/scratch-enemies/sol8'
u=p/'fp-label-audit/units/Enemy/Unbaba'
out=p/'unbaba-fp'
name='calcUnbabaHermiteSegment__Ffffffff'
d=json.loads((u/'objdiff.json').read_text())
h=lambda b:hashlib.sha256(b).hexdigest()
records={}
for side,filename in [('left','target.o'),('right','source.o')]:
 f=next(x for x in d[side]['symbols'] if x['name']==name)
 elf=PoolELF(u/filename);sec=elf.names.index('.text');off=int(f['address']);size=int(f['size'])
 assert size==80 and not any(s==sec and off<=a<off+size for s,a in elf.relocations)
 raw=elf.data[sec][off:off+size]
 lines=[i['instruction']['formatted'].strip() for i in f['instructions'] if 'instruction' in i]
 assert len(lines)==20 and lines[-1]=='blr'
 # Independently decode every operation and register operand from all 20 raw words.
 for word,line in zip(struct.unpack('>20I',raw),lines):
  if line=='blr':assert word==0x4e800020;continue
  op,args=line.split(None,1);args=args.replace(',','').split()
  assert word>>26==59 and not word&1
  xo=(word>>1)&31;assert {'fdivs':18,'fsubs':20,'fadds':21,'fmuls':25}[op]==xo
  dest=(word>>21)&31;a=(word>>16)&31;b=(word>>11)&31;c=(word>>6)&31
  assert args==[f'f{dest}',f'f{a}',f'f{c if op=="fmuls" else b}']
  assert (b==0 if op=='fmuls' else c==0)
 regs={f'f{i}':('input',x) for i,x in enumerate(['t','start','value','slope','end','endValue','endSlope'],1)}
 nodes=[]
 for line in lines[:-1]:
  op,args=line.split(None,1);dest,a,b=args.replace(',','').split()
  node=(op,regs[a],regs[b]);regs[dest]=node;nodes.append(node)
 records[side]={'instructions':lines,'raw_hex':raw.hex(),'raw_sha256':h(raw),'operations':nodes,'return':regs['f1']}
# Bind the complete target body independently to the original DOL.
dol=(root/'orig/SB4E01/sys/main.dol').read_bytes();address=0x80215d50
for off,base,size in zip(struct.unpack_from('>18I',dol,0),struct.unpack_from('>18I',dol,72),struct.unpack_from('>18I',dol,144)):
 if base<=address and address+80<=base+size:
  assert dol[off+address-base:off+address-base+80].hex()==records['left']['raw_hex'];break
else:raise AssertionError('target not mapped')
# Keep every node's opcode/rounding boundary and tree shape. Only explicitly
# recognize a multiply operand exchange, never reassociation, fusion or width.
def compare(a,b,path=()):
 if a==b:return []
 assert a[0]==b[0] and a[0]!='input',(path,a,b)
 if a[0]=='fmuls' and a[1]==b[2] and a[2]==b[1]:return [{'path':path,'target':a,'source':b}]
 return compare(a[1],b[1],path+(1,))+compare(a[2],b[2],path+(2,))
diff=compare(records['left']['return'],records['right']['return'])
assert len(diff)==1
# Deliberately regroup and change precision: both must reject.
a=('input','a');b=('input','b');c=('input','c')
for x,y in [(('fadds',('fadds',a,b),c),('fadds',a,('fadds',b,c))),(('fmuls',a,b),('fmul',a,b)),(('fsubs',a,b),('fsubs',b,a))]:
 try:compare(x,y)
 except AssertionError:pass
 else:raise AssertionError('negative accepted')
result={'scope':'Grouping audit only; NOT a complete Wii-exact proof. Default REAL remains. No production checker/benign changes.', 'function':name,'target_address':hex(address),'inputs':{str(x):h((u/x).read_bytes()) for x in ['source.cpp','source.o','target.o','objdiff.json']},'original_DOL_sha256':h(dol),'records':records,'only_tree_difference':diff,'negatives':'regrouping, precision and subtraction reversal rejected','conclusion':'Every FP opcode, single-precision rounding boundary, grouping and subtraction/division operand direction agrees. One fmuls exchanges (rate+rate) and curve; no regrouping/fusion/conversion defect found. No claim about NaN payload/FPSCR or whole native behavior.'}
(out/'hermite-grouping.json').write_text(json.dumps(result,indent=2)+'\n')
print('PASS complete80B/20words actualDOL/rawdecode;19FPops;onecommutedmul;3negativecases. Auditonly,defaultREAL.')
