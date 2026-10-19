"""Bounded PPC decoder extensions for semclass; no global decoder mutation.

This module describes instruction operands/effects only. The caller must prove
indexed effective addresses and must preserve opaque-call/volatile havoc. An
unsupported spelling, range, or form raises rdchecked.Unproved.
"""
from __future__ import annotations
import re
import rdchecked as rd


def _gpr(token):
    if not re.fullmatch(r'r(?:[0-9]|[12][0-9]|3[01])', token):
        raise rd.Unproved('invalid GPR operand '+token)
    return token


def _integer(token):
    try:
        return int(token, 0)
    except (ValueError, TypeError):
        raise rd.Unproved('nonconstant instruction immediate '+str(token))


def decode(line):
    fields=line.strip().split(None,1)
    op=fields[0] if fields else ''
    args=[x.strip() for x in fields[1].split(',')] if len(fields)==2 else []
    if op in ('extrwi','extrwi.'):
        if len(args)!=4:
            raise rd.Unproved('extrwi operand count')
        dest,source=_gpr(args[0]),_gpr(args[1])
        width,start=_integer(args[2]),_integer(args[3])
        # Nonwrapping extraction, PowerPC bit numbering from the MSB. Other
        # encodings stay unknown even if the rotate-mask primitive can express.
        if not (1<=width<=32 and 0<=start<32 and start+width<=32):
            raise rd.Unproved('extrwi nonwrapping range')
        rotate=(start+width)&31
        return rd.decode(f'rlwinm{"." if op.endswith(".") else ""} {dest}, {source}, {rotate}, {32-width}, 31')
    if op=='subi':
        if len(args)!=3:
            raise rd.Unproved('subi operand count')
        dest,base=_gpr(args[0]),_gpr(args[1])
        value=_integer(args[2])
        if not -32767<=value<=32768:
            raise rd.Unproved('subi negated immediate outside signed16')
        # addi's RA=0 special case is inherited from the audited decoder.
        return rd.decode(f'addi {dest}, {base}, {-value}')
    if op=='lwzx':
        if len(args)!=3:
            raise rd.Unproved('lwzx operand count')
        dest,base,index=map(_gpr,args)
        base=None if base=='r0' else base
        return dict(op='lwzx',args=args,kind='load',base=base,index=index,
                    offset=0,width=4,mode=(),use=([base] if base else [])+[index],
                    **{'def':[dest]},const=(),fp_memory=False)
    if op=='bctrl':
        if args:
            raise rd.Unproved('bctrl operand count')
        return dict(op='bctrl',args=[],kind='call',
                    use=['ctr','r1','r2','r13','cr1']+list(rd.ARGS),
                    **{'def':list(rd.CLOBBER)},const=('indirect-ctr',))
    return rd.decode(line)
