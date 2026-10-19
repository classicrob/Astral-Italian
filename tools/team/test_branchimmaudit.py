import json
from pathlib import Path
import sys
from types import SimpleNamespace
import unittest
from unittest.mock import patch

from branchimmaudit import audit_pairs, compare_streams, decode_word, extract_stream, main, ppc_mask, signature


def compare(imm, unsigned=False, wide=False, ra=3, cr=0):
    return ((10 if unsigned else 11) << 26) | (cr << 23) | (int(wide) << 21) | (ra << 16) | (imm & 0xffff)


def rotate(sh=0, mb=0, me=31, rc=0, rs=3, ra=4):
    return (21 << 26) | (rs << 21) | (ra << 16) | (sh << 11) | (mb << 6) | (me << 1) | rc


def view(words, rels=()):
    return {"address": 0, "size": len(words) * 4, "words": words,
            "section_index": 1, "symbol_index": 1, "relocations": list(rels)}


def pair(target, source, status="PAIRED", name="f"):
    return {"unit": "Game/Test", "name": name, "demangled_name": name, "owner_lane": "core",
            "target_va": 0x80001000, "target_size": len(target) * 4,
            "source_path": "src/Game/Test.cpp", "pair_status": status, "occurrence_id": name,
            "target": view(target), "source": None if source is None else view(source)}


def screen(target, source):
    return compare_streams(extract_stream(view(target)), extract_stream(view(source)))


class TestDecoder(unittest.TestCase):
    def test_signed_unsigned_thresholds(self):
        self.assertEqual(decode_word(0x2c03ffff)["immediate"], -1)
        self.assertEqual(decode_word(0x2803ffff)["immediate"], 65535)
        self.assertEqual(decode_word(compare(0x8000))["immediate"], -32768)
        self.assertEqual(decode_word(compare(0x7fff, True))["immediate"], 32767)

    def test_width_retained(self):
        self.assertEqual(decode_word(compare(7, wide=True))["width"], 64)
        self.assertNotEqual(signature(decode_word(compare(7))), signature(decode_word(compare(7, wide=True))))

    def test_rlwinm_alias_words(self):
        clrlwi = decode_word(0x5460063e)
        self.assertEqual((clrlwi["opcode"], clrlwi["sh"], clrlwi["mb"], clrlwi["me"]), ("rlwinm", 0, 24, 31))
        self.assertEqual(clrlwi["effective_mask"], 255)
        slwi = decode_word(rotate(sh=3, mb=0, me=28))
        self.assertEqual(slwi["effective_mask"], 0xfffffff8)
        extrwi = decode_word(rotate(sh=13, mb=24, me=31))
        self.assertEqual(extrwi["sh"], 13)
        self.assertEqual(extrwi["effective_mask"], 255)

    def test_wrapping_mask(self):
        self.assertEqual(ppc_mask(28, 3), 0xf000000f)
        self.assertEqual(ppc_mask(0, 31), 0xffffffff)
        self.assertEqual(ppc_mask(13, 13), 1 << 18)

    def test_andi_andis_separate(self):
        low, high = decode_word(0x70640080), decode_word(0x74640080)
        self.assertEqual(low["effective_mask"], 0x80)
        self.assertEqual(high["effective_mask"], 0x800000)
        self.assertNotEqual(signature(low), signature(high))

    def test_reject_bad_words_and_extents(self):
        with self.assertRaises(ValueError):
            decode_word(-1)
        with self.assertRaises(ValueError):
            extract_stream({"words": [0], "size": 3})
        self.assertIsNone(decode_word(0x60000000))


