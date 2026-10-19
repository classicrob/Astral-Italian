#!/usr/bin/env python3
"""Bounded candidate-only MWCC source variants. No LLM/network calls.

Schema/CLI checkpoint (v1):
  permute.py UNIT 'Class::method' --symbol MANGLED --out /absolute/scratch \
      --input-root /immutable/archive --seed 0 --budget 20
Default is plan-only. Compilation requires --run --compile-cleared, plus captured
--command-file (one final ninja shell command, not JSON), --original-object,
--target, --input-manifest. A known transform_dep suffix is stripped, not run.
--queue accepts {schema_version:1,source_revision,entries:[{unit,function,symbol,
input_root,command_file,original_object,target_object,input_manifest}]}.

Initial transforms: adjacent independent builtin literal declarations; simple
builtin-bool-parameter &&/nested-if forms, preserving short-circuit order. No FP
expression rewrite, casts, flags, qualifiers, pragmas, helpers or forcing. These
are candidates, NOT automatic semantic proofs; independent review is mandatory.
Input manifest v1 requires source_revision, source_sha256, command_sha256,
original_object_sha256, target_object_sha256, and dependencies:{relative:sha256}
covering actual compiler .d includes, generated headers, compiler and wrappers.
The original object is one captured whole-TU scratch compile, reused throughout.
There is no automatic Ninja/configure or live-object fallback. Queue runs are
sequential. --resume requires unchanged inputs/tool/seed/budget; --exhausted
accepts {original_source_sha256:[exhausted_full_candidate_sha256,...]}.
--timeout bounds each child; --seconds bounds each function (max1800, persisted).
No scheduling/overnight launch occurs in this tool. Exit2 means blocked rows.

Automatic extraction strips other supported member definitions while preserving
headers, globals and free/local helpers verbatim. Unsupported context fails
closed. Extracted baseline must equal the captured original function/runtime
references before scoring. A survivor needs a scratch whole-TU guard and review.
"""
from __future__ import annotations
import argparse
from collections import Counter
from dataclasses import dataclass, asdict
import difflib
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import random
import re
import shlex
import signal
import subprocess
import sys
import time

SCHEMA_VERSION = 1
TOOL_ROOT = Path(__file__).resolve().parents[2]

@dataclass(frozen=True)
class Token:
    text: str
    start: int
    end: int
    kind: str = 'code'

@dataclass(frozen=True)
class FunctionSpan:
    start: int
    end: int
    body_start: int
    body_end: int

@dataclass(frozen=True)
class Variant:
    source: str
    transform: str
    start: int
    end: int
    before: str
    after: str
    obligations: tuple[str, ...] = ('Independent semantic/readability review required.',)


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def lex(source: str) -> list[Token]:
    """Tokenize boundaries, retaining PP lines but ignoring comments/literals."""
    if re.search(r'\\\r?\n', source): raise ValueError('phase2 backslash-newline context unsupported')
    result = []
    i = 0
    while i < len(source):
        if source[i].isspace():
            i += 1
            continue
        start = i
        if source.startswith('//', i):
            i = source.find('\n', i)
            if i < 0: break
            continue
        if source.startswith('/*', i):
            end = source.find('*/', i + 2)
            if end < 0: raise ValueError('unterminated comment')
            i = end + 2
            continue
        if source[i] == '#' and not source[source.rfind('\n', 0, i)+1:i].strip():
            i = source.find('\n', i)
            if i < 0: i = len(source)
            while i > start and source[i-1] == '\\':
                i = source.find('\n', i+1)
                if i < 0: i = len(source); break
            result.append(Token(source[start:i], start, i, 'pp'))
            continue
        raw = re.match(r'(?:u8|u|U|L)?R"([^\s()\\]{0,16})\(', source[i:])
        if raw:
            end = source.find(')' + raw[1] + '"', i + raw.end())
            if end < 0: raise ValueError('unterminated raw string')
            i = end + len(raw[1]) + 2
            result.append(Token(source[start:i], start, i, 'literal'))
            continue
        quoted = re.match(r'(?:u8|u|U|L)?[\'\"]', source[i:])
        if quoted:
            quote = quoted[0][-1]
            i += quoted.end()
            while i < len(source):
                if source[i] == '\\': i += 2; continue
                if source[i] == quote: i += 1; break
                i += 1
            else: raise ValueError('unterminated string/character')
            result.append(Token(source[start:i], start, i, 'literal'))
            continue
        m = re.match(r'[A-Za-z_]\w*|(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?[uUlLfF]*|::|&&|\|\||->|==|!=|<=|>=|\+\+|--|.', source[i:])
        assert m
        i += m.end()
        result.append(Token(m[0], start, i))
    return result


def paired(tokens: list[Token]) -> dict[int, int]:
    stack = []
    result = {}
    for i, t in enumerate(tokens):
        if t.kind != 'code': continue
        if t.text in ('(', '{', '['): stack.append((t.text, i))
        elif t.text in (')', '}', ']'):
            if not stack or {'(': ')', '{': '}', '[': ']'}[stack[-1][0]] != t.text:
                raise ValueError('unbalanced C++ delimiters')
            _, j = stack.pop()
            result[i] = j
            result[j] = i
    if stack: raise ValueError('unbalanced C++ delimiters')
    return result


