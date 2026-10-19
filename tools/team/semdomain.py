#!/usr/bin/env python3
"""Conservative memory-domain evidence from a compiled unit's Ninja deps.

No C++ purity inference: any uncertain volatile spelling chooses strict order.
This supplies only the *unit* domain. Absolute-immediate base provenance is a
separate per-instruction obligation in semclass and always overrides ordinary.
"""
from __future__ import annotations
import hashlib
import json
from pathlib import Path
import re
import subprocess

STRICT_PREFIXES = ('RVL_SDK/', 'NDEV/', 'MetroTRK/', 'Runtime/', 'MSL_C/',
                   'RVLFaceLib/', 'JSystem/JAudio2/')
_CACHE = {}
_TRIVIA = re.compile(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def code_only(text):
    # Blank rather than delete comments/strings: evidence line numbers survive.
    return _TRIVIA.sub(lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text)


ALIASES = {'vu8','vu16','vu32','vu64','vs8','vs16','vs32','vs64','vf32','vf64'}


def volatile_evidence(text):
    """Binding domain rule: member => unit; global => symbol; unknown => unit.

    Standard volatile aliases are ignored at their typedef definition only, not
    at object/member uses. This is lexical evidence, not a C++ type theorem.
    Ambiguous syntax, nonstandard aliases and macros conservatively stay strict.
    """
    original = code_only(text)
    # An asm qualifier describes compiler scheduling, not C++ object storage.
    original = re.sub(r'\b(?:asm|__asm__)\s+(?:volatile|__volatile__|__volatile)\b',
                      lambda m: ' '*len(m[0]), original)
    # Conditional branches are all inspected; preprocessing lines themselves
    # cannot lend declaration qualifiers to the following C++ statement.
    clean = re.sub(r'(?m)^\s*#[^\n]*', lambda m: ' '*len(m[0]), original)
    tokens = list(re.finditer(r'[A-Za-z_]\w*|[{};]', clean))
    stack=[];scope_at={};last=0
    for token in tokens:
        word=token[0]
        if word=='{':
            head=clean[last:token.start()]
            kind='class' if re.search(r'\b(?:class|struct|union)\b',head) else (
                 'namespace' if re.search(r'\b(?:namespace|extern)\b',head) else
                 'function' if ')' in head else 'unknown')
            stack.append(kind);last=token.end()
        elif word=='}':
            if stack:stack.pop()
            last=token.end()
        elif word==';':last=token.end()
        else:scope_at[token.start()]=tuple(stack)
    hits=[]
    pattern=r'\b(?:volatile|__volatile__|__volatile|'+'|'.join(sorted(ALIASES))+r')\b'
    # Preprocessor aliases may hide volatile at later uses. Unknown => unit.
    for directive in re.finditer(r'(?m)^\s*#[^\n]*', original):
        if re.search(pattern,directive[0]):
            hits.append(dict(line=original.count('\n',0,directive.start())+1,
                        reason='volatile-preprocessor-uncertainty',level='unit',
                        excerpt=' '.join(directive[0].split())[:200]))
    for m in re.finditer(pattern,clean):
        begin=max(clean.rfind(';',0,m.start()),clean.rfind('{',0,m.start()),
                  clean.rfind('}',0,m.start()))+1
        end=clean.find(';',m.end())
        decl=clean[begin:end if end>=0 else len(clean)].strip()
        head=decl.split('=',1)[0].strip()
        equal=clean.find('=',begin,end if end>=0 else len(clean))
        if equal>=0 and m.start()>equal:
            hits.append(dict(line=clean.count('\n',0,m.start())+1,
                        reason='volatile-in-initializer-or-expression',level='unit',
                        spelling=m[0],excerpt=' '.join(decl.split())[:200]))
            continue
        # These declarations define a qualified TYPE, not storage. Arbitrary
        # aliases remain strict because their future uses need type analysis.
        if re.fullmatch(r'typedef\s+volatile\s+[A-Za-z_]\w*\s+(?:'+
                       '|'.join(sorted(ALIASES))+r')',head):continue
        if re.search(r'\b(?:asm|__asm__)\s*$',clean[max(begin,m.start()-12):m.start()]):continue
        simple=(end>=0 and not re.search(r'[(),#]|\b(?:typedef|using|asm|__asm__)\b',head))
        direct_const=(simple and re.search(r'\bconst\b',head)
                      and not re.search(r'[*&]',head))
        if direct_const:continue
        info=dict(line=clean.count('\n',0,m.start())+1,
                  reason='volatile-declaration-unrecognized',level='unit',
                  spelling=m[0],excerpt=' '.join(decl.split())[:200])
        # Named fixed-address objects use the kernel's immediate DEVICE rule.
        absolute=re.search(r':\s*0[xX][0-9a-fA-F]+\s*$',head)
        if simple and absolute:continue
        match=re.search(r'\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\]\s*)*$',head) if simple else None
        if match and match[1] not in ALIASES|{'volatile','const'}:
            scopes=scope_at.get(m.start(),())
            if 'function' not in scopes and 'unknown' not in scopes:
                member='class' in scopes and not re.search(r'\bstatic\b',head)
                info.update(level='unit' if member else 'symbol',name=match[1],
                            pointer=bool(re.search(r'[*&]',head)),
                            reason='volatile-member' if member else 'volatile-global')
        hits.append(info)
    return hits


def parse_deps(output, object_path):
    lines = output.splitlines()
    if not lines:
        raise ValueError('empty Ninja dependency report')
    match = re.fullmatch(re.escape(str(object_path))+r': #deps (\d+), deps mtime \d+ \(VALID\)', lines[0])
    if not match:
        raise ValueError('missing/stale/unrecognized Ninja dependency closure')
    paths = [line[4:] for line in lines[1:] if line.startswith('    ')]
    if not paths or len(paths) != int(match[1]) or any(not p for p in paths):
        raise ValueError('incomplete Ninja dependency path inventory')
    if any(line and not line.startswith('    ') for line in lines[1:]):
        raise ValueError('unexpected extra Ninja dependency output')
    return paths


def load_layouts(root, pins):
    """Only explicitly audited, header-hash-bound member ranges are admitted."""
    path=Path(__file__).with_name('semdomain_layouts.json')
    if not path.is_file():return []
    raw=path.read_bytes();pins[str(path)]=hashlib.sha256(raw).hexdigest()
    result=[]
    for entry in json.loads(raw).get('layouts',[]):
        header=root/entry['path']
        if not header.is_file() or sha(header)!=entry['header_sha256']:continue
        valid=True
        for name,expected in entry.get('prerequisite_sha256',{}).items():
            dep=root/name
            if not dep.is_file() or sha(dep)!=expected:valid=False;break
            pins[str(dep)]=expected
        if valid and isinstance(entry.get('offset'),int) and isinstance(entry.get('width'),int) and entry['offset']>=0 and entry['width']>0:
            result.append(entry)
    return result


def load_qualifier_exceptions(root,pins):
    path=Path(__file__).with_name('semdomain_layouts.json')
    if not path.is_file():return []
    raw=path.read_bytes();pins[str(path)]=hashlib.sha256(raw).hexdigest()
    admitted=[]
    for record in json.loads(raw).get('conversion_qualifier_exceptions',[]):
        header=root/record['path']
        if not header.is_file() or sha(header)!=record['header_sha256']:continue
        good=True
        for name,expected in record.get('prerequisite_sha256',{}).items():
            p=root/name
            if not p.is_file() or sha(p)!=expected:good=False;break
            pins[str(p)]=expected
        if good:admitted.append(record)
    return admitted


def audit_unit(root, unit, target_path=None, source_path=None):
    """Return ordinary only with a complete immutable dependency/source closure.

    Unknown or stale closure always chooses strict-unit, never ordinary. The
    boolean complete records whether evidence was available, not proof of code.
    """
    root = Path(root).resolve()
    unit = unit.removeprefix('main/')
    evidence, pins, strict_symbols, volatile_members = [], {}, [], []
    strict = unit.startswith(STRICT_PREFIXES)
    if strict:
        evidence.append(dict(reason='binding-strict-unit-prefix', unit=unit))
    complete = True
    try:
        config_path = root/'objdiff.json'
        config_raw = config_path.read_bytes()
        pins[str(config_path)] = hashlib.sha256(config_raw).hexdigest()
        configs = [u for u in json.loads(config_raw)['units']
                   if u['name'].removeprefix('main/') == unit]
        if len(configs) != 1:
            raise ValueError('missing/ambiguous configured unit')
        config = configs[0]
        source_name = config.get('metadata', {}).get('source_path')
        if not source_name:
            raise ValueError('source path unavailable')
        flags = config.get('scratch', {}).get('c_flags', '')
        if re.search(r'\b(?:volatile|__volatile__|__volatile)\b', flags):
            strict = True
            evidence.append(dict(reason='volatile-in-compiler-flags', flags=flags))
        object_path = Path(source_path) if source_path else root/config['base_path']
        if not object_path.is_absolute():
            object_path = root/object_path
        obj_stat = object_path.stat()
        pins[str(object_path)] = sha(object_path)
        arg = str(object_path.relative_to(root))
        depdb = root/'.ninja_deps'
        pins[str(depdb)] = sha(depdb)
        run = subprocess.run(['ninja', '-t', 'deps', arg], cwd=root,
                             capture_output=True, text=True)
        if run.returncode:
            raise ValueError('Ninja dependency query failed: '+run.stderr[-200:])
        paths = [Path(p) if Path(p).is_absolute() else root/p
                 for p in parse_deps(run.stdout, arg)]
        source = (root/source_name).resolve()
        if source not in {p.resolve() for p in paths}:
            raise ValueError('source absent from dependency closure')
        # The closure itself is immutable evidence. Pin .ninja_deps, because a
        # concurrent regeneration/build must invalidate an in-flight audit.
        layouts = load_layouts(root, pins)
        exceptions = load_qualifier_exceptions(root, pins)
        for path in sorted(set(p.resolve() for p in paths)):
            stat = path.stat()
            if stat.st_mtime_ns > obj_stat.st_mtime_ns:
                raise ValueError('dependency newer than compiled object: '+str(path))
            raw = path.read_bytes()
            digest = hashlib.sha256(raw).hexdigest()
            pins[str(path)] = digest
            if any((root/v['path']).resolve()==path and v['header_sha256']==digest for v in exceptions):
                evidence.append(dict(reason='pinned-conversion-qualifier-exception',path=str(path),sha256=digest))
                continue
            key = (str(path), digest)
            if key not in _CACHE:
                _CACHE[key] = volatile_evidence(raw.decode('utf-8', errors='replace'))
            for hit in _CACHE[key]:
                matched = [v for v in layouts if (root/v['path']).resolve() == path and v['name'] == hit.get('name') and hit['reason'] == 'volatile-member']
                if len(matched) == 1:
                    member = dict(matched[0], path=str(path), line=hit['line'])
                    volatile_members.append(member)
                    evidence.append(dict(reason='verified-volatile-member-range', **member))
                    continue
                evidence.append(dict(path=str(path), **hit))
                if hit['level'] == 'symbol':
                    strict_symbols.append(dict(path=str(path), **hit))
                else:
                    strict = True
        changed = [p for p, h in pins.items() if sha(p) != h]
        if changed:
            raise ValueError('domain inputs changed: '+', '.join(changed))
        evidence.append(dict(reason='complete-Ninja-dependency-closure', files=len(paths)))
    except (OSError, ValueError, KeyError, TypeError, json.JSONDecodeError) as error:
        strict, complete = True, False
        evidence.append(dict(reason='domain-unknown-defaults-strict', detail=str(error)))
    return dict(domain='strict' if strict else 'ordinary',
                output_domain='strict-unit' if strict else 'ordinary',
                complete=complete, evidence=evidence, pins=pins, strict_symbols=strict_symbols, volatile_members=volatile_members,
                obligation='absolute-immediate bases remain device-base even in ordinary units')
