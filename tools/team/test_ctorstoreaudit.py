#!/usr/bin/env python3
"""Synthetic instruction tests; no repository object compilation or scan."""
import contextlib
import importlib.util
import io
from pathlib import Path
import sys
import tempfile
import types
import unittest
from unittest.mock import patch

HERE = Path(__file__).resolve().parent
SPEC = importlib.util.spec_from_file_location("ctorstoreaudit", HERE / "ctorstoreaudit.py")
audit = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = audit
SPEC.loader.exec_module(audit)


def d(op, rt, ra=0, imm=0):
    return op << 26 | rt << 21 | ra << 16 | (imm & 0xFFFF)


def x(xo, rt, ra, rb=0):
    return 31 << 26 | rt << 21 | ra << 16 | rb << 11 | xo << 1


def mr(dst, src):
    return x(444, src, dst, src)


def bc(displacement, bo=12):
    return 16 << 26 | bo << 21 | (displacement & 0xFFFC)


def b(displacement, link=False):
    return 18 << 26 | (displacement & 0x3FFFFFC) | int(link)


BLR = 0x4E800020
NOP = 0x60000000


def view(words, relocations=()):
    return {"address": 0x20, "size": 4 * len(words), "words": words,
            "section_index": 2, "symbol_index": 7, "relocations": list(relocations)}


def pair(target, source, target_relocs=(), source_relocs=(), **kwargs):
    result = {"unit": "Game/Example/Actor", "name": "__ct__5ActorFv",
              "demangled_name": "Actor::Actor()", "owner_lane": "objects",
              "target_va": 0x80001000, "target_size": len(target) * 4,
              "source_path": "src/Game/Example/Actor.cpp", "pair_status": "PAIRED",
              "occurrence_id": "raw-section-symbol-7", "target": view(target, target_relocs),
              "source": view(source, source_relocs)}
    result.update(kwargs)
    return result


def differences(result):
    return [f for f in result[0] if f["kind"] not in ("extraction_incomplete", "coverage_gap")]