def definitions(source: str) -> list[tuple[str, str, FunctionSpan]]:
    ts = lex(source)
    ps = paired(ts)
    result = []
    for i, t in enumerate(ts):
        if t.kind != 'code' or t.text != '(' or i == 0: continue
        j = i-1
        if not re.fullmatch(r'[A-Za-z_]\w*', ts[j].text): continue
        while j >= 2 and ts[j-1].text == '::' and re.fullmatch(r'[A-Za-z_]\w*', ts[j-2].text): j -= 2
        name = ''.join(x.text for x in ts[j:i])
        if name in ('if', 'for', 'while', 'switch', 'catch'): continue
        close = ps[i]
        k = close + 1
        while k < len(ts) and ts[k].text in ('const', 'override', 'final', 'noexcept'):
            if ts[k].text == 'noexcept' and k+1 < len(ts) and ts[k+1].text == '(': k = ps[k+1]
            k += 1
        # Initializer lists/function-try/operators/templates deliberately unsupported.
        if k >= len(ts) or ts[k].text != '{': continue
        before = j-1
        while before >= 0 and ts[before].text not in (';', '{', '}') and ts[before].kind != 'pp': before -= 1
        start = ts[before+1].start
        sig = ''.join(x.text for x in ts[i:close+1])
        span = FunctionSpan(start, ts[ps[k]].end, ts[k].start, ts[ps[k]].start)
        result.append((name, sig, span))
    return result


def find_function(source: str, selector: str) -> FunctionSpan:
    m = re.fullmatch(r'([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*)(\(.*\))?', selector.strip(), re.S)
    if not m: raise ValueError('unsupported function selector; use qualified name, optional parameter signature')
    sig = ''.join(t.text for t in lex(m[2])) if m[2] else None
    hits = [s for n, p, s in definitions(source) if n == m[1] and (sig is None or sig == p)]
    if len(hits) != 1: raise ValueError(f'function selector must identify exactly one supported definition; got {len(hits)}')
    span = hits[0]
    if any(t.kind == 'pp' for t in lex(source[span.start:span.end])):
        raise ValueError('preprocessor within function unsupported')
    return span


def _bool_parameters(source: str, span: FunctionSpan) -> set[str]:
    head = source[span.start:span.body_start]
    m = re.search(r'\(([^()]*)\)\s*(?:const\s*)?$', head)
    if not m: return set()
    names = set()
    for p in m[1].split(','):
        b = re.fullmatch(r'\s*bool\s+([A-Za-z_]\w*)\s*', p)
        if b: names.add(b[1])
    body = source[span.body_start+1:span.body_end]
    # Shadowing/type/macro-sensitive names are not inferred from headers.
    for name in tuple(names):
        if re.search(r'\b[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*\s+[*&]?\s*'+re.escape(name)+r'\b', body): names.remove(name)
    return names


def generate_variants(source: str, span: FunctionSpan, seed: int = 0, budget: int = 20, transforms: tuple[str, ...] | None = None) -> list[Variant]:
    if not 0 <= budget <= 300: raise ValueError('budget must be 0..300')
    if any(t.kind == 'pp' for t in lex(source[span.start:span.end])): return []
    if re.search(r'\b__(?:LINE|COUNTER|FILE|DATE|TIME|TIMESTAMP)__\b', source): return []
    edits = []
    body = source[span.body_start+1:span.body_end]
    origin = span.body_start+1
    # Only literal-initialized, nonconst/nonvolatile builtin scalars, one declaration/line.
    decl = re.compile(r'(?m)^[ \t]*(?:bool|int|unsigned int|signed int|float|double)\s+([A-Za-z_]\w*)\s*=\s*([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?[uUlLfF]*|true|false)\s*;[ \t]*(?=\n|$)')
    bt = lex(body)
    code_starts = {t.start for t in bt if t.kind == 'code'}
    ds = [m for m in decl.finditer(body) if m.start()+len(m[0])-len(m[0].lstrip(' \t')) in code_starts]
    for a, b in zip(ds, ds[1:]):
        gap = body[a.end():b.start()]
        if gap.strip() or a[1] == b[1]: continue
        start, end = origin+a.start(), origin+b.end()
        before = source[start:end]
        after = b[0] + gap + a[0]
        edits.append(('independent_literal_declarations', start, end, before, after))
    ts = lex(body)
    ps = paired(ts)
    bools = _bool_parameters(source, span)
    if any(t.text == '[' for t in ts): bools = set()  # lambda captures/array scopes: no type inference
    depth = 0
    for i, t in enumerate(ts):
        if t.text == '{': depth += 1
        elif t.text == '}': depth -= 1
        if depth != 0: continue  # no nested shadow/lambda type inference
        if t.text != 'if' or i+1 >= len(ts) or ts[i+1].text != '(': continue
        op = i+1; cp = ps[op]; ob = cp+1
        if ob >= len(ts) or ts[ob].text != '{': continue
        cb = ps[ob]
        cond = ts[op+1:cp]
        if cb+1 < len(ts) and ts[cb+1].text == 'else':
            eob = cb+2
            if len(cond)==1 and cond[0].text in bools and eob<len(ts) and ts[eob].text=='{':
                ecb=ps[eob]
                before=body[t.start:ts[ecb].end]
                after='if (!'+cond[0].text+') '+body[ts[eob].start:ts[ecb].end]+' else '+body[ts[ob].start:ts[cb].end]
                edits.append(('builtin_bool_if_else_inversion',origin+t.start,origin+ts[ecb].end,before,after))
            continue
        if len(cond) == 3 and cond[0].text in bools and cond[1].text == '&&' and cond[2].text in bools:
            a, b = cond[0].text, cond[2].text
            inner = body[ts[ob].start:ts[cb].end]
            old = body[t.start:ts[cb].end]
            new = f'if ({a}) {{ if ({b}) {inner} }}'
            edits.append(('bool_and_to_nested_if',origin+t.start,origin+ts[cb].end,old,new))
        if len(cond) != 1 or cond[0].text not in bools: continue
        inside = ob+1
        if inside+1 >= cb or ts[inside].text != 'if' or ts[inside+1].text != '(': continue
        icp = ps[inside+1]; iob = icp+1
        if iob >= cb or ts[iob].text != '{' or ps[iob]+1 != cb: continue
        icond = ts[inside+2:icp]
        if len(icond) != 1 or icond[0].text not in bools: continue
        inner = body[ts[iob].start:ts[ps[iob]].end]
        old = body[t.start:ts[cb].end]
        new = f'if ({cond[0].text} && {icond[0].text}) {inner}'
        edits.append(('nested_if_to_bool_and',origin+t.start,origin+ts[cb].end,old,new))
    allowed = set(transforms) if transforms is not None else {'independent_literal_declarations','bool_and_to_nested_if','nested_if_to_bool_and','builtin_bool_if_else_inversion'}
    edits = [e for e in edits if e[0] in allowed]
    random.Random(seed).shuffle(edits)
    variants = []
    seen = {sha(source.encode())}
    for name, start, end, before, after in edits:
        if not span.body_start < start <= end <= span.body_end: raise ValueError('edit escaped selected body')
        candidate = source[:start]+after+source[end:]
        h = sha(candidate.encode())
        if h in seen: continue
        seen.add(h)
        variants.append(Variant(candidate,name,start,end,before,after))
        if len(variants) >= budget: break
    return variants if budget else []


