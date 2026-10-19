#!/usr/bin/env python3
"""Compiler-free safety regressions for the candidate-only source permuter.

PERMUTE_TOOL may select a peer's uncommitted implementation during review.
The normal entry point imports sibling permute.py; no checkout fixture is needed.
"""
import copy
import importlib.util
import os
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest import mock


def load_tool():
    path = Path(os.environ.get("PERMUTE_TOOL", Path(__file__).with_name("permute.py")))
    spec = importlib.util.spec_from_file_location("permute_under_test", path)
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def elf_fixture(path, *, first_data=b"\x3f\x80\x00\x00\x40\x00\x00\x00",
                executable_tail=b"\x60\x00\x00\x00", bss_size=12, local_ref=None,
                global_ref=False):
    """Small real ELF32/PPC object, including duplicate section names and text gaps."""
    names = ["", ".text", ".rodata", ".rodata", ".bss", ".symtab",
             ".strtab", ".shstrtab"]
    section_strings = b"\0.text\0.rodata\0.bss\0.symtab\0.strtab\0.shstrtab\0"
    strings = b"\0chosen\0neighbor\0leftFlag\0rightFlag\0"
    sym = lambda name, value, size, info, section: struct.pack(
        ">IIIBBH", strings.index(name.encode() + b"\0") if name else 0,
        value, size, info, 0, section)
    symbols = bytes(16) + sym("chosen", 0, 4, 0x12, 1) + sym("neighbor", 4, 4, 0x12, 1)
    payloads = [b"", bytes.fromhex("4e8000204e800020") + executable_tail,
                first_data, b"abc\0", b"", symbols, strings, section_strings]
    types = [0, 1, 1, 1, 8, 2, 3, 3]
    flags = [0, 6, 2, 2, 3, 0, 0, 0]
    if local_ref is not None:
        # Equal-valued local BSS objects remain distinct mutable identities.
        section_strings += b".rela.text\0"
        names.append(".rela.text")
        binding, storage = (0x11, 2) if global_ref else (0x01, 4)
        symbols = (bytes(16) + sym("leftFlag", 0, 4, binding, storage) +
                   sym("rightFlag", 4, 4, binding, storage) +
                   sym("chosen", 0, 12, 0x12, 1) + sym("neighbor", 12, 4, 0x12, 1))
        payloads[1] = bytes.fromhex("3c600000386300004e8000204e800020") + executable_tail
        payloads[5] = symbols
        payloads[7] = section_strings
        symbol_index = 1 if local_ref == "left" else 2
        payloads.append(struct.pack(">IIiIIi", 2, (symbol_index << 8) | 6, 0,
                                    6, (symbol_index << 8) | 4, 0))
        types.append(4)
        flags.append(0)
    blob = bytearray(52)
    headers = []
    for index, data in enumerate(payloads):
        while len(blob) % 4:
            blob.append(0)
        offset = len(blob)
        blob.extend(data)
        headers.append((section_strings.index(names[index].encode() + b"\0") if index else 0,
                        types[index], flags[index], 0, offset,
                        bss_size if index == 4 else len(data),
                        6 if index == 5 else (5 if index == 8 else 0),
                        (3 if local_ref is not None and not global_ref else 1) if index == 5 else (1 if index == 8 else 0),
                        4 if index in [1, 2, 3, 4, 5] else 1,
                        16 if index == 5 else (12 if index == 8 else 0)))
    headers[0] = (0,) * 10
    while len(blob) % 4:
        blob.append(0)
    section_offset = len(blob)
    for header in headers:
        blob.extend(struct.pack(">10I", *header))
    ident = b"\x7fELF\x01\x02\x01" + bytes(9)
    blob[:52] = struct.pack(">16sHHIIIIIHHHHHH", ident, 1, 20, 1, 0, 0,
                            section_offset, 0, 52, 0, 0, 40, len(headers), 7)
    path.write_bytes(blob)


