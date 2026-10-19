#!/usr/bin/env python3
"""Read-only raw PPC pairing for advisory mechanical screens.

iter_pairs(root, prefixes=("Game/",), cache_dir=None) returns an iterable with
.coverage and .run_metadata. It completes all start/end stability checks before
exposing rows. Empty prefixes select all configured/report units. View addresses
are ELF section offsets; words[k] starts at function-relative byte offset 4*k.
Target virtual addresses are separate integers. Function sizes can differ.
Duplicate names never pair by ordinal, fuzzy score, or same local label text.
Optional cache files are PAIRS.json and RUN-MANIFEST.json; every cache reuse
checks the live input pins. This module invokes only git rev-parse, never a
compiler, Ninja, objdiff, semantic classifier, or proof mode.

Coverage keeps all raw observations and their original statuses. Only an
unreported raw .text symbol named exactly gap_XX_XXXXXXXX_text (hex digits) is
accounted as generated DTK padding. It is not removed from PAIRS.json or from
either screen. raw_observations equals generated_padding_observations plus
code_function_observations, globally and per lane. Existing function_occurrences,
pair_status_counts, paired and coverage_gaps remain the historical raw totals;
code_function_pair_status_counts and code_function_coverage_gaps describe code
coverage. This name convention is metadata, not a semantic padding proof. Real
unreported functions and near-matching names remain code coverage gaps.
"""
from pathlib import Path
import collections
import hashlib
import json
import math
import os
import re
import struct
import subprocess
import tempfile

SCHEMA = "mechanical-pairs-v1"
GENERATED_PADDING_NAME = r"gap_[0-9A-Fa-f]{2}_[0-9A-Fa-f]{8}_text"
OWNER_DIRS = {
    "astra": "Player NPC Screen",
    "enemies": "Enemy RhythmLib AudioLib GameAudio Speaker",
    "objects": "MapObj Ride AreaObj Boss",
    "core": "Util LiveActor NameObj System Scene Map Camera Animation Gravity Effect Demo NWC24",
}
LIBRARY_PREFIXES = {"JSystem", "nw4r", "RVLFaceLib", "RVL_SDK", "libms", "MSL_C", "Runtime", "MetroTRK", "NDEV"}


class PairingError(ValueError):
    pass


class InputChangedError(PairingError):
    pass


def require(condition, message):
    if not condition:
        raise PairingError(message)


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def integer(value):
    if value is None:
        return None
    return int(value, 16 if value.lower().startswith("0x") else 10) if isinstance(value, str) else int(value)


def normalized(unit):
    return unit.removeprefix("main/")


def head(root):
    return subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=root, text=True).strip()


def path_for(root, path):
    if not path:
        return None
    path = Path(path)
    return path if path.is_absolute() else root / path


def snapshot(path):
    path = Path(path)
    if not path.exists():
        return {"exists": False, "symlink": os.readlink(path) if path.is_symlink() else None}
    stat = path.stat()
    require(path.is_file(), "Input is not a regular file: " + str(path))
    hasher = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            hasher.update(block)
    return {"exists": True, "sha256": hasher.hexdigest(), "bytes": stat.st_size,
            "mtime_ns": stat.st_mtime_ns, "inode": stat.st_ino,
            "symlink": os.readlink(path) if path.is_symlink() else None}


class Pins:
    def __init__(self):
        self.before = {}

    def add(self, path):
        if path is not None:
            path = str(Path(path).absolute())
            self.before.setdefault(path, snapshot(path))

    def read(self, path):
        self.add(path)
        raw = Path(path).read_bytes()
        if digest(raw) != self.before[str(Path(path).absolute())].get("sha256"):
            raise InputChangedError("Input changed before parsing: " + str(path))
        return raw

    def finish(self):
        after = {path: snapshot(path) for path in self.before}
        changed = sorted(path for path in after if after[path] != self.before[path])
        if changed:
            raise InputChangedError("Input changed during mechanical pairing: " + json.dumps(changed))
        return after