def check_output_file(path: Path) -> None:
    path=Path(path)
    if any(p.is_symlink() for p in (path,*path.parents)):raise ValueError('symlinked output forbidden: '+str(path))
    if path.exists() and (not path.is_file() or path.stat().st_nlink!=1):raise ValueError('nonregular/hardlinked output forbidden: '+str(path))


def safe_output(out: Path, input_root: Path) -> Path:
    out = Path(out)
    if not out.is_absolute(): raise ValueError('scratch output must be absolute')
    resolved = out.resolve()
    for root in (Path(input_root).resolve(), TOOL_ROOT.resolve()):
        if resolved == root or root in resolved.parents: raise ValueError('output must be outside input/root checkout')
    # Reject symlinked existing ancestors even when final resolve is outside root.
    if any(p.is_symlink() for p in (out,*out.parents)): raise ValueError('symlinked output path unsupported')
    if out.exists():
        for child in out.rglob('*'):
            if child.is_symlink():raise ValueError('symlinked scratch descendant forbidden: '+str(child))
            if not child.is_dir():check_output_file(child)
    return resolved


def redirect_command(command: str, unit: str, source_path: Path, out_dir: Path, input_root: Path) -> list[str]:
    root = Path(input_root).resolve()
    src = Path(source_path)
    dest = safe_output(Path(out_dir),root)
    if not src.is_absolute() or src.resolve().parent != dest: raise ValueError('candidate source must be absolute inside scratch object directory')
    tokens = shlex.split(command.strip())
    if '&&' in tokens:
        cut = tokens.index('&&');tail=tokens[cut+1:];tokens=tokens[:cut]
        if len(tail)!=4 or not Path(tail[0]).name.startswith('python') or tail[1]!='tools/transform_dep.py' or tail[2]!=tail[3]:
            raise ValueError('unknown compound shell suffix')
    if any(any(c in t for c in (';', '|', '&', '<', '>', '`', '\n', '\r')) or '$' in t for t in tokens): raise ValueError('shell syntax unsupported')
    if tokens.count('-c') != 1 or tokens.count('-o') != 1: raise ValueError('exactly one -c source and -o output required')
    ci,oi=tokens.index('-c'),tokens.index('-o')
    if ci+1>=len(tokens) or oi+1>=len(tokens): raise ValueError('missing compiler option operand')
    if not any(Path(t).name=='mwcceppc.exe' for t in tokens): raise ValueError('captured command must invoke the real MWCC compiler')
    original=(root/tokens[ci+1]).resolve()
    expected=(root/'src'/(unit+'.cpp')).resolve()
    if original!=expected: raise ValueError('captured command source does not match unit')
    tokens[ci+1]=str(src)
    tokens[oi+1]=str(dest)
    for token in tokens:
        if token.startswith(('-o','-MF','-MT','-MQ')) and token not in ('-o','-MF','-MT','-MQ'): raise ValueError('joined output option unsupported')
        if token in ('-S','-E','-preprocess','-list','-asmout','-disassemble'): raise ValueError('additional output mode unsupported')
    i=0
    while i<len(tokens):
        if tokens[i] in ('-MF','-MT','-MQ'):
            if i+1>=len(tokens): raise ValueError('missing dependency option operand')
            tokens[i+1]=str(dest/(src.stem+('.d' if tokens[i]=='-MF' else '.o')));i+=2;continue
        if tokens[i].startswith(('-MF','-MT','-MQ')) and tokens[i] not in ('-MMD','-MD'): raise ValueError('joined dependency output option unsupported')
        if tokens[i] in ('-o','-c'): i+=2;continue
        i+=1
    # Executable and nested Windows compiler/wrapper paths must be captured files.
    for i,t in enumerate(tokens):
        if (i==0 or t.endswith('.exe')) and not t.startswith('-'):
            p=Path(t);tokens[i]=str(p if p.is_absolute() else root/p)
    return tokens


