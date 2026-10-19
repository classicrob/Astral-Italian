#!/usr/bin/env python3
"""Conservative C/C++ evaluation-order candidate inventory without a compiler.

Tokenizes original sources; balances delimiters/templates; never edits sources.
Class A is argument order, class B is binary operand order. Findings require
effects in two distinct operands, not just two nested calls in one argument.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from dataclasses import dataclass
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import shlex


LANES = {
    **dict.fromkeys(("Util", "LiveActor", "NameObj", "System", "Scene", "Map",
                     "Camera", "Animation", "Gravity", "Effect", "Demo", "AudioLib",
                     "GameAudio", "Speaker", "NWC24"), "core"),
    **dict.fromkeys(("MapObj", "Ride", "AreaObj", "Boss"), "objects"),
    **dict.fromkeys(("Enemy", "RhythmLib", "Screen", "NPC"), "enemies"),
    "Player": "astra",
}
DEFAULT_PATHS = ("src/Game", "src/JSystem", "src/nw4r", "src/RVLFaceLib")
SUFFIXES = {".cpp", ".c", ".cc", ".cxx", ".inc"}
# These names are reviewed value queries, not a blanket get*/is*/calc* exemption.
# RNG and stream classifications take precedence over this list.
PURE_CALLS = frozenset("""
MR::getPlayerPos MR::getPlayerCenterPos MR::getPlayerVelocity MR::getPlayerGravity
MR::getCamPos MR::getCamXdir MR::getCamYdir MR::getCamZdir MR::getCamUp
MR::getCurrentStageName MR::getCurrentScenarioNo MR::isNearZero MR::isFirstStep
MR::isLessStep MR::isGreaterStep MR::isStep MR::isDead MR::isHiddenModel
MR::isOnGround MR::isOnGroundPlayer MR::isPlayerDead MR::isPlayerInBind
MR::isPlayerOnWaterSurface MR::isPlayerInWater MR::isPlayerSwingAction
MR::isPlayerHipDropFalling MR::isPlayerHipDropLand MR::isPlayerInRush
MR::isPlayerModeBee MR::isPlayerModeTeresa MR::isPlayerModeIce MR::isPlayerModeFire
MR::getEaseInValue MR::getEaseOutValue MR::getInterpolateValue MR::getLinerValue
MR::sin MR::cos MR::sqrt MR::abs MR::clamp MR::min MR::max MR::pi
sin sinf cos cosf tan tanf sqrt sqrtf fabs fabsf abs fmod fmodf pow powf
atan atan2 atan2f acos acosf asin asinf floor floorf ceil ceilf round roundf
min max std::min std::max JPASinShort JPACosShort
PetariNative::readU16BE PetariNative::readU24BE PetariNative::readU32BE
strcmp strncmp strlen memcmp tolower toupper sizeof alignof __alignof__ decltype noexcept
""".split())
PURE_METHODS = frozenset("""
x y z dot squared length squareMag magnitude getX getY getZ getW
getPosRndm getBaseVel getBaseVelRndm getVelInhRate getGravity getScaleX getScaleY
getScaleInhRate getAlphaInhRate getColorInhRate getLife getPrmAlpha getLoopOfstValue
getLifetimeRndm getInitVelRatio getMomentRndm getScaleRndm getAlphaFreqRndm
getRotateInitAngle getRotateRndmAngle getRotateInitSpeed getRotateRndmSpeed
getRotateDirection getRotInitSpeed getRotateAngle getCsp
getNerveStep getEnd getBtkCtrl getMag getMagRndm
isEnableScaleAnm isEnableAlphaFlick isEnableRotateAnm isFieldAffected
isScaleInherited isColorInherited isAlphaInherited isRotateOn checkFlag
""".split())
PURE_TYPES = frozenset("""
bool char short int long float double signed unsigned void size_t ptrdiff_t
s8 s16 s32 s64 u8 u16 u32 u64 f32 f64 Vec Vec2 Vec3 GXColor GXColorS10
TVec2 TVec3 TVec4 TVec2f TVec3f TVec4f TVec2s TVec3s TQuat4f TPos3f TMtx34f
static_cast reinterpret_cast const_cast dynamic_cast
""".split())
# readReg in this unit delegates to value queries (JASSeqParser.cpp:213;
# JASRegisterParam.cpp:70; JASTrack.hpp:144; JASTrackPort.hpp:17).
PURE_SCOPED = {"src/JSystem/JAudio2/JASSeqParser.cpp": {"readReg"}}
CONTROL = {"if", "for", "while", "switch", "catch", "return", "throw",
           "delete", "case", "__attribute__", "__declspec", "asm", "__asm__"}
UNEVALUATED = {"sizeof", "alignof", "__alignof__", "decltype", "noexcept"}
ASSIGNMENTS = {"=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="}
PRECEDENCE = {
    ",": 1, **dict.fromkeys(ASSIGNMENTS, 2), "?": 3, ":": 3,
    "||": 4, "&&": 5, "|": 6, "^": 7, "&": 8, "==": 9, "!=": 9,
    "<": 10, ">": 10, "<=": 10, ">=": 10, "<=>": 10,
    "<<": 11, ">>": 11, "+": 12, "-": 12, "*": 13, "/": 13, "%": 13,
}
# C++17 sequences assignment RHS, logical operators, comma and shift operands.
UNSEQUENCED_BINARY = set(PRECEDENCE) - ASSIGNMENTS - {",", "?", ":", "||", "&&", "<<", ">>"}
IDENT = re.compile(r"[A-Za-z_$][\w$]*")
LITERAL = re.compile(r'(?:u8|u|U|L)?(R")|(?:u8|u|U|L)?([\"\'])')
OPERATORS = sorted({"::", "->*", "->", ".*", "++", "--", "...", "==", "!=",
                    "<=", ">=", "&&", "||", "<=>", "<<=", ">>=", "##"} | ASSIGNMENTS,
                   key=len, reverse=True)


@dataclass(frozen=True)
class Token:
    text: str
    start: int
    end: int
    line: int
    kind: str = "punct"


@dataclass(frozen=True)
class Effect:
    kind: str
    name: str
    token: int


@dataclass
class Call:
    start: int
    opening: int
    closing: int
    name: str
    arguments: list[tuple[int, int]]
    declaration: bool = False


def tokenize(source: str, directive_log=None) -> tuple[list[Token], int]:
    """Keep source offsets, skip comments/directives, and consume literals atomically."""
    tokens = []
    i, line, directives = 0, 1, 0
    while i < len(source):
        ch = source[i]
        if ch.isspace():
            line += ch == "\n"
            i += 1
            continue
        if ch == "#" and not source[source.rfind("\n", 0, i) + 1:i].strip():
            directives += 1
            end = i
            while True:
                end = source.find("\n", end)
                if end < 0:
                    end = len(source)
                    break
                if not source[i:end].rstrip("\r").endswith("\\"):
                    break
                end += 1
            line += source[i:end].count("\n")
            if directive_log is not None:
                directive_log.append((i, end, source[i:end]))
            i = end
            continue
        if source.startswith("//", i):
            end = source.find("\n", i)
            i = len(source) if end < 0 else end
            continue
        if source.startswith("/*", i):
            end = source.find("*/", i + 2)
            end = len(source) if end < 0 else end + 2
            line += source[i:end].count("\n")
            i = end
            continue
        # Prefixes and raw strings must be recognized before identifiers.
        literal = LITERAL.match(source, i)
        if literal:
            start, start_line = i, line
            if literal.group(1):
                opening = source.find("(", i + len(literal.group()))
                if opening < 0:
                    end = len(source)
                else:
                    delimiter = source[i + len(literal.group()):opening]
                    marker = ")" + delimiter + '"'
                    close = source.find(marker, opening + 1)
                    end = len(source) if close < 0 else close + len(marker)
            else:
                quote = literal.group(2)
                end = i + len(literal.group())
                while end < len(source):
                    if source[end] == "\\":
                        end += 2
                    elif source[end] == quote:
                        end += 1
                        break
                    else:
                        end += 1
            line += source[i:end].count("\n")
            tokens.append(Token(source[start:end], start, end, start_line, "literal"))
            i = end
            continue
        match = IDENT.match(source, i)
        if match:
            end, kind = match.end(), "id"
        elif ch.isdigit() or (ch == "." and i + 1 < len(source) and source[i + 1].isdigit()):
            # +/- belong to a number only immediately following an exponent.
            end = i + 1
            while end < len(source):
                c = source[end]
                if c.isalnum() or c in "_.'" or (c in "+-" and source[end - 1] in "eEpP"):
                    end += 1
                else:
                    break
            kind = "number"
        else:
            op = next((op for op in OPERATORS if source.startswith(op, i)), ch)
            end, kind = i + len(op), "punct"
        tokens.append(Token(source[i:end], i, end, line, kind))
        i = end
    return tokens, directives


def delimiters(tokens):
    pairs, stack, errors = {}, [], []
    closers = {")": "(", "]": "[", "}": "{"}
    for i, token in enumerate(tokens):
        if token.text in {"(", "[", "{"}:
            stack.append(i)
        elif token.text in closers:
            if stack and tokens[stack[-1]].text == closers[token.text]:
                j = stack.pop()
                pairs[i], pairs[j] = j, i
            else:
                errors.append(f"line {token.line}: unmatched {token.text}")
    errors.extend(f"line {tokens[i].line}: unmatched {tokens[i].text}" for i in stack)
    return pairs, errors


def template_pairs(tokens, groups):
    """Recognize template-id contexts; leave ordinary relational < and > alone.

    A call/qualification suffix is strong evidence. Declaration contexts also
    accept capitalized types and standard lower-case template container names.
    Unresolvable C++ '<' ambiguity is documented rather than claimed semantic.
    """
    result = {}
    type_names = {"vector", "array", "pair", "map", "set", "list", "unique_ptr", "shared_ptr"}
    for i, token in enumerate(tokens):
        if token.text != "<" or i == 0 or tokens[i - 1].kind != "id":
            continue
        stack, local, j = [i], {}, i + 1
        while j < len(tokens):
            v = tokens[j].text
            if v in {"(", "["} and j in groups:
                j = groups[j] + 1
                continue
            if v in {";", "{", "}", "&&", "||", "?", "="}:
                break
            if v == "<":
                stack.append(j)
            elif v == ">":
                opening = stack.pop()
                local[opening], local[j] = j, opening
                if not stack:
                    after = tokens[j + 1].text if j + 1 < len(tokens) else ""
                    name = tokens[i - 1].text
                    if after in {"(", "::", ".", "->"} or (
                            after and (tokens[j + 1].kind == "id" or after in {"*", "&", ",", ")", ">"})
                            and (name[0].isupper() or name in type_names)):
                        result.update(local)
                    break
            j += 1
    return result


def classify_call(name, path=""):
    base = re.split(r"::|->|\.", name)[-1]
    if name in PURE_CALLS or any(path.endswith(p) and name in names for p, names in PURE_SCOPED.items()):
        return None
    if (base.startswith(("getRandom", "get_r_", "get_rndm_"))
            or base in {"rand", "random", "randf", "getRand", "getRandF", "get_rndm"}):
        return "RNG"
    if re.match(r"(?:read|Read|pop|Pop|next|Next|consume|Consume|extract|Extract)", base):
        return "stream/iterator"
    if base in PURE_METHODS or base in PURE_TYPES:
        return None
    return "unknown/stateful"


class Scanner:
    def __init__(self, source, path="<memory>"):
        self.source, self.path = source, path
        self.binary_operators = UNSEQUENCED_BINARY | (
            ASSIGNMENTS | {"<<", ">>"} if Path(path).suffix == ".c" else set())
        self.tokens, self.directive_count = tokenize(source)
        self.groups, self.errors = delimiters(self.tokens)
        self.templates = template_pairs(self.tokens, self.groups)
        self.pairs = self.groups | self.templates
        self.calls = self.find_calls()
        self.calls_by_open = {c.opening: c for c in self.calls}
        self.inert = self.inert_ranges()
        self.findings = []
        self._effects_cache = {}

    def split(self, start, end, separators={","}):
        parts, mark, i = [], start, start
        while i < end:
            if i in self.pairs and self.pairs[i] > i:
                i = self.pairs[i] + 1
                continue
            if self.tokens[i].text in separators:
                if mark < i:
                    parts.append((mark, i))
                mark = i + 1
            i += 1
        if mark < end:
            parts.append((mark, end))
        return parts

    def callee(self, opening):
        t, j = self.tokens, opening - 1
        if j < 0:
            return None
        if (j >= 2 and t[j].text in {"<", ">"} and t[j - 1].text == t[j].text
                and t[j - 2].text == "operator"):
            return j - 2, "operator" + t[j].text * 2
        if t[j].text == ">" and j in self.templates:
            j = self.templates[j] - 1
        if j < 0:
            return None
        if t[j].kind == "id" and t[j].text not in CONTROL:
            end, start = j + 1, j
            while start >= 2 and t[start - 1].text in {"::", ".", "->"}:
                k = start - 2
                if t[k].text == ">" and k in self.templates:
                    k = self.templates[k] - 1
                if k < 0 or t[k].kind != "id":
                    break
                start = k
            name = "".join(x.text for x in t[start:end] if x.text not in {"template"})
            # Strip template arguments from names for side-effect classification.
            name = re.sub(r"<.*>", "", name)
            return start, name
        if t[j].text in {")", "]"} and j in self.groups:
            start = self.groups[j]
            if start and t[start - 1].text == "operator":
                return start - 1, "operator" + t[start].text + t[j].text
            inside = t[start + 1:j]
            if (t[j].text == ")" and inside and inside[0].kind == "id"
                    and (inside[0].text in PURE_TYPES or inside[0].text[0].isupper())
                    and all(x.kind == "id" or x.text in {"::", "*", "&", "&&", "<", ">", ","}
                            for x in inside)):
                return None
            if t[j].text == ")" and start and t[start - 1].kind == "id":
                start -= 1
            return start, "<indirect-call>"
        if t[j].text in set(OPERATORS) | {"+", "-", "*", "/", "%", "<", ">", "!", "&", "|", "^", "~"}:
            if j and t[j - 1].text == "operator":
                return j - 1, "operator" + t[j].text
        return None

    def is_declaration(self, start, opening, closing):
        t = self.tokens
        after = closing + 1
        while after < len(t) and t[after].text in {"const", "volatile", "override", "final", "noexcept"}:
            after += 1
        if after < len(t) and t[after].text == "->":
            return True
        if after < len(t) and t[after].text in {"{", ":"}:
            # foo(...) { is a definition unless the next group is a brace
            # argument to an outer call. Named function bodies need type/name.
            if start and (t[start - 1].kind == "id" or t[start - 1].text in {"*", "&", "~"}):
                return True
            parts = self.split(opening + 1, closing)
            if parts and all(self.parameter_like(a, b) for a, b in parts):
                return True
        # Function declarations with default values: types precede the name and
        # each parameter starts with a type, not an evaluated nested call.
        if after < len(t) and t[after].text in {";", "=", "{"} and start:
            prefix = t[start - 1]
            if prefix.kind == "id" and prefix.text not in {"return", "throw", "new", "co_return"}:
                parts = self.split(opening + 1, closing)
                if parts and all(self.parameter_like(a, b) for a, b in parts):
                    return True
        return False

    def parameter_like(self, start, end):
        t = self.tokens
        if start >= end or t[start].kind != "id":
            return False
        j, identifiers = start, []
        while j < end:
            v = t[j].text
            if v == "=":
                break
            if v == "<" and j in self.templates:
                j = self.templates[j] + 1
                continue
            if v == "[" and j in self.groups:
                j = self.groups[j] + 1
                continue
            if t[j].kind == "id":
                identifiers.append(v)
            elif v not in {"::", "*", "&", "&&", "..."}:
                return False
            j += 1
        return len(identifiers) >= 2 or bool(identifiers and identifiers[0] in PURE_TYPES)

    def find_calls(self):
        calls = []
        for i, token in enumerate(self.tokens):
            if token.text != "(" or i not in self.groups:
                continue
            callee = self.callee(i)
            if callee is None:
                continue
            start, name = callee
            closing = self.groups[i]
            calls.append(Call(start, i, closing, name, self.split(i + 1, closing),
                              self.is_declaration(start, i, closing)))
        return calls

    def inert_ranges(self):
        ranges = []
        for c in self.calls:
            if c.name in UNEVALUATED or c.declaration:
                ranges.append((c.opening + 1, c.closing))
        # Lambda body evaluation is deferred; captures are evaluated now. Body
        # findings are scanned independently, but effects don't leak to callers.
        for i, token in enumerate(self.tokens):
            if token.text != "[" or i not in self.groups:
                continue
            before = self.tokens[i - 1].text if i else ""
            if before and before not in {"(", ",", "=", "return", "{", ";", ":", "?"}:
                continue
            j = self.groups[i] + 1
            if j < len(self.tokens) and self.tokens[j].text == "(" and j in self.groups:
                j = self.groups[j] + 1
            while j < len(self.tokens) and self.tokens[j].text in {"mutable", "noexcept", "constexpr"}:
                j += 1
            if j < len(self.tokens) and self.tokens[j].text == "{" and j in self.groups:
                body_end = self.groups[j]
                # Immediately invoked lambdas execute their body in this operand.
                if body_end + 1 >= len(self.tokens) or self.tokens[body_end + 1].text != "(":
                    ranges.append((j, body_end + 1))
        return sorted((a, b) for a, b in ranges if a < b)

    def effects(self, start, end):
        if (start, end) in self._effects_cache:
            return self._effects_cache[start, end]
        effects, i = [], start
        skip = {a: b for a, b in self.inert if start <= a and b <= end}
        while i < end:
            if i in skip:
                i = skip[i]
                continue
            token = self.tokens[i]
            c = self.calls_by_open.get(i)
            if c and not c.declaration and c.closing < end:
                kind = classify_call(c.name, self.path)
                if kind:
                    effects.append(Effect(kind, c.name, c.start))
            if token.text in ASSIGNMENTS or token.text in {"++", "--"}:
                effects.append(Effect("mutation", token.text, i))
            i += 1
        self._effects_cache[start, end] = tuple(effects)
        return self._effects_cache[start, end]

    def emit(self, cls, start, end, label, operands):
        active = [(a, b, self.effects(a, b)) for a, b in operands]
        active = [(a, b, effects) for a, b, effects in active if effects]
        if len(active) < 2:
            return
        effects = sorted({e for _, _, es in active for e in es}, key=lambda e: e.token)
        severity = "HIGH" if any(e.kind != "unknown/stateful" for e in effects) else "MEDIUM"
        first, last = self.tokens[start], self.tokens[end - 1]
        self.findings.append({
            "class": cls, "severity": severity, "path": self.path,
            "line": first.line, "column": first.start - self.source.rfind("\n", 0, first.start),
            "offset": first.start, "end_offset": last.end, "callee_or_operator": label,
            "expression": " ".join(self.source[first.start:last.end].split()),
            "effects": [{"kind": e.kind, "name": e.name, "line": self.tokens[e.token].line} for e in effects],
            "effectful_operands": len(active),
        })

    def binary(self, start, end):
        """Parse precedence of infix operators within one delimiter level.

        Group contents are visited separately. We only split at an infix token
        after an operand; prefix *, &, +/- and declaration pointers aren't binary.
        Semicolons/braces and top-level commas constrain the expression span.
        """
        t, i = self.tokens, start
        mark = start
        while i < end:
            v = t[i].text
            if i in self.pairs and self.pairs[i] > i:
                close = self.pairs[i]
                if v == "{":
                    # Blocks separate statements; a braced initializer remains
                    # an operand but its internal comma list is sequenced.
                    if mark < i:
                        self.binary_ops(mark, i)
                    mark = close + 1
                elif v == "(" and i and t[i - 1].text in CONTROL - {"return", "throw"}:
                    mark = close + 1
                i = close + 1
                continue
            if v in {";", ",", "{" , "}"}:
                if mark < i:
                    self.binary_ops(mark, i)
                mark = i + 1
            i += 1
        if mark < end:
            self.binary_ops(mark, end)

    def binary_ops(self, start, end):
        t, i, operators, have_operand = self.tokens, start, [], False
        while start < end and t[start].text in {"return", "throw", "co_return", "case"}:
            start += 1
        i = start
        while i < end:
            v = t[i].text
            if i in self.pairs and self.pairs[i] > i:
                i = self.pairs[i] + 1
                have_operand = True
                continue
            # Unpaired adjacent angle tokens are shifts; they're sequenced in C++17.
            if v in {"<", ">"} and i + 1 < end and t[i + 1].text == v and t[i].end == t[i + 1].start:
                if have_operand:
                    operators.append((i, i + 2, v + v))
                    have_operand = False
                i += 2
                continue
            if v in PRECEDENCE and have_operand:
                operators.append((i, i + 1, v))
                have_operand = False
            elif v in {"!", "~", "+", "-", "*", "&", "++", "--"} and not have_operand:
                pass
            elif v in {"::", ".", "->", "return", "throw", "case", "co_return"}:
                pass
            elif t[i].kind in {"id", "number", "literal"} or v in {")", "]"}:
                have_operand = True
            i += 1
        if not operators:
            return

        pending = [(start, end, operators)]
        while pending:
            a, b, ops = pending.pop()
            if not ops:
                continue
            prec = min(PRECEDENCE[o[2]] for o in ops)
            candidates = [o for o in ops if PRECEDENCE[o[2]] == prec]
            root = candidates[0] if prec in {2, 3} else candidates[-1]
            left, right, op = root
            if a < left and right < b:
                if op in self.binary_operators:
                    self.emit("B", a, b, op, [(a, left), (right, b)])
                pending.append((a, left, [o for o in ops if o[0] < left]))
                pending.append((right, b, [o for o in ops if o[0] >= right]))

    def scan(self):
        for c in self.calls:
            if not c.declaration and c.name not in UNEVALUATED:
                self.emit("A", c.start, c.closing + 1, c.name, c.arguments)
        self.binary(0, len(self.tokens))
        for a, b in sorted(self.groups.items()):
            if a < b:
                # Function declarations/defaults and sizeof operands aren't run.
                if any(x <= a and b <= y for x, y in self.inert
                       if self.tokens[x].text != "{"):
                    continue
                self.binary(a + 1, b)
        # Calls inside an unevaluated operand/declaration were discovered for
        # structural balance, but cannot produce a runtime finding.
        self.findings = [f for f in self.findings if not any(
            self.tokens[a].start <= f["offset"] < self.tokens[b - 1].end
            for a, b in self.inert if a < b and self.tokens[a].text != "{")]
        unique = {(f["class"], f["offset"], f["end_offset"], f["callee_or_operator"]): f for f in self.findings}
        return sorted(unique.values(), key=lambda f: (f["offset"], f["class"], f["end_offset"]))


def group_for(path):
    parts = Path(path).parts
    return "/".join(parts[:3] if len(parts) > 3 else parts[:2])


def lane_for(group):
    parts = group.split("/")
    return LANES.get(parts[2], "unassigned") if len(parts) > 2 and parts[1] == "Game" else "m1"


def native_view(source, enabled):
    """Select GALAXY_NATIVE branches without expanding includes/macros.

    Other conditions remain an over-approximation (both branches). Directive
    spans come from the lexer so #if text inside a raw literal isn't interpreted.
    Masking preserves every original offset and physical line number.
    """
    directives = []
    tokenize(source, directives)
    pieces, position, active, stack = [], 0, True, []

    def mask(text):
        return "".join("\n" if c == "\n" else " " for c in text)

    def condition(command, rest):
        rest = re.sub(r"/\*.*?\*/|//[^\n]*", "", rest, flags=re.S).strip()
        compact = re.sub(r"\s+", "", rest)
        if command in {"ifdef", "ifndef"} and rest == "GALAXY_NATIVE":
            return enabled if command == "ifdef" else not enabled
        if compact in {"GALAXY_NATIVE", "defined(GALAXY_NATIVE)", "definedGALAXY_NATIVE"}:
            return enabled
        if compact in {"!GALAXY_NATIVE", "!defined(GALAXY_NATIVE)", "!definedGALAXY_NATIVE"}:
            return not enabled
        return None

    for start, end, directive in directives:
        segment = source[position:start]
        pieces.append(segment if active else mask(segment))
        pieces.append(mask(source[start:end]))
        position = end
        parsed = re.match(r"#\s*(\w+)\s*(.*)", directive, re.S)
        if not parsed:
            continue
        command, rest = parsed.groups()
        if command in {"if", "ifdef", "ifndef"}:
            value = condition(command, rest)
            stack.append({"parent": active, "known": value is not None,
                          "taken": value is True})
            active = active and value is not False
        elif command in {"else", "elif"} and stack:
            frame = stack[-1]
            if frame["known"]:
                value = True if command == "else" else condition("if", rest)
                active = frame["parent"] and not frame["taken"] and value is not False
                if value is True:
                    frame["taken"] = True
            else:
                active = frame["parent"]
        elif command == "endif" and stack:
            active = stack.pop()["parent"]
    pieces.append(source[position:] if active else mask(source[position:]))
    return "".join(pieces)


def scan_source(source, path):
    scanner = Scanner(source, path)
    metadata = {"tokens": len(scanner.tokens), "calls": len(scanner.calls),
                "directives_skipped": scanner.directive_count,
                "raw_delimiter_warnings": scanner.errors, "views": ["combined"]}
    if not scanner.errors:
        metadata["delimiter_warnings"] = []
        return scanner.scan(), metadata
    # Native hooks can put two alternative function signatures/open braces
    # before a shared body. They must not be concatenated into one syntax tree.
    findings, warnings = {}, []
    metadata["views"] = ["GALAXY_NATIVE=0", "GALAXY_NATIVE=1"]
    for enabled in (False, True):
        view = Scanner(native_view(source, enabled), path)
        warnings.extend(f"GALAXY_NATIVE={int(enabled)}: {e}" for e in view.errors)
        for site in view.scan():
            key = site["class"], site["offset"], site["end_offset"], site["callee_or_operator"]
            site["views"] = [f"GALAXY_NATIVE={int(enabled)}"]
            if key in findings:
                previous = findings[key]
                if previous["severity"] == "HIGH" or site["severity"] == "MEDIUM":
                    previous["views"].extend(site["views"])
                    continue
                site["views"].extend(previous["views"])
            findings[key] = site
    metadata["delimiter_warnings"] = warnings
    return sorted(findings.values(), key=lambda f: (f["offset"], f["class"])), metadata


def sweep(root, paths):
    root = Path(root).resolve()
    files = set()
    for name in paths:
        p = root / name
        if not p.exists():
            raise ValueError(f"missing input: {p}")
        files.update([p] if p.is_file() else (f for f in p.rglob("*") if f.suffix in SUFFIXES))
    findings, inventory = [], []
    for path in sorted(files):
        raw = path.read_bytes()
        source = raw.decode("utf-8", errors="replace")
        name = str(path.relative_to(root))
        sites, metadata = scan_source(source, name)
        findings.extend(sites)
        inventory.append({"path": name, "sha256": hashlib.sha256(raw).hexdigest(),
                          **metadata, "findings": len(sites)})
    return {"schema_version": 1, "method": "tokenizer", "root": str(root),
            "generated_at": datetime.now(timezone.utc).isoformat(),
            "tool_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
            "paths": list(paths), "files": inventory, "findings": findings}


def totals(report):
    return Counter((f["class"], f["severity"]) for f in report["findings"])


def cross_checks(root, model_root=None):
    fixed = ("src/Game/Enemy/KirairaChain.cpp", "src/Game/Map/Flag.cpp",
             "src/Game/Map/FishGroup.cpp", "src/Game/MapObj/StarPiece.cpp",
             "src/Game/Map/WaterPlant.cpp", "src/Game/Util/MathUtil.cpp")
    checks = []
    for name in fixed:
        p = Path(root) / name
        if not p.is_file():
            checks.append({"path": name, "status": "unavailable"})
            continue
        sites, _ = scan_source(p.read_text(errors="replace"), name)
        repeated = [f for f in sites if sum(e["kind"] == "RNG" for e in f["effects"]) > 1]
        checks.append({"path": name, "status": "PASS" if not repeated else "FAIL",
                       "check": "no repeated-RNG expression after named-temporary fixes",
                       "unexpected_sites": repeated})
    model = {"status": "not requested", "sites": []}
    if model_root:
        path = Path(model_root).resolve() / "src/JSystem/JParticle/JPAParticle.cpp"
        if not path.is_file():
            model = {"status": "unavailable", "path": str(path), "sites": []}
        else:
            source = path.read_text(errors="replace")
            anchor = source.find("void JPABaseParticle::init_c(")
            line = source[:anchor].count("\n") + 1 if anchor >= 0 else len(source.splitlines()) + 1
            model_sites, _ = scan_source(source, str(path))
            sites = [f for f in model_sites
                     if f["line"] >= line and f["class"] == "A"
                     and f["callee_or_operator"] in {"rnd", "base_vel"}]
            model = {"status": "PASS" if len(sites) == 2 and all(
                f["severity"] == "HIGH" for f in sites) else "FAIL",
                     "path": str(path), "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
                     "sites": sites}
    return {"fixed_game_sites": checks, "model_init_c": model}


def render(report):
    counts = totals(report)
    command = "python3 tools/port/argorder_sweep.py --root " + shlex.quote(report["root"]) + " "
    command += " ".join(shlex.quote(p) for p in report["paths"])
    if report.get("model_root"):
        command += " --model-root " + shlex.quote(report["model_root"])
    command += " --output docs/port/ARG_ORDER.md --json-output /tmp/argorder.json"
    lines = ["# Argument evaluation order sweep", "",
             "Generated candidate inventory; no source fixes or native behavior claims.", "",
             f"Source root: `{report['root']}`. All relative source locations below refer to this snapshot. Generated {report.get('generated_at', 'unknown time')}; per-file and tool hashes are retained in JSON.", "",
             "```sh", command, "```", "",
             "## Method and review rules", "",
             "The native syntax baseline fails most translation units (docs/port/COMPILE_STATUS.md); this tool uses a C/C++ tokenizer, balanced (), [] and {}, template-id heuristics, top-level argument splitting and binary-operator precedence. It does not require a native build or dependencies.", "",
             "Class A: two or more different arguments contain a classified effect. Parenthesized constructors, explicit operator calls, indirect calls and unexpanded macro invocations are included. C++17 function arguments are indeterminately sequenced, but their order remains unspecified. Class B: two operands of a potentially unsequenced binary operator contain effects. A containing B can appear in both classes; totals are sites, not unique bugs.", "",
             "HIGH sources: RNG (including JPA get_r_* / get_rndm_*), read/pop/next/consume/extract, ++/-- and assignment. Any HIGH source plus an effect in a second operand makes a HIGH candidate. Unknown calls are MEDIUM, including getters not on the explicit reviewed-pure allowlist in the tool. Distinct objects may be independent: verify shared state/aliasing before fixing. Known-pure value queries and scalar/vector casts do not count as effects themselves; effects in their arguments still count. Random-parameter getters such as getScaleRndm are values, not RNG draws.", "",
             "Brace initializer clauses are ordered and do not create A sites. Their expressions can still contain an A/B site, and an entire braced argument can reorder with another argument. Logical &&/||, comma, conditional branch selection, assignment RHS/LHS and shifts are not B sites (C++17). Deferred lambda bodies do not contribute to their caller; bodies are scanned separately. sizeof/decltype/noexcept operands and function declarations are skipped.", "",
             "Language references: [function argument sequencing](https://eel.is/c++draft/expr.call#8), [ordered brace initializer clauses](https://eel.is/c++draft/dcl.init.list#4), [execution sequencing](https://eel.is/c++draft/intro.execution). The native audit uses C++17. For .c files the scanner conservatively also reports assignment and shift operand pairs as B; check whether each native build compiles these as C or C++. Initializer-list sequencing in C and MWCC-era language modes requires separate review.", "",
             "Limits: no type/alias/overload resolution, macro expansion or includes. Conditional branches are combined conservatively; where that unbalances delimiters, separate GALAXY_NATIVE=0/1 views are scanned and merged. Unknown guards remain combined, not exhaustive build configurations. Macro replacement definitions are skipped; invocation arguments are candidates, whose actual expansion needs review. Template vs relational < ambiguity, complex declarators, user-defined casts, implicit operator calls/conversions/destructors and type-dependent unevaluated contexts need AST/manual follow-up. Read-vs-write pairs with only one effectful operand are outside this requested two-effect sweep. Const does not alone prove purity; do not broadly exclude get*/is*/calc*.", "",
             "Reviewed read exclusions: PetariNative::readU16BE/readU24BE/readU32BE only assemble bytes (native/include/petari/endian.hpp:5). readReg is excluded only inside src/JSystem/JAudio2/JASSeqParser.cpp:213; it delegates to value queries (src/JSystem/JAudio2/JASRegisterParam.cpp:70; libs/JSystem/include/JSystem/JAudio2/JASTrack.hpp:144; JASTrackPort.hpp:17). Other read* calls remain HIGH. getNerveStep/getEnd and JPA parameter getters are known value queries; unlisted getters stay MEDIUM.", "",
             "Fix only after reading original target asm: `python3 tools/team/sg.py diff <unit> <symbol>` (this helper builds; use `nice -n 10`, at most six jobs, and check disk space first). Introduce named temporaries in the observed DOL order, then pass those values to the original expression. Do not assume every function evaluates right-to-left or reorder FP arithmetic. Preserve the Wii match; coordinate any native-only guarded fix with the owning lane. This sweep edits no src/ or libs/ files.", "",
             "## Totals", "", f"Scanned **{len(report['files'])} files**; **{len(report['findings'])} candidate sites**.", "",
             "| Class | HIGH | MEDIUM | Total |", "| --- | ---: | ---: | ---: |"]
    for cls in ("A", "B"):
        h, m = counts[cls, "HIGH"], counts[cls, "MEDIUM"]
        lines.append(f"| {cls} | {h} | {m} | {h + m} |")
    primary = [f for f in report["findings"] if not f["path"].startswith("src/RVL_SDK/")]
    sdk = [f for f in report["findings"] if f["path"].startswith("src/RVL_SDK/")]
    lines += ["", f"Primary scope: **{len(primary)} sites**, including **{sum(f['severity'] == 'HIGH' for f in primary)} HIGH**. Optional SDK scope: **{len(sdk)} sites**, including **{sum(f['severity'] == 'HIGH' for f in sdk)} HIGH**. SDK C assignment/shift and unexpanded-macro candidates are conservative; HIGH does not establish shared-state interference or require a fix."]
    groups = defaultdict(Counter)
    for entry in report["files"]:
        groups[group_for(entry["path"])]["files"] += 1
    for f in report["findings"]:
        groups[group_for(f["path"])][f["class"], f["severity"]] += 1
    lines += ["", "## Directory ownership", "",
              "| Directory | Lane | Files | A HIGH | A MEDIUM | B HIGH | B MEDIUM |",
              "| --- | --- | ---: | ---: | ---: | ---: | ---: |"]
    for group, c in sorted(groups.items()):
        lines.append(f"| {group} | {lane_for(group)} | {c['files']} | {c['A', 'HIGH']} | {c['A', 'MEDIUM']} | {c['B', 'HIGH']} | {c['B', 'MEDIUM']} |")
    warnings = sum(bool(f["delimiter_warnings"]) for f in report["files"])
    lines += ["", f"Delimiter warnings in **{warnings} files**; full file hashes, warnings and all MEDIUM/HIGH sites are in the JSON output. Scope: {', '.join(report['paths'])}; includes .inc fragments. RVL_SDK is optional via --include-sdk (included here only if listed). Source hashes identify this scan despite concurrent lane edits.", "",
              "## Full HIGH inventory", "", "Each line is one site; reasons name the classified effects. Candidate rank is RNG, stream/iterator, mutation; class A before B within each rank.", ""]
    for f in high_sites(report):
        reasons = ", ".join(dict.fromkeys(e["kind"] + ":" + e["name"] for e in f["effects"]))
        expr = f["expression"].replace("`", "'")
        lines.append(f"- **{f['class']}** `{f['path']}:{f['line']}` ({lane_for(group_for(f['path']))}; {reasons}) — `{expr}`")
    if "cross_checks" in report:
        checks = report["cross_checks"]
        lines += ["", "## Regression cross-checks", "",
                  "The named-temporary fixes below contain no remaining expression with repeated RNG effects. This checks the targeted hazard, not every possible call/alias in each file.", ""]
        for check in checks["fixed_game_sites"]:
            lines.append(f"- `{check['path']}`: **{check['status']}**.")
        model = checks["model_init_c"]
        lines += ["", f"Read-only MacioGalaxy JPABaseParticle::init_c cross-check: **{model['status']}**. Model findings are excluded from the SMG2 totals above.", ""]
        for f in model["sites"]:
            lines.append(f"- **A HIGH** `{f['path']}:{f['line']}` — `{f['expression']}`")
    return "\n".join(lines) + "\n"


def high_sites(report):
    def priority(f):
        kinds = {e["kind"] for e in f["effects"]}
        return (0 if "RNG" in kinds else 1 if "stream/iterator" in kinds else 2,
                f["class"], f["path"], f["line"], f["column"])
    return sorted((f for f in report["findings"] if f["severity"] == "HIGH"), key=priority)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("paths", nargs="*", help="files/directories relative to root")
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--include-sdk", action="store_true")
    parser.add_argument("--model-root", type=Path, help="optional read-only MacioGalaxy regression check")
    parser.add_argument("--output", type=Path, help="Markdown inventory")
    parser.add_argument("--json-output", type=Path, help="full machine-readable inventory")
    args = parser.parse_args()
    paths = args.paths or list(DEFAULT_PATHS)
    if args.include_sdk:
        paths = [*paths, "src/RVL_SDK"]
    report = sweep(args.root, paths)
    report["model_root"] = str(args.model_root.resolve()) if args.model_root else None
    report["cross_checks"] = cross_checks(args.root, args.model_root)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(render(report))
    if args.json_output:
        args.json_output.parent.mkdir(parents=True, exist_ok=True)
        args.json_output.write_text(json.dumps(report, indent=2) + "\n")
    counts = totals(report)
    print(f"Scanned {len(report['files'])} files: {len(report['findings'])} sites; "
          + "; ".join(f"{cls} {severity}={counts[cls, severity]}"
                      for cls in ("A", "B") for severity in ("HIGH", "MEDIUM")))
    for f in high_sites(report)[:20]:
        print(f"{f['class']} HIGH {f['path']}:{f['line']} {f['expression']}")


if __name__ == "__main__":
    main()
