#!/usr/bin/env python3
"""Tiny raw ELF/DOL controls; never compile or invoke objdiff/Ninja."""
import copy
import json
import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch
import mech_audit_common as common

REV = "a" * 40
TEXT = bytes.fromhex("386000014e800020")


def elf(text=TEXT, functions=None, relocations=(), text_name=".text", nobits=False):
    functions = functions if functions is not None else [("func", 0, len(text))]
    names = ["", text_name, ".rodata", ".strtab", ".symtab", ".shstrtab"]
    if relocations:names.append(".rela.text")
    section_strings = b"\0" + b"\0".join(x.encode() for x in names[1:]) + b"\0"
    strings = bytearray(b"\0");symbols = bytearray(16)
    for name, address, size in functions:
        at = len(strings);strings.extend(name.encode() + b"\0")
        symbols.extend(struct.pack(">IIIBBH", at, address, size, 2, 0, 1))
    at = len(strings);strings.extend(b"constant\0")
    symbols.extend(struct.pack(">IIIBBH", at, 0, 4, 1, 0, 2))
    data = [b"", text, bytes.fromhex("3f800000"), bytes(strings), bytes(symbols), section_strings]
    if relocations:data.append(b"".join(struct.pack(">IIi", offset, (index << 8) | kind, addend) for offset, kind, index, addend in relocations))
    raw = bytearray(52);sections = [(0,) * 10]
    for index, value in enumerate(data[1:], 1):
        while len(raw) % 4:raw.append(0)
        start = len(raw);raw.extend(value)
        kind = 8 if index == 1 and nobits else 2 if index == 4 else 4 if index == 6 else 3 if index in (3, 5) else 1
        sections.append((section_strings.index(names[index].encode()), kind, 6 if index == 1 else 2, 0,
                         start, len(value), 3 if index == 4 else 4 if index == 6 else 0,
                         1 if index == 6 else 0, 4, 16 if index == 4 else 12 if index == 6 else 0))
    while len(raw) % 4:raw.append(0)
    shoff = len(raw);raw.extend(b"".join(struct.pack(">10I", *section) for section in sections))
    raw[:52] = struct.pack(">16sHHIIIIIHHHHHH", b"\x7fELF\x01\x02" + bytes(10), 1, 20, 1, 0, 0, shoff, 0, 52, 0, 0, 40, len(sections), 5)
    return bytes(raw)


def dol(text=TEXT):
    raw = bytearray(0x100) + text
    for offset, value in ((0, 0x100), (0x48, 0x80001000), (0x90, len(text))):struct.pack_into(">I", raw, offset, value)
    return raw


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(bytes(value) if isinstance(value, (bytes, bytearray)) else (json.dumps(value) + "\n").encode())


def fixture(root, target=None, source=None, functions=None):
    functions = functions if functions is not None else [("func", 0, len(TEXT))]
    cfg = {"name": "main/Game/Enemy/Fixture", "target_path": "target.o", "base_path": "source.o",
           "metadata": {"source_path": "src/Game/Enemy/Fixture.cpp"}}
    report = {"name": cfg["name"], "sections": [{"name": ".text", "metadata": {"virtual_address": str(0x80001000)}}],
              "functions": [{"name": name, "address": str(address), "size": str(size), "fuzzy_match_percent": 100,
                             "metadata": {"virtual_address": str(0x80001000 + address), "demangled_name": name + "()"}}
                            for name, address, size in functions]}
    write(root / "objdiff.json", {"units": [cfg]});write(root / "build/SB4E01/report.json", {"units": [report]})
    write(root / "target.o", target if target is not None else elf(functions=functions))
    write(root / "source.o", source if source is not None else elf(functions=functions))
    write(root / "orig/SB4E01/sys/main.dol", dol());write(root / cfg["metadata"]["source_path"], b"synthetic input\n")
    for name in ("BOARD.md", "TEAM.md", "mechanical-audits.md"):write(root / "docs/team" / name, b"synthetic routing context\n")
    write(root / "tools/team/mechanical-audits.md", b"synthetic current routing\n")
    return cfg, report