def object_snapshot(path: Path) -> dict:
    spec=importlib.util.spec_from_file_location('_permute_elf',Path(__file__).with_name('datacontent.py'))
    module=importlib.util.module_from_spec(spec);sys.modules[spec.name]=module;spec.loader.exec_module(module)
    e=module.Elf(Path(path))
    def identity(r,seen=frozenset()):
        s=r.symbol
        if s.section==0 or s.kind==2: return ['symbol',s.name,r.addend]
        # Whole local storage payload+extent, never bare TU-local symbol names.
        if not 0<s.section<len(e.sections): return ['unsupported',s.name,s.section,r.addend]
        owner_key=(s.section,s.address,s.name)
        if owner_key in seen:return ['storage_cycle',*owner_key,r.addend]
        seen=seen|{owner_key}
        sec=e.sections[s.section]
        start=s.address if s.size else 0
        size=s.size if s.size else sec[5]
        payload=e.contents[s.section][start:start+size]
        nested=[]
        for (owner,at),rr in e.relocations.items():
            if owner==s.section and start<=at<start+size:
                nested.append([at-start,rr.kind,identity(rr,seen)])
        return ['local_storage',e.names[s.section],s.section,s.name,s.address,size,payload.hex(),sorted(nested),s.address+r.addend-start]
    def rels(sec,start,size):
        return sorted([[at-start,r.kind,identity(r)]for(owner,at),r in e.relocations.items()if owner==sec and start<=at<start+size])
    funcs={}
    covered={}
    for name,f in e.functions.items():
        funcs[name]={'size':f.size,'bytes':e.contents[f.section][f.address:f.address+f.size].hex(),'relocations':rels(f.section,f.address,f.size),'local':f.binding==0}
        covered.setdefault(f.section,set()).update(range(f.address,f.address+f.size))
    data={}
    for i,sec in enumerate(e.sections):
        if not sec[2]&2: continue
        key=f'{i}:{e.names[i]}'
        payload=e.contents[i]
        if sec[2]&4:
            # All uncovered executable storage remains guarded, including padding.
            data[key]={'size':sum(j not in covered.get(i,set())for j in range(sec[5])),'bytes':bytes(v for j,v in enumerate(payload)if j not in covered.get(i,set())).hex(),'relocations':[[at,r.kind,identity(r)]for(owner,at),r in e.relocations.items()if owner==i and at not in covered.get(i,set())]}
        else:data[key]={'size':sec[5],'bytes':payload.hex(),'relocations':rels(i,0,sec[5])}
    return {'functions':funcs,'data':data}


def preservation_check(baseline: dict, candidate: dict, selected_symbol: str) -> dict:
    violations=[]
    a,b=baseline['functions'],candidate['functions']
    if set(a)!=set(b): violations.append('emitted function inventory changed')
    if selected_symbol not in a or selected_symbol not in b: violations.append('selected function missing')
    for name in sorted(set(a)&set(b)):
        if name!=selected_symbol and a[name]!=b[name]: violations.append('neighbor function changed: '+name)
    if selected_symbol in a and selected_symbol in b and [r[1:]for r in a[selected_symbol]['relocations']]!=[r[1:]for r in b[selected_symbol]['relocations']]: violations.append('selected ordered reference identities changed')
    if baseline['data']!=candidate['data']: violations.append('allocated data/uncovered text/extent/references changed')
    return {'passed':not violations,'violations':violations}


def extract_function(source: str, span: FunctionSpan) -> str:
    """Conservative import: same includes/globals/free helpers, other members removed."""
    if re.search(r'^\s*#\s*include\s*[<\"][^>\"]+\.(?:cpp|c|cc|cxx)[>\"]',source,re.M): raise ValueError('unity source include requires explicit import; unsupported')
    cuts=[]
    for name,_,other in definitions(source):
        if other==span or '::' not in name: continue
        if other.start<span.end and span.start<other.end: raise ValueError('nested/ambiguous definition extraction')
        if any(t.kind=='pp' for t in lex(source[other.start:other.end])): raise ValueError('member definition contains preprocessor context')
        cuts.append((other.start,other.end))
    if not cuts: raise ValueError('no smaller independent member-function TU available')
    result=source
    for a,b in sorted(cuts,reverse=True):result=result[:a]+''.join('\n'if c=='\n'else' 'for c in result[a:b])+result[b:]
    if len(result.replace(' ','').replace('\n',''))>200000: raise ValueError('extracted context still exceeds bounded TU size')
    return result


def extraction_baseline_check(original: dict, extracted: dict, selected_symbol: str, canonical_original_providers: dict | None = None) -> dict:
    """Exact emitted provider closure; absent public providers need pinned originals."""
    violations=[];bindings=canonical_original_providers or {}
    a,b=original['functions'],extracted['functions'];todo=[selected_symbol];seen=set();external=[]
    def symbols(value):
        if isinstance(value,list):
            if value and value[0]=='symbol':yield value[1]
            else:
                for child in value:yield from symbols(child)
    while todo:
        name=todo.pop()
        if name in seen:continue
        seen.add(name)
        if name not in a:
            violations.append('missing original selected/provider: '+name);continue
        if name not in b:
            digest=sha(json.dumps(a[name],sort_keys=True).encode())
            if name!=selected_symbol and not a[name].get('local') and bindings.get(name)==digest:
                external.append({'symbol':name,'original_fingerprint_sha256':digest})
            else:violations.append('missing selected/emitted provider: '+name)
            continue
        if a[name]!=b[name]:violations.append('original/extracted body or effective references differ: '+name)
        for _,_,key in a[name]['relocations']:
            if key and key[0]=='unsupported':violations.append('unsupported referenced storage: '+name)
            for target in symbols(key):
                if target in a:todo.append(target)
    return {'passed':not violations,'violations':violations,'checked_functions':sorted(seen),'canonical_original_provider_bindings':external}


