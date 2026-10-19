#!/usr/bin/env python3
"""Inventory native ABI and byte-order candidates without changing game sources.

This is a conservative lexical sweep, not a C++ type checker. JSON retains the
operand, evidence, confidence, and reference provenance for every finding.
"""

import argparse
from bisect import bisect_right
from collections import Counter, defaultdict
from dataclasses import dataclass
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import subprocess

from classify_audit import LANES

ROOT = Path(__file__).resolve().parents[2]
EXTENSIONS = {'.h', '.hpp', '.hh', '.c', '.cc', '.cpp', '.cxx', '.inc'}
INT32 = r'(?:u32|s32|int|unsigned\s+int|UIntPtr|SIntPtr|uint32_t|int32_t)'
SCALARS = set('u8 s8 u16 s16 u32 s32 u64 s64 f32 f64 int char short long float double bool unsigned signed size_t uintptr_t intptr_t UIntPtr SIntPtr uint32_t int32_t'.split())
COMMENT_STRING = re.compile(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')
DECL = re.compile(
    r'\b(?P<type>(?:(?:const|volatile|unsigned|signed|long|short)\s+)*'
    r'[A-Za-z_]\w*(?:::\w+)*(?:\s*<[^;{}()\n]+>)?(?:\s+const)?)'
    r'(?P<ptr>\s*[*&]+\s*(?:const\s*)?|\s+)'
    r'(?P<name>[A-Za-z_]\w*)\s*(?P<tail>\[|[;=,)({:])'
)
KEYWORDS = set('return delete new case else goto using namespace class struct enum union typedef operator throw sizeof static_assert'.split())
CAST32 = re.compile(r'(?P<named>\b(?:reinterpret_cast|static_cast)\s*<\s*' + INT32 + r'\s*>\s*\()'
                    r'|(?P<c>\(\s*' + INT32 + r'\s*\))|(?P<functional>\b' + INT32 + r'\s*\()')
POINTER_CAST = re.compile(r'(?P<named>\b(?:reinterpret_cast|static_cast)\s*<\s*(?:const\s+)?[\w:]+\s*\*\s*>\s*\()'
                          r'|(?P<c>\(\s*(?:const\s+)?[\w:]+\s*\*\s*\))')
RECORD = re.compile(r'\b(?P<kind>struct|class|union)\s*(?P<name>[A-Za-z_]\w*)?\s*(?:\:\s*[^;{}]+)?\{')


def mask(source):
    return COMMENT_STRING.sub(lambda m: ''.join('\n' if c == '\n' else ' ' for c in m.group()), source)


def brackets(code):
    stack, pairs = [], {}
    for pos, char in enumerate(code):
        if char in '({[':
            stack.append((char, pos))
        elif char in ')}]' and stack and stack[-1][0] == {')': '(', '}': '{', ']': '['}[char]:
            _, start = stack.pop()
            pairs[start] = pos
    return pairs


def unwrap(expression):
    expression = expression.strip()
    while expression.startswith('('):
        end = brackets(expression).get(0)
        if end != len(expression) - 1:
            break
        expression = expression[1:-1].strip()
    return expression


def operand(code, start, paired=False, unary=False):
    if paired:
        end = brackets(code[start - 1:]).get(0)
        if end is not None:
            return code[start:start - 1 + end].strip()
    depth, end, atom = [], start, False
    for end in range(start, min(len(code), start + 600)):
        char = code[end]
        if unary and not depth and atom and char in '+-*/%&|^=<>!?:~':
            if code[end:end + 2] != '->' and not (char == '>' and end and code[end - 1] == '-'):
                break
        if char in '([':
            depth.append(char)
        elif char in ')]':
            if not depth:
                break
            depth.pop()
        elif not depth and char in ',;\n{}':
            break
        if char.isalnum() or char in '_)]':
            atom = True
    else:
        end += 1
    return code[start:end].strip()


def native_excluded_lines(source):
    """Recognize native-only/legacy-only branches; other conditions stay unknown."""
    excluded, stack = set(), []
    for number, line in enumerate(source.splitlines(), 1):
        directive = re.match(r'\s*#\s*(if|ifdef|ifndef|else|elif|endif)\b(.*)', line)
        if directive:
            kind, condition = directive.groups()
            if kind in {'if', 'ifdef', 'ifndef'}:
                known = None
                if re.fullmatch(r'\s*GALAXY_NATIVE\s*', condition) and kind in {'ifdef', 'ifndef'}:
                    known = kind == 'ifdef'
                elif kind == 'if' and re.fullmatch(r'\s*!?\s*defined\s*\(?\s*GALAXY_NATIVE\s*\)?\s*', condition):
                    known = '!' not in condition
                stack.append(known)
            elif kind == 'else' and stack:
                stack[-1] = None if stack[-1] is None else not stack[-1]
            elif kind == 'elif' and stack:
                stack[-1] = None
            elif kind == 'endif' and stack:
                stack.pop()
        if False in stack:
            excluded.add(number)
    return excluded


@dataclass(frozen=True)
class Decl:
    name: str
    base: str
    pointers: int
    array: bool
    function: bool
    pos: int

    @property
    def kind(self):
        if self.pointers or self.array:
            return 'pointer'
        if self.base.split()[-1] in SCALARS or self.base.startswith('Res'):
            return 'scalar'
        return 'value'


class Source:
    def __init__(self, path, text):
        self.path, self.text = path, text
        self.excluded = native_excluded_lines(text)
        self.code = ''.join(''.join('\n' if c == '\n' else ' ' for c in line) if number in self.excluded else line
                            for number, line in enumerate(mask(text).splitlines(keepends=True), 1))
        self.line_starts = [0] + [m.end() for m in re.finditer('\n', text)]
        self.lines = text.splitlines()
        self.pairs = brackets(self.code)
        self.decls = defaultdict(list)
        typedefs = set(re.findall(r'\btypedef\b[^;{}]+\b(\w+)\s*;', self.code))
        for match in DECL.finditer(self.code):
            base = re.sub(r'\b(?:const|volatile)\b', '', match['type']).strip()
            if not base or base.split()[0] in KEYWORDS:
                continue
            token = base.split()[-1]
            if token not in SCALARS | typedefs | {'void'} and not (token[0].isupper() or '::' in token or token.endswith('_t')):
                continue
            decl = Decl(match['name'], base, match['ptr'].count('*'), match['tail'] == '[', match['tail'] == '(', match.start())
            self.decls[decl.name].append(decl)
        self.records = []
        for match in RECORD.finditer(self.code):
            opening = match.end() - 1
            if opening in self.pairs:
                self.records.append((match, opening, self.pairs[opening]))

    def line(self, pos):
        return bisect_right(self.line_starts, pos)

    def snippet(self, line):
        return self.lines[line - 1].strip() if 0 < line <= len(self.lines) else ''


def directory_for(path):
    parts = Path(path).parts
    if parts[0] in {'src', 'include'} and len(parts) >= 3 and parts[1] == 'Game':
        name = Path(parts[2]).stem if len(parts) == 3 else parts[2]
        return 'Game/' + (name if name in LANES else '<root>' if len(parts) == 3 else name)
    if parts[0] == 'libs':
        library = parts[1]
        tail = parts[3:]
        if tail and tail[0] == library:
            tail = tail[1:]
        return library + ('/' + (tail[0] if len(tail) > 1 else '<root>') if tail and library in {'JSystem', 'nw4r'} else '')
    if parts[0] == 'include':
        return 'include/<root>'
    return '/'.join(parts[1:3]) if len(parts) > 3 and parts[1] in {'JSystem', 'nw4r'} else parts[1]


def lane_for(directory):
    if directory.startswith('Game/'):
        return LANES.get(directory.split('/')[1], 'unassigned')
    return 'unassigned' if directory.startswith('include/') else 'm1'


class Resolver:
    def __init__(self, sources):
        self.sources = sources
        self.includes = {}
        self.header_names = {}
        for path in sources:
            if '/include/' in path:
                self.header_names[path.split('/include/', 1)[1]] = path
            elif path.startswith('include/'):
                self.header_names[path[8:]] = path
        self.stats = Counter()

    def headers(self, source):
        if source.path not in self.includes:
            found = set()
            def visit(item, depth):
                if depth == 0:
                    return
                for name in re.findall(r'^\s*#\s*include\s*[<"]([^>"]+)', item.text, re.MULTILINE):
                    path = self.header_names.get(name)
                    if path and path not in found:
                        found.add(path)
                        visit(self.sources[path], depth - 1)
            visit(source, 2)
            self.includes[source.path] = sorted(found)
        return self.includes[source.path]

    def lookup(self, source, name, pos, function=False):
        local = [d for d in source.decls.get(name, []) if d.function == function]
        prior = [d for d in local if d.pos <= pos]
        if prior:
            signatures = {(d.kind, d.pointers, d.array) for d in local}
            return prior[-1] if len(signatures) == 1 else None
        choices = local + [d for path in self.headers(source) for d in self.sources[path].decls.get(name, []) if d.function == function]
        signatures = {(d.base, d.pointers, d.array) for d in choices}
        return choices[0] if len(signatures) == 1 else None

    def infer(self, source, expression, pos, depth=0):
        if depth > 8:
            return 'unknown', None
        expression = unwrap(expression)
        if not expression:
            return 'unknown', None
        if re.match(r'^(?:sizeof|alignof)\b', expression) or re.fullmatch(r'(?:[-+]?\d[\w.]*|true|false|nullptr|NULL)', expression):
            return 'scalar', None
        if re.search(r'(?<![-=])(?:==|!=|<=|>=|&&|\|\|)', expression):
            return 'scalar', None
        if expression.startswith('&') and not expression.startswith('&&'):
            return 'pointer', self.infer(source, expression[1:], pos, depth + 1)[1]
        if expression == 'this':
            return 'pointer', None
        dereference = re.match(r'^(\*+)\s*(.*)', expression)
        if dereference:
            kind, decl = self.infer(source, dereference[2], pos, depth + 1)
            if kind == 'pointer' and decl:
                remaining = decl.pointers + int(decl.array) - len(dereference[1])
                return ('pointer' if remaining > 0 else 'scalar' if decl.base in SCALARS else 'value'), decl
            return 'unknown', None
        if expression.startswith(('~', '!')):
            return 'scalar', None
        level = 0
        for index, char in enumerate(expression):
            if char in '([':
                level += 1
            elif char in ')]':
                level -= 1
            elif level == 0 and char in '+-' and index and expression[index:index + 2] != '->' and expression[index - 1] not in '+-eE':
                left = self.infer(source, expression[:index], pos, depth + 1)[0]
                right = self.infer(source, expression[index + 1:], pos, depth + 1)[0]
                if left == right == 'pointer' and char == '-':
                    return 'scalar', None
                if 'pointer' in {left, right} and 'scalar' in {left, right}:
                    return 'pointer', None
                if left == right == 'scalar':
                    return 'scalar', None
                return 'unknown', None
            elif level == 0 and char in '*/%&|^' and index:
                return 'unknown', None
        pairs = brackets(expression)
        opening = next((o for o, c in pairs.items() if c == len(expression) - 1 and expression[o] == '('), None)
        call = re.search(r'([A-Za-z_]\w*)\s*$', expression[:opening]) if opening is not None else None
        if call:
            if call[1] in {'sizeof', 'alignof'}:
                return 'scalar', None
            decl = self.lookup(source, call[1], pos, function=True)
            if decl:
                return decl.kind, decl
            if call[1] in SCALARS or re.match(r'^(?:sin|cos|tan|sqrt|fabs|floor|ceil|round|abs|pow|fmod|atan)', call[1]):
                return 'scalar', None
            return 'unknown', None
        cast = re.match(r'^(?:reinterpret_cast|static_cast|const_cast)\s*<\s*([^>]+)>\s*\(', expression)
        if not cast:
            cast = re.match(r'^\(\s*((?:const\s+)?[\w:]+\s*\*+)\s*\)', expression)
        if cast:
            return ('pointer' if '*' in cast[1] else 'scalar'), None
        if not re.fullmatch(r'(?:[A-Za-z_]\w*(?:::\w+)*|\([^;]+\))(?:(?:->|\.)\w+|\[[^\]]+\])*', expression):
            return 'unknown', None
        member = re.search(r'([A-Za-z_]\w*)\s*(\[[^\]]*\])?$', expression)
        if member:
            decl = self.lookup(source, member[1], pos)
            if decl:
                if member[2]:
                    return ('pointer' if decl.pointers > 1 or decl.array and decl.pointers else 'scalar' if decl.base in SCALARS else 'value'), decl
                return decl.kind, decl
        return 'unknown', None


# Paths and record-name filters are a curated format inventory, not a claim that
# every struct in a loader header is an on-disc record. Consumers are named in
# the generated format table and the prose worklist.
def format_for(path, name):
    if name == '<anonymous>':
        return None, None
    if path == 'include/Game/Util/JMapInfo.hpp' and name in {'JMapItem', 'JMapData'}:
        return 'BCSV/JMap', 'include/Game/Util/JMapInfo.hpp'
    if path == 'include/Game/Map/KCollision.hpp' and name in {'KCLFile', 'KC_PrismData'}:
        return 'KCL', 'native/resource/kcl_collision.cpp'
    if path == 'libs/libms/include/libms.h' and name in {'MsbHeader', 'MsbBlockHeader', 'MsbLabelBucket', 'MsbfNode', 'MsbtAttribute'}:
        return 'LMS/MSBT/MSBF', None
    if path == 'include/Game/System/MessageHolder.hpp' and name.startswith('Message') and name.endswith('Block'):
        return 'BMG legacy declarations (active use unproven)', 'include/Game/System/MessageHolder.hpp'
    if path == 'include/Game/System/BinaryDataChunkHolder.hpp' and name == 'BinaryDataChunkHolderChunkData':
        return 'Save chunks', 'src/Game/System/BinaryDataChunkHolder.cpp'
    if path == 'libs/JSystem/include/JSystem/JUtility/JUTTexture.hpp' and name == 'ResTIMG':
        return 'BTI/ResTIMG', 'src/JSystem/JUtility/JUTNativeResource.cpp'
    if path == 'libs/JSystem/include/JSystem/JUtility/JUTNameTab.hpp' and name in {'ResNTAB', 'Entry'}:
        return 'J3D name tables', 'src/JSystem/JUtility/JUTNativeResource.cpp'
    if '/J3DGraphLoader/' in path and re.match(r'J3D.*(?:Data|Block|Info)(?:_v\d+)?$', name) and name != 'J3DAnmLoaderDataBase':
        return 'J3D model/animation', 'native/resource/j3d_animation.cpp' if 'Anm' in name else 'native/resource/j3d_model.cpp'
    if path == 'libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp' and re.fullmatch(r'J3DAnm.*(?:Data|Table|Header)|J3DAnmKeyTableBase|J3DAnmTransform_ANK1|JUTData.*Header', name):
        return 'J3D model/animation', 'native/resource/j3d_animation.cpp'
    if path == 'libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp' and name.endswith('Info'):
        return 'J3D material tables', 'native/resource/j3d_model.cpp'
    if '/JParticle/' in path and re.match(r'JPA.*Data$', name) and not re.search(r'Work|VolumeCalc', name):
        return 'JPA/JPAC', 'native/resource/jpa_resource.cpp'
    if path == 'libs/JSystem/include/JSystem/JKernel/JKRArchive.hpp' and name in {'RarcHeader', 'RarcInfoBlock', 'SDIFileEntry', 'SDIDirEntry'}:
        return 'RARC', 'src/JSystem/JKernel/JKRArchivePri.cpp'
    if path == 'libs/nw4r/include/nw4r/lyt/resources.h':
        return 'BRLYT/BRLAN', 'src/nw4r/lyt/lyt_nativeResource.cpp'
    if path == 'libs/nw4r/include/nw4r/ut/binaryFileFormat.h' or path == 'libs/nw4r/include/nw4r/ut/fontResourceFormat.h' and name != 'CharWidths':
        return 'BRFNT', 'src/nw4r/ut/ut_ResFont.cpp'
    if path == 'libs/RVLFaceLib/include/RFLi_Types.h' and name in {'RFLiCharData', 'RFLiStoreData', 'RFLiHiddenCharData'}:
        return 'RFL serialized faces', 'native/resource/rfl/rfl_native.cpp'
    if path == 'libs/RVLFaceLib/include/RFLi_Format.h' and name == 'RFLiTableData':
        return 'RFL database table', 'src/RVLFaceLib/RFL_Format.c'
    if path == 'libs/RVLFaceLib/include/RFLi_Database.h' and name == 'RFLiDatabase':
        return 'RFL database table', 'src/RVLFaceLib/RFL_Database.c'
    if path == 'libs/RVLFaceLib/include/RFLi_HiddenDatabase.h' and name == 'RFLiHiddenDB':
        return 'RFL database table', 'src/RVLFaceLib/RFL_HiddenDatabase.c'
    if path == 'libs/RVLFaceLib/include/RFLi_Texture.h' and name == 'RFLiTexture':
        return 'RFL texture', 'native/resource/rfl/rfl_native.cpp'
    if path.startswith('libs/JSystem/include/JSystem/JAudio2/'):
        file = Path(path).name
        if name in {'JASWSParser', 'JASBNKParser', 'JAUSoundTable_'}:
            return None, None
        if file in {'JASBNKParser.hpp', 'JASWSParser.hpp'} or file == 'JAUSoundTable.hpp' and not name.endswith('Table'):
            return 'JAudio banks/waves/tables', path
        if file == 'JASAramStream.hpp' and name in {'Header', 'BlockHeader'}:
            return 'JAudio sequences/streams', path
        if name in {'JAUSeqCollectionData', 'JAUSeqCollectionSoundTable', 'JAUSoundAnimationSound', 'JAUSoundAnimation', 'BinaryStreamFileTable'}:
            return 'JAudio sequences/streams', path
    return None, None


class Reference:
    def __init__(self, root):
        self.root = root
        self.cache = {}
        self.hunks = defaultdict(list)
        result = subprocess.run(['git', '-C', str(root), 'diff', 'e5bc761c0', 'HEAD', '--', 'src', 'include', 'libs'], capture_output=True, text=True, check=True)
        path, hunk = None, []
        def flush():
            if path and any('PETARI_NATIVE' in line or 'PetariNative' in line for line in hunk):
                old = {self.key(line[1:]) for line in hunk if line.startswith((' ', '-')) and not line.startswith('---')}
                self.hunks[path].append(old)
        for line in result.stdout.splitlines():
            if line.startswith('diff --git '):
                flush()
                path, hunk = line.split(' b/', 1)[1], []
            elif line.startswith('@@'):
                flush()
                hunk = []
            else:
                hunk.append(line)
        flush()
        self.commit = subprocess.run(['git', '-C', str(root), 'rev-parse', 'HEAD'], capture_output=True, text=True, check=True).stdout.strip()

    @staticmethod
    def key(line):
        return re.sub(r'\s+', '', re.sub(r'/\*.*?\*/|//.*', '', line)).replace('GALAXY_NATIVE', 'PETARI_NATIVE')

    def source(self, path):
        if path not in self.cache:
            file = self.root / path
            self.cache[path] = Source(path, file.read_text()) if file.is_file() else None
        return self.cache[path]

    def match(self, path, snippet):
        key = self.key(snippet)
        return path if len(key) >= 12 and any(key in hunk for hunk in self.hunks[path]) else None

    def record(self, source, name, fix):
        other = self.source(source.path)
        same = next((r for r in other.records if r[0]['name'] == name), None) if other else None
        if not same:
            return {'status': 'SMG2-only declaration: no same-path/name reference record', 'fix': None}
        if fix and (self.root / fix).is_file():
            text = (self.root / fix).read_text()
            if re.search(r'JSU_BE|BigEndian|Native|native|PETARI_NATIVE|readU\d+BE', text):
                return {'status': 'format precedent; validate SMG2 record/version before transfer', 'fix': fix}
        return {'status': 'reference record exists; no verified native fix', 'fix': None}


class Sweep:
    def __init__(self, sources, reference=None):
        self.sources, self.reference = sources, reference
        self.resolver = Resolver(sources)
        self.findings = {}
        self.inventory = []
        self.stats = Counter()
        self.wrapper_cache = {}

    def wrapper_types(self, source):
        if source.path not in self.wrapper_cache:
            protected = set()
            pending = list(source.records)
            while pending:
                next_pending = []
                for match, opening, closing in pending:
                    body = source.code[opening + 1:closing]
                    raw = re.search(r'\b(?:u16|s16|u32|s32|f32|int|float|double)\s+\w+\s*(?:;|\[)', body)
                    pointer = re.search(r'[\w>]\s*\*+\s*\w+\s*(?:;|\[)', body)
                    wrapper = re.search(r'JSU_BE(?:_UNALIGNED)?\s*\(|(?:BigEndianValue|JMapBigEndian)\s*<', body)
                    nested = any(re.search(r'\b' + re.escape(name) + r'\b', body + match.group()) for name in protected)
                    if match['name'] and not raw and not pointer and (wrapper or nested):
                        protected.add(match['name'])
                    else:
                        next_pending.append((match, opening, closing))
                if len(next_pending) == len(pending):
                    break
                pending = next_pending
            self.wrapper_cache[source.path] = protected
        return self.wrapper_cache[source.path]

    def conversion_evidence(self, fmt, body):
        wrapped = re.search(r'JSU_BE(?:_UNALIGNED)?\s*\(|(?:BigEndianValue|JMapBigEndian)\s*<', body)
        raw = re.search(r'\b(?:u16|s16|u32|s32|f32|int|float|double)\s+\w+\s*(?:;|\[)', body)
        if wrapped and not raw:
            return 'big-endian field wrappers present; no direct raw primitive fields detected'
        hooks = {
            'KCL': ('src/Game/System/ResourceHolder.cpp', 'PetariNative::KCL::convertInPlace'),
            'BTI/ResTIMG': ('src/Game/System/ResourceHolder.cpp', 'JUTNativeNormalizeResTIMG'),
            'J3D model/animation': ('src/JSystem/J3DGraphLoader/J3DModelLoader.cpp', 'makeHostModelImage'),
            'J3D material tables': ('src/JSystem/J3DGraphLoader/J3DModelLoader.cpp', 'makeHostModelImage'),
            'J3D name tables': ('src/Game/System/ResourceHolder.cpp', 'JUTNativeNormalizeResNTAB'),
            'JPA/JPAC': ('src/Game/Effect/ParticleResourceHolder.cpp', 'makeHostImage'),
            'BRLYT/BRLAN': ('src/nw4r/lyt/lyt_layout.cpp', 'NativeNormalizeResource'),
            'BRFNT': ('src/nw4r/ut/ut_ResFont.cpp', 'BuildNativeFont(brfnt'),
            'RARC': ('src/JSystem/JKernel/JKRMemArchive.cpp', 'setupNativeTables'),
        }
        path, token = hooks.get(fmt, ('', ''))
        source = self.sources.get(path)
        if source and token in source.code:
            return f'conversion entry hook present: {path} ({token}); format/version coverage still requires validation'
        if fmt == 'J3D model/animation':
            source = self.sources.get('src/JSystem/J3DGraphLoader/J3DAnmLoader.cpp')
            if source and 'makeHostAnimImage' in source.code:
                return 'animation conversion entry hook present; format/version coverage still requires validation'
        return None

    def add(self, source, pos, category, confidence, reason, **extra):
        line = source.line(pos)
        if line in source.excluded:
            self.stats['excluded_native_legacy_findings'] += 1
            return
        key = (source.path, line, category)
        entry = {'file': source.path, 'line': line, 'class': category, 'confidence': confidence,
                 'directory': directory_for(source.path), 'lane': lane_for(directory_for(source.path)),
                 'reason': reason, 'snippet': source.snippet(line), **extra}
        if self.reference:
            fix = self.reference.match(source.path, entry['snippet'])
            if fix:
                entry['reference_fix'] = fix
                entry['reference_evidence'] = 'matching Wii code in a reference native patch hunk; verify transfer'
        previous = self.findings.get(key)
        if not previous or previous['confidence'] != 'high' or confidence == 'high':
            self.findings[key] = entry

    def scan(self, source):
        code = source.code
        for match in CAST32.finditer(code):
            if match['c'] and re.search(r'\b(?:JSU_BE(?:_UNALIGNED)?|sizeof|alignof|alignas|decltype)\s*$', code[:match.start()]):
                self.stats['suppressed_type_macro_arguments'] += 1
                continue
            expression = operand(code, match.end(), paired=bool(match['named'] or match['functional']), unary=bool(match['c']))
            if not expression:
                continue
            kind, decl = self.resolver.infer(source, expression, match.start())
            if kind == 'pointer':
                self.add(source, match.start(), 'pointer-truncation', 'high',
                         '32-bit cast of a declared pointer/array/address expression', operand=expression,
                         declared_type=decl.base + '*' * decl.pointers if decl else None)
            elif kind == 'unknown':
                self.add(source, match.start(), 'cast-unresolved', 'review',
                         '32-bit cast operand type unresolved; may be an ordinary numeric conversion', operand=expression)
            else:
                self.stats['suppressed_nonpointer_casts'] += 1
        for match in re.finditer(r'\b([A-Za-z_]\w*(?:(?:->|\.)\w+)*(?:\[[^\]\n]+\])?)\s*=(?!=)\s*([^;\n]+)', code):
            lhs, rhs = match.groups()
            if CAST32.search(rhs):
                continue
            target = re.search(r'(\w+)(?:\[[^\]]+\])?$', lhs)[1]
            decl = self.resolver.lookup(source, target, match.start())
            scalar_slot = decl and (decl.kind == 'scalar' or '[' in lhs and
                                    (decl.array and decl.pointers == 0 or not decl.array and decl.pointers == 1))
            if scalar_slot and re.fullmatch(INT32, decl.base) and self.resolver.infer(source, rhs, match.start())[0] == 'pointer':
                direct = source.line(decl.pos) == source.line(match.start()) and '->' not in lhs and '.' not in lhs
                self.add(source, match.start(), 'pointer-storage32', 'high' if direct else 'review',
                         'pointer assigned to an apparent 32-bit slot; verify receiver type and local scope', operand=rhs, target=lhs)
        for match in POINTER_CAST.finditer(code):
            expression = operand(code, match.end(), paired=bool(match['named']), unary=bool(match['c']))
            kind, decl = self.resolver.infer(source, expression, match.start())
            if kind == 'scalar' and decl and re.fullmatch(INT32, decl.base) and not re.search(r'\b(?:offset|offs|index|count|size)\b', expression, re.I):
                self.add(source, match.start(), 'integer-pointer-storage', 'review',
                         '32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets', operand=expression)
            if re.fullmatch(r'(?:\b(?:const|volatile)\s+)?(?:u16|s16|u32|s32|f32|int|float)\s*\*',
                            re.sub(r'^.*?<\s*|\s*>\s*\($|^\(\s*|\s*\)$', '', match.group())):
                before = code[max(0, match.start() - 4):match.start()]
                if '*' in before or re.search(r'^\s*\*', code[match.end():match.end() + 2]):
                    if decl and decl.base in {'f32', 'float', 'u32', 's32'} and expression.startswith('&'):
                        self.stats['suppressed_same_width_bitcasts'] += 1
                    else:
                        family = source.path.startswith('src/libms/') or 'TColor.hpp' in source.path or source.path.endswith('JMapInfo.hpp')
                        self.add(source, match.start(), 'endian-raw-access', 'high' if family else 'review',
                                 'multi-byte pointer dereference over byte/resource memory; review byte order and alignment', operand=expression)
        for match in re.finditer(r'\bALIGN_(?:NEXT|PREV|UP|DOWN)\s*\(', code):
            expression = operand(code, match.end()).split(',', 1)[0]
            if self.resolver.infer(source, expression, match.start())[0] == 'pointer':
                self.add(source, match.start(), 'pointer-alignment-review', 'review', 'alignment macro applied to a pointer; inspect integer width', operand=expression)
        for match in re.finditer(r'sizeof\s*\(\s*(?:void\s*\*|[\w:]+\s*\*+)\s*\)\s*(?:==|!=)\s*(?:4|0x0*4)\b', code):
            self.add(source, match.start(), 'pointer-size-assumption', 'high', 'explicit four-byte pointer-size assumption')
        for decls in source.decls.values():
            for decl in decls:
                if re.fullmatch(INT32, decl.base) and not decl.function and not decl.pointers:
                    line = source.snippet(source.line(decl.pos))
                    if re.search(r'//.*(?:pointer|\bptr\b|address)|/\*[^*]*(?:pointer|\bptr\b|address)[^*]*\*/', line, re.I) and not re.search('offset', line, re.I):
                        self.add(source, decl.pos, 'integer-pointer-storage', 'review', '32-bit field documented as holding an address/pointer')
        for match, opening, closing in source.records:
            body, name = code[opening + 1:closing], match['name'] or '<anonymous>'
            fmt, fix = format_for(source.path, name)
            if fmt:
                reference = self.reference.record(source, name, fix) if self.reference else {'status': 'reference not supplied', 'fix': None}
                entry = {'file': source.path, 'line': source.line(match.start()), 'record': name, 'format': fmt,
                         'pointer_fields': bool(re.search(r'[\w>]\s*\*+\s*\w+\s*(?:;|\[)', body)), **reference}
                protection = self.conversion_evidence(fmt, body)
                if name in self.wrapper_types(source):
                    protection = 'big-endian field/nested wrapper types present; no raw primitive or pointer fields detected'
                covered = protection and not reference['status'].startswith('SMG2-only')
                entry['current_conversion'] = protection
                entry['state'] = 'conversion evidence present' if covered else 'needs review'
                self.inventory.append(entry)
                if covered:
                    self.stats['disc_records_with_conversion_evidence'] += 1
                else:
                    self.add(source, match.start(), 'endian-disc-record', 'review' if 'legacy' in fmt else 'high',
                             f'{fmt}: review byte swapping and four-byte wire offsets before native use',
                             record=name, reference_fix=reference['fix'], reference_evidence=reference['status'])
            if match['kind'] == 'union':
                if re.search(r'\bu32\s+\w+\s*;', body) and (re.search(r'\bu8\s+\w+\s*\[\s*4\s*\]', body) or len(re.findall(r'\bu8\s+\w+\s*;', body)) >= 4):
                    self.add(source, match.start(), 'endian-byte-union', 'high', 'u32 overlaps four bytes; packed word order changes on little-endian')
                if re.search(r'[\w>]\s*\*+\s*\w+\s*(?:;|\[)', body):
                    mixed = bool(re.search(r'\b(?:u32|s32|f32|int)\s+\w+\s*;', body))
                    anonymous = not match['name']
                    self.add(source, match.start(), 'union-pointer-alias', 'high' if mixed else 'review',
                             f'{"Anonymous" if anonymous else "Named"} union contains native pointers' +
                             (' and 32-bit value aliases; clear/write the full native slot' if mixed else '; verify consumers, growth alone is not proof of a bug'))
            fields = list(re.finditer(r'\b(?:u8|u16|u32|s8|s16|s32|unsigned(?:\s+int)?|int|bool)\s+(\w+)\s*:\s*(\d+)\s*;', body))
            nested = [(o, c) for _, o, c in source.records if opening < o < c < closing]
            direct = [f for f in fields if not any(o < opening + 1 + f.start() < c for o, c in nested)]
            if direct:
                field = direct[0]
                serialized = bool(fmt) or source.path.startswith('libs/RVLFaceLib/')
                self.add(source, opening + 1 + field.start(), 'bitfield-layout', 'high' if serialized else 'review',
                         ('serialized/mapped record bit order changes on LE; explicit masks or decode required' if serialized else
                          'runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer'),
                         fields=[f'{f[1]}:{f[2]}' for f in direct], record=name)
        for match in re.finditer(r'^\s*#\s*pragma\s+pack\s*\([^\n]*|\b__attribute__\s*\(\(\s*packed\b', code, re.MULTILINE):
            if not re.search(r'\bpop\b', match.group()):
                self.add(source, match.start(), 'packed-layout', 'review', 'packed layout: audit multi-byte access, serialized offsets, and pointer-bearing members')

    def add_diagnostics(self, reports):
        for path in reports:
            report = json.loads(path.read_text())
            root = report.get('root', '')
            for result in report['results']:
                for match in re.finditer(r'^(.*?):(\d+):\d+: error: (cast from pointer to smaller type .*|cast from pointer to .*loses information.*)$', result['diagnostics'], re.MULTILINE):
                    file = match[1]
                    if root and file.startswith(root + '/'):
                        file = file[len(root) + 1:]
                    file = file.removeprefix('build/audit-sjis/')
                    source = self.sources.get(file)
                    line = int(match[2])
                    if source and line <= len(source.line_starts):
                        diagnostic_tail = result['diagnostics'][match.end():]
                        frame = re.match(r'\n\s*\d+\s*\|\s?(.*)', diagnostic_tail)
                        if frame and Reference.key(frame[1]) != Reference.key(source.snippet(line)):
                            self.stats['stale_audit_diagnostics_skipped'] += 1
                            continue
                        self.add(source, source.line_starts[line - 1], 'pointer-truncation', 'high',
                                 'Clang-confirmed in audit snapshot: ' + match[3], audit_source=result['source'], audit_report=str(path))


def collect(root):
    files = set()
    for base in [root / 'src', root / 'include', *sorted((root / 'libs').glob('*/include'))]:
        files.update(path for path in base.rglob('*') if path.is_file() and path.suffix in EXTENSIONS)
    return {str(path.relative_to(root)): Source(str(path.relative_to(root)), path.read_text(encoding='utf-8')) for path in sorted(files)}


def cell(value):
    return str(value).replace('|', '\\|').replace('\n', ' ')


def markdown(report):
    findings = report['findings']
    lines = ['# Native ABI and endianness hazard worklist', '',
             f"Swept {report['generated_at']}; source root: `{report['root']}`. Native target: arm64, little-endian, 8-byte pointers; Wii: PPC, big-endian, 4-byte pointers.", '',
             f"Scanned **{report['files_scanned']} files**; **{len(findings)} candidate locations**. Counts are one finding per file/line/class; classes can overlap.", '',
             'This is a lexical worklist with declaration lookup, not a full C++ analysis or proof of runtime failure. High confidence means a pointer/address expression, compiler-confirmed truncation, a known wire record, or a mixed pointer/value union was identified. Review rows retain unresolved numeric casts and runtime layouts separately. Comments, string literals, standalone Wii offset annotations, numeric conversions, and recognized native-excluded branches do not create findings.', '',
             'Declarations are resolved in the current file and two include levels, using the nearest earlier declaration or an unambiguous header declaration. Full scope, overload, typedef, macro, and inheritance resolution is unavailable. Pointer subtraction is treated as a numeric difference; byte-array elements are numeric. A zero count is not a safety guarantee.', '',
             f"Suppressed nonpointer casts: {report['stats'].get('suppressed_nonpointer_casts', 0)}. Serialized records with conversion evidence: {report['stats'].get('disc_records_with_conversion_evidence', 0)}; retained in the inventory, excluded from unresolved findings. Recognized GALAXY_NATIVE-inactive branches are masked before declaration lookup. Conversion evidence is structural, not runtime validation; unsupported variants remain possible.", '',
             '## Reproduce', '', '```sh',
             f"nice -n 10 python3 tools/port/hazard_sweep.py --root {report['root']} --reference ~/MacioGalaxy --audit ~/MacioGalaxy2-team/scratch-port/solb/native-audit-hooked.json --baseline ~/MacioGalaxy2-team/scratch-port/solb/native-hazards-worktree-baseline.json --output docs/port/HAZARDS.md --json-output build/native-hazards.json",
             "python3 -m unittest discover -s tools/port -p 'test_hazard_sweep.py'", '```', '',
             f"Reference: `{report.get('reference_root')}`, commit `{report.get('reference_commit')}`; native patch comparisons use pristine Petari `e5bc761c0` through reference HEAD. The reference was read only. Input content digest: `{report['source_digest']}`.", '',
             '## Per-class totals', '',
             'The earlier worktree snapshot is preserved for context. Branch masking and the curated record inventory were refined afterward, so this column is not a controlled measurement of fixes. The compiler comparison in COMPILE_STATUS.md uses the original raw baseline and the hooked overlay.', '',
             '| Class | Earlier worktree snapshot | Overlay high | Overlay review | Overlay total |', '| --- | ---: | ---: | ---: | ---: |']
    baseline = report.get('baseline', {})
    classes = sorted({f['class'] for f in findings} | set(baseline.get('classes', {})) |
                     {'pointer-size-assumption', 'pointer-alignment-review'})
    for category in classes:
        rows = [f for f in findings if f['class'] == category]
        high = sum(f['confidence'] == 'high' for f in rows)
        lines.append(f"| {category} | {baseline.get('classes', {}).get(category, 0 if baseline else '—')} | {high} | {len(rows) - high} | {len(rows)} |")
    lines += ['', '## Triage and format handling', '',
              '1. Prioritize remaining Game/JSystem/nw4r pointer truncations, especially SMG2-only conversation/control code and address fields with no matching reference patch. Unresolved casts are review candidates; they are not counted as proven pointer conversions.',
              '2. LMS/MSBT/MSBF is SMG2-only relative to this reference. MsbHeader, MsbBlockHeader, MsbLabelBucket, MsbfNode, and MsbtAttribute overlay big-endian data in src/libms/commonlib.c, flowfile.c, and msgfile.c. Counts, offsets, label values, branch tables, attributes, and UTF-16 text all need explicit wire decoding. MsbInfo, MsbBlock, MsbtInfo, and MsbfInfo are host handles and should grow with native pointers. The BMG declarations in MessageHolder.hpp do not establish that SMG1 message hooks cover LMS.',
              '3. JSU_BE(T) / JSU_BE_UNALIGNED(T) in JSystem/JSupport/JSUBigEndian.hpp preserve serialized field width and decode big-endian values. The reference applies them to JASBNKParser, JASWSParser, JASAramStream, JAUSeqCollection, JAUSoundAnimator, JAUSoundTable, and JAUStreamFileTable. JMap uses BigEndianValue wrappers. Width-preserving offset aliases alone do not swap bytes.',
              '4. J3D model/animation, JPAC, KCL, BRLYT/BRLAN, BTI/ResTIMG, BRFNT, and RARC use resource conversion or separate host descriptors. Transfer both converters and their entry hooks. KCLFile and font records must not overwrite four-byte wire offsets with eight-byte pointers. The inventory distinguishes detected hooks from unconverted or SMG2-only records; same format names do not prove support for every block/version.',
              '5. Mixed pointer/value unions such as ActorParamEntry and TalkMessageCtrl need all active aliases and initialization paths reviewed. Pointer-only unions are review items: ordinary class growth and Wii offset comments alone are not failures. RFL face/database bitfields need explicit bit-order decoding; the reference face codec is in native/resource/rfl/rfl_native.cpp, with entry hooks in RFL_Database.c and RFL_HiddenDatabase.c. Runtime-only bitfields are review items unless serialized or aliased to a packed word.',
              '6. RVL_SDK, Runtime, MSL_C, and MetroTRK are lower priority than game/resource paths. Hardware address casts may disappear behind native platform implementations; their static presence does not prove they execute in the native application.', '',
              '## Directory ownership', '', '| Directory (source + headers) | Lane | Files scanned | Earlier candidates | Overlay high | Overlay review | Overlay classes |', '| --- | --- | ---: | ---: | ---: | ---: | --- |']
    for directory, checked in sorted(report['directories'].items()):
        rows = [f for f in findings if f['directory'] == directory]
        counts = Counter(f['class'] for f in rows)
        high = sum(f['confidence'] == 'high' for f in rows)
        lines.append('| ' + ' | '.join(cell(v) for v in (directory, lane_for(directory), checked, baseline.get('directories', {}).get(directory, 0 if baseline else '—'), high, len(rows) - high,
                      '; '.join(f'{k}: {v}' for k, v in counts.most_common()) or '—')) + ' |')
    lines += ['', '## On-disc record inventory', '',
              'This table names curated serialized records and their reference conversion strategy. The library lane must check version tags, nested tables, alignment, and the loader entry hook before reusing a converter. A same-path/name match establishes a declaration precedent only; it does not prove the binary formats are identical. Offset fields stored as u32 remain u32 on disc; relocate them into separate native descriptors when pointers are required.', '',
              '| Format | Record at file:line | Pointer members | Current conversion evidence | Reference status |', '| --- | --- | --- | --- | --- |']
    for item in sorted(report['disc_records'], key=lambda x: (x['format'], x['file'], x['line'])):
        status = ('MacioGalaxy has a fix: ' + item['fix'] + ' — ' if item['fix'] else '') + item['status']
        lines.append('| ' + ' | '.join(cell(v) for v in (item['format'], f"{item['record']} — {item['file']}:{item['line']}", 'yes' if item['pointer_fields'] else 'no', item['current_conversion'] or 'none detected', status)) + ' |')
    lines += ['', '## Findings (high confidence first)', '',
              '“MacioGalaxy has a fix” identifies either matching Wii code in a native patch hunk or a known format converter. It describes reference evidence, not an applied or tested SMG2 fix. Unmarked rows require manual comparison; absence of a marker does not prove absence of a reference fix.', '']
    for confidence in ['high', 'review']:
        for category in classes:
            rows = [f for f in findings if f['class'] == category and f['confidence'] == confidence]
            if not rows:
                continue
            lines += [f'### {category} — {confidence} ({len(rows)})', '', '| File:line | Lane | Evidence | Reference |', '| --- | --- | --- | --- |']
            for row in rows:
                detail = row['reason']
                if row.get('operand'):
                    detail += '; operand: ' + row['operand'][:180]
                if row.get('record'):
                    detail += '; record: ' + row['record']
                if row.get('fields'):
                    detail += '; fields: ' + ', '.join(row['fields'])
                ref = 'MacioGalaxy has a fix: ' + row['reference_fix'] if row.get('reference_fix') else 'review needed'
                lines.append('| ' + ' | '.join(cell(v) for v in (f"{row['file']}:{row['line']}", row['lane'], detail, ref)) + ' |')
            lines.append('')
    return '\n'.join(lines) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--reference', type=Path)
    parser.add_argument('--audit', action='append', default=[], type=Path)
    parser.add_argument('--baseline', type=Path, help='Earlier hazard JSON for contextual count columns')
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--json-output', required=True, type=Path)
    args = parser.parse_args()
    root = args.root.expanduser().resolve()
    sources = collect(root)
    reference = Reference(args.reference.expanduser().resolve()) if args.reference else None
    sweep = Sweep(sources, reference)
    for source in sources.values():
        sweep.scan(source)
    sweep.add_diagnostics(args.audit)
    digest = hashlib.sha256()
    for source in sources.values():
        digest.update(source.path.encode() + b'\0' + source.text.encode() + b'\0')
    findings = sorted(sweep.findings.values(), key=lambda f: (f['confidence'] != 'high', f['class'], f['file'], f['line']))
    report = {'generated_at': datetime.now(timezone.utc).isoformat(), 'root': str(root),
              'reference_root': str(reference.root) if reference else None,
              'reference_commit': reference.commit if reference else None,
              'source_digest': digest.hexdigest(), 'files_scanned': len(sources),
              'directories': dict(Counter(directory_for(path) for path in sources)),
              'stats': dict(sweep.stats), 'findings': findings, 'disc_records': sweep.inventory}
    if args.baseline:
        baseline = json.loads(args.baseline.read_text())
        report['baseline'] = {'path': str(args.baseline), 'generated_at': baseline['generated_at'],
                              'source_digest': baseline['source_digest'],
                              'classes': dict(Counter(f['class'] for f in baseline['findings'])),
                              'directories': dict(Counter(f['directory'] for f in baseline['findings']))}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.json_output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(markdown(report))
    args.json_output.write_text(json.dumps(report, indent=2) + '\n')
    print(f"Scanned {len(sources)} files: {len(findings)} candidate locations ({sum(f['confidence'] == 'high' for f in findings)} high confidence); {len(sweep.inventory)} serialized records")


if __name__ == '__main__':
    main()
