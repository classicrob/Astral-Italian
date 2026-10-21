"""Independent compile-token expectations from the canonical resolved unit spec.

No compiler or producer recipe is executed. Only POSIX shell tokenization is
normalized: token order, tools, flags, source/output names and postprocessors
remain exact. Graph traversal order is supplied by the pinned official config.
"""
from pathlib import PurePosixPath
import shlex


def need(value, reason):
    if not value:
        raise ValueError(reason)


def canonical_path(value):
    p = PurePosixPath(value)
    need(not p.is_absolute() and '..' not in p.parts and str(p) == value,
         'noncanonical phase-relative path')
    return p


def derive(spec, order, context):
    need(set(context) == {'wrapper','compilers','sjiswrap','assembler','dtk','python'},
         'exact command context required')
    need(all(isinstance(v,str) and PurePosixPath(v).is_absolute() for v in context.values()),
         'operator absolute tool paths required')
    need(len(order) == len(set(order)) == len(spec['units']) and set(order) == set(spec['units']),
         'official unit order/inventory mismatch')
    result = []
    for unit in order:
        row = spec['units'][unit]
        source = canonical_path(row['source'])
        output = canonical_path('build/SB4E01/'+unit)
        opt = row['options']
        need(opt['add_to_all'] is True and opt['extab_padding'] is None,
             'unreviewed source or extab rule')
        if source.suffix in ('.c','.cpp'):
            flags, extra = list(opt['cflags']), list(opt['extra_cflags'])
            if not any(f.startswith('-lang') for f in flags+extra):
                extra.insert(0,'-lang=c++' if source.suffix == '.cpp' else '-lang=c')
            compiler = str(PurePosixPath(context['compilers'])/opt['mw_version']/'mwcceppc.exe')
            prefix = [context['wrapper']]
            need(type(opt['shift_jis']) is bool,'unknown encoding rule')
            if opt['shift_jis']:
                prefix.append(context['sjiswrap'])
            dep = str(output.with_suffix('.d'))
            tokens = prefix+[compiler]+shlex.split(' '.join(flags+extra))+[
                '-MMD','-c',str(source),'-o',str(output.parent),'&&',context['python'],
                'tools/transform_dep.py',dep,dep]
        elif source.suffix == '.s':
            tokens = [context['assembler']]+shlex.split(' '.join(opt['asflags']+opt['extra_asflags']))+[
                '-o',str(output),str(source),'&&',context['dtk'],'elf','fixup',str(output),str(output)]
        else:
            raise ValueError('unreviewed source type')
        result.append({'unit':unit,'source':str(source),'tokens':tokens})
    return result


def check_commands(text, rows):
    actual = [shlex.split(line) for line in text.splitlines() if line.strip()]
    need(actual == [r['tokens'] for r in rows], 'official graph differs from independent token derivation')
    return actual


def dependency_name(value, phase_root):
    """Only the separately pinned exact phase root can be stripped."""
    root = PurePosixPath(phase_root)
    need(root.is_absolute() and '..' not in root.parts and str(root)==phase_root,
         'exact canonical absolute phase root required')
    path = PurePosixPath(value)
    need('..' not in path.parts and str(path)==value, 'noncanonical dependency')
    if path.is_absolute():
        need(path.is_relative_to(root),'foreign absolute dependency root')
        value = path.relative_to(root).as_posix()
    canonical_path(value)
    return value
