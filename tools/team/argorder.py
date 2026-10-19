#!/usr/bin/env python3
"""Read-only advisory for calls with effects in at least two arguments.

No preprocessing, compilation or source-order assumption. Optional existing-object
inspection reports a unique, straight-line *out-of-line effect-call order* only;
inline/mutation/indirect/ambiguous occurrences remain unknown. Not a proof tool.
"""
import argparse
import collections
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import re
import struct
import sys

CONTROL = {'if', 'for', 'while', 'switch', 'catch', 'sizeof', 'alignof',
           '__alignof__', 'decltype', 'typeid', 'noexcept', 'static_assert',
           'static_cast', 'dynamic_cast', 'reinterpret_cast', 'const_cast',
           'else','do','try','namespace','class','struct','union','enum','const','volatile','override','final'}
ASSIGN = {'=', '+=', '-=', '*=', '/=', '%=', '&=', '|=', '^=', '<<=', '>>='}
OPS = sorted({'::', '->', '->*', '.*', '...', '++', '--', '==', '!=', '<=',
              '>=', '<=>', '<<', '>>', '&&', '||', '+=', '-=', '*=', '/=',
              '%=', '&=', '|=', '^=', '<<=', '>>=', '##'}, key=len, reverse=True)
SUFFIXES = {'.c', '.cc', '.cpp', '.cxx', '.h', '.hpp', '.inc', '.inl'}
LIMITATIONS = [
    'Lexical C/C++ advisory, not a preprocessor, type checker or sequencing proof.',
    'Inactive conditional branches are scanned; directive bodies are skipped. Macro-name union is conservative, not expansion.',
    'Calls are potential effects even when their implementation may be pure.',
    'Template/comparison ambiguity, casts, declarations and macro expansion can produce advisory candidates.',
    'Function-pointer invocation syntax is inventoried with unknown order; overloaded operator, implicit conversion/destructor effects are not inferred.',
    'Parenthesized/braced constructor syntax is inventoried heuristically; declaration ambiguities are marked. Spliced identifiers and unnamed/assembly function bodies are unsupported.',
    'Binary argument association requires unique direct callee spellings in the complete enclosing source body.',
    'Repeated/inline/indirect calls and mutation effects have unknown argument order.',
    'A resolved order covers explicit out-of-line effect calls in a straight-line interval, not full argument completion.',
    'Existing objects may be stale relative to source or headers; hashes/mtime are provenance, not a rebuild guarantee.',
]

@dataclass(frozen=True)
class Token:
    text: str
    start: int
    end: int
    line: int
    kind: str = 'punct'


