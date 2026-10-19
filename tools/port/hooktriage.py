#!/usr/bin/env python3
"""Classify unapplied hooktransfer groups and locate SMG2 work in an overlay.

Read-only with respect to game sources and the model repository. Decisions are
planning judgments, not approval to alter the Wii build or an objdiff proof.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from functools import lru_cache
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import subprocess

from argorder_sweep import CONTROL, delimiters, tokenize


KINDS = {"manual", "unconditional", "missing"}
DECISIONS = ("drop-smg1", "drop-fixed", "port-clang", "port-native", "unsure")
SOURCE_EXTS = {".cpp", ".c", ".hpp", ".h", ".inc", ".cc"}
HERE = Path(__file__).resolve().parents[2]

# These are reviewed file/feature absences, checked against the overlay index
# before dropping. A missing path alone is never an absence rule.
SMG1_FILES = {
    "PrologueDirector", "PeachLetter", "PrologueLetter", "ProloguePictureBook",
    "PictureBookLayout", "AstroDomeBlueStar", "MiniatureGalaxy", "GalaxyMap",
    "GalaxyMapController", "GalaxyMapSelectButton", "KinopioAstro", "LuigiLetter",
    "GalaxyCometScheduler", "GameSequenceProgress", "StorySequenceExecutor",
}
DEFERRED = re.compile(
    r"petari/(?:editor/|mods\.hpp|skip\.hpp)|PetariEditor|petariMod|ModGalaxy|mod_galaxy|"
    r"CutsceneSkip|PETARI_SKIP_|PhotoMode|EditMode|nativeCurrentUserFile|nativeTemporaryInGalaxy|"
    r"nativeAcceptGalaxy|HideHud|editorReload|nativeRemove|PETARI_GREY_RGBA|"
    r"GenericFigure|MarioCappy|odyssey_move|petari_mod_enabled|PetariNative::Odyssey")
NATIVE = re.compile(
    r"GALAXY_NATIVE|JSU_BE|JUT_RESTIMG_OFFSET|GXSETARRAY|DSP_SHARED_|DSP_MAIL_ADDR|JASDspAddr|"
    r"JKR_YAZ0_LENGTH|JKR_DECOMP_|JKR_UNIT_HEAP_|JKR_ARC_TOLOWER|J3D_ROT_SHIFT|"
    r"petari_|PetariNative|NativeUiObserve|NativeActorObserve|NATIVE_MILESTONE|TRACE_BOOT|"
    r"TITLE_SEQUENCE_MILESTONE|FILE_SELECTOR_MILESTONE|JASBNKPointsToHost|THP_STEREO_SAMPLE_BYTES|readDLU16")
CLANG = re.compile(
    r"uintptr_t|intptr_t|size_t|GravityHostID|JASAllocationSize|RFL_HEADER_|va_end|va_list|"
    r"this->|\btypename\b|template\s+set|static_cast|sizeof\(|\bnullptr\b|\boverride\b|"
    r"LoadCallback|&\w+::\w+|JASPtrTable<")

# Reviewed migrations and replacements: scope these by path, not by a global
# method basename. They are refreshed from symbols/definitions on each run.
FILE_ALIASES = {
    "src/Game/Animation/XanimeResource.cpp": "src/Game/Animation/XanimeResourceTable.cpp",
    "src/Game/AreaObj/ChangeBgmCube.cpp": "src/Game/AreaObj/BgmChangeArea.cpp",
    "src/Game/System/MessageHolder.cpp": "src/Game/System/MarioFaceShipEventDataHolder.cpp",
    "src/Game/Util/FurShader.cpp": "src/Game/Util/CShader.cpp",
    "src/Game/Player/PlayerEvent.cpp": "src/Game/Player/EventSequence.cpp",
    "src/Game/Player/MarioShadow.cpp": "src/Game/Player/CollisionShadow.cpp",
    "src/Game/System/GameDataGalaxyStorage.cpp": "src/Game/System/GameDataAllGalaxyStorage.cpp",
    "src/Game/Player/MarioActorDraw.cpp": "src/Game/Player/MarioActor.cpp",
    "src/Game/Player/MarioActorParts.cpp": "src/Game/Player/MarioActor.cpp",
    "src/Game/Player/MarioActorShadow.cpp": "src/Game/Player/MarioActor.cpp",
    "src/Game/Player/MarioCollision.cpp": "src/Game/Player/Mario.cpp",
    "src/Game/Player/MarioJump.cpp": "src/Game/Player/Mario.cpp",
    "src/Game/Player/MarioSound.cpp": "src/Game/Player/Mario.cpp",
    "src/Game/Player/MarioWalk.cpp": "src/Game/Player/Mario.cpp",
}

FUNCTION_ALIASES = {"ChangeBgmCube::movement": "BgmChangeArea::movement"}


def digest(text):
    return hashlib.sha256(text.encode("utf-8", errors="surrogateescape")).hexdigest()


def source_tokens(text):
    return [t.text for t in tokenize(text)[0]]


def meaningful(text):
    """Discard guards/comments from a signed group, retaining semantic payload."""
    lines = []
    for line in text.splitlines():
        line = line.strip()
        if re.match(r"#\s*(?:if|else|elif|endif)\b", line):
            continue
        lines.append(line)
    return "\n".join(lines)


def changed(group, sign):
    return "\n".join(line[1:] for line in group["text"].splitlines()
                     if line.startswith(sign) and not line.startswith(sign * 3))


def branch_view(source, native, mw, other=False):
    """Position-preserving simple compiler/signature branch selection.

    Unknown complex expressions remain an over-approximation. Simple optional
    inlining flags are selected in both views to avoid concatenating signatures.
    """
    directives = []
    tokenize(source, directives)
    pieces, position, active, stack = [], 0, True, []

    def mask(s):
        return "".join("\n" if c == "\n" else " " for c in s)

    def condition(command, rest):
        rest = re.sub(r"/\*.*?\*/|//[^\n]*", "", rest, flags=re.S).strip()
        if rest in {"0", "1"} and command == "if":
            return rest == "1"
        if command in {"ifdef", "ifndef"} and re.fullmatch(r"\w+", rest):
            value = {"GALAXY_NATIVE": native, "__MWERKS__": mw}.get(rest, other)
            return value if command == "ifdef" else not value
        m = re.fullmatch(r"(!?)\s*defined\s*\(?\s*(\w+)\s*\)?", rest)
        if m:
            value = {"GALAXY_NATIVE": native, "__MWERKS__": mw}.get(m[2], other)
            return not value if m[1] else value
        return None

    for start, end, directive in directives:
        pieces.append(source[position:start] if active else mask(source[position:start]))
        pieces.append(mask(source[start:end]))
        position = end
        parsed = re.match(r"#\s*(\w+)\s*(.*)", directive, re.S)
        if not parsed:
            continue
        command, rest = parsed.groups()
        if command in {"if", "ifdef", "ifndef"}:
            value = condition(command, rest)
            stack.append({"parent": active, "known": value is not None, "taken": value is True})
            active = active and value is not False
        elif command in {"else", "elif"} and stack:
            frame = stack[-1]
            if frame["known"]:
                value = True if command == "else" else condition("if", rest)
                active = frame["parent"] and not frame["taken"] and value is not False
                frame["taken"] |= value is True
            else:
                active = frame["parent"]
        elif command == "endif" and stack:
            active = stack.pop()["parent"]
    pieces.append(source[position:] if active else mask(source[position:]))
    return "".join(pieces)


def functions(source):
    """Index spelled definitions, including indented namespace functions.

    Tokens prevent comment/string matches; braces determine enclosing ranges.
    The two common native/Wii alternative-signature forms are indexed in views.
    This is a locator, not C++ overload/type resolution.
    """
    tokens, _ = tokenize(source)
    groups, errors = delimiters(tokens)
    if errors:
        found = {}
        for native, mw, other in ((True, False, False), (False, True, False), (False, True, True)):
            view = branch_view(source, native, mw, other)
            ts, _ = tokenize(view)
            gs, es = delimiters(ts)
            for f in _functions(ts, gs):
                found[f["name"], f["start"]] = f
        return sorted(found.values(), key=lambda f: f["start"])
    return _functions(tokens, groups)


def _functions(tokens, groups):
    namespaces, result = [], []
    for i, t in enumerate(tokens[:-1]):
        if t.text == "namespace":
            j, names = i + 1, []
            while j < len(tokens) and (tokens[j].kind == "id" or tokens[j].text == "::"):
                names.append(tokens[j].text)
                j += 1
            if j < len(tokens) and tokens[j].text == "{" and j in groups:
                namespaces.append((j, groups[j], "".join(names)))
        if t.text in {"class", "struct"} and tokens[i + 1].kind == "id":
            j = i + 2
            if j < len(tokens) and tokens[j].text == "<":
                depth = 0
                while j < len(tokens):
                    depth += tokens[j].text == "<"
                    depth -= tokens[j].text.count(">") if tokens[j].text in {">", ">>"} else 0
                    j += 1
                    if depth == 0:
                        break
            if j < len(tokens) and tokens[j].text in {"{", ":", "final"}:
                while j < len(tokens) and tokens[j].text not in {"{", ";"}:
                    j += 1
                if j < len(tokens) and tokens[j].text == "{" and j in groups:
                    namespaces.append((j, groups[j], tokens[i + 1].text))
    for i, token in enumerate(tokens):
        if token.text != "(" or i not in groups or not i:
            continue
        end, prev = groups[i], i - 1
        if tokens[prev].kind != "id" or tokens[prev].text in CONTROL | {"sizeof", "alignof", "noexcept", "decltype"}:
            continue
        start = prev
        while start >= 2 and tokens[start - 1].text == "::" and tokens[start - 2].kind == "id":
            start -= 2
        if start and tokens[start - 1].text == "~":
            start -= 1
        name = "".join(t.text for t in tokens[start:i])
        j = end + 1
        while j < len(tokens) and tokens[j].text in {"const", "volatile", "override", "final", "NO_INLINE", "noexcept"}:
            j += 1
        if j < len(tokens) and tokens[j].text == ":":
            j += 1
            while j < len(tokens):
                if tokens[j].text == "{":
                    break
                while j < len(tokens) and tokens[j].text not in {"(", "{"}:
                    j += 1
                if j not in groups:
                    break
                j = groups[j] + 1
                if j < len(tokens) and tokens[j].text == ",":
                    j += 1
                    continue
                break
        if j >= len(tokens) or tokens[j].text != "{" or j not in groups:
            continue
        prefix = "::".join(n for a, b, n in sorted(namespaces) if a < i < b and n)
        bare_ctor = bool(prefix and name.lstrip("~") == prefix.split("::")[-1])
        if start and (tokens[start - 1].text in {".", "->", "return", "=", "new", ","} or
                      tokens[start - 1].text == ":" and not bare_ctor):
            continue
        # Bare macro calls followed by a block are not function definitions.
        if "::" not in name and (not start or tokens[start - 1].text in {";", "{", "}", ")"}):
            continue
        if prefix and "::" not in name:
            name = prefix + "::" + name
        result.append({"name": name, "line": tokens[start].line,
                       "end_line": tokens[groups[j]].line,
                       "start": tokens[start].start, "end": tokens[groups[j]].end})
    return result


class Overlay:
    def __init__(self, root):
        self.root = Path(root).resolve()
        self.files, self.by_name, self.basenames = {}, defaultdict(list), defaultdict(list)
        for folder in ("src", "include", "libs"):
            if not (self.root / folder).exists():
                continue
            for path in sorted((self.root / folder).rglob("*")):
                if path.suffix not in SOURCE_EXTS or not path.is_file():
                    continue
                name = str(path.relative_to(self.root))
                text = path.read_text(errors="surrogateescape")
                spans = functions(text)
                self.files[name] = {"text": text, "functions": spans, "sha256": digest(text)}
                self.basenames[path.name].append(name)
                for f in spans:
                    self.by_name[f["name"]].append(dict(f, path=name))

    def has_class_definitions(self, name):
        return any(n.startswith(name + "::") for n in self.by_name)

    @lru_cache(maxsize=None)
    def active(self, path):
        return branch_view(self.files.get(path, {}).get("text", ""), True, False)

    def locate(self, original_path, function, hint=None, base_text="", base_line=1, old="", new="", signature=""):
        paths = [p for p in (hint, original_path, FILE_ALIASES.get(original_path)) if p in self.files]
        if not paths:
            paths = self.basenames.get(Path(original_path).name, [])
        if paths and not old.strip() and Path(original_path).stem + "::" in new:
            additions = [f for f in functions(new) if f["name"].startswith(Path(original_path).stem + "::")]
            if additions and self.has_class_definitions(Path(original_path).stem):
                path, text = paths[0], self.files[paths[0]]["text"]
                return {"path": path, "line": max(1, len(text.splitlines())),
                        "function": additions[0]["name"] + " (new)", "precision": "insertion",
                        "start": 0, "end": len(text)}
        candidates = self.by_name.get(FUNCTION_ALIASES.get(function, function), []) if function else []
        # A bare header/local helper name is not globally unique. Do not select
        # an unrelated class's dot()/init()/getCount() from another file.
        if "::" not in function:
            candidates = [c for c in candidates if c["path"] in paths]
        if candidates:
            def score(c):
                text = self.files[c["path"]]["text"][c["start"]:c["end"]]
                payload = " ".join(source_tokens(meaningful(old)))
                sig = set(source_tokens(signature))
                candidate_sig = set(source_tokens(text.split("{", 1)[0]))
                similarity = len(sig & candidate_sig) / max(1, len(sig | candidate_sig))
                return (c["path"] in paths, bool(payload and payload in " ".join(source_tokens(text))),
                        similarity, c["path"].startswith("src/"), -c["line"])
            c = max(candidates, key=score)
            return {"path": c["path"], "line": c["line"], "function": c["name"],
                    "precision": "function", "start": c["start"], "end": c["end"]}
        if not paths:
            return None
        path = paths[0]
        text = self.files[path]["text"]
        if not function and not old.strip() and not source_tokens(new):
            return {"path": path, "line": 1, "function": "<file-scope>",
                    "precision": "file-scope", "start": 0, "end": len(text)}
        # Use a substantial changed line or nearby Petari context. Avoid braces,
        # directives and generic return/zero fragments as location evidence.
        context = base_text.splitlines()
        pool = [*old.splitlines(), *new.splitlines()]
        pool += context[max(0, base_line - 4):base_line + 3]
        normalized_lines = defaultdict(list)
        for n, line in enumerate(text.splitlines(), 1):
            normalized_lines[" ".join(line.split())].append(n)
        for line in pool:
            key = " ".join(line.split())
            if len(key) < 18 or key.startswith(("#", "//", "/*")):
                continue
            hits = normalized_lines.get(key, [])
            if len(hits) == 1:
                n = hits[0]
                enclosing = [f for f in self.files[path]["functions"] if f["line"] <= n <= f["end_line"]]
                f = min(enclosing, key=lambda f: f["end_line"] - f["line"]) if enclosing else None
                return {"path": path, "line": n, "function": f["name"] if f else "<file-scope>",
                        "precision": "context", "start": f["start"] if f else 0,
                        "end": f["end"] if f else len(text)}
        if not function or original_path.startswith(("include/", "libs/")):
            return {"path": path, "line": 1, "function": "<file-scope>",
                    "precision": "file-scope", "start": 0, "end": len(text)}
        return None


class Reference:
    def __init__(self, root, base):
        self.root, self.base = Path(root).expanduser(), base
        self.errors = []

    @lru_cache(maxsize=None)
    def read(self, path):
        result = subprocess.run(["git", "-C", str(self.root), "show", f"{self.base}:{path}"],
                                capture_output=True)
        if result.returncode:
            self.errors.append(path)
            return ""
        return result.stdout.decode("utf-8", errors="surrogateescape")

    @lru_cache(maxsize=None)
    def spans(self, path):
        return functions(self.read(path))

    def enclosing(self, path, line, fallback):
        spans = [f for f in self.spans(path) if f["line"] <= line <= f["end_line"]]
        if spans:
            return min(spans, key=lambda f: f["end_line"] - f["line"])["name"]
        # Diff context is frequently a previous function, including FORCE_MATCH
        # helpers. Without an enclosing definition this is a file-scope insert.
        return ""

    def signature(self, path, line):
        spans = [f for f in self.spans(path) if f["line"] <= line <= f["end_line"]]
        if not spans:
            return ""
        f = min(spans, key=lambda f: f["end_line"] - f["line"])
        return self.read(path)[f["start"]:f["end"]].split("{", 1)[0]


def load_lanes(path):
    spec = importlib.util.spec_from_file_location("hooktriage_lanes", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.lane_of


def groups(report):
    seen = Counter()
    for file in report["files"]:
        for index, group in enumerate(file["groups"]):
            if group["kind"] not in KINDS:
                continue
            key = file["path"] + ":" + digest(group["text"])[:12]
            seen[key] += 1
            yield dict(group, path=file["path"], smg2_path=file.get("smg2_path"),
                       id=f"{key}:{seen[key]}", group_index=index)


def reviewed(group, overlay, target, function):
    """Source-checked exceptions from the ht2 manual review, without line keys.

    Narrow text hashes are used where multiple unrelated changes share a file.
    Broader path rules additionally require the reviewed semantic feature and
    replacement-source evidence. Changed/missing evidence invalidates a drop.
    """
    path, text = group["path"], group["text"]
    key = digest(text)[:12]

    def accept(decision, reason, source, *needles):
        content = overlay.active(source)
        if not content or not all(n in content for n in needles):
            return None
        evidence = [{"path": source, "line": content[:content.index(n)].count("\n") + 1,
                     "text": n} for n in needles]
        return {"decision": decision, "reason": reason, "method": "reviewed", "evidence": evidence}

    if path in {"include/Game/Util/HashUtil.hpp", "src/Game/Util/HashUtil.cpp"} and (
            key in {"1f220be00b24", "d5e6dce805d3", "4884764b4b63", "d3c0b738f3c0",
                    "2d490481ebf6", "c410cadda0ce", "35483837cafd", "285e04d28f5b",
                    "caa0e8cf3ec7", "3cf0b41bf2a1"}):
        source = "include/Game/Util/HashUtil.hpp"
        if all(n in overlay.active("src/Game/Util/HashUtil.cpp") for n in
               ("new uintptr_t[cnt]", "uintptr_t swapArray", "uintptr_t* a2")):
            return accept("drop-fixed", "SMG2 already has uintptr_t payload storage/search and addPtr/searchPtr; retain its API.",
                          source, "uintptr_t* mPayloads", "searchPtr", "addPtr")
    if path == "include/Game/Player/Mario.hpp":
        if key in {"08de906e6672", "f28a4f2f49ee"}:
            return accept("drop-fixed", "SMG2 MovementStates/DrawStates use explicit integer words/masks, so no host bitfield reversal is needed.",
                          path, "u32 mLowWord", "u32 mHighWord", "u32 mWord")
        if key == "53d3a97b1c89":
            return accept("drop-fixed", "SMG2 already types tornado centering as HitSensor* mTornadoCenterSensor; do not add the SMG1 _A38 slot.",
                          path, "HitSensor* mTornadoCenterSensor")
    if path == "include/Game/Player/J3DModelX.hpp" and key == "01e950837f6c":
        return accept("drop-fixed", "SMG2 uses the integer mDisplayListMask instead of the SMG1 flag bitfield; no reversal is needed.",
                      path, "u32 mDisplayListMask")
    if path == "include/Game/Player/MarioActor.hpp" and key == "9bed8fcfe814" and target:
        return accept("port-native", "Adapt all seven SMG2 effect flags (including mIsStationary/mBeeWind/mSpecialSmoke) to mEffectFlagsWord host bit order; do not copy SMG1's six-bit layout.",
                      path, "mEffectFlagsWord", "mIsStationary", "mBeeWind", "mSpecialSmoke")
    if path == "src/Game/Player/MarioJump.cpp" and key == "e9e122fc9c48":
        return accept("drop-fixed", "SMG2 startTornadoCentering already stores pSensor in a real HitSensor* field; no integer pointer cast is needed.",
                      "src/Game/Player/Mario.cpp", "mTornadoCenterSensor = pSensor")
    if path == "include/Game/Player/PlayerEvent.hpp" and key == "cd1022dea087":
        return accept("drop-fixed", "SMG2 addEventSequence already stores a typed EventSequence pointer with addPtr; retain its base-class cast.",
                      path, "mHashTable->addPtr(pName, static_cast< EventSequence* >(new T()), false)")
    if path == "src/Game/Player/PlayerEvent.cpp" and key == "11d351840336":
        return accept("drop-fixed", "SMG2 EventSequencer already uses a uintptr_t search payload and typed EventSequence* conversion; searchPtr is not required.",
                      "src/Game/Player/EventSequence.cpp", "uintptr_t payload", "mHashTable->search(pName, &payload)", "reinterpret_cast< EventSequence* >(payload)")
    if path == "include/Game/Player/RushEndInfo.hpp" and key == "7f7d533dce2a":
        if "__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__" in overlay.files.get(path, {}).get("text", ""):
            return accept("drop-fixed", "SMG2 already selects reversed RushEndInfo fields on little-endian hosts.",
                          path, "u32 _8 : 24;", "u32 mDamageType : 4;")
    vec = {
        "7b0fb9b767bd": ("const f32 x = -rSrc[0]",),
        "66380a28f5ff": ("z = vec2[2] - vec1[2]",),
        "39346623388a": ("const f32 x = vec1[0] * vec2[0]",),
        "7634bc78bef7": ("inline TVec2 addInline(",),
        "4b2dc41c6ff6": ("inline TVec2 addOperatorInline(",),
        "6eade6b27b13": ("inline TVec2 subInline(",),
        "94c28ea10d16": ("inline TVec2 subOperatorInline(",),
        "86e68842833a": ("inline TVec2 scaleInline(",),
        "d1abc00cd42b": ("TVec3(const Vec& vec)", "x = vec.x"),
        "47e2e68aff70": ("TVec3(const TVec3< f32 >& vec)", "x = vec.x"),
        "9722aef25445": ("x = vec.x;", "y = vec.y;", "z = vec.z;"),
        "2eec39d6ce69": ("set< f32 >(mtx[0][3], mtx[1][3], mtx[2][3])",),
        "a1ad95f09d6b": ("void setPSZeroVec()", "x = gZeroVec.x"),
        "61919bfa5332": ("std::fma(x, rOther.x, y * rOther.y) + z * rOther.z",),
        "62f7c2606883": ("std::fma(dx, dx, dy * dy) + dz * dz",),
    }
    if path.endswith("/JGeometry/TVec.hpp") and key in vec:
        return accept("drop-fixed", "The non-MW SMG2 implementation already supplies this scalar/value-return fallback; retain its alias-safe/fused operation order.",
                      path, *vec[key])
    if path.endswith("/JMath/JMath.hpp"):
        checks = {"cef3a014eef6": ("std::sqrt(input)",),
                  "70b406b23ec0": ("std::fma(twiceT", "std::fma(-offset"),
                  "687631ac0083": ("std::memmove(dest, src, 3 * sizeof(f32))",),
                  "e46698fb7726": ("std::memmove(dest, src, 6 * sizeof(f32))",),
                  "36531234129c": ("std::memmove(pDest, pSrc, 12 * sizeof(f32))",),
                  "3152d18f70b3": ("std::memmove(pDest, pSrc, 16 * sizeof(f32))",)}
        if key in checks:
            return accept("drop-fixed", "SMG2 already supplies the non-MW scalar/memmove fallback, including fused Hermite evaluation; keep it.",
                          path, *checks[key])
    if path.endswith("/JMath/JMATrigonometric.hpp") and key in {"7a70e82dbeb5", "dcb63e0cc8c6"}:
        return accept("drop-fixed", "SMG2 already avoids extending std::pair on hosts by storing JMath::TSinCosTableEntry<T>.",
                      path, "struct TSinCosTableEntry", "TSinCosTableEntry<T> table[LEN]")
    if path == "src/Game/Player/MarioSound.cpp" and key in {"95bc3c9b0045", "601162088ebe", "7e78a4815a47"}:
        return accept("drop-fixed", "Mario.cpp stores/reads sound policy as the explicit u8 mPolicy, avoiding the SMG1 union-byte endian bug.",
                      "src/Game/Player/Mario.cpp", "u8 mPolicy", "soundlist[index]._8.mPolicy & 0x3", "soundlist[index]._8.mPolicy & ~0x3")
    if path in {"src/Game/System/MessageHolder.cpp", "include/Game/System/MessageHolder.hpp"} and (
            key in {"73d35149663f", "ec09eb4e2fed", "01a78d89ddb8", "df1f526a112a",
                    "ef93b70fdb05", "a9cc730c4072", "58ff377170cc"}):
        return accept("drop-smg1", "SMG2 uses LMS MSBT/MSBF handles; discard this SMG1 BMG INF1/DAT1 parser/state hook and review LMS endianness separately.",
                      "src/Game/System/MarioFaceShipEventDataHolder.cpp", ".msbt", "MessageData::getNodeDataPtr", "MessageData::getNodeBranchPtr")
    if path.endswith("/TalkMessageCtrl.cpp") and key == "a47bf44b6fc2":
        return accept("drop-smg1", "The RMGK01-to-RMGE01 BMG branch-number conversion does not apply to SB4E01 LMS flow data.",
                      "src/Game/System/MarioFaceShipEventDataHolder.cpp", "MessageData::getNodeDataPtr", ".msbf")
    if path in {"src/Game/GameAudio/AudStageBgmTable.cpp", "src/Game/GameAudio/AudStageBgmWrap.cpp"} and not function:
        return accept("drop-smg1", "Discard SMG1 stage/music sentinel-table edits; SMG2 owns its stage BGM tables in AudBgmParamTable.cpp.",
                      "src/Game/AudioLib/AudBgmParamTable.cpp", "AudStageBgmWrap", "changeStageNameToSoundID")
    if path == "src/Game/AreaObj/ChangeBgmCube.cpp" and function == "ChangeBgmCube::movement":
        return accept("drop-fixed", "BgmChangeArea::movement already keeps locals within conditional blocks or before the switch; the SMG1 crossing-scope fix is redundant.",
                      "src/Game/AreaObj/BgmChangeArea.cpp", "s32 arg1 = mObjArg1", "switch (mObjArg0)", "if (arg1 < 0)")
    if path.endswith("/GalaxyNamePlate.cpp") and key == "74d3a713bdc5":
        return accept("drop-fixed", "SMG2 already uses TxtGalaxyName/TxtGalaxyNameU; the Petari pane-name typo is absent.",
                      path, '"TxtGalaxyName" : "TxtGalaxyNameU"')
    if path.endswith("/SceneNameObjListExecutor.cpp") and re.search(r"TODO.*(?:MovementType|CalcAnimType|DrawBufferType|DrawType)", text):
        return accept("drop-smg1", "SMG2 replaces the SMG1 sentinel/category tables with its own raw category IDs; do not port Petari table values.",
                      path, "cMovementListInitTable", "{0x00, 1}")
    if path == "src/Game/Player/MarioActor.cpp" and function == "MarioActor::calcAnimInMovement" and "PlayerMode_" in text and target and "switch" not in overlay.files[target["path"]]["text"][target["start"]:target["end"]]:
        return accept("drop-smg1", "SMG1 transformation-explanation calls are absent from SMG2's MarioActor; do not transplant their switch cases.",
                      path, "MarioActor::calcAnimInMovement", "calcSpinEffect()")
    if path.endswith("/DrawBufferExecuter.cpp") and key == "0b479aca7f1b":
        return accept("drop-smg1", "This count substitution only completes the deferred editor allocation-headroom hook; retain SMG2's existing count.",
                      path, "mDrawBuffer->init(mDrawBufferCount)")
    if path.endswith("/JAUSoundAnimator.hpp") and key == "41be8f526879" and target:
        return {"decision": "port-clang", "method": "reviewed", "reason": "Explicitly construct JAISoundID from the native JSU_BE numeric field; clang cannot chain both user conversions."}
    if path.endswith("/JAUSoundTable.hpp") and key == "daf72fa6b931" and target:
        return {"decision": "port-native", "method": "reviewed", "reason": "Read the root offset through the JSU_BE header introduced by the preceding group; port both groups together."}
    if path.endswith("/CameraAnim.cpp") and "const CanmValue* values" in text and target:
        return {"decision": "port-clang", "method": "reviewed", "reason": "Keep the CanmValue endian-wrapper type when taking mValues element pointers; the current f32* conversion fails on hosts."}
    if path.endswith("/JAISoundHandles.cpp") and key == "4fe5cb3a6a4e" and target:
        return {"decision": "port-clang", "method": "reviewed", "reason": "Remove the narrowing u32 user-data comparison; the SMG2 signature already accepts uintptr_t."}
    if path.endswith("/mem_allocator.c") and key in {"21fbcdd14e39", "9b619d55523e"} and target:
        return {"decision": "port-clang", "method": "reviewed", "reason": "Cast allocator pHeap to MEMHeapHandle at the SDK call boundary; preserve the Wii API and allocation semantics."}
    if path.endswith("/MathUtil.cpp") and key == "9a02f26e0e7e" and target:
        return {"decision": "port-native", "method": "reviewed", "reason": "Implement the currently empty non-MW PSVECKillElement branch; preserve paired-single rounding with the port math helpers."}
    if path.endswith("/J3DAnimation.cpp") and key == "1e0fae656685":
        return accept("drop-fixed", "SMG2 already forwards the non-MW s16 Hermite overload to the scalar fused JMAHermiteInterpolation fallback.",
                      path, "return JMAHermiteInterpolation(pp1, *pp2, *pp3, *pp4, *pp5, *pp6, *pp7)")
    if path.endswith("/JMapInfo.cpp") and key == "bd54767b3878" and target:
        return {"decision": "port-clang", "method": "reviewed", "reason": "Split valuePtr declaration/assignment so goto FAIL cannot jump across its initialization; preserve the s32 overload's flow."}
    if path.endswith("/KCollision.cpp") and key == "bca5c33bbd9c" and target:
        return {"decision": "port-clang", "method": "reviewed", "reason": "Split distance declaration/assignment so earlier goto success edges do not cross an initialized local."}
    if path.endswith("/J2DPane.cpp") and key == "0d04de7ff778" and target:
        return {"decision": "port-clang", "method": "reviewed", "reason": "Supply the missing MSL DEG_TO_RAD macro for host cmath using the original float constant; keep the Wii expansion."}
    if path.endswith("/gx/GXVert.h") and key == "7bf2d2b89ceb" and target:
        return {"decision": "port-clang", "method": "reviewed", "reason": "Declare the GXColor1x8 inline API used by the host renderer through the existing SDK __GXCDEFX macro."}
    if path.endswith("/mem/list.h") and key == "61b9a4e58516" and target:
        return {"decision": "port-clang", "method": "reviewed", "reason": "Use C-compatible stddef/header guards for native C callers; retain the required revolution/types.h dependency."}
    return None


def decide(group, overlay, target, function, feature_helpers=()):
    """Ordered, evidence-carrying rules; never infer a fix from byte similarity."""
    path, text = group["path"], group["text"]
    old, new = changed(group, "-"), changed(group, "+")
    stem = Path(path).stem
    content = overlay.files[target["path"]]["text"] if target else ""
    region = content[target["start"]:target["end"]] if target else ""

    def result(decision, reason, method="rule"):
        return {"decision": decision, "reason": reason, "method": method}

    # Strip comments for feature detection: a diagnostic mentioning an editor
    # is not necessarily editor-only code. Include paths remain significant.
    feature_text = " ".join(source_tokens(old + "\n" + new)) + "\n" + "\n".join(
        line for line in text.splitlines() if re.match(r"[+]\s*#\s*include", line))
    feature_text = re.sub(r"\s*::\s*", "::", feature_text)
    if DEFERRED.search(feature_text):
        return result("drop-smg1", "Deferred editor/mod/SMG1 launch feature; absent from the P1 baseline, retain only on the P3 backlog.")
    calls = set(re.findall(r"\b([A-Za-z_]\w*)\s*\(", new)) - CONTROL
    if calls and calls <= set(feature_helpers):
        replaced = new
        for name in calls:
            replaced = re.sub(r"(?:::)?\b" + re.escape(name) + r"\s*\([^()]*\)", "FEATURE_CALL", replaced)
        if re.fullmatch(r"\s*(?:if\s*\([^{};=]*FEATURE_CALL[^{};=]*\)\s*\{?|FEATURE_CALL\s*;)\s*", replaced):
            return result("drop-smg1", "Calls only helper(s) defined by a deferred feature group in this same input file: " + ", ".join(sorted(calls)) + "; defer the callers together.")
    if stem in SMG1_FILES and not overlay.has_class_definitions(stem):
        return result("drop-smg1", "Reviewed SMG1 story/observatory feature; no class implementation exists in overlay sources (declarations can remain).")
    review = reviewed(group, overlay, target, function)
    if review:
        return review
    # Applied hooks and independent SMG2 repairs can render a conflicted fragment
    # redundant. Only accept substantial replacement text, never a lone brace.
    nt = source_tokens(meaningful(new))
    ot = source_tokens(meaningful(old))
    active_region = overlay.active(target["path"])[target["start"]:target["end"]] if target else ""
    rt = " ".join(source_tokens(active_region))
    n, o = " ".join(nt), " ".join(ot)
    if target and target["precision"] == "function" and len(nt) >= 5 and n in rt and (not ot or o not in rt):
        return result("drop-fixed", "The complete substantive replacement is already present at the resolved SMG2 site.")
    if not target:
        if path.endswith("/THPSimplePlayerWrapper.cpp"):
            return result("unsure", "Overlay has a THP header but no wrapper implementation; identify SMG2's movie pipeline before importing the SMG1 decoder.", "reviewed")
        if path.endswith("/LayoutUtil.cpp"):
            return result("unsure", "The named SMG1 layout API has no overlay definition; map its SMG2 replacement before adding pane-readiness guards.", "reviewed")
        return result("unsure", "No unique SMG2 implementation/anchor found; distinguish a renamed feature from an SMG1-only removal.")
    if path.endswith("/LayoutUtil.cpp") and "nativeLayoutPaneReady" in text:
        return result("port-native", "Add pane-readiness diagnostics/guard at this SMG2 layout operation behind GALAXY_NATIVE.")
    behavior = re.search(r"publish|Observe|observer|MILESTONE|TRACE_BOOT|PetariNative::(?:Platform|HomeMenu)|GXSETARRAY|readDLU16|JSU_BE|JUT_RESTIMG_OFFSET", new)
    if not behavior and re.search(r"uintptr_t|intptr_t|GravityHostID|JASAllocationSize|LoadCallback|\bValue\b", new):
        return result("port-clang", "Preserve pointer-sized host payload/address or callback type; keep the Wii ABI/token behavior unchanged.")
    if not behavior and re.search(r"this->|\btypename\b|template\s+set|JASPtrTable<|&\w+::\w+", new):
        return result("port-clang", "Qualify the dependent base/template/member name for clang without changing Wii behavior.")
    if not NATIVE.search(text) and re.search(r"\bcase\b[^\n]*\{|static_cast|va_end|va_list|RFL_HEADER_|nullptr|CanmValue", new + "\n" + old):
        return result("port-clang", "Repair clang scoping, conversion, varargs or linkage syntax; preserve the SMG2 operation and Wii branch.")
    if NATIVE.search(text):
        if re.search(r"JSU_BE|BigEndian|endian|JUT_RESTIMG_OFFSET|readU\d+BE|JASBNKPointsToHost", text):
            why = "Adapt serialized-resource byte order/host views at the SMG2 site behind GALAXY_NATIVE."
        elif re.search(r"GXSETARRAY|GX_|petari_gx|pipeline", text):
            why = "Port native GX/resource registration and buffer extent handling using SMG2 sizes behind GALAXY_NATIVE."
        elif re.search(r"publish|MILESTONE|milestone|TRACE_BOOT|ui_observ|nativeNoteBigMove", text):
            why = "Port observation/boot telemetry using SMG2 fields and state names behind GALAXY_NATIVE."
        elif re.search(r"Heap|heap|sizeof", text):
            why = "Adapt host-layout allocation/lifetime headroom to SMG2; remeasure sizes rather than copying SMG1 constants."
        elif re.search(r"DSP_|JASDspAddr", text):
            why = "Port native DSP mailbox/shared-state semantics together with their helper definitions."
        else:
            why = "Rebase this guarded native behavior on the resolved SMG2 implementation; retain the Wii path."
        return result("port-native", why)
    if CLANG.search(text) or re.search(r"\+\s*(?:#include|\{|\}|(?:class|struct)\s+\w+;|(?:u32|s32|BOOL|void)\s+\w+\()", text):
        return result("port-clang", "Rebase the declaration/include/scoping or host-size compatibility repair without changing SMG2 semantics.")
    return result("unsure", "Unconditional semantic/data change needs SMG2 source and target-asm review; no safe drop/port rule proves it.")


def triage(report_path, overlay_root, reference_root, lane_path):
    report_path = Path(report_path)
    report = json.loads(report_path.read_text())
    overlay = Overlay(overlay_root)
    reference = Reference(reference_root, report["macio_base"])
    lane = load_lanes(lane_path)
    feature_helpers = defaultdict(set)
    for file in report["files"]:
        for g in file["groups"]:
            if g["kind"] == "feature" and not changed(g, "-").strip():
                for f in functions(changed(g, "+")):
                    if not any(c["path"] == file["path"] for c in overlay.by_name.get(f["name"], [])):
                        feature_helpers[file["path"]].add(f["name"].split("::")[-1])
    results = []
    for g in groups(report):
        base = reference.read(g["path"])
        fn = reference.enclosing(g["path"], g["line"], g.get("function", ""))
        target = overlay.locate(g["path"], fn, g.get("smg2_path"), base, g["line"],
                                changed(g, "-"), changed(g, "+"), reference.signature(g["path"], g["line"]))
        decision = decide(g, overlay, target, fn, feature_helpers[g["path"]])
        if decision["decision"].startswith("port-") and not target:
            raise ValueError(f"Port decision without an overlay target: {g['id']}")
        results.append({**g, "resolved_function": fn, "target": target,
                        "lane": lane(target["path"] if target else g["path"]), **decision})
    return {"schema_version": 1, "input": str(report_path.resolve()),
            "input_sha256": hashlib.sha256(report_path.read_bytes()).hexdigest(),
            "overlay": str(overlay.root), "reference": str(reference.root),
            "macio_base": report["macio_base"], "lane_map": str(Path(lane_path).resolve()),
            "lane_map_sha256": hashlib.sha256(Path(lane_path).read_bytes()).hexdigest(),
            "reference_errors": reference.errors, "groups": results,
            "deferred_helpers": {p: sorted(names) for p, names in feature_helpers.items() if names},
            "source_hashes": {p: f["sha256"] for p, f in overlay.files.items()}}


def payload_summary(group):
    """A short source cue so generic rule decisions are actionable in Markdown."""
    new = changed(group, "+")
    for line in new.splitlines():
        value = line.strip()
        if value and not value.startswith(("#", "//", "/*", "*")) and value not in {"{", "}", "};"}:
            return value[:120] + ("..." if len(value) > 120 else "")
    includes = re.findall(r"#\s*include\s+[^\n]+", new)
    return includes[0] if includes else "paired guard/scope/declaration group"


def render(report):
    es = report["groups"]
    counts = Counter(e["decision"] for e in es)
    lines = ["# Unapplied native-hook triage", "",
             f"Input: `{report['input']}`; source locations refer to `{report['overlay']}`.", "",
             "This is a P1 worklist, not a source patch or an objdiff proof. Only manual/unconditional/missing groups are counted; applied, feature and comment groups are excluded. Every group has one decision and a one-line reason. `rule` means a mechanical predicate; `reviewed` means a scoped human-reviewed exception. Group IDs use origin path, text hash and occurrence, so Petari line shifts do not silently reuse a review for changed text.", "",
             "Location resolution reads Petari base definitions via read-only git show, then searches overlay definitions (including Player unity .inc files and namespace functions). Original report function labels can refer to a prior force-match helper; resolved names take precedence. File-scope anchors identify declaration/include insertions. Ambiguous or absent implementation anchors remain unsure. Original paths/lines in the ledger identify the input group, not an SMG2 destination.", "",
             "Drops do not claim matching bytes: drop-fixed is source-level evidence of an existing equivalent repair/replacement. No match percentage is used as a correctness proof. port-clang rewrites must be Wii-neutral or GALAXY_NATIVE-guarded; port-native changes must be guarded. Recheck target asm and run the lead's Wii object/hash proof before landing. Multi-group scoping changes must be ported together. SMG1 offsets, enums, stage names and heap sizes must be adapted, not copied.", "",
             "Review notes: SMG2's explicit Mario state words, sound-policy bytes, hash payload API, and non-MW JMath/JGeometry fallbacks supersede the corresponding Petari fixes. The BMG parser and Korean-to-US BMG branch remapping do not describe SMG2's LMS MSBT/MSBF messages; review that replacement pipeline separately. GenericFigure/Cappy/Odyssey/editor hooks are deferred P3 features (recorded as drop-smg1 for this P1 pass). Existing SMG2 RNG-order repairs must be retained. An orphan header or missing decomp implementation is not enough evidence to drop a gameplay feature.", "",
             "Re-run from the port checkout for ht3:", "", "```sh",
             "nice -n 10 python3 tools/port/hooktriage.py ~/MacioGalaxy2-team/scratch-port/ht3/report.json --output docs/port/HOOKS_MANUAL.md --json-output /tmp/hooktriage.json", "```", "",
             f"## Totals ({len(es)} groups)", "", "| Decision | Groups |", "| --- | ---: |"]
    lines += [f"| {d} | {counts[d]} |" for d in DECISIONS]
    methods = Counter(e["method"] for e in es)
    kinds = Counter(e["kind"] for e in es)
    lines += ["", f"Methods: {methods['rule']} rule-decided; {methods['reviewed']} scoped source reviews. Input kinds: " + ", ".join(f"{k} {kinds[k]}" for k in sorted(KINDS)) + ".", "",
              f"Input SHA-256: `{report['input_sha256']}`. JSON output records every inspected source hash and the evidence snippets for reviewed drops."]
    lines += ["", "## Lane totals", "", "Ownership is loaded from tools/team/lanefiles.py for this run; it differs from the earlier argument-sweep brief's lane map.", "",
              "| Lane | drop-smg1 | drop-fixed | port-clang | port-native | unsure | Total |",
              "| --- | ---: | ---: | ---: | ---: | ---: | ---: |"]
    for lane in sorted({e["lane"] for e in es}):
        ls = [e for e in es if e["lane"] == lane]
        c = Counter(e["decision"] for e in ls)
        lines.append("| " + " | ".join([lane, *(str(c[d]) for d in DECISIONS), str(len(ls))]) + " |")
    lines += ["", "## Port work by lane", ""]
    for lane in sorted({e["lane"] for e in es}):
        lines += [f"### {lane}", ""]
        work = [e for e in es if e["lane"] == lane and e["decision"].startswith("port-")]
        for e in sorted(work, key=lambda e: (e["target"]["path"], e["target"]["line"], e["line"])):
            t = e["target"]
            cue = payload_summary(e).replace("`", "'")
            lines.append(f"- `{t['path']}:{t['line']}` `{t['function']}`: **{e['decision']}** — {e['reason']} Cue: `{cue}` [{e['method']}; `{e['id']}`]")
        if not work:
            lines.append("No port decisions in this lane.")
        lines.append("")
    lines += ["## Unresolved mapping/review queue", "",
              "Unsure is an explicit decision, not an omitted group. Resolve these before transferring their changes. THP/movie hooks need a confirmed SMG2 implementation pipeline; the layout entries need their replacement API; the two sound-name edits need SB4E01 sound/asm evidence. Other rows identify absent definitions, incomplete decomp sources or semantic changes without a safe equivalence proof.", "",
              "| Origin file | Unsure groups | Functions |", "| --- | ---: | --- |"]
    pending = defaultdict(list)
    for e in es:
        if e["decision"] == "unsure":
            pending[e["path"]].append(e)
    for path, entries in sorted(pending.items()):
        names = ", ".join(sorted({e["resolved_function"] or "<file-scope>" for e in entries}))
        lines.append(f"| {path} | {len(entries)} | {names} |")
    lines.append("")
    lines += ["## Complete decision ledger", "",
              "One input group per row; original line is the Petari base line. JSON retains exact -/+ text, source hashes and resolver details.", "",
              "| Origin path:line | Function | Kind | Decision | Method | Reason | Reviewed evidence | ID |",
              "| --- | --- | --- | --- | --- | --- | --- | --- |"]
    def cell(v):
        return str(v).replace("|", "\\|").replace("\n", " ")
    for e in es:
        row = [f"{e['path']}:{e['line']}", e["resolved_function"] or "<file-scope>",
               e["kind"], e["decision"], e["method"], e["reason"],
               ", ".join(dict.fromkeys(f"{v['path']}:{v['line']}" for v in e.get("evidence", []))), e["id"]]
        lines.append("| " + " | ".join(cell(v) for v in row) + " |")
    return "\n".join(lines) + "\n"


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("report", type=Path)
    p.add_argument("--overlay", type=Path)
    p.add_argument("--reference", type=Path, default=Path.home() / "MacioGalaxy")
    p.add_argument("--lane-map", type=Path, default=HERE / "tools/team/lanefiles.py")
    p.add_argument("--output", type=Path)
    p.add_argument("--json-output", type=Path)
    args = p.parse_args()
    report = triage(args.report, args.overlay or args.report.parent / "overlay", args.reference, args.lane_map)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(render(report))
    if args.json_output:
        args.json_output.parent.mkdir(parents=True, exist_ok=True)
        args.json_output.write_text(json.dumps(report, indent=2) + "\n")
    print(f"{len(report['groups'])} groups: {dict(Counter(e['decision'] for e in report['groups']))}")
    if report["reference_errors"]:
        print("Reference read errors:", ", ".join(report["reference_errors"]))


if __name__ == "__main__":
    main()