class PairingControls(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory();self.root = Path(self.tmp.name).resolve()
        self.git = patch.object(common, "head", return_value=REV);self.git.start()
    def tearDown(self):
        self.git.stop();self.tmp.cleanup()
    def rows(self, **kwargs):
        run = common.iter_pairs(self.root, **kwargs);return list(run), run

    def test_different_sizes_unique_occurrences_pair_and_exact100_is_included(self):
        fixture(self.root, source=elf(bytes.fromhex("4e800020")))
        rows, run = self.rows();self.assertEqual(len(rows), 1)
        row = rows[0];self.assertEqual(row["pair_status"], "PAIRED")
        self.assertEqual((row["target"]["size"], row["source"]["size"]), (8, 4))
        self.assertEqual((row["target"]["address"], row["target_va"]), (0, 0x80001000))
        self.assertEqual(row["target"]["words"], [0x38600001, 0x4e800020])
        self.assertTrue(run.run_metadata["stable"]);self.assertEqual(run.coverage["paired"], 1)

    def test_duplicate_local_target_occurrences_stay_distinct_and_ambiguous(self):
        functions = [("local", 0, 4), ("local", 4, 4)]
        fixture(self.root, functions=functions)
        rows, run = self.rows();self.assertEqual(len({r["occurrence_id"] for r in rows}), 2)
        self.assertEqual([r["pair_status"] for r in rows], ["TARGET_AMBIGUOUS"] * 2)
        self.assertTrue(all(r["source"] is None for r in rows));self.assertEqual(run.coverage["coverage_gaps"], 2)

    def test_source_duplicate_never_pairs_by_size_or_last_name(self):
        fixture(self.root, source=elf(functions=[("func", 0, 4), ("func", 4, 4)]))
        rows, _ = self.rows();self.assertEqual(rows[0]["pair_status"], "SOURCE_AMBIGUOUS")
        self.assertIsNone(rows[0]["source"])

    def test_source_different_section_retains_independent_identity(self):
        fixture(self.root, source=elf(text_name=".init"))
        rows, _ = self.rows();self.assertEqual(rows[0]["pair_status"], "PAIRED")
        self.assertEqual(rows[0]["source"]["section"], ".init")

    def test_missing_source_has_explicit_absence_pin(self):
        fixture(self.root);(self.root / "source.o").unlink()
        rows, run = self.rows();self.assertEqual(rows[0]["pair_status"], "SOURCE_MISSING")
        self.assertFalse(run.run_metadata["pins_before"][str(self.root / "source.o")]["exists"])

    def test_missing_target_retains_report_row(self):
        fixture(self.root);(self.root / "target.o").unlink()
        rows, _ = self.rows();self.assertEqual(rows[0]["pair_status"], "TARGET_MISSING")
        self.assertEqual(rows[0]["target_size"], 8)

    def test_target_va_disagreement_fails_closed(self):
        cfg, report = fixture(self.root);report["functions"][0]["metadata"]["virtual_address"] = str(0x80001004)
        write(self.root / "build/SB4E01/report.json", {"units": [report]})
        rows, _ = self.rows();self.assertEqual(rows[0]["pair_status"], "TARGET_REPORT_MISSING")
        self.assertIsNone(rows[0]["source"])

    def test_unreported_raw_occurrence_is_coverage_gap(self):
        fixture(self.root, target=elf(functions=[("func", 0, 8), ("extra", 4, 4)]))
        rows, _ = self.rows();self.assertEqual(len(rows), 2)
        self.assertEqual(next(r for r in rows if r["name"] == "extra")["pair_status"], "TARGET_REPORT_MISSING")

    def test_dol_nonrelocated_word_change_rejects_pair(self):
        fixture(self.root);write(self.root / "orig/SB4E01/sys/main.dol", dol(bytes.fromhex("386000024e800020")))
        rows, _ = self.rows();self.assertEqual(rows[0]["pair_status"], "DOL_MISMATCH")
        self.assertIsNone(rows[0]["source"])

    def test_relocation_keeps_raw_words_actual_dol_and_independent_local_ids(self):
        raw = elf(bytes.fromhex("3c6000004e800020"), relocations=[(2, 6, 2, 0)])
        fixture(self.root, target=raw, source=raw)
        write(self.root / "orig/SB4E01/sys/main.dol", dol(bytes.fromhex("3c6080664e800020")))
        rows, _ = self.rows();row = rows[0];self.assertEqual(row["pair_status"], "PAIRED")
        target, source = row["target"]["relocations"][0], row["source"]["relocations"][0]
        self.assertEqual((target["offset"], target["type"], target["literal_bytes_hex"]), (2, 6, "3f800000"))
        self.assertEqual(target["symbol_binding"], 0)
        self.assertNotEqual(target["symbol_occurrence_id"], source["symbol_occurrence_id"])
        self.assertNotEqual(row["target"]["words"], row["target"]["dol_words"])

    def test_unknown_relocation_has_no_usable_source_pair(self):
        fixture(self.root, target=elf(relocations=[(0, 255, 2, 0)]))
        rows, _ = self.rows();self.assertEqual(rows[0]["pair_status"], "DOL_MISMATCH")
        self.assertIn("Unsupported", rows[0]["pairing_reason"])

    def test_odd_extent_and_special_section_count_as_gaps(self):
        fixture(self.root, target=elf(TEXT[:3], functions=[("func", 0, 3)]), functions=[("func", 0, 3)])
        rows, _ = self.rows();self.assertEqual(rows[0]["pair_status"], "UNALIGNED")
        fixture(self.root, target=elf(nobits=True))
        rows, _ = self.rows();self.assertEqual(rows[0]["pair_status"], "TARGET_UNPAIRED")

    def test_drift_raises_before_any_records_are_exposed(self):
        fixture(self.root);original = common.Pins.finish
        def finish(pins):
            write(self.root / "source.o", elf(bytes.fromhex("386000024e800020")))
            return original(pins)
        run = common.iter_pairs(self.root)
        with patch.object(common.Pins, "finish", finish):
            with self.assertRaises(common.InputChangedError):next(iter(run))
        self.assertFalse(run.run_metadata)

    def test_head_drift_rejected(self):
        fixture(self.root)
        with patch.object(common, "head", side_effect=[REV, "b" * 40]):
            with self.assertRaisesRegex(common.InputChangedError, "HEAD changed"):self.rows()

    def test_cache_hit_is_revalidated_and_changed_object_invalidates(self):
        fixture(self.root);cache = self.root / "cache"
        rows, run = self.rows(cache_dir=cache);self.assertEqual(run.run_metadata["cache_status"], "MISS")
        with patch.object(common, "_unit_pairs", side_effect=AssertionError("No parse on valid cache")):
            same, run = self.rows(cache_dir=cache)
        self.assertEqual(rows, same);self.assertEqual(run.run_metadata["cache_status"], "HIT")
        write(self.root / "source.o", elf(bytes.fromhex("386000024e800020")))
        changed, run = self.rows(cache_dir=cache);self.assertEqual(run.run_metadata["cache_status"], "MISS")
        self.assertNotEqual(changed[0]["source"]["words"], rows[0]["source"]["words"])

    def test_prefix_and_all_prefix_scope(self):
        cfg, report = fixture(self.root);library = copy.deepcopy(cfg);library["name"] = "main/Runtime/test"
        other = copy.deepcopy(report);other["name"] = library["name"]
        write(self.root / "objdiff.json", {"units": [cfg, library]});write(self.root / "build/SB4E01/report.json", {"units": [report, other]})
        self.assertEqual(len(self.rows()[0]), 1);self.assertEqual(len(self.rows(prefixes=())[0]), 2)
        with self.assertRaises(common.PairingError):common.iter_pairs(self.root, prefixes="Game/")

    def test_current_owner_routing_and_unmapped_are_explicit(self):
        for directory, lane in [("Player", "astra"), ("NPC", "astra"), ("Screen", "astra"), ("AudioLib", "enemies"), ("Boss", "objects"), ("System", "core"), ("Mystery", "unmapped")]:
            self.assertEqual(common.owner_lane("Game/" + directory + "/X"), lane)

    def test_padding_accounting_requires_exact_generated_convention(self):
        def row(name, status="TARGET_REPORT_MISSING", section=".text"):
            return {"name": name, "pair_status": status, "target": {"section": section}}
        self.assertTrue(common.is_generated_padding_observation(row("gap_03_80069CCC_text")))
        self.assertTrue(common.is_generated_padding_observation(row("gap_0a_80069ccc_text")))
        for name in ("gap_3_80069CCC_text", "gap_003_80069CCC_text", "gap_03_80069CC_text", "gap_03_80069CCG_text",
                     "gap_03_80069CCC_text_helper", "prefix_gap_03_80069CCC_text", "gap_03_80069CCC_text\n", "real_unreported_function"):
            self.assertFalse(common.is_generated_padding_observation(row(name)), name)
        self.assertFalse(common.is_generated_padding_observation(row("gap_03_80069CCC_text", status="PAIRED")))
        self.assertFalse(common.is_generated_padding_observation(row("gap_03_80069CCC_text", section=".init")))
        self.assertFalse(common.is_generated_padding_observation({"name": "gap_03_80069CCC_text", "pair_status": "TARGET_REPORT_MISSING", "target": None}))

    def test_padding_counts_preserve_raw_records_and_real_unreported_gaps(self):
        rows = [{"name": name, "pair_status": status, "target": {"section": ".text"}, "occurrence_id": str(index)}
                for index, (name, status) in enumerate([("func", "PAIRED"), ("gap_03_80001008_text", "TARGET_REPORT_MISSING"),
                                                       ("real_extra", "TARGET_REPORT_MISSING"), ("no_source", "SOURCE_MISSING"),
                                                       ("duplicate", "TARGET_AMBIGUOUS"), ("gap_3_80001010_text", "TARGET_REPORT_MISSING")])]
        original = copy.deepcopy(rows);counts = common.observation_counts(rows)
        self.assertEqual(rows, original)
        self.assertEqual((counts["raw_observations"], counts["generated_padding_observations"], counts["code_function_observations"]), (6, 1, 5))
        self.assertEqual(counts["code_function_coverage_gaps"], 4)
        self.assertEqual(counts["code_function_pair_status_counts"]["TARGET_REPORT_MISSING"], 2)
        self.assertEqual(counts["historical_aggregate_counts"]["coverage_gaps"], counts["coverage_gaps"])
        self.assertEqual(counts["pair_status_counts"]["TARGET_REPORT_MISSING"], 3)

    def test_raw_padding_rows_and_per_lane_accounting_survive_cache(self):
        fixture(self.root, target=elf(functions=[("func", 0, 8), ("gap_03_80001004_text", 4, 4), ("unreported_real", 4, 4)]))
        rows, run = self.rows(cache_dir=self.root / "cache")
        self.assertEqual(len(rows), 3)
        self.assertEqual(run.coverage["generated_padding_observations"], 1)
        self.assertEqual(run.coverage["code_function_observations"], 2)
        self.assertEqual(run.coverage["code_function_pair_status_counts"], {"PAIRED": 1, "TARGET_REPORT_MISSING": 1})
        self.assertEqual(run.coverage["per_lane"]["enemies"]["raw_observations"], 3)
        self.assertEqual(run.coverage["per_lane"]["enemies"]["code_function_coverage_gaps"], 1)
        cached, second = self.rows(cache_dir=self.root / "cache")
        self.assertEqual(cached, rows);self.assertEqual(second.coverage, run.coverage)

    def test_markdown_distinguishes_code_padding_and_historical_totals(self):
        rows = [{"name": "func", "pair_status": "PAIRED", "target": {"section": ".text"}},
                {"name": "gap_03_80001008_text", "pair_status": "TARGET_REPORT_MISSING", "target": {"section": ".text"}},
                {"name": "unreported_real", "pair_status": "TARGET_REPORT_MISSING", "target": {"section": ".text"}}]
        counts = common.observation_counts(rows)
        coverage = {"pairing": dict(counts, per_lane={"core": counts})}
        paths = common.write_results(self.root, self.root / "framed", "screen", [], coverage, {})
        text = Path(paths["lanes"]["core"]["markdown"]).read_text()
        self.assertIn("Code-function coverage: 1/2 paired; 1 unpaired.", text)
        self.assertIn("Raw observations: 3; generated DTK padding observations: 1 (all rows retained).", text)
        self.assertIn("Historical raw accounting, including padding: 1/3 paired; 2 unpaired.", text)
        self.assertIn("Real unreported functions remain code coverage gaps", text)
        self.assertIn("not semantic verdicts", text)

    def test_writer_preserves_rank_fields_coverage_and_gap_only_lane(self):
        coverage = {"pairing": {"per_lane": {"core": {"coverage_gaps": 2}, "enemies": {"coverage_gaps": 0}}}, "custom": {"count": 42}}
        findings = [{"unit": "Game/Enemy/X", "owner_lane": "enemies", "name": "f", "rank": rank, "custom": [rank], "detail": "offset4|width4"} for rank in (10, 90)]
        paths = common.write_results(self.root, self.root / "out", "screen", findings, coverage, {"stable": True})
        raw = Path(paths["json"]).read_bytes();data = json.loads(raw)
        self.assertEqual(data["coverage"], coverage);self.assertEqual([r["rank"] for r in data["findings"]], [90, 10])
        self.assertEqual(data["findings"][0]["custom"], [90]);self.assertIn("core", paths["lanes"])
        self.assertIn("offset4\\|width4", Path(paths["lanes"]["enemies"]["markdown"]).read_text())
        common.write_results(self.root, self.root / "out", "screen", findings, coverage, {"stable": True})
        self.assertEqual(raw, Path(paths["json"]).read_bytes())
        explicit = common.write_results(self.root, self.root / "explicit.json", "screen", [], coverage, {})
        self.assertTrue(Path(explicit["lanes"]["core"]["json"]).is_file())


if __name__ == "__main__":
    unittest.main()