def lex(text):
    """Keep character/line coordinates while ignoring comments/directives and opaque literals."""
    out, macros, skipped = [], set(), 0
    i, line, n, at_line_start = 0, 1, len(text), True
    while i < n:
        ch = text[i]
        if ch.isspace():
            if ch == '\n': line += 1; at_line_start = True
            i += 1
            continue
        if at_line_start and ch == '#':
            end = i
            while True:
                end = text.find('\n', end)
                if end < 0: end = n; break
                q = end - 1
                if q >= i and text[q] == '\r': q -= 1
                if q >= i and text[q] == '\\': end += 1; continue
                break
            m = re.match(r'#\s*define\s+([A-Za-z_]\w*)', text[i:end])
            if m: macros.add(m[1])
            line += text[i:end].count('\n'); i = end; skipped += 1
            continue
        at_line_start = False
        if text.startswith('//', i):
            e = text.find('\n', i + 2)
            while e > 0 and text[e-1] == '\\': e = text.find('\n', e + 1)
            e = n if e < 0 else e
            line += text[i:e].count('\n'); i = e; continue
        if text.startswith('/*', i):
            e = text.find('*/', i + 2); e = n if e < 0 else e + 2
            line += text[i:e].count('\n'); i = e; continue
        # Raw strings (including encoding prefixes), ordinary strings and chars.
        m = re.match(r'(?:u8|u|U|L)?R"([^ ()\\\t\r\n]{0,16})\(', text[i:])
        if m:
            e = text.find(')' + m[1] + '"', i + m.end())
            e = n if e < 0 else e + len(m[1]) + 2
            out.append(Token(text[i:e], i, e, line, 'literal'))
            line += text[i:e].count('\n'); i = e; continue
        m = re.match(r'(?:u8|u|U|L)?[\'\"]', text[i:])
        if m:
            quote, e = text[i + m.end() - 1], i + m.end()
            while e < n:
                if text[e] == '\\': e += 2
                elif text[e] == quote: e += 1; break
                else: e += 1
            e = min(e, n); out.append(Token(text[i:e], i, e, line, 'literal'))
            line += text[i:e].count('\n'); i = e; continue
        m = re.match(r'[A-Za-z_$][A-Za-z0-9_$]*', text[i:])
        if m:
            e = i + m.end(); out.append(Token(text[i:e], i, e, line, 'id')); i = e; continue
        m = re.match(r'(?:\d[\w.\']*|\.\d[\w.\']*)', text[i:])
        if m:
            e = i + m.end(); out.append(Token(text[i:e], i, e, line, 'number')); i = e; continue
        op = next((x for x in OPS if text.startswith(x, i)), ch)
        out.append(Token(op, i, i + len(op), line)); i += len(op)
    return out, macros, skipped


def pairs(tokens):
    """Exact (),[] and {} balance. Unbalanced constructs never get invented endpoints."""
    match, stack, errors = {}, [], []
    for i, t in enumerate(tokens):
        if t.text in ('(', '[', '{'): stack.append(i)
        elif t.text in (')', ']', '}'):
            if stack and tokens[stack[-1]].text == {')':'(', ']':'[', '}':'{'}[t.text]:
                a = stack.pop(); match[a] = i; match[i] = a
            else: errors.append({'line':t.line, 'token':t.text, 'reason':'unbalanced delimiter'})
    errors.extend({'line':tokens[i].line, 'token':tokens[i].text, 'reason':'unclosed delimiter'} for i in stack)
    return match, errors


def templates(ts, match):
    """Bound template-id angle spans; accept only credible post-close syntax.

    This is deliberately lexical. Whitespace-sensitive/relational ambiguities
    remain flagged in the row; they can never authorize a binary order result.
    """
    spans, ambiguous = {}, set()
    for i, t in enumerate(ts):
        if t.text != '<' or i == 0 or ts[i-1].kind != 'id': continue
        depth, j, bad = 1, i + 1, False
        while j < len(ts):
            s = ts[j].text
            if s in (';', '{', '}') and depth == 1: bad = True; break
            if s in ('(', '[') and j in match: j = match[j] + 1; continue
            if s == '<': depth += 1
            elif s in ('>', '>>'):
                depth -= 2 if s == '>>' else 1
                if depth <= 0: break
            j += 1
        if bad or j >= len(ts) or depth > 0: continue
        nxt = ts[j+1] if j+1 < len(ts) else None
        credible = nxt and (nxt.text in ('(', '::', '{', '>', '>>', '*', '&') or
                             (nxt.kind == 'id' and ts[i-1].text[:1].isupper()))
        if credible:
            spans[i] = j
            if ts[i-1].end != t.start and not ts[i-1].text[:1].isupper(): ambiguous.add(i)
    return spans, ambiguous