def atomic_json(path: Path, data: dict) -> None:
    check_output_file(path)
    temp=path.with_suffix(path.suffix+'.tmp')
    check_output_file(temp)
    if path.is_symlink()or temp.is_symlink():raise ValueError('symlinked manifest output forbidden')
    temp.write_text(json.dumps(data,indent=2)+'\n')
    os.replace(temp,path)


def run_process(argv: list[str], cwd: Path, timeout: float, log: Path) -> dict:
    """No shell; timeout kills only this command's process group."""
    start=time.monotonic()
    check_output_file(log)
    with log.open('wb') as stream:
        proc=subprocess.Popen(argv,cwd=cwd,env={**os.environ,'MG2_INTERACTIVE':'1'},stdout=stream,stderr=subprocess.STDOUT,start_new_session=True)
        timed_out=False
        try:code=proc.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            timed_out=True
            os.killpg(proc.pid,signal.SIGTERM)
            try:code=proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                try:os.killpg(proc.pid,signal.SIGKILL)
                except ProcessLookupError:pass
                code=proc.wait()
            # Parent may exit on TERM while its child ignores TERM. Always clean
            # the timed-out process group, even after the parent has returned.
            try:os.killpg(proc.pid,signal.SIGKILL)
            except ProcessLookupError:pass
    return {'returncode':code,'timed_out':timed_out,'seconds':time.monotonic()-start,'argv':argv,'log':str(log)}


def validate_inputs(root: Path, source_bytes: bytes, command_bytes: bytes, original: Path, target: Path, manifest: dict) -> dict:
    if manifest.get('schema_version')!=1:raise ValueError('input manifest schema_version must be1')
    wanted={'source_sha256':sha(source_bytes),'command_sha256':sha(command_bytes),'original_object_sha256':sha(original.read_bytes()),'target_object_sha256':sha(target.read_bytes())}
    for k,v in wanted.items():
        if manifest.get(k)!=v:raise ValueError('frozen input mismatch: '+k)
    deps=manifest.get('dependencies')
    if not isinstance(deps,dict)or not deps:raise ValueError('complete captured dependency SHA256 map required')
    for name,digest in deps.items():
        p=Path(name)
        if p.is_absolute()or '..'in p.parts:raise ValueError('dependency must be captured root-relative path: '+name)
        if sha((root/p).read_bytes())!=digest:raise ValueError('frozen dependency mismatch: '+name)
    return {**wanted,'source_revision':manifest.get('source_revision'),'dependency_count':len(deps),'input_manifest_sha256':sha(json.dumps(manifest,sort_keys=True).encode())}


def dependency_closure(depfile: Path, root: Path, manifest: dict, source_path: Path) -> dict:
    """Only simple captured MWCC make dependencies; unknown syntax blocks a run."""
    if not depfile.exists():return {'passed':False,'violations':['compiler did not emit captured dependency file']}
    raw=depfile.read_text()
    text=raw.replace('\\\n',' ')
    separator=re.search(r':(?:\s|$)',text)
    if not separator:return {'passed':False,'violations':['unsupported dependency syntax']}
    rhs=text[separator.end():]
    try:
        if re.search(r'^\s*[A-Za-z]:[\\/]',raw,re.M):
            tokens=[]
            for line in raw.splitlines()[1:]:
                value=line.strip()
                if value.endswith(' \\'):value=value[:-2]
                if not value:continue
                if not re.match(r'[zZ]:[\\/]',value):return {'passed':False,'violations':['unmapped Windows dependency drive']}
                tokens.append(value[2:].replace('\\','/'))
        else:tokens=shlex.split(rhs)
    except ValueError:return {'passed':False,'violations':['unsupported dependency escaping']}
    missing=[];checked=[]
    allowed=manifest['dependencies']
    for token in tokens:
        path=Path(token);path=(path if path.is_absolute()else root/path).resolve()
        if path==source_path.resolve():continue
        try:key=str(path.relative_to(root.resolve()))
        except ValueError:missing.append('dependency outside captured tree: '+str(path));continue
        if key not in allowed or sha(path.read_bytes())!=allowed[key]:missing.append('unbound dependency: '+key)
        checked.append(key)
    return {'passed':bool(tokens)and not missing,'violations':missing or ([]if tokens else['empty dependency list']),'checked':checked}


def compile_scratch(source_text: str, unit: str, directory: Path, root: Path, command: str, timeout: float, input_manifest: dict) -> tuple[Path,dict]:
    directory=safe_output(directory,root);directory.mkdir(parents=True,exist_ok=True)
    path=directory/(Path(unit).name+'.cpp')
    if path.is_symlink():raise ValueError('symlinked candidate source forbidden')
    path.write_text(source_text)
    argv=redirect_command(command,unit,path,directory,root)
    for index,token in enumerate(argv):
        if index==0 or token.endswith('.exe'):
            executable=Path(token).resolve()
            try:key=str(executable.relative_to(root.resolve()))
            except ValueError:raise ValueError('compiler/wrapper outside captured input tree')
            if input_manifest['dependencies'].get(key)!=sha(executable.read_bytes()):raise ValueError('unbound compiler/wrapper executable: '+key)
    result=run_process(argv,root,timeout,directory/'compiler.log')
    atomic_json(directory/'command.json',result)
    obj=directory/(path.stem+'.o')
    result['object']=str(obj)
    if result['returncode']==0 and obj.exists():result['dependency_closure']=dependency_closure(directory/(path.stem+'.d'),root,input_manifest,path)
    else:result['dependency_closure']={'passed':False,'violations':['compiler failure/timeout/missing object']}
    return obj,result