class TestScreen(unittest.TestCase):
    def test_planted_mask_bit13_to19(self):
        rows = screen([rotate(mb=18, me=18)], [rotate(mb=12, me=12)])
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]["changed_mask_bits_lsb0"], [13, 19])
        self.assertEqual(rows[0]["rank"], 100)
        self.assertEqual(set(rows[0]["changed_fields"]), {"mb", "me"})

    def test_single_compare_constant(self):
        row = screen([compare(12)], [compare(13)])[0]
        self.assertEqual(row["kind"], "compare-immediate-drift")
        self.assertEqual(row["changed_fields"]["immediate_bits"], {"target": 12, "source": 13})

    def test_signedness_is_screen_only_and_threshold_extension_matters(self):
        same = screen([compare(1)], [compare(1, True)])[0]
        different = screen([compare(0xffff)], [compare(0xffff, True)])[0]
        self.assertEqual(same["kind"], "compare-mode-drift")
        self.assertTrue(same["same_threshold_bit_pattern"])
        self.assertFalse(different["same_threshold_bit_pattern"])
        self.assertLess(same["rank"], 100)

    def test_single_insertion_does_not_cascade(self):
        rows = screen([compare(1), compare(2), compare(3)],
                      [compare(1), compare(99), compare(2), compare(3)])
        self.assertEqual([r["kind"] for r in rows], ["source-extra-events"])
        self.assertEqual(rows[0]["source"][0]["immediate"], 99)

    def test_deletion_and_reordering_are_lower_confidence(self):
        self.assertEqual(screen([compare(1), compare(2)], [compare(1)])[0]["kind"], "source-missing-events")
        row = screen([compare(1), compare(2)], [compare(2), compare(1)])[0]
        self.assertEqual(row["kind"], "stream-reordered")
        self.assertLess(row["rank"], 60)

    def test_register_rename_and_unselected_offsets_ignored(self):
        self.assertEqual(screen([compare(1, ra=3, cr=1), rotate(rs=3, ra=4)],
                                [0x60000000, compare(1, ra=12, cr=6), rotate(rs=9, ra=10)]), [])

    def test_record_bit_change(self):
        row = screen([rotate(rc=0)], [rotate(rc=1)])[0]
        self.assertEqual(row["changed_fields"], {"rc": {"target": 0, "source": 1}})

    def test_branch_proximity_is_explicit_context(self):
        words = [compare(1), 0x41820008, 0x4e800020]
        event = extract_stream(view(words))[0]
        self.assertEqual(len(event["nearby_branches"]), 1)
        branch = event["nearby_branches"][0]
        self.assertEqual((branch["cr_bit"], branch["distance_instructions"], branch["same_cr_field"]), ("EQ", 1, True))
        self.assertTrue(screen(words, [compare(2), *words[1:]])[0]["branch_context_is_not_def_use_proof"])

    def test_relocation_is_uncompared_not_raw_equal(self):
        p = pair([compare(0)], [compare(0)])
        p["source"]["relocations"] = [{"offset": 2, "type": 4, "symbol": "x", "addend": 0}]
        rows, coverage = audit_pairs([p])
        self.assertEqual(rows[0]["kind"], "relocated-immediate-stream-uncompared")
        self.assertEqual(coverage["equal_stream_occurrences"], 0)

    def test_coverage_missing_ambiguous_and_zero_events(self):
        rows, coverage = audit_pairs([pair([], None, "SOURCE_MISSING", "missing"),
                                     pair([], None, "SOURCE_AMBIGUOUS", "duplicate"), pair([], [], name="empty")])
        self.assertEqual(rows, [])
        self.assertEqual(coverage["uncompared_occurrences"], 2)
        self.assertEqual(coverage["zero_selected_event_occurrences"], 1)

    def test_invalid_view_and_empty_coverage_are_explicit(self):
        p = pair([compare(1)], [compare(1)])
        p["source"]["size"] = 3
        rows, coverage = audit_pairs([p])
        self.assertEqual(rows[0]["kind"], "invalid-function-view")
        self.assertEqual(coverage["invalid_view_occurrences"], 1)
        self.assertEqual(coverage["compared_occurrences"], 0)
        _, empty = audit_pairs([])
        self.assertEqual(empty["registered_occurrences"], 0)
        self.assertEqual(empty["equal_stream_occurrences"], 0)

    def test_deterministic_rank_and_metadata(self):
        pairs = [pair([compare(1)], [], name="low"), pair([compare(1)], [compare(2)], name="high")]
        a, ca = audit_pairs(pairs)
        b, cb = audit_pairs(reversed(pairs))
        self.assertEqual(json.dumps((a, ca), sort_keys=True), json.dumps((b, cb), sort_keys=True))
        self.assertEqual(a[0]["name"], "high")
        self.assertTrue(all(x["advisory_only"] and not x["semantic_bug_claimed"] for x in a))
        self.assertEqual(a[0]["target"]["virtual_address"], 0x80001000)
        self.assertEqual(a[0]["target_event_count"], 1)
        self.assertIn("target +0x0; source +0x0", a[0]["detail"])
        self.assertIn("immediate_bits 1 -> 2", a[0]["detail"])

    def test_cli_shared_interface_without_workspace_read(self):
        class Pairs(list):
            coverage = {"synthetic": True}
            run_metadata = {"inputs_stable": True}

        observed = {}

        def iterate(root, prefixes):
            observed["root"], observed["prefixes"] = root, prefixes
            return Pairs([pair([compare(1)], [compare(2)])])

        def write(root, output, name, findings, coverage, metadata):
            observed.update(output=output, name=name, findings=findings, coverage=coverage, metadata=metadata)
            return {"json": "synthetic.json"}

        module = SimpleNamespace(iter_pairs=iterate, write_results=write)
        with patch.dict(sys.modules, {"mech_audit_common": module}), patch("builtins.print"):
            self.assertEqual(main(["--root", "/synthetic", "--output", "/synthetic-output"]), 0)
        self.assertEqual(observed["root"], Path("/synthetic"))
        self.assertEqual(observed["prefixes"], ("Game/",))
        self.assertEqual(observed["coverage"]["pairing"], {"synthetic": True})
        self.assertTrue(observed["findings"][0]["advisory_only"])
        provenance = observed["metadata"]["analyzer_source"]
        self.assertEqual(len(provenance["sha256"]), 64)
        self.assertEqual(provenance["start_sha256"], provenance["end_sha256"])
        self.assertEqual(Path(provenance["path"]).name, "branchimmaudit.py")
        self.assertTrue(provenance["stable"])
        with patch.dict(sys.modules, {"mech_audit_common": module}), patch("builtins.print"):
            main(["--output", "/synthetic-output", "--prefix", "Game/Foo/", "--prefix", "Game/Bar/"])
        self.assertEqual(observed["prefixes"], ("Game/Foo/", "Game/Bar/"))

    def test_analyzer_drift_aborts_before_writer(self):
        class Pairs(list):
            coverage = {}
            run_metadata = {}

        writer = unittest.mock.Mock()
        module = SimpleNamespace(iter_pairs=lambda *args, **kwargs: Pairs(), write_results=writer)
        with patch.dict(sys.modules, {"mech_audit_common": module}), \
                patch("branchimmaudit._tool_sha256", side_effect=["a" * 64, "b" * 64]):
            with self.assertRaisesRegex(RuntimeError, "report publication aborted"):
                main(["--output", "/synthetic-output"])
        writer.assert_not_called()


if __name__ == "__main__":
    unittest.main()
