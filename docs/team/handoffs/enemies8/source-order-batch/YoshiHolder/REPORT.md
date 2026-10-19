# YoshiHolder — one natural definition move, ready for root landing

ONLY `src/Game/Player/YoshiHolder.cpp` changed. Move unchanged `fn_8041BB00()` from its earlier definition site to immediately after `fn_8041B700()`, so the constructor and balloon initializer emit their real strings before `Flutter`. Remove its now-obsolete NONMATCHING comment. No header, symbol, layout, split, config, benign, or git changes. No full build; root owns final build/commit/landing. One natural form tried.

## Measured result

- Strict SG:38/59 ->40/59;1944 ->2376 strict code bytes, +432 (constructor384B and fn_8041BB00 48B).
- Exact relocation-masked whole bodies:50 ->51; all50prior masked anchors retained. New masked function is fn_8041B700,280B. This is a different metric from strict+432; root full-report gains remain root's measurement.
- All70emitted source function names and sizes retained. Exactly3raw instruction words change, all same-register ADDI string displacements within B700:0x24->0x1C,0x38->0x30,0x48->0x40. Each still reaches the same full terminated balloon/model/Shadow string.
- All145code relocation identities preserved (complete literal/scalar bytes and widths, exact ordinary callees/other symbols; B700 pool-base change is closed by its three complete actual string uses). Other ALLOC sections are byte/size/flags identical.
- Full80B.data now equals actual original DOL806EB128..806EB178 exactly, zeroADDR32 records. Every real literal including terminator and padding checked. No split change.
- Countdown46behavior repair preserved: BB00's raw body is unchanged, sound request remains outside the `if(mCountdown>0)` decrement guard and still executes when countdown starts zero.

Full data:

```
838883628356815b00000000838883628356815b8341838d815b0000838883628356815b836f838b815b839300000000596f73686942616c6c6f6f6e00000000536861646f770000466c757474657200
```

B700 retains strict99.78571 despite masked code now exact: SG displays the3identical-text ADDI rows as relocation/object annotation differences. No instruction forcing or metadata change proposed.

## Evidence and reproduction

`before/`: original CPP, original source/target objects, fresh unchanged-source baseline compile object, full SG, objdiff and initial hashes. `after/`: final CPP, compiled object, full SG, objdiff, B700 remaining diff. Final copied CPP/object hashes equal the active checkout's final files.

`preservation.json` records all names/anchors, every changed word/full NUL string, all145relocation identities, full data and source/object/config/DOL hashes. All59paired target function bodies are bound to actual original DOL nonrelocated instruction bits and every SDA21 effective address. Startup bytes pin r2=807DECA0/r13=807D7320.

Reproduce without building:

```
PYTHONDONTWRITEBYTECODE=1 nice -n 10 python3 /Users/roberthaisfield/MacioGalaxy2-team/scratch-enemies/sol8/yoshiholder/verify.py
```

PASS: strict38->40(+432B), masked50->51,70functions/145refs/3same-stringADDI/full80Bdata/zeroADDR32,59actualtarget/DOLbindings. No row certificate or benign publication. Source is FROZEN for parent batch landing; Yoshi.cpp remains PAUSED.