def diff_scores(target: Path, candidate: Path, objdiff: Path, directory: Path, timeout: float) -> dict:
    path=directory/'objdiff.json'
    check_output_file(path)
    result=run_process([str(objdiff),'-L','error','diff','-1',str(target),'-2',str(candidate),'-o',str(path)],directory,timeout,directory/'objdiff.log')
    if result['returncode']!=0 or not path.exists():raise ValueError('objdiff failed: '+str(directory/'objdiff.log'))
    data=json.loads(path.read_text())
    scores={s['name']:{'size':int(s.get('size',0)),'match_percent':s.get('match_percent')}for s in data['left']['symbols']if s.get('instructions')}
    atomic_json(directory/'scores.json',scores)
    return scores


def selected_percent(scores: dict, symbol: str) -> float:
    if symbol not in scores or scores[symbol].get('match_percent')is None:raise ValueError('selected symbol absent/unpaired in objdiff')
    return float(scores[symbol]['match_percent'])


def function_run(entry: dict, settings: dict, out: Path) -> dict:
    started=time.monotonic();deadline=started+settings['seconds']
    def remaining():
        left=deadline-time.monotonic()
        if left<=0: raise ValueError('function wall-time budget exhausted')
        return min(settings['timeout'],left)
    root=Path(entry['input_root']).resolve();out=safe_output(out,root)
    unit=entry['unit'];selector=entry['function'];symbol=entry.get('symbol')
    if not re.fullmatch(r'(?:Game|JSystem|revolution|Runtime|MSL_C|MSL_C\+\+)/[A-Za-z0-9_/]+',unit)or'..'in unit:raise ValueError('unsupported/unsafe unit path')
    source_path=root/'src'/(unit+'.cpp');source_bytes=source_path.read_bytes();source=source_bytes.decode()
    span=find_function(source,selector)
    variants=generate_variants(source,span,settings['seed'],settings['budget'])
    fingerprint={'schema_version':1,'unit':unit,'function':selector,'symbol':symbol,'source_sha256':sha(source_bytes),'tool_sha256':sha(Path(__file__).read_bytes()),'seed':settings['seed'],'budget':settings['budget']}
    live=out/'manifest.json'
    if live.exists():
        if not settings['resume']:raise ValueError('existing output requires --resume')
        state=json.loads(live.read_text())
        if state.get('fingerprint')!=fingerprint:raise ValueError('resume source/tool/seed/budget identity changed')
    else:
        if out.exists()and any(out.iterdir()):raise ValueError('output directory must be new/empty')
        out.mkdir(parents=True,exist_ok=True)
        state={'schema_version':1,'fingerprint':fingerprint,'status':'PLAN_ONLY','review_required':True,'variants':[]}
    prior_elapsed=float(state.get('elapsed_seconds',0))
    deadline=min(deadline,started+max(0,settings['seconds']-prior_elapsed))
    records={r['source_sha256']:r for r in state['variants']}
    for i,v in enumerate(variants,1):
        h=sha(v.source.encode());directory=safe_output(out/f'variant-{i:03d}',root);directory.mkdir(exist_ok=True)
        candidate_path=directory/(Path(unit).name+'.cpp')
        if candidate_path.is_symlink():raise ValueError('symlinked candidate source forbidden')
        candidate_path.write_text(v.source)
        record=asdict(v);record.pop('source');record.update(id=i,source_sha256=h,review_status='CANDIDATE_ONLY_NOT_PROVEN')
        if h in records:record=records[h]
        atomic_json(directory/'transform.json',record)
        check_output_file(directory/'change.diff')
        (directory/'change.diff').write_text(''.join(difflib.unified_diff(source.splitlines(True),v.source.splitlines(True),fromfile='original.cpp',tofile='candidate.cpp')))
        records[h]=record
    state['variants']=list(records.values())
    if not variants:state['status']='NO_APPLICABLE_NATURAL_TRANSFORM'
    atomic_json(live,state)
    if not settings['run'] or not variants:return state
    if 'deadline_unix' not in state:state['deadline_unix']=time.time()+max(0,deadline-time.monotonic())
    deadline=min(deadline,time.monotonic()+max(0,state['deadline_unix']-time.time()))
    atomic_json(live,state)
    required=('command_file','original_object','target_object','input_manifest')
    if not symbol or any(not entry.get(k)for k in required):raise ValueError('run requires symbol, command_file, original_object, target_object and input_manifest')
    command_path=Path(entry['command_file']);original=Path(entry['original_object']);target=Path(entry['target_object']);im_path=Path(entry['input_manifest'])
    command_bytes=command_path.read_bytes();command=command_bytes.decode();im=json.loads(im_path.read_text())
    bound=validate_inputs(root,source_bytes,command_bytes,original,target,im)
    objdiff_path=Path(settings['objdiff']or root/'build/tools/objdiff-cli').resolve()
    bound.update(objdiff_sha256=sha(objdiff_path.read_bytes()),elf_reader_sha256=sha(Path(__file__).with_name('datacontent.py').read_bytes()))
    if 'input_binding'in state and state['input_binding']!=bound:raise ValueError('resume original/target/command/dependency identity changed')
    state['input_binding']=bound
    exhausted=set()
    if settings.get('exhausted'):
        registry=json.loads(Path(settings['exhausted']).read_text());exhausted=set(registry.get(fingerprint['source_sha256'],[]))
    if not variants:
        state['status']='NO_APPLICABLE_NATURAL_TRANSFORM';atomic_json(live,state);return state
    extracted=extract_function(source,span)
    state['extraction']={'original_sha256':sha(source_bytes),'extracted_sha256':sha(extracted.encode()),'method':'verbatim includes/globals/free helpers; blank other supported member definitions','review_required':True}
    original_snap=object_snapshot(original)
    original_provider_bindings={name:sha(json.dumps(info,sort_keys=True).encode())for name,info in original_snap['functions'].items()if not info.get('local')}
    state['canonical_original_provider_bindings']=original_provider_bindings
    objdiff=Path(settings['objdiff']or root/'build/tools/objdiff-cli').resolve()
    if 'baseline'in state:
        baseline=state['baseline']
        if baseline.get('status')!='PASS':return state
        baseline_obj=Path(baseline['object'])
        if sha(baseline_obj.read_bytes())!=baseline['object_sha256']:raise ValueError('cached extracted baseline changed')
        extracted_snap=object_snapshot(baseline_obj)
        rebound=extraction_baseline_check(original_snap,extracted_snap,symbol,original_provider_bindings)
        if not rebound['passed']:raise ValueError('cached extracted baseline no longer passes binding gate')
        baseline['gate']=rebound
    else:
        state['status']='EXTRACTED_BASELINE_RUNNING';atomic_json(live,state)
        state['baseline']={'status':'IN_FLIGHT','reason':'interrupted baselines are not automatically repeated'};atomic_json(live,state)
        baseline_obj,comp=compile_scratch(extracted,unit,out/'baseline',root,command,remaining(),im)
        baseline={'compile':comp,'object':str(baseline_obj),'status':'BLOCKED'}
        if comp['dependency_closure']['passed']:
            extracted_snap=object_snapshot(baseline_obj)
            gate=extraction_baseline_check(original_snap,extracted_snap,symbol,original_provider_bindings);baseline['gate']=gate
            if gate['passed']:
                baseline.update(status='PASS',object_sha256=sha(baseline_obj.read_bytes()),scores=diff_scores(target,baseline_obj,objdiff,out/'baseline',remaining()))
        state['baseline']=baseline
        if baseline['status']!='PASS':state['status']='EXTRACTION_BASELINE_BLOCKED';atomic_json(live,state);return state
    baseline_percent=selected_percent(state['baseline']['scores'],symbol)
    if baseline_percent==100:
        state['status']='ALREADY_SCORE100_NO_GAIN_POSSIBLE';atomic_json(live,state);return state
    target_snap=object_snapshot(target);best=None;cached_exact=False
    # Reconstruct a surviving improvement after a crash/checkpoint, not just new runs.
    for i,v in enumerate(variants,1):
        record=records[sha(v.source.encode())]
        if record.get('status')in('SCORED_CANDIDATE_ONLY','IDENTICAL_RUNTIME_EXHAUSTED'):
            cached_obj=Path(record['compile']['object'])
            if sha(cached_obj.read_bytes())!=record.get('object_sha256'):raise ValueError('cached variant object changed')
            cached_snapshot=object_snapshot(cached_obj)
            rebound=preservation_check(extracted_snap,cached_snapshot,symbol)
            if not rebound['passed']:raise ValueError('cached variant no longer passes preservation gate')
            if record.get('strict_object_exact'):
                if cached_snapshot['functions'].get(symbol)!=target_snap['functions'].get(symbol):raise ValueError('cached exact variant no longer equals target')
                cached_exact=True
        if record.get('status')=='SCORED_CANDIDATE_ONLY' and record.get('after_percent',-1)>baseline_percent:
            if best is None or record['after_percent']>best[0]:best=(record['after_percent'],v,record,out/f'variant-{i:03d}')
    for i,v in enumerate(variants,1):
        if cached_exact:
            state['status']='EXTRACTED_EXACT_CANDIDATE';break
        h=sha(v.source.encode());record=records[h];directory=out/f'variant-{i:03d}'
        if record.get('status')=='IN_FLIGHT':
            record['status']='INTERRUPTED_ATTEMPT_NOT_RETRIED';atomic_json(live,state)
        if 'status'in record:continue
        if h in exhausted:record['status']='EXHAUSTED_FINGERPRINT_SKIPPED';atomic_json(live,state);continue
        left=deadline-time.monotonic()
        if left<=0:state['status']='FUNCTION_TIME_BUDGET';break
        validate_inputs(root,source_bytes,command_bytes,original,target,im)
        candidate_extracted=extract_function(v.source,find_function(v.source,selector))
        record['status']='IN_FLIGHT';atomic_json(live,state)
        obj,comp=compile_scratch(candidate_extracted,unit,directory/'extracted',root,command,remaining(),im)
        record['compile']=comp;record['status']='COMPILE_FAILED'
        if comp['dependency_closure']['passed']:
            snap=object_snapshot(obj);guard=preservation_check(extracted_snap,snap,symbol);record['extracted_preservation']=guard
            record['status']='EXTRACTED_GUARD_REJECTED'
            if guard['passed']:
                if snap['functions'][symbol]==extracted_snap['functions'][symbol]:
                    record.update(status='IDENTICAL_RUNTIME_EXHAUSTED',object_sha256=sha(obj.read_bytes()),before_percent=baseline_percent,after_percent=baseline_percent,score_delta=0,baseline_score_reused_after_exact_runtime_guard=True)
                    atomic_json(directory/'transform.json',record);atomic_json(live,state);continue
                scores=diff_scores(target,obj,objdiff,directory/'extracted',remaining());pct=selected_percent(scores,symbol)
                record.update(status='SCORED_CANDIDATE_ONLY',scores=scores,before_percent=baseline_percent,after_percent=pct,score_delta=pct-baseline_percent,object_sha256=sha(obj.read_bytes()))
                if pct>baseline_percent and (best is None or pct>best[0]):best=(pct,v,record,directory)
                if pct==100 and symbol in target_snap['functions'] and snap['functions'].get(symbol)==target_snap['functions'][symbol]:
                    record['strict_object_exact']=True;state['status']='EXTRACTED_EXACT_CANDIDATE';atomic_json(live,state);break
        atomic_json(directory/'transform.json',record);atomic_json(live,state)
    else:state['status']='VARIANT_BUDGET_EXHAUSTED'
    # Exactly one whole original-TU compile, only for the best extracted survivor.
    if best is not None and 'whole_unit'in state:
        whole=state['whole_unit']
        if whole.get('source_sha256')!=sha(best[1].source.encode()):raise ValueError('resume survivor changed; no implicit second whole-unit compile')
        if whole.get('object_sha256'):
            cached_obj=Path(whole['compile']['object'])
            if sha(cached_obj.read_bytes())!=whole['object_sha256']:raise ValueError('cached whole-unit object changed')
            if not preservation_check(original_snap,object_snapshot(cached_obj),symbol)['passed']:raise ValueError('cached whole-unit preservation mismatch')
    elif best is not None:
        pct,v,record,directory=best
        validate_inputs(root,source_bytes,command_bytes,original,target,im)
        state['whole_unit']={'status':'IN_FLIGHT','source_sha256':sha(v.source.encode()),'reason':'at most one whole-unit compile; interrupted work needs review'};atomic_json(live,state)
        obj,comp=compile_scratch(v.source,unit,out/'survivor-whole-unit',root,command,remaining(),im)
        whole={'compile':comp,'source_sha256':sha(v.source.encode()),'status':'BLOCKED','review_required':True}
        if comp['dependency_closure']['passed']:
            snap=object_snapshot(obj);gate=preservation_check(original_snap,snap,symbol);whole['preservation']=gate;whole['object_sha256']=sha(obj.read_bytes())
            if gate['passed']:
                scores=diff_scores(target,obj,objdiff,out/'survivor-whole-unit',remaining());whole['scores']=scores
                whole['status']='REVIEWABLE_CANDIDATE'if selected_percent(scores,symbol)>baseline_percent else'WHOLE_UNIT_GAIN_NOT_REPRODUCED'
        state['whole_unit']=whole
    validate_inputs(root,source_bytes,command_bytes,original,target,im)
    state['elapsed_seconds']=prior_elapsed+time.monotonic()-started
    atomic_json(live,state)
    return state