def callee_before(ts, opening, angles):
    j = opening - 1
    if j < 0: return None
    if ts[j].text in (')', ']'):
        close=ts[j].text; opening={')':'(',']':'['}[close]; depth=1;k=j-1
        while k>=0:
            if ts[k].text==close:depth+=1
            elif ts[k].text==opening:
                depth-=1
                if depth==0:break
            k-=1
        if k<0:return None
        if close==']' and k and ts[k-1].kind=='id':k-=1
        return {'name':'<indirect>','start_token':k,'name_token':j,
                'spelling':''.join(t.text for t in ts[k:j+1]),'indirect':True}
    if ts[j].text in ('>', '>>'):
        starts = [a for a, z in angles.items() if z == j]
        if not starts: return None
        j = min(starts) - 1
    if j < 0 or ts[j].kind != 'id' or ts[j].text in CONTROL: return None
    name, start = ts[j].text, j
    # Qualified names are retained without inventing the type of a receiver.
    while start >= 2 and ts[start-1].text == '::' and ts[start-2].kind == 'id': start -= 2
    return {'name':name, 'start_token':start, 'name_token':j,
            'spelling':''.join(t.text for t in ts[start:j+1]),
            'receiver':simple_receiver(ts,start)}


def simple_receiver(ts,name_start):
    if name_start<2 or ts[name_start-1].text not in('.', '->'):return None
    j=name_start-2
    if ts[j].kind!='id':return None
    a=j
    while a>=2 and ts[a-1].text in('.', '->')and ts[a-2].kind=='id':a-=2
    return ''.join(t.text for t in ts[a:name_start-1])


def split_args(ts, opening, closing, match, angles):
    parts, start, i = [], opening + 1, opening + 1
    while i < closing:
        if ts[i].text in ('(', '[', '{') and i in match:
            i = match[i] + 1; continue
        if i in angles and angles[i] < closing:
            i = angles[i] + 1; continue
        if ts[i].text == ',': parts.append((start, i)); start = i + 1
        i += 1
    if start < closing: parts.append((start, closing))
    return parts


def function_spans(ts, match, angles):
    """Named function definitions, including qualified constructors/init lists."""
    spans = []
    for op, cl in sorted(match.items()):
        if ts[op].text != '(': continue
        c = callee_before(ts, op, angles)
        if not c or c.get('indirect'): continue
        # A receiver/dotted expression cannot introduce a function definition.
        b = c['start_token']
        if b and ts[b-1].text in ('.', '->', '=', 'return', ',', '('): continue
        j = cl + 1
        while j < len(ts) and ts[j].text in ('const', 'volatile', 'override', 'final', '&', '&&'): j += 1
        if j < len(ts) and ts[j].text == 'noexcept':
            j += 1
            if j < len(ts) and ts[j].text == '(' and j in match: j = match[j] + 1
        # Constructor member initializers precede the actual body.
        if j < len(ts) and ts[j].text == ':':
            j += 1
            while j < len(ts):
                if ts[j].text in ('(', '{') and j in match:
                    end = match[j]
                    # A braced member initializer follows an identifier or a template-id.
                    if ts[j].text == '{' and (j == 0 or ts[j-1].kind != 'id'):
                        break
                    j = end + 1
                    if j < len(ts) and ts[j].text == ',': j += 1; continue
                    if j < len(ts) and ts[j].text == '{': break
                elif ts[j].text == ';': break
                else: j += 1
        if j < len(ts) and ts[j].text == '{' and j in match:
            spans.append({'name':c['name'], 'spelling':c['spelling'], 'signature_open':op,
                          'signature_close':cl, 'start':c['start_token'], 'body':j, 'end':match[j]})
    # Reject apparent initializer/lambda-local signatures inside another signature/init list.
    return [s for s in spans if not any(o['start'] < s['start'] < o['body'] and s['end'] == o['end'] for o in spans if o is not s)]