class TransformSafety(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tool = load_tool()

    def variants(self, source, budget=20):
        span = self.tool.find_function(source, "Actor::run")
        variants = self.tool.generate_variants(source, span, seed=7, budget=budget)
        for variant in variants:
            self.assertLessEqual(span.body_start + 1, variant.start)
            self.assertLessEqual(variant.end, span.body_end)
            self.assertEqual(source[variant.start:variant.end], variant.before)
            self.assertEqual(
                variant.source,
                source[:variant.start] + variant.after + source[variant.end:],
            )
            self.assertNotEqual(source, variant.source)
            self.assertEqual(source[:span.body_start + 1],
                             variant.source[:span.body_start + 1])
            suffix_length = len(source) - span.body_end
            self.assertEqual(source[span.body_end:], variant.source[-suffix_length:])
        self.assertLessEqual(len(variants), budget)
        return variants

    def test_function_boundaries_ignore_comment_and_string_decoys(self):
        source = '''// void Actor::run() { not_a_definition(); }
const char* decoy = "Actor::run() { }";
void Actor::runner() { unrelated(); }
void Actor::run() {
    const char* text = "} not the end {";
    /* } */
    work();
}
void Actor::after() { untouched(); }
'''
        span = self.tool.find_function(source, "Actor::run")
        self.assertEqual(source[span.body_start], "{")
        self.assertEqual(source[span.body_end], "}")
        self.assertIn("work();", source[span.body_start:span.body_end])
        self.assertNotIn("untouched", source[span.body_start:span.body_end])

    def test_ambiguous_overload_is_rejected(self):
        source = "void Actor::run(int a) {}\nvoid Actor::run(float a) {}\n"
        with self.assertRaises((ValueError, RuntimeError)):
            self.tool.find_function(source, "Actor::run")

    def test_selector_does_not_match_longer_identifier(self):
        with self.assertRaises((ValueError, RuntimeError)):
            self.tool.find_function("void Actor::runner() {}", "Actor::run")

    def test_literal_declaration_swap_is_local_and_deterministic(self):
        source = '''int before = 9;
void Actor::run() {
    float first = 1.0f;
    float second = 2.0f;
    use(first, second);
}
int after = 8;
'''
        variants = self.variants(source)
        self.assertTrue(variants)
        self.assertEqual(variants, self.variants(source))
        self.assertTrue(any(v.source.index("float second") <
                            v.source.index("float first") for v in variants))

    def test_zero_budget_has_no_candidates(self):
        source = "void Actor::run() { int first = 1; int second = 2; }"
        self.assertEqual(self.variants(source, budget=0), [])

    def test_literal_swap_does_not_cross_side_effect_or_dependency(self):
        bodies = [
            "float first = read();\nfloat second = 2.0f;",
            "float first = 1.0f;\nfloat second = first;",
            "float first = 1.0f;\nobserve();\nfloat second = 2.0f;",
            "volatile int first = 1;\nvolatile int second = 2;",
            "Scalar first = 1;\nScalar second = 2;",
            "int first = 1, second = 2;",
            "int& first = *p;\nint second = 2;",
            "*p = 1;\n*q = 2;",
        ]
        for body in bodies:
            with self.subTest(body=body):
                self.assertEqual(self.variants("void Actor::run() {\n" + body + "\n}"), [])

    def test_fp_grouping_and_precision_are_never_rewritten(self):
        for expression in ["(a + b) + c", "a + (b + c)", "a * b + c",
                           "(double)a * b", "a - b", "a / b"]:
            source = "void Actor::run() { float value = " + expression + "; consume(value); }"
            with self.subTest(expression=expression):
                self.assertEqual(self.variants(source), [])

    def test_declaration_text_in_comment_and_raw_string_is_not_code(self):
        bodies = ['''/*
int first = 1;
int second = 2;
*/''', '''const char* text = R"payload(
int first = 1;
int second = 2;
)payload";
consume(text);''']
        for body in bodies:
            with self.subTest(body=body):
                self.assertEqual(self.variants("void Actor::run() {\n" + body + "\n}"), [])

    def test_spliced_line_comment_does_not_expose_a_declaration(self):
        source = ("int first = 9;\nint second = 8;\nvoid Actor::run() {\n"
                  "    // disabled declaration " + "\\" + "\n"
                  "    int first = 1;\n    int second = 2;\n"
                  "    consume(first, second);\n}\n")
        try:
            variants = self.variants(source)
        except ValueError:
            return  # Unsupported translation-phase splicing may fail closed.
        self.assertEqual(variants, [])

    def test_class_shadows_of_bool_parameters_are_not_builtin(self):
        source = '''void Actor::run(bool first, bool second) {
    {
        Odd first;
        Odd second;
        if (first) {
            if (second) { accept(); }
        }
    }
}
'''
        self.assertEqual(self.variants(source), [])

    def test_builtin_short_circuit_has_a_candidate(self):
        source = '''void Actor::run(bool first, bool second) {
    if (first) {
        if (second) {
            accept();
        }
    }
}
'''
        variants = self.variants(source)
        self.assertTrue(variants)
        for variant in variants:
            self.assertLess(variant.source.index("first", variant.source.index("{")),
                            variant.source.index("second", variant.source.index("{")))
            self.assertEqual(variant.source.count("accept();"), 1)

    def test_unknown_or_observable_conditions_are_not_rewritten(self):
        signatures_and_conditions = [
            ("Boolish first, Boolish second", "first", "second"),
            ("volatile bool first, volatile bool second", "first", "second"),
            ("bool second", "mReady", "second"),
            ("bool second", "ready()", "second"),
            ("bool second", "*p", "second"),
            ("int first, int second", "first++", "second"),
        ]
        for parameters, first, second in signatures_and_conditions:
            source = ("void Actor::run(" + parameters + ") { if (" + first +
                      ") { if (" + second + ") { accept(); } } }")
            with self.subTest(parameters=parameters, first=first):
                self.assertEqual(self.variants(source), [])

    def test_else_and_preprocessor_regions_are_not_rewritten(self):
        bodies = [
            "if (first) { if (second) { accept(); } else { reject(); } }",
            "#if READY\nint a = 1;\nint b = 2;\n#endif",
        ]
        for body in bodies:
            with self.subTest(body=body):
                source = "void Actor::run(bool first, bool second) {\n" + body + "\n}"
                try:
                    variants = self.variants(source)
                except ValueError:
                    if "#" not in body:
                        raise
                else:
                    self.assertEqual(variants, [])

    def test_builtin_bool_if_else_inversion_preserves_both_arms(self):
        source = '''void Actor::run(bool first) {
    if (first) { accept(); } else { reject(); }
}
'''
        variants = self.variants(source)
        self.assertTrue(variants)
        for variant in variants:
            tokens = [t.text for t in self.tool.lex(variant.source)]
            start = tokens.index("if")
            self.assertEqual(tokens[start:start + 5], ["if", "(", "!", "first", ")"])
            self.assertLess(variant.source.index("reject();"), variant.source.index("accept();"))
            self.assertEqual(variant.source.count("accept();"), 1)
            self.assertEqual(variant.source.count("reject();"), 1)


class PreservationSafety(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tool = load_tool()

    def setUp(self):
        self.baseline = {
            "functions": {
                "chosen": {"size": 4, "bytes": "4e800020", "relocations": []},
                "neighbor": {"size": 4, "bytes": "38600001", "relocations": []},
            },
            "data": {
                ".rodata": {"size": 8, "bytes": "3f80000040000000", "relocations": []},
                ".data": {"size": 4, "bytes": "00000000",
                          "relocations": [[0, 1, "canonical_table"]]},
            },
        }

    def verdict(self, candidate):
        result = self.tool.preservation_check(self.baseline, candidate, "chosen")
        self.assertIsInstance(result["passed"], bool)
        self.assertIsInstance(result["violations"], list)
        return result["passed"]

    def test_selected_body_change_can_remain_a_candidate(self):
        candidate = copy.deepcopy(self.baseline)
        candidate["functions"]["chosen"]["bytes"] = "4e800021"
        self.assertTrue(self.verdict(candidate))

    def test_neighbor_body_change_rejected(self):
        candidate = copy.deepcopy(self.baseline)
        candidate["functions"]["neighbor"]["bytes"] = "38600000"
        self.assertFalse(self.verdict(candidate))

    def test_neighbor_reference_identity_change_rejected(self):
        baseline = copy.deepcopy(self.baseline)
        baseline["functions"]["neighbor"]["relocations"] = [[0, 10, "real_callee"]]
        candidate = copy.deepcopy(baseline)
        candidate["functions"]["neighbor"]["relocations"] = [[0, 10, "wrong_callee"]]
        result = self.tool.preservation_check(baseline, candidate, "chosen")
        self.assertFalse(result["passed"])

    def test_new_allocated_data_and_bss_extent_change_rejected(self):
        candidate = copy.deepcopy(self.baseline)
        candidate["data"][".bss"] = {"size": 16, "bytes": "", "relocations": []}
        self.assertFalse(self.verdict(candidate))
        baseline = copy.deepcopy(candidate)
        candidate["data"][".bss"]["size"] = 20
        self.assertFalse(self.tool.preservation_check(baseline, candidate, "chosen")["passed"])

    def test_complete_data_tail_and_extent_are_guarded(self):
        for field, value in [("bytes", "3f80000040400000"), ("size", 12)]:
            candidate = copy.deepcopy(self.baseline)
            candidate["data"][".rodata"][field] = value
            with self.subTest(field=field):
                self.assertFalse(self.verdict(candidate))

    def test_data_reference_identity_change_rejected(self):
        candidate = copy.deepcopy(self.baseline)
        candidate["data"][".data"]["relocations"][0][2] = "wrong_table"
        self.assertFalse(self.verdict(candidate))

    def test_new_helper_or_removed_body_rejected(self):
        candidate = copy.deepcopy(self.baseline)
        candidate["functions"]["forcing_helper"] = candidate["functions"]["chosen"].copy()
        self.assertFalse(self.verdict(candidate))
        candidate = copy.deepcopy(self.baseline)
        del candidate["functions"]["neighbor"]
        self.assertFalse(self.verdict(candidate))

    def test_missing_selected_symbol_is_not_success(self):
        candidate = copy.deepcopy(self.baseline)
        del candidate["functions"]["chosen"]
        self.assertFalse(self.verdict(candidate))


class RunnerIsolation(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tool = load_tool()

    def test_compiler_source_output_and_include_are_redirected(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            root, scratch = p / "checkout", p / "scratch"
            root.mkdir()
            scratch.mkdir()
            output = scratch / "objects"
            output.mkdir()
            source = output / "Unit.cpp"
            source.write_text("void run() {}\n")
            command = ("build/tools/wibo build/tools/sjiswrap.exe "
                       "build/compilers/Wii/1.0/mwcceppc.exe -proc gekko -fp hardware "
                       "-i include -MMD -c src/Game/MapObj/Unit.cpp "
                       "-o build/SB4E01/src/Game/MapObj")
            argv = self.tool.redirect_command(command, "Game/MapObj/Unit", source, output, root)
            self.assertEqual(Path(argv[argv.index("-c") + 1]), source)
            self.assertEqual(Path(argv[argv.index("-o") + 1]), output)
            include = Path(argv[argv.index("-i") + 1])
            self.assertEqual(include if include.is_absolute() else root / include, root / "include")
            self.assertFalse(any(x in argv for x in ["&&", ";", "ninja"]))

    def test_root_output_and_symlink_escape_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            root, scratch = p / "checkout", p / "scratch"
            root.mkdir()
            scratch.mkdir()
            source = scratch / "Unit.cpp"
            source.write_text("void run() {}\n")
            command = ("build/tools/wibo build/tools/sjiswrap.exe "
                       "build/compilers/Wii/1.0/mwcceppc.exe "
                       "-c src/Game/MapObj/Unit.cpp -o build/SB4E01/src/Game/MapObj")
            link = scratch / "linked-checkout"
            link.symlink_to(root, target_is_directory=True)
            for output in [root / "build/output", link / "output"]:
                with self.subTest(output=output):
                    with self.assertRaises((ValueError, RuntimeError)):
                        self.tool.redirect_command(command, "Game/MapObj/Unit", source, output, root)

    def test_arbitrary_shell_suffix_and_missing_source_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            root, scratch = p / "checkout", p / "scratch"
            root.mkdir()
            scratch.mkdir()
            source = scratch / "Unit.cpp"
            source.write_text("void run() {}\n")
            base = ("build/tools/wibo build/tools/sjiswrap.exe "
                    "build/compilers/Wii/1.0/mwcceppc.exe "
                    "-c src/Game/MapObj/Unit.cpp -o build/SB4E01/src/Game/MapObj")
            for command in [base + " && touch checkout/bad", base + "; touch checkout/bad",
                            base.replace("-c src/Game/MapObj/Unit.cpp", "")]:
                with self.subTest(command=command):
                    with self.assertRaises((ValueError, RuntimeError)):
                        self.tool.redirect_command(command, "Game/MapObj/Unit", source,
                                                   scratch / "objects", root)


class ObjectSnapshotSafety(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tool = load_tool()

    def compare_fixture(self, **changes):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            baseline, candidate = p / "baseline.o", p / "candidate.o"
            elf_fixture(baseline)
            elf_fixture(candidate, **changes)
            a = self.tool.object_snapshot(baseline)
            b = self.tool.object_snapshot(candidate)
            return self.tool.preservation_check(a, b, "chosen")

    def test_identical_real_elf_snapshot_passes(self):
        self.assertTrue(self.compare_fixture()["passed"])

    def test_duplicate_named_data_sections_do_not_hide_first_tail(self):
        self.assertFalse(self.compare_fixture(
            first_data=bytes.fromhex("3f80000040400000"))["passed"])

    def test_unowned_executable_tail_is_not_ignored(self):
        self.assertFalse(self.compare_fixture(
            executable_tail=bytes.fromhex("48000000"))["passed"])

    def test_nobits_extent_is_part_of_the_guard(self):
        self.assertFalse(self.compare_fixture(bss_size=16)["passed"])

    def test_distinct_mutable_local_objects_do_not_collapse(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            left, right = p / "left.o", p / "right.o"
            elf_fixture(left, local_ref="left")
            elf_fixture(right, local_ref="right")
            a = self.tool.object_snapshot(left)
            b = self.tool.object_snapshot(right)
            self.assertFalse(self.tool.preservation_check(a, b, "chosen")["passed"])


class ExtractionSafety(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tool = load_tool()

    def test_selected_body_globals_helpers_and_line_context_are_retained(self):
        source = '''#include "Actor.hpp"
static const int values[] = {1, 2, 3};
static int helper(int i) { return values[i]; }
int Actor::neighbor() { return helper(0); }
int Actor::run() { return helper(1); }
int Actor::after() { return helper(2); }
'''
        span = self.tool.find_function(source, "Actor::run")
        extracted = self.tool.extract_function(source, span)
        self.assertEqual(len(source), len(extracted))
        self.assertEqual(source.count("\n"), extracted.count("\n"))
        self.assertEqual(source[span.start:span.end], extracted[span.start:span.end])
        self.assertIn('#include "Actor.hpp"', extracted)
        self.assertIn("static const int values[] = {1, 2, 3};", extracted)
        self.assertIn("static int helper(int i) { return values[i]; }", extracted)
        self.assertNotIn("Actor::neighbor", extracted)
        self.assertNotIn("Actor::after", extracted)

    def test_cpp_unity_import_is_rejected(self):
        source = '#include "Other.cpp"\nvoid Actor::run() {}\nvoid Actor::after() {}\n'
        span = self.tool.find_function(source, "Actor::run")
        with self.assertRaises((ValueError, RuntimeError)):
            self.tool.extract_function(source, span)

    def test_preprocessor_inside_removed_member_is_rejected(self):
        source = '''void Actor::run() { use(); }
void Actor::after() {
#pragma inline_depth(0)
    other();
}
'''
        span = self.tool.find_function(source, "Actor::run")
        with self.assertRaises((ValueError, RuntimeError)):
            self.tool.extract_function(source, span)

    def test_unsupported_function_try_block_is_not_a_selected_span(self):
        source = 'void Actor::run() try { use(); } catch (...) { recover(); }\n'
        with self.assertRaises((ValueError, RuntimeError)):
            self.tool.find_function(source, "Actor::run")


class ExtractedBaselineSafety(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tool = load_tool()

    def snapshots(self, local):
        original = {"functions": {
            "chosen": {"size": 4, "bytes": "48000001", "local": False,
                       "relocations": [[0, 10, ["symbol", "helper", 0]]]},
            "helper": {"size": 4, "bytes": "4e800020", "local": local,
                       "relocations": []}}, "data": {}}
        return original, copy.deepcopy(original)

    def test_identical_selected_and_local_provider_pass(self):
        a, b = self.snapshots(True)
        result = self.tool.extraction_baseline_check(a, b, "chosen")
        self.assertTrue(result["passed"])
        self.assertIn("helper", result["checked_functions"])

    def test_changed_selected_or_local_provider_is_rejected(self):
        for selected in ["chosen", "helper"]:
            a, b = self.snapshots(True)
            b["functions"][selected]["bytes"] = "60000000"
            with self.subTest(selected=selected):
                self.assertFalse(self.tool.extraction_baseline_check(a, b, "chosen")["passed"])

    def test_changed_emitted_nonlocal_provider_is_not_only_a_name(self):
        a, b = self.snapshots(False)
        b["functions"]["helper"]["bytes"] = "60000000"
        self.assertFalse(self.tool.extraction_baseline_check(a, b, "chosen")["passed"])

    def test_missing_local_provider_is_rejected(self):
        a, b = self.snapshots(True)
        del b["functions"]["helper"]
        self.assertFalse(self.tool.extraction_baseline_check(a, b, "chosen")["passed"])

    def test_defined_global_referenced_payload_is_checked(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            original, extracted = p / "original.o", p / "extracted.o"
            elf_fixture(original, local_ref="left", global_ref=True)
            elf_fixture(extracted, local_ref="left", global_ref=True,
                        first_data=bytes.fromhex("4000000040000000"))
            a = self.tool.object_snapshot(original)
            b = self.tool.object_snapshot(extracted)
            self.assertFalse(self.tool.extraction_baseline_check(a, b, "chosen")["passed"])


class ProcessIsolation(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tool = load_tool()

    def test_timeout_starts_private_group_and_cleans_surviving_children(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            process = mock.Mock(pid=424242)
            process.wait.side_effect = [self.tool.subprocess.TimeoutExpired(["fake"], 1), 0]
            # Parent exit after TERM does not establish that every child exited.
            with mock.patch.object(self.tool.subprocess, "Popen", return_value=process) as popen, \
                    mock.patch.object(self.tool.os, "killpg") as killpg:
                result = self.tool.run_process(["fake", "argument"], p, 1, p / "process.log")
            self.assertTrue(result["timed_out"])
            self.assertTrue(popen.call_args.kwargs["start_new_session"])
            self.assertFalse(popen.call_args.kwargs.get("shell", False))
            killpg.assert_any_call(424242, self.tool.signal.SIGTERM)
            killpg.assert_any_call(424242, self.tool.signal.SIGKILL)

    def test_symlinked_log_cannot_truncate_an_input_file(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            victim = p / "input.cpp"
            victim.write_text("protected original source\n")
            log = p / "compiler.log"
            log.symlink_to(victim)
            process = mock.Mock(pid=424242)
            process.wait.return_value = 0
            with mock.patch.object(self.tool.subprocess, "Popen", return_value=process):
                try:
                    self.tool.run_process(["fake"], p, 1, log)
                except (ValueError, OSError):
                    pass
            self.assertEqual(victim.read_text(), "protected original source\n")


class DependencySafety(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tool = load_tool()

    def test_dependency_hashes_and_tree_boundary_are_enforced(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            root = p / "inputs"
            root.mkdir()
            header = root / "Types.hpp"
            header.write_text("typedef int Scalar;\n")
            source = p / "Candidate.cpp"
            source.write_text("void run() {}\n")
            dep = p / "Candidate.d"
            manifest = {"dependencies": {"Types.hpp": self.tool.sha(header.read_bytes())}}
            dep.write_text("Candidate.o: " + str(source) + " " + str(header) + "\n")
            self.assertTrue(self.tool.dependency_closure(dep, root, manifest, source)["passed"])
            header.write_text("typedef float Scalar;\n")
            self.assertFalse(self.tool.dependency_closure(dep, root, manifest, source)["passed"])
            dep.write_text("Candidate.o: " + str(source) + " " + str(p / "outside.hpp") + "\n")
            self.assertFalse(self.tool.dependency_closure(dep, root, manifest, source)["passed"])


class MockedRunnerSafety(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tool = load_tool()

    def fixture(self, p):
        root = p / "inputs"
        source = root / "src/Game/MapObj/Unit.cpp"
        source.parent.mkdir(parents=True)
        source.write_text('''void Actor::run() {
    float first = 1.0f;
    float second = 2.0f;
    use(first, second);
}
void Actor::other() { other(); }
''')
        header = root / "Types.hpp"
        header.write_text("typedef int Scalar;\n")
        command = p / "command.txt"
        for relative in ["build/tools/wibo", "build/tools/sjiswrap.exe",
                         "build/compilers/Wii/1.0/mwcceppc.exe"]:
            executable = root / relative
            executable.parent.mkdir(parents=True, exist_ok=True)
            executable.write_bytes(b"frozen compiler fixture, never executable")
        command.write_text("build/tools/wibo build/tools/sjiswrap.exe "
                           "build/compilers/Wii/1.0/mwcceppc.exe -i . "
                           "-c src/Game/MapObj/Unit.cpp -o build/SB4E01/src/Game/MapObj\n")
        (p / "fake-objdiff").write_bytes(b"frozen scorer fixture, never executable")
        original, target = p / "original.o", p / "target.o"
        original.write_bytes(b"original frozen object")
        target.write_bytes(b"target frozen object")
        manifest = p / "inputs.json"
        manifest.write_text(self.tool.json.dumps({
            "schema_version": 1, "source_revision": "fixture",
            "source_sha256": self.tool.sha(source.read_bytes()),
            "command_sha256": self.tool.sha(command.read_bytes()),
            "original_object_sha256": self.tool.sha(original.read_bytes()),
            "target_object_sha256": self.tool.sha(target.read_bytes()),
            "dependencies": {"Types.hpp": self.tool.sha(header.read_bytes())}}))
        entry = {"unit": "Game/MapObj/Unit", "function": "Actor::run", "symbol": "chosen",
                 "input_root": str(root), "command_file": str(command),
                 "original_object": str(original), "target_object": str(target),
                 "input_manifest": str(manifest)}
        settings = {"seed": 7, "budget": 20, "run": True, "resume": False,
                    "timeout": 5, "seconds": 30, "objdiff": str(p / "fake-objdiff"),
                    "exhausted": None}
        selected = {"size": 4, "bytes": "4e800020", "relocations": [], "local": False}
        original_snapshot = {"functions": {"chosen": selected,
                                           "neighbor": copy.deepcopy(selected)}, "data": {}}
        extracted = {"functions": {"chosen": copy.deepcopy(selected)}, "data": {}}
        changed = copy.deepcopy(extracted)
        changed["functions"]["chosen"]["bytes"] = "60000000"

        def compile_fake(text, unit, directory, input_root, command_text, timeout, input_manifest):
            directory.mkdir(parents=True, exist_ok=True)
            obj = directory / "Unit.o"
            obj.write_bytes(str(directory).encode())
            return obj, {"returncode": 0, "timed_out": False,
                         "dependency_closure": {"passed": True}, "object": str(obj)}

        def snapshot_fake(path):
            path = Path(path)
            if path == original or path == target:
                return copy.deepcopy(original_snapshot)
            if path.parent.name == "baseline":
                return copy.deepcopy(extracted)
            if path.parent.name == "survivor-whole-unit":
                whole = copy.deepcopy(original_snapshot)
                whole["functions"]["chosen"] = copy.deepcopy(changed["functions"]["chosen"])
                return whole
            return copy.deepcopy(changed)

        def scores_fake(target_path, object_path, objdiff, directory, timeout):
            return {"chosen": {"size": 4, "match_percent":
                               10.0 if directory.name == "baseline" else 20.0}}

        return entry, settings, compile_fake, snapshot_fake, scores_fake

    def test_bad_extracted_baseline_stops_before_any_variant_scoring(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            entry, settings, compile_fake, snapshot_fake, scores_fake = self.fixture(p)

            def bad_baseline(path):
                result = snapshot_fake(path)
                if Path(path).parent.name == "baseline":
                    result["functions"]["chosen"]["bytes"] = "deadbeef"
                return result

            with mock.patch.object(self.tool, "compile_scratch", side_effect=compile_fake) as compile_call, \
                    mock.patch.object(self.tool, "object_snapshot", side_effect=bad_baseline), \
                    mock.patch.object(self.tool, "diff_scores", side_effect=scores_fake) as score_call:
                result = self.tool.function_run(entry, settings, p / "results")
            self.assertEqual(result["status"], "EXTRACTION_BASELINE_BLOCKED")
            self.assertEqual(compile_call.call_count, 1)
            score_call.assert_not_called()

    def test_improvement_gets_one_whole_unit_check_and_resume_recovers_best(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            entry, settings, compile_fake, snapshot_fake, scores_fake = self.fixture(p)
            real_write = self.tool.atomic_json

            def checkpoint_after_variant(path, data):
                real_write(path, data)
                if (path.name == "manifest.json" and "whole_unit" not in data and
                        any(v.get("status") == "SCORED_CANDIDATE_ONLY"
                            for v in data.get("variants", []))):
                    raise RuntimeError("simulated interruption before whole-unit attempt")

            with mock.patch.object(self.tool, "compile_scratch", side_effect=compile_fake) as compile_call, \
                    mock.patch.object(self.tool, "object_snapshot", side_effect=snapshot_fake), \
                    mock.patch.object(self.tool, "diff_scores", side_effect=scores_fake):
                with mock.patch.object(self.tool, "atomic_json", side_effect=checkpoint_after_variant):
                    with self.assertRaisesRegex(RuntimeError, "simulated interruption"):
                        self.tool.function_run(entry, settings, p / "results")
                self.assertEqual(compile_call.call_count, 2)
                compile_call.reset_mock()
                settings["resume"] = True
                resumed = self.tool.function_run(entry, settings, p / "results")
                self.assertEqual(compile_call.call_count, 1)
                self.assertEqual(resumed["whole_unit"]["status"], "REVIEWABLE_CANDIDATE")
                compile_call.reset_mock()
                self.tool.function_run(entry, settings, p / "results")
                compile_call.assert_not_called()

    def test_interrupted_baseline_attempt_is_not_automatically_repeated(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            entry, settings, compile_fake, snapshot_fake, scores_fake = self.fixture(p)
            with mock.patch.object(self.tool, "compile_scratch", side_effect=RuntimeError("crash")), \
                    mock.patch.object(self.tool, "object_snapshot", side_effect=snapshot_fake), \
                    mock.patch.object(self.tool, "diff_scores", side_effect=scores_fake):
                with self.assertRaisesRegex(RuntimeError, "crash"):
                    self.tool.function_run(entry, settings, p / "results")
            settings["resume"] = True
            with mock.patch.object(self.tool, "compile_scratch", side_effect=compile_fake) as compile_call, \
                    mock.patch.object(self.tool, "object_snapshot", side_effect=snapshot_fake), \
                    mock.patch.object(self.tool, "diff_scores", side_effect=scores_fake):
                try:
                    self.tool.function_run(entry, settings, p / "results")
                except ValueError:
                    pass
                compile_call.assert_not_called()

    def test_interrupted_whole_unit_attempt_is_not_automatically_repeated(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            entry, settings, compile_fake, snapshot_fake, scores_fake = self.fixture(p)

            def crash_whole(*args):
                if args[2].name == "survivor-whole-unit":
                    raise RuntimeError("crash during whole-unit attempt")
                return compile_fake(*args)

            with mock.patch.object(self.tool, "compile_scratch", side_effect=crash_whole), \
                    mock.patch.object(self.tool, "object_snapshot", side_effect=snapshot_fake), \
                    mock.patch.object(self.tool, "diff_scores", side_effect=scores_fake):
                with self.assertRaisesRegex(RuntimeError, "crash during whole-unit"):
                    self.tool.function_run(entry, settings, p / "results")
            settings["resume"] = True
            with mock.patch.object(self.tool, "compile_scratch", side_effect=compile_fake) as compile_call, \
                    mock.patch.object(self.tool, "object_snapshot", side_effect=snapshot_fake), \
                    mock.patch.object(self.tool, "diff_scores", side_effect=scores_fake):
                try:
                    self.tool.function_run(entry, settings, p / "results")
                except ValueError:
                    pass
                compile_call.assert_not_called()

    def test_changed_frozen_dependency_stops_resume_before_compilation(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            entry, settings, compile_fake, snapshot_fake, scores_fake = self.fixture(p)
            with mock.patch.object(self.tool, "compile_scratch", side_effect=compile_fake) as compile_call, \
                    mock.patch.object(self.tool, "object_snapshot", side_effect=snapshot_fake), \
                    mock.patch.object(self.tool, "diff_scores", side_effect=scores_fake):
                self.tool.function_run(entry, settings, p / "results")
                (Path(entry["input_root"]) / "Types.hpp").write_text("changed input\n")
                settings["resume"] = True
                compile_call.reset_mock()
                with self.assertRaises(ValueError):
                    self.tool.function_run(entry, settings, p / "results")
                compile_call.assert_not_called()

    def test_cached_strict_exact_stops_resume_before_pending_variants(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            entry, settings, compile_fake, snapshot_fake, _ = self.fixture(p)
            source = (Path(entry["input_root"]) / "src/Game/MapObj/Unit.cpp").read_text()
            span = self.tool.find_function(source, entry["function"])
            variants = self.tool.generate_variants(source, span, settings["seed"], settings["budget"])
            self.assertEqual(len(variants), 1)
            # A harmless distinct pending form exposes a wasted resumed attempt.
            before = source[span.body_start + 1:span.body_end]
            after = before + " "
            pending = self.tool.Variant(
                source=source[:span.body_start + 1] + after + source[span.body_end:],
                transform="pending-whitespace-fixture", start=span.body_start + 1,
                end=span.body_end, before=before, after=after)
            variants.append(pending)

            def exact_target_snapshot(path):
                result = snapshot_fake(path)
                if Path(path) == Path(entry["target_object"]):
                    result["functions"]["chosen"]["bytes"] = "60000000"
                return result

            def exact_scores(target, obj, objdiff, directory, timeout):
                return {"chosen": {"size": 4, "match_percent":
                                   10.0 if directory.name == "baseline" else 100.0}}

            with mock.patch.object(self.tool, "generate_variants", return_value=variants), \
                    mock.patch.object(self.tool, "compile_scratch", side_effect=compile_fake) as compile_call, \
                    mock.patch.object(self.tool, "object_snapshot", side_effect=exact_target_snapshot), \
                    mock.patch.object(self.tool, "diff_scores", side_effect=exact_scores):
                first = self.tool.function_run(entry, settings, p / "results")
                self.assertEqual(compile_call.call_count, 3)  # baseline, hit, whole
                self.assertTrue(first["variants"][0]["strict_object_exact"])
                self.assertNotIn("status", first["variants"][1])
                self.assertEqual(first["whole_unit"]["status"], "REVIEWABLE_CANDIDATE")
                settings["resume"] = True
                compile_call.reset_mock()
                resumed = self.tool.function_run(entry, settings, p / "results")
                compile_call.assert_not_called()
                self.assertEqual(resumed["whole_unit"]["status"], "REVIEWABLE_CANDIDATE")

    def test_resume_diff_symlink_does_not_overwrite_the_captured_source(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            entry, settings, _, _, _ = self.fixture(p)
            settings["run"] = False
            self.tool.function_run(entry, settings, p / "results")
            victim = Path(entry["input_root"]) / "src/Game/MapObj/Unit.cpp"
            original = victim.read_bytes()
            diff = p / "results/variant-001/change.diff"
            diff.unlink()
            diff.symlink_to(victim)
            settings["resume"] = True
            try:
                self.tool.function_run(entry, settings, p / "results")
            except (ValueError, OSError):
                pass
            self.assertEqual(victim.read_bytes(), original)

    def test_compiler_output_symlink_is_rejected_before_child_process(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            entry, _, _, _, _ = self.fixture(p)
            root = Path(entry["input_root"])
            manifest = self.tool.json.loads(Path(entry["input_manifest"]).read_text())
            for relative in ["build/tools/wibo", "build/tools/sjiswrap.exe",
                             "build/compilers/Wii/1.0/mwcceppc.exe"]:
                manifest["dependencies"][relative] = self.tool.sha((root / relative).read_bytes())
            output = p / "compile-output"
            output.mkdir()
            victim = root / "protected.o"
            victim.write_bytes(b"protected object")
            (output / "Unit.o").symlink_to(victim)
            with mock.patch.object(self.tool, "run_process", return_value={
                    "returncode": 1, "timed_out": False}) as child:
                try:
                    self.tool.compile_scratch("void run() {}", entry["unit"], output, root,
                                              Path(entry["command_file"]).read_text(), 1, manifest)
                except (ValueError, OSError):
                    pass
                child.assert_not_called()
            self.assertEqual(victim.read_bytes(), b"protected object")

    def test_resume_cannot_reset_an_exhausted_function_time_budget(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp).resolve()
            entry, settings, compile_fake, snapshot_fake, scores_fake = self.fixture(p)
            with mock.patch.object(self.tool, "compile_scratch", side_effect=compile_fake) as compile_call, \
                    mock.patch.object(self.tool, "object_snapshot", side_effect=snapshot_fake), \
                    mock.patch.object(self.tool, "diff_scores", side_effect=scores_fake):
                result = self.tool.function_run(entry, settings, p / "results")
                result.pop("whole_unit")
                result["elapsed_seconds"] = settings["seconds"] + 1
                (p / "results/manifest.json").write_text(self.tool.json.dumps(result))
                settings["resume"] = True
                compile_call.reset_mock()
                try:
                    self.tool.function_run(entry, settings, p / "results")
                except ValueError:
                    pass
                compile_call.assert_not_called()


if __name__ == "__main__":
    unittest.main()