def main(argv=None):
    p=argparse.ArgumentParser(description=__doc__,formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('unit',nargs='?');p.add_argument('function',nargs='?');p.add_argument('--symbol')
    p.add_argument('--input-root',type=Path,default=TOOL_ROOT);p.add_argument('--out',required=True,type=Path)
    p.add_argument('--seed',type=int,default=0);p.add_argument('--budget',type=int,default=20)
    p.add_argument('--command-file',type=Path);p.add_argument('--original-object',type=Path);p.add_argument('--target',type=Path);p.add_argument('--input-manifest',type=Path)
    p.add_argument('--objdiff',type=Path);p.add_argument('--run',action='store_true');p.add_argument('--compile-cleared',action='store_true')
    p.add_argument('--queue',type=Path);p.add_argument('--resume',action='store_true');p.add_argument('--exhausted',type=Path)
    p.add_argument('--timeout',type=float,default=120);p.add_argument('--seconds',type=float,default=1800)
    args=p.parse_args(argv)
    if not 0<=args.budget<=300:p.error('budget must be0..300')
    if not 0<args.timeout<=1800 or not 0<args.seconds<=1800:p.error('timeouts/function budgets must be positive and <=1800sec')
    if args.run and not args.compile_cleared:p.error('--run requires --compile-cleared after root coordination')
    settings={k:getattr(args,k)for k in('seed','budget','run','resume','timeout','seconds','objdiff','exhausted')}
    if args.queue:
        queue=json.loads(args.queue.read_text())
        if queue.get('schema_version')!=1 or not isinstance(queue.get('entries'),list):p.error('queue must use documented schema_version1/entries')
        entries=queue['entries']
        for entry in entries:
            if entry.get('input_manifest'):
                captured=json.loads(Path(entry['input_manifest']).read_text())
                if captured.get('source_revision')!=queue.get('source_revision'):p.error('queue/input revision mismatch')
    else:
        if not args.unit or not args.function:p.error('unit and qualified function required')
        entries=[{'unit':args.unit,'function':args.function,'symbol':args.symbol,'input_root':str(args.input_root),'command_file':str(args.command_file)if args.command_file else None,'original_object':str(args.original_object)if args.original_object else None,'target_object':str(args.target)if args.target else None,'input_manifest':str(args.input_manifest)if args.input_manifest else None}]
    results=[]
    for i,entry in enumerate(entries):
        out=args.out if not args.queue else args.out/f'{i:03d}-{Path(entry["unit"]).name}'
        try:
            state=function_run(entry,settings,out)
            result={'unit':entry['unit'],'symbol':entry.get('symbol'),'out':str(out),'status':state['status'],'variants':len(state['variants'])}
        except (ValueError,OSError,KeyError)as exc:
            result={'unit':entry.get('unit'),'symbol':entry.get('symbol'),'status':'BLOCKED','reason':str(exc)}
        results.append(result)
        if args.queue:
            safe_output(args.out,Path(entry['input_root']));args.out.mkdir(parents=True,exist_ok=True)
            atomic_json(args.out/'queue-status.json',{'schema_version':1,'queue_sha256':sha(args.queue.read_bytes()),'tool_sha256':sha(Path(__file__).read_bytes()),'results':results,'review_required':True})
        print(json.dumps(result),flush=True)
    return 0 if all(r['status']!='BLOCKED'for r in results) else 2

if __name__=='__main__':
    try:sys.exit(main())
    except (ValueError,OSError)as exc:sys.exit(str(exc))