def scan_text(text, filename='<memory>', macro_names=()):
    ts, local_macros, directives = lex(text); match, errors = pairs(ts)
    angles, ambiguous_angles = templates(ts, match); funcs = function_spans(ts, match, angles)
    macros = set(macro_names) | local_macros; calls = []
    sigs = {s['signature_open'] for s in funcs}
    body_opens={s['body']for s in funcs}
    for op, cl in sorted(match.items()):
        if ts[op].text not in ('(', '{') or op in sigs or op in body_opens: continue
        c = callee_before(ts, op, angles)
        if not c: continue
        if ts[op].text=='{'and(c.get('indirect')or(c['start_token']and ts[c['start_token']-1].text in('class','struct','union','enum','namespace'))):continue
        enclosing = [s for s in funcs if s['signature_close'] < op < s['end']]
        fn = min(enclosing, key=lambda s:s['end']-s['start']) if enclosing else None
        if not fn: continue  # File/class declarations are not invocation sites.
        ctor = ts[op].text == '{'
        prior = ts[c['start_token']-1] if c['start_token'] else None
        if not c.get('indirect') and prior and (prior.kind == 'id' and prior.text not in ('return', 'co_return', 'throw', 'new') or prior.text in ('>', '>>')):
            # T variable(args) is a declaration-style constructor (possibly vexing parse).
            ctor = True
            c['constructor_type_hint'] = prior.text
            if prior.text in ('>', '>>'):
                starts = [a for a,z in angles.items() if z == c['start_token']-1]
                if starts: c['constructor_type_hint'] = ts[min(starts)-1].text
        if prior and prior.text=='new':ctor=True;c['constructor_type_hint']=c['name']
        calls.append({**c, 'opening':op, 'closing':cl, 'constructor':ctor, 'function':fn,
                      'args_tokens':split_args(ts,op,cl,match,angles)})
    rows = []
    for call in calls:
        parts=[]
        for a,z in call['args_tokens']:
            nested=[c for c in calls if a <= c['start_token'] and c['closing'] < z]
            mutation=[t for t in ts[a:z] if t.text in ASSIGN or t.text in ('++','--')]
            part={'index':len(parts),'text':text[ts[a].start:ts[z-1].end] if a<z else'',
                  'effect_count':len(nested)+len(mutation),'call_count':len(nested),
                  'mutation_count':len(mutation),'call_names':[c['name']for c in nested],
                  'calls':[{'name':c['name'],'spelling':c['spelling'],'line':ts[c['start_token']].line,
                            'constructor':c['constructor'],'receiver':c.get('receiver'),'indirect':c.get('indirect',False),'macro':c['name']in macros or c['name'].isupper()} for c in nested],
                  'mutation_ops':[t.text for t in mutation]}
            parts.append(part)
        if sum(p['effect_count']>0 for p in parts)<2:continue
        f=call['function'];fncalls=[c for c in calls if c['function'] is f]
        uncertainty=[]
        if call['name']in macros or call['name'].isupper():uncertainty.append('macro invocation or macro-like identifier')
        if any(call['opening']<i<call['closing']for i in ambiguous_angles):uncertainty.append('template/comparison ambiguity')
        if errors:uncertainty.append('unbalanced source delimiters')
        if call.get('indirect'):uncertainty.append('indirect callee expression')
        if any(t.text in ('sizeof','decltype','typeid','noexcept') for a,z in call['args_tokens']for t in ts[a:z]):uncertainty.append('unevaluated/context-dependent expression')
        if any(t.text==']' for a,z in call['args_tokens']for t in ts[a:z]) and any(t.text=='{' for a,z in call['args_tokens']for t in ts[a:z]):uncertainty.append('possible deferred lambda body in argument')
        if call['constructor']:uncertainty.append('constructor/declaration syntax requires type confirmation')
        row={'file':filename,'line':ts[call['start_token']].line,'column':ts[call['start_token']].start-text.rfind('\n',0,ts[call['start_token']].start),
             'offset':ts[call['start_token']].start,'enclosing_function':f['spelling'],
             'enclosing_short_name':f['name'],'callee':call['spelling'],'callee_short_name':call['name'],
             'constructor':call['constructor'],'constructor_type_hint':call.get('constructor_type_hint'),
             'arguments':parts,'effectful_arguments':sum(p['effect_count']>0 for p in parts),
             'effect_count':sum(p['effect_count']for p in parts),
             'function_call_name_counts':dict(collections.Counter(c['name']for c in fncalls)),
             'uncertainty':uncertainty,'argument_order':{'status':'unknown','reason':'binary inspection not requested'}}
        from argorder_effects import typed_bindings
        row['typed_receivers']=typed_bindings(text[ts[f['start']].start:ts[f['end']].end])
        owners=f['spelling'].split('::')[:-1]
        if owners and owners[-1]in('JPABaseEmitter','JPARandom','JPAEmitterWorkData','JMath::TRandom_fast_'):row['typed_receivers']['this']=owners[-1]
        rows.append(row)
    return {'rows':rows,'diagnostics':errors,'macro_names':sorted(local_macros),'directives_skipped':directives,'named_functions':len(funcs),'invocations_examined':len(calls)}