class Elf:
    """ELF32 big-endian PPC, retaining every raw symbol-table occurrence."""
    def __init__(self, raw):
        self.raw = raw
        require(len(raw) >= 52 and raw[:6] == b"\x7fELF\x01\x02", "Expected ELF32BE")
        header = struct.unpack_from(">16sHHIIIIIHHHHHH", raw)
        require(header[2] == 20, "Expected PowerPC ELF")
        require(header[11] == 40 and header[6] + header[12] * 40 <= len(raw), "Invalid section table")
        self.sections = [struct.unpack_from(">10I", raw, header[6] + index * 40) for index in range(header[12])]
        self.data = []
        for section in self.sections:
            require(section[1] == 8 or section[4] + section[5] <= len(raw), "Section exceeds ELF")
            self.data.append(b"" if section[1] == 8 else raw[section[4]:section[4] + section[5]])
        require(header[13] < len(self.sections), "Invalid section names")
        def string(raw, offset):
            end = raw.find(b"\0", offset)
            require(0 <= offset < len(raw) and end >= offset, "Invalid ELF string")
            return raw[offset:end].decode("utf-8", errors="backslashreplace")
        self.names = [string(self.data[header[13]], section[0]) for section in self.sections]
        self.symbols = {}
        for table, section in enumerate(self.sections):
            if section[1] != 2:
                continue
            require(section[9] == 16 and section[5] % 16 == 0 and section[6] < len(self.data), "Invalid symbol table")
            for index in range(section[5] // 16):
                name, value, size, info, other, si = struct.unpack_from(">IIIBBH", self.data[table], index * 16)
                self.symbols[table, index] = {
                    "name": string(self.data[section[6]], name), "address": value, "size": size,
                    "type": info & 15, "bind": info >> 4, "other": other,
                    "section_index": si, "section": self.names[si] if si < len(self.names) else None,
                    "symbol_index": index, "symbol_table_section_index": table}
        self.functions = [symbol for symbol in self.symbols.values() if symbol["type"] == 2 and symbol["size"] > 0]
        self.relocations = []
        for relocation_section, section in enumerate(self.sections):
            if section[1] not in (4, 9):
                continue
            width = 12 if section[1] == 4 else 8
            require(section[9] == width and section[5] % width == 0, "Invalid relocation table")
            for index in range(0, section[5], width):
                offset, info = struct.unpack_from(">II", self.data[relocation_section], index)
                symbol = self.symbols.get((section[6], info >> 8))
                require(symbol is not None, "Relocation has no raw symbol occurrence")
                addend = struct.unpack_from(">i", self.data[relocation_section], index + 8)[0] if width == 12 else None
                self.relocations.append({"section_index": section[7], "offset": offset,
                                         "type": info & 255, "symbol_record": symbol, "addend": addend,
                                         "implicit_addend": width == 8})

    def body(self, symbol):
        section, offset, size = symbol["section_index"], symbol["address"], symbol["size"]
        require(section < len(self.data) and offset + size <= len(self.data[section]), "Function has no file-backed body")
        return self.data[section][offset:offset + size]

    def view(self, symbol, object_id):
        raw = self.body(symbol)
        aligned = symbol["address"] % 4 == 0 and len(raw) % 4 == 0
        result = dict(symbol, words=list(struct.unpack(">" + "I" * (len(raw) // 4), raw)) if aligned else [],
                      aligned=aligned, relocations=[])
        for rel in self.relocations:
            if rel["section_index"] != symbol["section_index"] or not symbol["address"] <= rel["offset"] < symbol["address"] + symbol["size"]:
                continue
            ref = rel["symbol_record"]
            row = {"offset": rel["offset"] - symbol["address"], "type": rel["type"],
                   "symbol": ref["name"], "addend": rel["addend"], "implicit_addend": rel["implicit_addend"],
                   "section": ref["section"], "section_index": ref["section_index"],
                   "symbol_value": ref["address"], "symbol_size": ref["size"], "symbol_type": ref["type"],
                   "symbol_binding": ref["bind"],
                   "symbol_index": ref["symbol_index"], "symbol_table_section_index": ref["symbol_table_section_index"],
                   "symbol_occurrence_id": object_id + ":" + str(ref["symbol_table_section_index"]) + ":" + str(ref["symbol_index"])}
            si = ref["section_index"]
            if rel["addend"] is not None and 0 < si < len(self.data) and not self.sections[si][2] & 4:
                start = ref["address"] + rel["addend"]
                end = ref["address"] + ref["size"] if ref["size"] else min(
                    [s["address"] for s in self.symbols.values() if s["section_index"] == si and s["address"] > start] + [len(self.data[si])])
                if 0 <= start < min(end, len(self.data[si])):
                    end = min(end, len(self.data[si]), start + 16)
                    row.update(literal_bytes_hex=self.data[si][start:end].hex(), literal_size=end-start,
                               literal_bytes_kind="RAW_OBJECT_BYTES_NOT_RELOCATED",
                               literal_relocations_present=any(r["section_index"] == si and start <= r["offset"] < end for r in self.relocations))
            result["relocations"].append(row)
        return result


def dol_body(dol, address, size):
    require(len(dol) >= 0xD8 and address is not None, "DOL or target VA missing")
    matches = []
    for index in range(18):
        offset, base, length = [struct.unpack_from(">I", dol, start + index * 4)[0] for start in (0, 0x48, 0x90)]
        if base <= address and address + size <= base + length and offset + address - base + size <= len(dol):
            matches.append(dol[offset + address - base:offset + address - base + size])
    require(len(matches) == 1, "DOL target extent missing/ambiguous")
    return matches[0]


def bind_dol(view, dol, address):
    actual = dol_body(dol, address, view["size"])
    raw = b"".join(word.to_bytes(4, "big") for word in view["words"])
    masked, bound = bytearray(raw), bytearray(actual)
    for rel in view["relocations"]:
        offset, kind = rel["offset"], rel["type"]
        if rel["implicit_addend"]:
            raise PairingError("Implicit REL addend requires explicit binding")
        if kind == 0:
            continue
        if kind in (3, 4, 5, 6, 32):
            require(offset + 2 <= len(raw), "Relocation exceeds body")
            masked[offset:offset+2] = bound[offset:offset+2] = b"\0\0"
        elif kind in (1, 2, 7, 8, 9, 10, 11, 12, 13, 26, 109):
            require(offset % 4 == 0 and offset + 4 <= len(raw), "Relocation exceeds/misaligns instruction")
            mask = 0 if kind in (1, 26) else 0xFC000003 if kind in (2, 10) else 0xFFE00000 if kind == 109 else 0xFFFF0003
            for stream in (masked, bound):
                stream[offset:offset+4] = (int.from_bytes(stream[offset:offset+4], "big") & mask).to_bytes(4, "big")
        else:
            raise PairingError("Unsupported DOL code relocation: " + str(kind))
    require(masked == bound, "Nonrelocated target bits differ from DOL")
    view["virtual_address"] = address
    view["dol_words"] = list(struct.unpack(">" + "I" * (len(actual) // 4), actual))


def owner_lane(unit):
    if unit.startswith("Game/"):
        directory = unit.split("/")[1]
        return next((lane for lane, dirs in OWNER_DIRS.items() if directory in dirs.split()), "unmapped")
    return "m1" if unit.split("/")[0] in LIBRARY_PREFIXES else "unmapped"


def _team_files(root):
    board = root / "docs/team/BOARD.md"
    if not board.is_file():
        board = Path.home() / "MacioGalaxy2-team/BOARD.md"
    return board, [root / "docs/team/TEAM.md", root / "tools/team/mechanical-audits.md"]


def _identity(unit, symbol):
    values = [unit, symbol["symbol_table_section_index"], symbol["symbol_index"], symbol["section_index"], symbol["address"], symbol["size"]]
    return "ELF" + digest(json.dumps(values, separators=(",", ":")).encode())[:24]


def _report_va(function, symbol, sections):
    explicit = integer(function.get("metadata", {}).get("virtual_address"))
    bases = [integer(section.get("metadata", {}).get("virtual_address")) for section in sections if section["name"] == symbol["section"]]
    bases = [base for base in bases if base is not None]
    if len(bases) != 1:
        return None
    computed = bases[0] + symbol["address"]
    return computed if explicit is None or explicit == computed else None


def _unit_pairs(unit, cfg, reports, pins, root, dol):
    paths = {side: path_for(root, cfg.get(key)) for side, key in (("target", "target_path"), ("source", "base_path"))}
    images, errors = {}, {}
    for side, path in paths.items():
        try:
            images[side] = Elf(pins.read(path)) if path and path.is_file() else None
        except (PairingError, struct.error, IndexError, UnicodeError) as error:
            images[side] = None; errors[side] = str(error)
    report_rows = [(ui, fi, function, report.get("sections", [])) for ui, report in reports
                   for fi, function in enumerate(report.get("functions", [])) if integer(function.get("size", 0)) > 0]
    source_path = cfg.get("metadata", {}).get("source_path")
    def base(name, size, va, occurrence):
        return {"unit": unit, "name": name, "demangled_name": name, "owner_lane": owner_lane(unit),
                "target_va": va, "target_size": size, "source_path": source_path,
                "occurrence_id": occurrence, "pair_status": None, "target": None, "source": None,
                "objects": {side: str(path.relative_to(root)) if path and path.is_relative_to(root) else str(path) if path else None for side, path in paths.items()}}
    target = images["target"]
    used_reports = set()
    if target:
        target_names = collections.Counter(symbol["name"] for symbol in target.functions)
        source_names = collections.defaultdict(list)
        if images["source"]:
            for symbol in images["source"].functions:source_names[symbol["name"]].append(symbol)
        for symbol in target.functions:
            record = base(symbol["name"], symbol["size"], None, _identity(unit, symbol))
            matching = [(ui, fi, function, sections, _report_va(function, symbol, sections))
                        for ui, fi, function, sections in report_rows
                        if function["name"] == symbol["name"] and integer(function["size"]) == symbol["size"]
                        and integer(function.get("address")) == symbol["address"]]
            bound = [entry for entry in matching if entry[4] is not None]
            used_reports.update((entry[0], entry[1]) for entry in matching)
            try:
                record["target"] = target.view(symbol, "target:" + str(paths["target"]))
            except PairingError as error:
                record.update(pair_status="TARGET_UNPAIRED", pairing_reason=str(error));yield record;continue
            if len(bound) != 1:
                record.update(pair_status="TARGET_AMBIGUOUS" if len(bound) > 1 else "TARGET_REPORT_MISSING",
                              pairing_reason="Raw target occurrence has no unique section/size/offset/VA report binding")
                yield record;continue
            ui, fi, function, sections, va = bound[0]
            record.update(target_va=va, report_unit_index=ui, report_function_index=fi,
                          demangled_name=function.get("metadata", {}).get("demangled_name", symbol["name"]))
            if target_names[symbol["name"]] > 1:
                record.update(pair_status="TARGET_AMBIGUOUS", pairing_reason="Duplicate raw target symbol name")
            elif not record["target"]["aligned"]:
                record.update(pair_status="UNALIGNED", pairing_reason="Target function extent is not word aligned")
            else:
                try:
                    bind_dol(record["target"], dol, va)
                except PairingError as error:
                    record.update(pair_status="DOL_MISMATCH", pairing_reason=str(error))
                if record["pair_status"] is None:
                    candidates = source_names[symbol["name"]]
                    if not candidates:
                        record.update(pair_status="SOURCE_MISSING", pairing_reason=errors.get("source", "No exact raw source function occurrence"))
                    elif len(candidates) != 1:
                        record.update(pair_status="SOURCE_AMBIGUOUS", pairing_reason="Duplicate raw source symbol name")
                    else:
                        try:
                            source = images["source"].view(candidates[0], "source:" + str(paths["source"]))
                            if not source["aligned"]:
                                record.update(pair_status="UNALIGNED", pairing_reason="Source function extent is not word aligned")
                            else:
                                record.update(pair_status="PAIRED", source=source,
                                              pairing_reason="Unique exact raw symbol name per side; independent section/index/size retained")
                        except PairingError as error:
                            record.update(pair_status="SOURCE_MISSING", pairing_reason=str(error))
            yield record
    for ui, fi, function, sections in report_rows:
        if (ui, fi) in used_reports:
            continue
        occurrence = "REPORT" + digest(json.dumps([unit, ui, fi, function["name"], function.get("address"), function.get("size")]).encode())[:24]
        record = base(function["name"], integer(function["size"]), integer(function.get("metadata", {}).get("virtual_address")), occurrence)
        record.update(demangled_name=function.get("metadata", {}).get("demangled_name", function["name"]),
                      pair_status="TARGET_MISSING" if target is None else "TARGET_UNPAIRED",
                      pairing_reason=errors.get("target", "No exact raw target extent/report occurrence binding"),
                      report_unit_index=ui, report_function_index=fi)
        yield record


class PairRun:
    def __init__(self, root, prefixes, cache_dir):
        self.root = Path(root).resolve()
        self.prefixes = tuple(prefixes)
        self.cache_dir = Path(cache_dir).resolve() if cache_dir is not None else None
        self.coverage = {}; self.run_metadata = {}; self._pairs = None

    def __iter__(self):
        if self._pairs is None:self._collect()
        return iter(self._pairs)

    def _collect(self):
        root = self.root; pins = Pins(); first_head = head(root)
        config_path, report_path, dol_path = [root / name for name in ("objdiff.json", "build/SB4E01/report.json", "orig/SB4E01/sys/main.dol")]
        config = json.loads(pins.read(config_path)); report = json.loads(pins.read(report_path))
        pins.add(dol_path); pins.add(Path(__file__))
        board, routing_inputs = _team_files(root)
        board_context = snapshot(board)
        for path in routing_inputs:pins.add(path)
        configs = collections.defaultdict(list); reports = collections.defaultdict(list)
        for entry in config["units"]:configs[normalized(entry["name"])].append(entry)
        for ui, entry in enumerate(report["units"]):reports[normalized(entry["name"])].append((ui, entry))
        units = sorted(unit for unit in set(configs) | set(reports) if not self.prefixes or unit.startswith(self.prefixes))
        for unit in units:
            for cfg in configs[unit]:
                for key in ("target_path", "base_path"):pins.add(path_for(root, cfg.get(key)))
                pins.add(path_for(root, cfg.get("metadata", {}).get("source_path")))
        cache_hit = False
        if self.cache_dir:
            try:
                old = json.loads((self.cache_dir / "RUN-MANIFEST.json").read_text())
                raw = (self.cache_dir / "PAIRS.json").read_bytes()
                if (old["schema"] == SCHEMA and old["head_before"] == first_head and old["production_root"] == str(root)
                        and old["prefixes"] == list(self.prefixes) and old["pins_before"] == pins.before
                        and old["pairs_sha256"] == digest(raw) and old["stable"] is True):
                    pairs = json.loads(raw);cache_hit = True
            except (OSError, ValueError, KeyError):pass
        if not cache_hit:
            dol = pins.read(dol_path) if dol_path.is_file() else b""
            pairs = []
            for unit in units:
                cfg = configs[unit][0] if len(configs[unit]) == 1 else {}
                rows = list(_unit_pairs(unit, cfg, reports[unit], pins, root, dol))
                if len(configs[unit]) > 1:
                    for row in rows:row.update(pair_status="TARGET_AMBIGUOUS", source=None, pairing_reason="Duplicate configured unit metadata")
                pairs.extend(rows)
        after = pins.finish(); last_head = head(root)
        if first_head != last_head:raise InputChangedError("HEAD changed during mechanical pairing")
        require(len({row["occurrence_id"] for row in pairs}) == len(pairs), "Collapsed raw occurrence IDs")
        pairs.sort(key=lambda row:(row["unit"], row["target_va"] is None, row["target_va"] or 0, row["occurrence_id"]))
        self.coverage = dict(observation_counts(pairs), units=len(units),
                             owner_lane_counts=dict(sorted(collections.Counter(row["owner_lane"] for row in pairs).items())),
                             selection="All positive-size raw/report observations retained; no match-score/proof/padding screen filter",
                             generated_padding_accounting={"name_fullmatch": GENERATED_PADDING_NAME,
                                 "required_status": "TARGET_REPORT_MISSING", "required_raw_section": ".text",
                                 "changes_row_selection_or_status": False, "semantic_padding_verdict": None})
        self.coverage["zero_size_report_functions"] = sum(integer(function.get("size", 0)) == 0
            for unit in units for _, report_unit in reports[unit] for function in report_unit.get("functions", []))
        self.coverage["per_lane"] = {}
        for lane in sorted({row["owner_lane"] for row in pairs}):
            lane_rows = [row for row in pairs if row["owner_lane"] == lane]
            self.coverage["per_lane"][lane] = observation_counts(lane_rows)
        self.coverage["units_without_positive_functions"] = sorted(set(units)-{row["unit"] for row in pairs})
        self.run_metadata = {"schema": SCHEMA, "production_root": str(root), "head_before": first_head, "head_after": last_head,
                             "stable": True, "prefixes": list(self.prefixes), "pins_before": pins.before, "pins_after": after,
                             "cache_status": "HIT" if cache_hit else "MISS" if self.cache_dir else "DISABLED",
                             "advisory_only": True, "semantic_verdict": None,
                             "owner_routing": OWNER_DIRS,
                             "owner_routing_provenance": "Parent current routing supersedes historical BOARD table; actual tools/team/mechanical-audits.md and TEAM are binding pins",
                             "historical_board_context": {"path": str(board), "snapshot": board_context,
                                                          "binding": False, "reason": "Live historical BOARD does not define current routing"},
                             "source_freshness": "Existing objects only; no compiler/dependency-closure or source-equivalence certification",
                             "word_provenance": "Raw ELF words; target nonrelocated bits bound to DOL, dol_words retain actual relocated words"}
        self._pairs = pairs
        if self.cache_dir:
            self.cache_dir.mkdir(parents=True, exist_ok=True)
            raw = (json.dumps(pairs, sort_keys=True, separators=(",", ":")) + "\n").encode()
            _atomic(self.cache_dir / "PAIRS.json", raw)
            _atomic(self.cache_dir / "RUN-MANIFEST.json", _json(dict(self.run_metadata, pairs_sha256=digest(raw), coverage=self.coverage)))


def iter_pairs(root, prefixes=("Game/",), cache_dir=None):
    require(not isinstance(prefixes, str), "prefixes must be a tuple/list, not one string")
    return PairRun(root, prefixes, cache_dir)


def is_generated_padding_observation(row):
    """Conservative DTK raw-symbol convention, without changing a row/status."""
    return (row.get("pair_status") == "TARGET_REPORT_MISSING"
            and row.get("target") is not None and row["target"].get("section") == ".text"
            and re.fullmatch(GENERATED_PADDING_NAME, row.get("name", "")) is not None)


def observation_counts(rows):
    statuses = dict(sorted(collections.Counter(row["pair_status"] for row in rows).items()))
    code = [row for row in rows if not is_generated_padding_observation(row)]
    padding = [row for row in rows if is_generated_padding_observation(row)]
    code_statuses = dict(sorted(collections.Counter(row["pair_status"] for row in code).items()))
    historical = {"function_occurrences": len(rows), "pair_status_counts": statuses,
                  "paired": statuses.get("PAIRED", 0), "coverage_gaps": len(rows)-statuses.get("PAIRED", 0)}
    return dict(historical, raw_observations=len(rows), generated_padding_observations=len(padding),
                code_function_observations=len(code), code_function_pair_status_counts=code_statuses,
                code_function_paired=code_statuses.get("PAIRED", 0),
                code_function_coverage_gaps=len(code)-code_statuses.get("PAIRED", 0),
                generated_padding_pair_status_counts=dict(sorted(collections.Counter(row["pair_status"] for row in padding).items())),
                historical_aggregate_counts=historical)


def coverage_lines(coverage):
    if not coverage or "code_function_observations" not in coverage:
        return ["Legacy pairing totals (may include generated padding): " + json.dumps(coverage, sort_keys=True),
                "Code-function/padding accounting unavailable in this legacy coverage record."]
    return [
        f"Code-function coverage: {coverage['code_function_paired']}/{coverage['code_function_observations']} paired; {coverage['code_function_coverage_gaps']} unpaired.",
        f"Raw observations: {coverage['raw_observations']}; generated DTK padding observations: {coverage['generated_padding_observations']} (all rows retained).",
        "Code-function pairing statuses: " + json.dumps(coverage["code_function_pair_status_counts"], sort_keys=True),
        f"Historical raw accounting, including padding: {coverage['paired']}/{coverage['raw_observations']} paired; {coverage['coverage_gaps']} unpaired.",
        "Padding accounting uses the exact gap_XX_XXXXXXXX_text convention on unreported raw .text symbols. Real unreported functions remain code coverage gaps. This changes no screen selection or semantic conclusion.",
    ]


def _json(value):
    return (json.dumps(value, indent=2, sort_keys=True) + "\n").encode()


def _atomic(path, raw):
    path = Path(path);path.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary = tempfile.mkstemp(prefix="." + path.name, dir=path.parent)
    try:
        with os.fdopen(fd, "wb") as stream:stream.write(raw)
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):os.unlink(temporary)


def write_results(root, output, audit_name, findings, coverage, run_metadata):
    """Write advisory JSON and lane JSON/Markdown, preserving caller fields."""
    require(re.fullmatch(r"[A-Za-z0-9_-]+", audit_name) is not None, "Unsafe audit name")
    output = Path(output)
    main = output if output.suffix == ".json" else output / (audit_name + ".json")
    lanes = output.with_name(output.stem + "-lanes") if output.suffix == ".json" else output / "lanes"
    def rank(row):
        try:value = float(row.get("rank", 0))
        except (TypeError, ValueError):value = 0
        return -value if math.isfinite(value) else 0
    ordered = sorted(findings, key=lambda row:(rank(row), str(row.get("unit", "")), integer(row.get("target_va")) or 0, str(row.get("name", "")), str(row.get("occurrence_id", ""))))
    result = {"schema": "mechanical-audit-results-v1", "audit_name": audit_name, "root": str(Path(root).resolve()),
              "advisory_only": True, "semantic_verdict": None, "findings": ordered,
              "coverage": coverage, "run_metadata": run_metadata}
    _atomic(main, _json(result))
    groups = collections.defaultdict(list)
    pairing_coverage = coverage.get("pairing", coverage)
    lane_coverage = pairing_coverage.get("per_lane", {})
    for lane in lane_coverage:groups[lane] = []
    for row in ordered:
        lane = str(row.get("owner_lane", "unmapped"))
        groups[lane if re.fullmatch(r"[A-Za-z0-9_-]+", lane) else "unmapped"].append(row)
    paths = {}
    def cell(value):return str(value).replace("|", "\\|").replace("\n", " ")
    for lane, rows in sorted(groups.items()):
        lane_json, lane_md = lanes / (lane + ".json"), lanes / (lane + ".md")
        _atomic(lane_json, _json({"audit_name": audit_name, "owner_lane": lane, "advisory_only": True, "findings": rows, "pairing_coverage": lane_coverage.get(lane), "global_results": str(main)}))
        text = ["# " + audit_name + " — " + lane, "", "Advisory mechanical screen; findings require investigation and are not semantic verdicts.", "",
                *coverage_lines(lane_coverage.get(lane)), "",
                "| Rank | Unit | Function | Target VA | Kind | Confidence | Detail |", "| ---: | --- | --- | --- | --- | --- | --- |"]
        for row in rows:
            va = integer(row.get("target_va"));shown = f"0x{va:08X}" if va is not None else "unknown"
            text.append("| " + " | ".join(cell(x) for x in (row.get("rank", 0), row.get("unit", ""), row.get("demangled_name", row.get("name", "")), shown, row.get("kind", ""), row.get("confidence", ""), row.get("detail", ""))) + " |")
        _atomic(lane_md, ("\n".join(text) + "\n").encode());paths[lane] = {"json": str(lane_json), "markdown": str(lane_md)}
    return {"json": str(main), "lanes": paths}
