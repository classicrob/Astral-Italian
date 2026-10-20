#!/usr/bin/env python3
"""Generate src/Game/Player/PlayerAnimeTablesNative.cpp (GALAXY_NATIVE only): the SB4E01 Mario / Mario-ride /
Yoshi Xanime tables (DOL .data outside any split, so no Wii source defines them). Reuses Sol B's verified
assembly decoder (solb/prepare_p15.py: every byte checked against the original DOL, every string pointer
resolved to its assembly object). Row layouts follow include/Game/Animation/XanimeResource.hpp."""
import sys, struct
from pathlib import Path
sys.argv=[sys.argv[0]]
src=Path('/Users/roberthaisfield/MacioGalaxy2-team/scratch-port/solb/prepare_p15.py').read_text().split("if __name__=='__main__':")[0]
g={'__name__':'gen'}; exec(src,g)
import json
def quote(b):  # readable source: Shift-JIS names as text (the native encoder re-encodes narrow literals)
    try: t=b.decode('cp932')
    except UnicodeDecodeError: return '"'+''.join('\\x%02X'%x for x in b)+'"'
    assert t.encode('cp932')==b
    return json.dumps(t,ensure_ascii=False)
g['quote']=quote
data,cstring,fl,u32,s32=g['data'],g['cstring'],g['fl'],g['u32'],g['s32']
SIZE={'XanimeBckTable4':36,'XanimeBckTable3':28,'XanimeBckTable2':20,'XanimeBckTable1':16,'XanimeGroupInfo':72,'XanimeOfsInfo':20,'XanimeSwapTable':8}
specs=[('tripleAnimeTable','XanimeBckTable4'),('doubleAnimeTable','XanimeBckTable3'),('singleAnimeTable','XanimeBckTable2'),
 ('marioAnimeTable','XanimeGroupInfo'),('marioAnimeAuxTable','XanimeOfsInfo'),('marioAnimeOfsTable','XanimeBckTable1'),('luigiAnimeSwapTable','XanimeSwapTable'),
 ('marioRideAnimeTable','XanimeGroupInfo'),('marioRideAnimeAuxTable','XanimeOfsInfo'),('marioRideAnimeOfsTable','XanimeBckTable1'),
 ('marioRideSingleAnimeTable','XanimeBckTable2'),('marioRideDoubleAnimeTable','XanimeBckTable3'),('marioRideTripleAnimeTable','XanimeBckTable4'),
 ('yoshiAnimationGroups','XanimeGroupInfo'),('yoshiAnimationAux','XanimeOfsInfo'),('yoshiAnimationOffsets','XanimeBckTable1'),
 ('yoshiAnimationBckTable','XanimeBckTable2'),('yoshiAnimationBckTable2','XanimeBckTable3'),('yoshiAnimationBckTable3','XanimeBckTable4')]
lines=['// SB4E01 Mario, Mario-ride and Yoshi animation tables (.data outside every split), decoded from the',
 '// original DOL into host-width pointers and scalars by tools/port/gen_player_anime_tables.py.',
 '// The Wii build links these from the original assembly; Game/Player/MarioAnimatorData.hpp holds SMG1 data.','']
for n,typ in specs:
    b=data(n); size=SIZE[typ]; rows=len(b)//size
    lines.append(f'{typ} {n}[] = {{')
    for k in range(rows):
        row=b[k*size:(k+1)*size]; name=cstring(u32(row))
        if typ.startswith('XanimeBckTable') and typ!='XanimeBckTable1':
            f=['{'+name+'}','{'+', '.join('{'+cstring(u32(row,i))+', '+fl(row,i+4)+'}' for i in range(4,size,8))+'}']
        elif typ=='XanimeBckTable1':f=['{'+name+'}',cstring(u32(row,4)),hex(u32(row,8)),hex(u32(row,12))]
        elif typ=='XanimeGroupInfo':
            assert row[32:48]==bytes(16),(n,k,'runtime animation pointers expected zero')
            w=', '.join(fl(row,48+4*i) for i in range(4))
            f=['{'+name+'}',fl(row,4),str(u32(row,8)),fl(row,12),fl(row,16),fl(row,20),str(s32(row,24)),str(row[28]),str(row[29]),
               '{nullptr, nullptr, nullptr, nullptr}','{'+w+'}',hex(u32(row,64)),cstring(u32(row,68))]
        elif typ=='XanimeOfsInfo':f=['{'+name+'}',fl(row,4),fl(row,8),fl(row,12),str(u32(row,16))]
        else:f=[name,cstring(u32(row,4))]
        lines.append('    {'+', '.join(f)+'},')
    tail=b[rows*size:]
    assert tail==bytes(len(tail)),(n,'non-zero partial row')
    lines+=['};','']
out=Path('/Users/roberthaisfield/MacioGalaxy2-wt/port/src/Game/Player/PlayerAnimeTablesNative.cpp')
out.write_text('#ifdef GALAXY_NATIVE\n#include "Game/Animation/XanimeResource.hpp"\n\n'+'\n'.join(lines)+'\n#endif\n')
print('wrote',out,'tables',len(specs))