def short_symbol(name):
    if name.startswith('__'): return None  # Constructors/operators need type/ABI resolution.
    return name.split('__',1)[0]


def symbol_owners(name):
    """Decode only length-prefixed MW scope names; refuse unsupported encodings."""
    if '__' not in name:return []
    tail=name.split('__',1)[1];i=0;count=1
    if tail.startswith('Q'):
        if len(tail)<2 or not tail[1].isdigit():return []
        count=int(tail[1]);i=2
    owners=[]
    for _ in range(count):
        m=re.match(r'\d+',tail[i:])
        if not m:return []
        z=int(m[0]);i+=len(m[0])
        if not z or i+z>len(tail):return []
        owners.append(tail[i:i+z]);i+=z
    return owners


def branch_in_interval(words,start,end):
    for off in range(start,end,4):
        w=words[off//4];op=w>>26
        if op==16 or op==18 and not(w&1) or op==19 and ((w>>1)&1023)in(16,528): return True
    return False


def resolve_order(row, source_calls, target_calls, source_words, target_words):
    """Unique spelled calls only; never assign repeated/inline/indirect events to args."""
    unknown=lambda why:{'status':'unknown','reason':why}
    if row['uncertainty']:return unknown('; '.join(row['uncertainty']))
    if any(a['mutation_count']for a in row['arguments']):return unknown('mutation/assignment effect has no unique direct-call event')
    if any(c['constructor']or c['macro']or c.get('indirect')for a in row['arguments']for c in a['calls']):return unknown('nested constructor/macro/indirect call is unresolved')
    effects=[(a['index'],c['name'])for a in row['arguments']for c in a['calls']]
    names=[n for _,n in effects]+[row['callee_short_name']]
    if len(set(names))!=len(names):return unknown('repeated callee: cannot bind distinct source occurrences to argument indices')
    if any(row['function_call_name_counts'].get(n)!=1 for n in names):return unknown('callee occurs elsewhere in enclosing function; no line/occurrence binding')
    chosen=[]
    for side,calls,words in [('source',source_calls,source_words),('target',target_calls,target_words)]:
        picks={}
        for n in names:
            hits=[c for c in calls if c.get('addend',0)==0 and short_symbol(c.get('callee',''))==n]
            if len(hits)!=1:return unknown(f'{side}: {n} inline/indirect/overloaded/missing or repeated direct callee')
            picks[n]=hits[0]
        outer=picks[row['callee_short_name']]['offset'];events=[]
        for arg,n in effects:
            c=picks[n]
            if c['offset']>=outer:return unknown(f'{side}: effect event does not precede the unique outer call')
            events.append({'argument':arg,'callee':c['callee'],'offset':c['offset'],
                           'callee_address':c.get('actual_DOL_destination',c.get('mapped_destination_hint')),
                           'address_basis':'actual original-DOL direct destination' if side=='target' else 'source ELF relocation identity mapped to original symbol address'})
        start=min(e['offset']for e in events)
        if branch_in_interval(words,start,outer):return unknown(f'{side}: conditional/indirect/tail control flow in effect-to-outer interval')
        if any(not e['callee_address']for e in events):return unknown(f'{side}: callee address unresolved')
        chosen.append({'side':side,'events':sorted(events,key=lambda e:e['offset']),
                       'outer_call':picks[row['callee_short_name']]})
    return {'status':'resolved-direct-effect-call-order','scope':'Explicit unique out-of-line effect calls only; advisory source association, not full argument completion or equivalence proof.',
            'source':chosen[0],'target':chosen[1],
            'argument_call_order_equal':[e['argument']for e in chosen[0]['events']]==[e['argument']for e in chosen[1]['events']]}


class BinaryAudit:
    def __init__(self,root):
        self.root=root;sys.path.append(str(root/'tools/team'))
        import callaudit
        self.meta_paths=[root/'config/SB4E01/symbols.txt',root/'orig/SB4E01/sys/main.dol'];self.meta_hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest()for p in self.meta_paths}
        self.ca=callaudit;self.cat=callaudit.catalog(root/'config/SB4E01/symbols.txt');self.dol=(root/'orig/SB4E01/sys/main.dol').read_bytes();self.cache={}; self.addresses={}; self.duplicate_addresses=set()
        for line in (root/'config/SB4E01/symbols.txt').read_text().splitlines():
            m=re.match(r'(.+?) = \.\w+:0x([0-9A-Fa-f]+);',line)
            if m:
                n,a=m[1],int(m[2],16)
                if n in self.addresses and self.addresses[n]!=a:self.duplicate_addresses.add(n)
                self.addresses[n]=a
    def stable_metadata(self):
        return all(hashlib.sha256(p.read_bytes()).hexdigest()==self.meta_hashes[str(p)]for p in self.meta_paths)
    def unit(self,unit):
        if unit not in self.cache:
            self.cache.clear()  # File traversal processes a unit together; bound ELF memory.
            paths=[self.root/f'build/SB4E01/{kind}/{unit}.o'for kind in ['src','obj']]
            if not all(p.is_file()for p in paths):raise ValueError('missing built source/target object')
            es=[self.ca.ELF(p)for p in paths]
            self.cache[unit]=(es,{'source_object':str(paths[0]),'target_object':str(paths[1]),
                                'source_object_sha256':hashlib.sha256(es[0].raw).hexdigest(),
                                'target_object_sha256':hashlib.sha256(es[1].raw).hexdigest(),
                                'source_object_mtime_ns':paths[0].stat().st_mtime_ns}, {})
        return self.cache[unit]
    def inspect(self,row):
        try:
            (s,t),pins,fcache=self.unit(row['unit']);row['objects']=pins
            stem=row['enclosing_short_name']; scope=row['enclosing_function'].split('::')[:-1]
            fs=[n for n in s.f if short_symbol(n)==stem and n in t.f]
            if scope:fs=[n for n in fs if scope[-1] in symbol_owners(n)]
            if len(fs)!=1:raise ValueError('enclosing compiled function missing/overloaded/ambiguous; no DWARF line binding')
            n=fs[0]
            if n in self.cat.duplicates:raise ValueError('target caller symbol has ambiguous registered addresses')
            if n not in fcache:
                comp=self.ca.compare(t,s,n,self.cat,self.dol)
                _,ta,tz,te=t.f[n];addr=self.cat[n]['address'];raw=self.ca.read_dol(self.dol,addr,tz)
                if raw is None:raise ValueError('caller is not completely original-DOL backed')
                for off in range(0,tz,4):
                    w=struct.unpack_from('>I',t.c[te],ta+off)[0];v=struct.unpack_from('>I',raw,off)[0];r=t.rel.get((te,ta+off))
                    mask=0xfc000003 if r and r[0]==10 else 0xffe00000 if r and r[0]==109 else 0xffff0000 if r else 0xffffffff
                    if w&mask!=v&mask:raise ValueError('target object body does not bind to original DOL')
                if any(comp['unresolved'].values()):raise ValueError('direct callee identity or actual DOL address unresolved')
                # Every actual destination is checked, including canonical ABI helper labels.
                for call in comp['target_calls']:
                    name=call.get('callee'); mapped=self.addresses.get(name)
                    if mapped is None or name in self.duplicate_addresses:
                        raise ValueError('target callee symbol address missing/ambiguous')
                    actual=int(call['actual_DOL_destination'],16)
                    if mapped+call.get('addend',0)!=actual:raise ValueError('target relocation disagrees with actual direct callee')
                    call['relocation_matches_actual_destination']=True
                for call in comp['source_calls']:
                    name=call.get('callee')
                    if name in self.addresses and name not in self.duplicate_addresses:
                        call['mapped_destination_hint']=hex(self.addresses[name]+call.get('addend',0))
                sw=struct.unpack('>'+str(s.f[n][2]//4)+'I',s.c[s.f[n][3]][s.f[n][1]:s.f[n][1]+s.f[n][2]])
                tw=struct.unpack('>'+str(tz//4)+'I',raw)
                fcache[n]=(comp,sw,tw)
            comp,sw,tw=fcache[n]
            row['binary_function']=n;row['binary_calls']={'source':comp['source_calls'],'target':comp['target_calls'],'indirect':comp['indirect_calls_outside_scope']}
            row['target_caller_original_DOL_masked_words_bound']=True
            row['argument_order']=resolve_order(row,comp['source_calls'],comp['target_calls'],sw,tw)
            if row.get('source_mtime_ns',0)>pins['source_object_mtime_ns']:
                raise ValueError('source file newer than existing source object; current-source association refused')
            for side in ['source','target']:
                if hashlib.sha256(Path(pins[side+'_object']).read_bytes()).hexdigest()!=pins[side+'_object_sha256']:
                    raise ValueError('built object changed while being inspected')
        except (ValueError,KeyError,OSError,struct.error)as e:row['argument_order']={'status':'unknown','reason':str(e)}


def macro_inventory(root,paths):
    names=set();seen=set()
    # Union inventory intentionally overapproximates inactive/undefined macros.
    # Header lookup/preprocessor conditions are never guessed from the call spelling.
    candidates=set(paths)
    for folder in ['include','libs','src']:
        base=root/folder
        if base.is_dir():candidates.update(p for p in base.rglob('*')if p.is_file()and p.suffix in SUFFIXES|{'.inc'})
    for p in sorted(candidates):
        try:
            text=p.read_text(errors='replace');seen.add(str(p))
            names.update(re.findall(r'^\s*#\s*define\s+([A-Za-z_]\w*)',text,re.M))
        except OSError:pass
    return names,len(seen)


def main(argv=None):
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--root',type=Path,default=Path.cwd());ap.add_argument('--prefix',default='src/')
    ap.add_argument('--paths',nargs='+',help='specific source paths, relative to root')
    ap.add_argument('--source-only',action='store_true');ap.add_argument('--markdown',type=Path,help='optional readable inventory');ap.add_argument('--out',type=Path,required=True)
    a=ap.parse_args(argv);root=a.root.resolve()
    paths=[root/p for p in a.paths]if a.paths else sorted(p for p in(root/a.prefix).rglob('*')if p.is_file()and p.suffix in SUFFIXES)
    macros,macro_files=macro_inventory(root,paths)
    results=[];errors=[];files=[];audit=None
    if not a.source_only:
        try:audit=BinaryAudit(root)
        except (OSError,ValueError,ImportError)as e:errors.append({'stage':'binary setup','reason':str(e)})
    from argorder_effects import EffectAudit, rank
    effects=EffectAudit(root,audit)
    for path in paths:
        try:
            raw=path.read_bytes();text=raw.decode(errors='replace');rel=path.relative_to(root).as_posix();scan=scan_text(text,rel,macros)
            files.append({'file':rel,'named_functions':scan['named_functions'],'invocations_examined':scan['invocations_examined'],'candidates':len(scan['rows']),'directives_skipped':scan['directives_skipped']})
            unit=str(Path(rel).with_suffix('')).removeprefix('src/')
            for row in scan['rows']:
                row['unit']=unit;row['source_sha256']=hashlib.sha256(raw).hexdigest();row['source_mtime_ns']=path.stat().st_mtime_ns
                if audit:audit.inspect(row)
                row['priority']=rank(row,lambda call:effects.summary(row,call))
                results.append(row)
            if path.read_bytes()!=raw:
                for row in scan['rows']:row['argument_order']={'status':'unknown','reason':'source changed during scan'}
                errors.append({'file':rel,'reason':'source changed during scan'})
            if scan['diagnostics']:errors.append({'file':rel,'lexical':scan['diagnostics']})
        except (OSError,ValueError)as e:errors.append({'file':str(path),'reason':str(e)})
    if audit and not audit.stable_metadata():
        errors.append({'stage':'metadata','reason':'symbols/DOL changed during scan; resolved associations invalidated'})
        for row in results:
            row['argument_order']={'status':'unknown','reason':'metadata changed during scan'}
            row['priority']={'level':'UNKNOWN','reason':'metadata changed during scan','arguments':[],'known_effect_argument_count':0}
    if not effects.stable_contracts():
        errors.append({'stage':'effect contracts','reason':'owning RNG source/header changed during scan'})
        for row in results:row['priority']={'level':'UNKNOWN','reason':'effect contract changed during scan','arguments':[],'known_effect_argument_count':0}
    counts=collections.Counter(r['argument_order']['status']for r in results)
    report={'schema':'argorder-advisory-v1','root':str(root),'source_files_scanned':len(paths),'candidate_count':len(results),
            'priority_counts':dict(collections.Counter(r['priority']['level']for r in results)), 'effect_contract_input_sha256':effects.inputs,'order_status_counts':dict(counts),'macro_inventory_files':macro_files,'macro_names_in_conservative_union':len(macros),'original_DOL_sha256':hashlib.sha256(audit.dol).hexdigest()if audit else None,
            'limitations':LIMITATIONS,'files':files,'errors':errors,'rows':results}
    a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(report,indent=2)+'\n')
    if a.markdown:
        lines=['# Argument-order advisory','',f'{len(paths)} files; {len(results)} candidates. Explicit out-of-line call order only; unknown is not source order.','']
        lines += ['| Priority | Location | Enclosing function | Known-effect slots |','|---|---|---|---|']
        ranks={'HIGH':0,'MEDIUM':1,'UNKNOWN':2,'LOW':3}
        for row in sorted(results,key=lambda r:(ranks[r['priority']['level']],r['file'],r['line'])):
            function=row['enclosing_function'].replace('|',r'\|')
            lines += [f"| {row['priority']['level']} | `{row['file']}:{row['line']}` | `{function}` | {row['priority']['known_effect_argument_count']} |"]
        lines += ['']
        for row in results:
            lines += [f"- `{row['file']}:{row['line']}` — `{row['enclosing_function']}` / `{row['unit']}` → `{row['callee']}` — **{row['priority']['level']}**",'']
            for arg in row['arguments']:
                text=' '.join(arg['text'].split()).replace('`','\\`')
                lines += [f"  - arg{arg['index']}: `{text}` — {arg['call_count']} calls, {arg['mutation_count']} mutations"]
            lines += ['', '  Priority: '+row['priority']['reason']]
            order=row['argument_order'];lines += ['', '  '+order['status']+': '+order.get('reason',order.get('scope','')),'']
            if order['status']=='resolved-direct-effect-call-order':
                for side in ['source','target']:
                    lines += ['  '+side+': '+' → '.join(f"arg{e['argument']}:{e['callee']}@{e['callee_address']}"for e in order[side]['events']),'']
        a.markdown.parent.mkdir(parents=True,exist_ok=True);a.markdown.write_text('\n'.join(lines)+'\n')
    print(json.dumps({k:report[k]for k in ['source_files_scanned','candidate_count','order_status_counts']})+'; advisory only; '+str(a.out))
    return 0

if __name__=='__main__':raise SystemExit(main())