class StoreAuditTests(unittest.TestCase):
    def test_missing_zero(self):
        result = audit.compare_pair(pair([d(14, 0), d(36, 0, 3, 8), BLR], [BLR]))
        finding, = differences(result)
        self.assertEqual((finding["kind"], finding["rank"]), ("missing_zero_init", 100))
        self.assertEqual(finding["target_store"]["offset"], 8)
        self.assertTrue(finding["advisory"])

    def test_unknown_source_value_at_same_field_is_coverage_not_missing_zero(self):
        result = audit.compare_pair(pair([d(14, 0), d(36, 0, 3, 8), BLR],
                                         [d(36, 12, 3, 8), BLR]))
        finding, = differences(result)
        self.assertEqual(finding["kind"], "source_value_unresolved")
        self.assertEqual(finding["rank"], 30)
        self.assertEqual(finding["source_store"]["offset"], 8)

    def test_incomplete_source_extraction_downgrades_nominal_missing_zero(self):
        result = audit.compare_pair(pair([d(14, 0), d(36, 0, 3, 8), BLR], [0, BLR]))
        finding, = differences(result)
        self.assertEqual(finding["kind"], "missing_zero_init")
        self.assertLess(finding["rank"], 50)
        self.assertIn("incomplete_extraction", finding["uncertainty"])

    def test_conditional_missing_zero_downgraded(self):
        finding, = differences(audit.compare_pair(pair(
            [d(14, 0), bc(8), d(36, 0, 3, 8), BLR], [BLR])))
        self.assertLess(finding["rank"], 100)
        self.assertIn("conditional_path", finding["uncertainty"])

    def test_wrong_offset(self):
        finding, = differences(audit.compare_pair(pair(
            [d(14, 0, imm=7), d(36, 0, 3, 8), BLR],
            [d(14, 0, imm=7), d(36, 0, 3, 12), BLR])))
        self.assertEqual(finding["kind"], "store_offset_change")
        self.assertEqual(finding["source_store"]["offset"], 12)

    def test_wrong_width(self):
        finding, = differences(audit.compare_pair(pair(
            [d(14, 0), d(36, 0, 3, 8), BLR], [d(14, 0), d(38, 0, 3, 8), BLR])))
        self.assertEqual(finding["kind"], "store_width_change")
        self.assertEqual(finding["source_store"]["width"], 1)

    def test_value_change(self):
        finding, = differences(audit.compare_pair(pair(
            [d(14, 0), d(44, 0, 3, 8), BLR], [d(14, 0, imm=1), d(44, 0, 3, 8), BLR])))
        self.assertEqual(finding["kind"], "value_source_change")

    def test_wrong_width_still_visible_when_source_value_unknown(self):
        finding, = differences(audit.compare_pair(pair(
            [d(14, 0), d(36, 0, 3, 8), BLR], [d(38, 12, 3, 8), BLR])))
        self.assertEqual(finding["kind"], "store_width_change")
        self.assertLess(finding["rank"], 50)

    def test_relocated_symbol_address_requires_correct_ha_lo_pair(self):
        words = [d(15, 6), d(14, 6, 6), d(36, 6, 3, 8), BLR]
        relocs = [{"offset": 0, "symbol": "vtable", "type": 6, "addend": 0},
                  {"offset": 4, "symbol": "vtable", "type": 4, "addend": 0}]
        result = audit.analyze_view(view(words, relocs))
        self.assertEqual(result["stores"][0]["register_value"], ("symbol", "vtable", 0))
        wrong = [{**relocs[0], "type": 5}, relocs[1]]
        result = audit.analyze_view(view(words, wrong))
        self.assertIsNone(result["stores"][0]["value_source"])

    def test_local_and_implicit_relocation_identity_not_equated(self):
        self.assertFalse(audit.symbolic_relocation({"symbol": "@12", "addend": 0}))
        self.assertFalse(audit.symbolic_relocation({"symbol": "table", "addend": 0, "symbol_binding": 0}))
        self.assertFalse(audit.symbolic_relocation({"symbol": "table", "addend": None, "implicit_addend": True}))

    def test_saved_this_and_reordered_renamed_stores(self):
        target = [mr(31, 3), d(14, 0), d(36, 0, 31, 8), d(38, 4, 31, 12), BLR]
        source = [mr(30, 3), d(14, 8), d(38, 4, 30, 12), d(36, 8, 30, 8), BLR]
        result = audit.compare_pair(pair(target, source))
        self.assertEqual(result[0], [])
        self.assertEqual(result[1]["matched_store_occurrences"], 2)
        self.assertEqual(result[1]["constructors_with_reordered_known_stores"], 1)

    def test_this_addi_and_negative_offsets(self):
        result = audit.analyze_view(view([d(14, 31, 3, 20), d(14, 0), d(36, 0, 31, -8), BLR]))
        self.assertEqual(result["stores"][0]["offset"], 12)

    def test_repeated_stores_not_collapsed(self):
        result = audit.compare_pair(pair([d(14, 0), d(36, 0, 3, 8), d(36, 0, 3, 8), BLR],
                                         [d(14, 0), d(36, 0, 3, 8), BLR]))
        self.assertEqual(len(differences(result)), 1)
        self.assertEqual(result[1]["matched_store_occurrences"], 1)
        self.assertEqual(result[1]["unmatched_target_store_occurrences"], 1)

    def test_integer_truncation_is_width_aware(self):
        result = audit.compare_pair(pair([d(14, 0, imm=-1), d(38, 0, 3, 8), BLR],
                                         [d(14, 0, imm=255), d(38, 0, 3, 8), BLR]))
        self.assertEqual(result[0], [])

    def test_this_field_and_argument_value_sources(self):
        target = [d(32, 6, 3, 8), d(36, 6, 3, 12), d(36, 4, 3, 16), BLR]
        source = [d(32, 9, 3, 8), d(36, 4, 3, 16), d(36, 9, 3, 12), BLR]
        self.assertEqual(audit.compare_pair(pair(target, source))[0], [])

    def test_branch_join_drops_disagreeing_values(self):
        words = [bc(12), d(14, 0), b(8), d(14, 0, imm=1), d(36, 0, 3, 8), BLR]
        result = audit.analyze_view(view(words))
        self.assertIsNone(result["stores"][0]["value_source"])
        self.assertTrue(result["stores"][0]["conditional"])
        self.assertIn("store_value_unknown", {i["kind"] for i in result["issues"]})

    def test_branch_paths_identical_constant_retained_but_conditional(self):
        result = audit.analyze_view(view([d(14, 0), bc(8), NOP, d(36, 0, 3, 8), BLR]))
        self.assertEqual(result["stores"][0]["value_source"], ("bits", 4, 0))
        self.assertTrue(result["stores"][0]["conditional"])

    def test_unreachable_store_not_counted(self):
        result = audit.analyze_view(view([b(12), d(14, 0), d(36, 0, 3, 8), BLR]))
        self.assertEqual(result["stores"], [])
        self.assertEqual(result["reachable_instructions"], 2)

    def test_loop_converges_without_inventing_iterations(self):
        words = [d(14, 0), d(36, 0, 3, 8), bc(-4), BLR]
        result = audit.analyze_view(view(words))
        self.assertEqual(len(result["stores"]), 1)
        self.assertEqual(result["reachable_instructions"], 4)

    def test_unsupported_instruction_cannot_leave_stale_this(self):
        result = audit.analyze_view(view([0, d(14, 0), d(36, 0, 3, 8), BLR]))
        self.assertEqual(result["stores"], [])
        self.assertEqual({i["kind"] for i in result["issues"]},
                         {"unsupported_instruction", "unresolved_store_base"})

    def test_unknown_store_value_explicit_and_never_matches_unknown(self):
        words = [d(36, 12, 3, 8), BLR]
        result = audit.compare_pair(pair(words, words))
        self.assertEqual(result[1]["matched_store_occurrences"], 0)
        self.assertEqual(result[1]["target_unknown_stores"], 1)
        self.assertEqual(result[0][0]["kind"], "extraction_incomplete")

    def test_call_clobber_loses_volatile_alias_but_preserves_saved_this(self):
        words = [mr(31, 3), mr(4, 3), b(400, True), d(14, 0),
                 d(36, 0, 4, 8), d(36, 0, 31, 12), BLR]
        result = audit.analyze_view(view(words))
        self.assertEqual([s["offset"] for s in result["stores"]], [12])
        self.assertIn("unresolved_store_base", {i["kind"] for i in result["issues"]})

    def test_call_clobbers_argument_value_and_floating_input(self):
        result = audit.analyze_view(view([mr(31, 3), b(400, True), d(36, 4, 31, 8),
                                          d(52, 1, 31, 12), BLR]))
        self.assertTrue(all(s["value_source"] is None for s in result["stores"]))

    def test_exact_save_helper_preserves_incoming_this(self):
        words = [b(400, True), mr(31, 3), d(14, 0), d(36, 0, 31, 8), BLR]
        relocs = [{"offset": 0, "symbol": "_savegpr_29", "type": 10, "addend": 0}]
        result = audit.analyze_view(view(words, relocs))
        self.assertEqual(result["scaffold_calls"], 1)
        self.assertEqual(result["stores"][0]["offset"], 8)

    def test_alias_helper_addend_and_near_names_fail_closed(self):
        self.assertEqual(audit.scaffold({"symbol": "__save_gpr", "addend": 60, "type": 10}), ("save", "gpr", 29))
        self.assertIsNone(audit.scaffold({"symbol": "__save_gpr", "addend": 2, "type": 10}))
        self.assertIsNone(audit.scaffold({"symbol": "my_savegpr_29", "addend": 0, "type": 10}))
        self.assertIsNone(audit.scaffold({"symbol": "_savegpr_29", "addend": 4, "type": 10}))
        self.assertIsNone(audit.scaffold({"symbol": "_savegpr_29", "addend": 0, "type": 1}))

    def test_restore_helper_invalidates_restored_saved_register(self):
        words = [mr(31, 3), b(400, True), d(14, 0), d(36, 0, 31, 8), d(36, 0, 3, 12), BLR]
        relocs = [{"offset": 4, "symbol": "_restgpr_29", "type": 10, "addend": 0}]
        result = audit.analyze_view(view(words, relocs))
        self.assertEqual([s["offset"] for s in result["stores"]], [12])

    def test_tail_restore_helper_is_known_scaffold_exit(self):
        words = [d(14, 0), d(36, 0, 3, 8), b(400)]
        reloc = {"offset": 8, "symbol": "__restore_gpr", "type": 10, "addend": 60}
        result = audit.analyze_view(view(words, [reloc]))
        self.assertEqual(result["issues"], [])
        self.assertEqual(result["scaffold_calls"], 1)
        self.assertEqual(len(result["stores"]), 1)

    def test_float_literal_bits_and_fmr(self):
        target = [d(48, 1, 2, 32), d(52, 1, 3, 8), BLR]
        source = [d(48, 2, 2, 40), (63 << 26 | 1 << 21 | 2 << 11 | 72 << 1), d(52, 1, 3, 8), BLR]
        relocs = [{"offset": 2, "symbol": "@12", "addend": 0, "type": 109, "literal_bytes_hex": "00000000"}]
        other = [{**relocs[0], "symbol": "@99"}]
        result = audit.compare_pair(pair(target, source, relocs, other))
        self.assertEqual(result[0], [])
        self.assertEqual(result[1]["matched_store_occurrences"], 1)

    def test_float_signed_zero_difference(self):
        words = [d(48, 1, 2), d(52, 1, 3, 8), BLR]
        def literal(bits):
            return [{"offset": 0, "symbol": "@1", "type": 109, "addend": 0, "literal_bytes_hex": bits}]
        finding, = differences(audit.compare_pair(pair(words, words, literal("00000000"), literal("80000000"))))
        self.assertEqual(finding["kind"], "value_source_change")

    def test_float_literal_missing_or_ambiguous_is_unknown(self):
        reloc = {"offset": 0, "symbol": "@1", "type": 109, "addend": 0, "literal_bytes_hex": "0000000000000000"}
        words = [d(48, 1, 2), d(52, 1, 3, 8), BLR]
        result = audit.analyze_view(view(words, [reloc]))
        self.assertIsNone(result["stores"][0]["value_source"])
        result = audit.analyze_view(view(words, [reloc, reloc]))
        self.assertIsNone(result["stores"][0]["value_source"])

    def test_literal_raw_relocations_never_interpreted_as_constant(self):
        words = [d(48, 1, 2), d(52, 1, 3, 8), BLR]
        reloc = {"offset": 0, "symbol": "@1", "type": 109, "addend": 0,
                 "literal_bytes_hex": "00000000", "literal_relocations_present": True}
        result = audit.analyze_view(view(words, [reloc]))
        self.assertIsNone(result["stores"][0]["value_source"])

    def test_double_to_single_only_safe_zero_conversion(self):
        self.assertEqual(audit.stored_value(("f64", 0), 4, True), ("bits", 4, 0))
        self.assertEqual(audit.stored_value(("f64", 0x8000000000000000), 4, True), ("bits", 4, 0x80000000))
        self.assertIsNone(audit.stored_value(("f64", 0x3FF0000000000000), 4, True))
        self.assertIsNone(audit.stored_value(("f64", 0x3FF0000000000001), 4, True))
        self.assertIsNone(audit.stored_value(("f64", 0x7FEFFFFFFFFFFFFF), 4, True))
        self.assertIsNone(audit.stored_value(("f32", 0x7F800001), 4, True))

    def test_original_double_bits_retained_in_store_evidence(self):
        words = [d(50, 1, 2), d(52, 1, 3, 8), BLR]
        reloc = {"offset": 0, "symbol": "@1", "type": 109, "addend": 0,
                 "literal_bytes_hex": "3ff0000000000000"}
        result = audit.analyze_view(view(words, [reloc]))
        self.assertEqual(result["stores"][0]["register_value"], ("f64", 0x3FF0000000000000))
        self.assertIsNone(result["stores"][0]["value_source"])

    def test_update_forms_and_stack_scaffold(self):
        words = [d(37, 1, 1, -32), d(14, 0), d(37, 0, 3, 8), d(38, 0, 3, 4), BLR]
        result = audit.analyze_view(view(words))
        self.assertEqual([(s["offset"], s["width"]) for s in result["stores"]], [(8, 4), (12, 1)])
        self.assertEqual(result["issues"], [])

    def test_inline_multiple_register_stack_scaffold_preserves_this(self):
        words = [d(37, 1, 1, -32), d(47, 30, 1, 24), mr(31, 3),
                 d(14, 0), d(36, 0, 31, 8), d(46, 30, 1, 24), BLR]
        result = audit.analyze_view(view(words))
        self.assertEqual([s["offset"] for s in result["stores"]], [8])
        self.assertEqual(result["issues"], [])

    def test_indirect_control_and_unmodeled_this_store_explicit(self):
        words = [d(54, 1, 3, 8), 0x4E800420]
        result = audit.analyze_view(view(words))
        self.assertEqual({i["kind"] for i in result["issues"]},
                         {"unsupported_this_store", "indirect_control_flow_unsupported"})

    def test_coverage_gap_retains_occurrence(self):
        record = pair([BLR], [], pair_status="SOURCE_AMBIGUOUS")
        record["source"] = None
        result = audit.compare_pair(record)
        self.assertEqual(result[0][0]["kind"], "coverage_gap")
        self.assertEqual(result[0][0]["occurrence_id"], "raw-section-symbol-7")
        self.assertEqual(result[1]["coverage_gaps"], 1)

    def test_constructor_filter(self):
        self.assertTrue(audit.is_constructor({"name": "__ct__5ActorFv"}))
        self.assertTrue(audit.is_constructor({"name": "alias", "demangled_name": "ns::Actor::Actor(int)"}))
        self.assertFalse(audit.is_constructor({"name": "foo__5ActorFv", "demangled_name": "Actor::foo()"}))
        self.assertFalse(audit.is_constructor({"name": "__dt__5ActorFv", "demangled_name": "Actor::~Actor()"}))

    def test_cli_uses_common_frontend_and_preserves_coverage_metadata(self):
        records = [pair([d(14, 0), d(36, 0, 3, 8), BLR], [BLR]),
                   pair([BLR], [BLR], name="other", demangled_name="Actor::other()")]
        class Pairs(list):
            coverage = {"PAIRED": 2}
            run_metadata = {"stable_pin": "synthetic"}
        module = types.ModuleType("mech_audit_common")
        calls = []
        def iter_pairs(root, prefixes, cache_dir):
            calls.append((root, prefixes, cache_dir))
            return Pairs(records)
        def write_results(root, output, name, findings, coverage, metadata):
            calls.append((name, findings, coverage, metadata))
            return {"json": str(output / "ctorstoreaudit.json")}
        module.iter_pairs, module.write_results = iter_pairs, write_results
        with tempfile.TemporaryDirectory() as tmp, patch.dict(sys.modules, {"mech_audit_common": module}):
            with contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(audit.main(["--root", tmp, "--output", tmp, "--prefix", "Game/Enemy/"]), 0)
        self.assertEqual(calls[0][1], ("Game/Enemy/",))
        self.assertEqual(calls[1][0], "ctorstoreaudit")
        self.assertEqual(calls[1][2]["pairing"], {"PAIRED": 2})
        self.assertEqual(calls[1][2]["algorithm"]["constructors"], 1)
        self.assertEqual(calls[1][3]["stable_pin"], "synthetic")
        self.assertEqual(calls[1][3]["analyzer"], audit.analyzer_pin())

    def test_analyzer_drift_aborts_before_any_results_written(self):
        class Pairs(list):
            coverage = {}
            run_metadata = {}
        module = types.ModuleType("mech_audit_common")
        module.iter_pairs = lambda *args, **kwargs: Pairs()
        written = []
        module.write_results = lambda *args, **kwargs: written.append(args)
        before = {"path": "/synthetic/ctorstoreaudit.py", "sha256": "0" * 64}
        after = {**before, "sha256": "1" * 64}
        with patch.dict(sys.modules, {"mech_audit_common": module}):
            with patch.object(audit, "analyzer_pin", side_effect=[before, after]):
                with self.assertRaisesRegex(RuntimeError, "changed during extraction"):
                    audit.main(["--output", "/synthetic/output"])
        self.assertEqual(written, [])


if __name__ == "__main__":
    unittest.main()
