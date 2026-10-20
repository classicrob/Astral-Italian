#!/usr/bin/env python3
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("trap_undefined", ROOT / "native/tools/trap_undefined.py")
trap = importlib.util.module_from_spec(spec)
spec.loader.exec_module(trap)


class TrapTests(unittest.TestCase):
    def test_only_positive_function_evidence_is_trapped(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            symbols = root / "symbols.txt"
            symbols.write_text("CFunction = .text:0x80000000; // type:function\nCData = .data:0x80000004; // type:object\n")
            cases = [("Thing::update(int) const", "__ZNK5Thing6updateEi"),
                     ("Thing::sValue", "__ZN5Thing6sValueE"), ("vtable for Thing", "__ZTV5Thing"),
                     ("typeinfo for Thing", "__ZTI5Thing"), ("_CFunction", "_CFunction"),
                     ("_CData", "_CData"), ("_unclassified", "_unclassified")]
            inventory = root / "inventory.json"
            inventory.write_text(json.dumps(dict(scope="fixture", entries=[dict(symbol=name, raw_symbols=[raw], owner="core") for name, raw in cases])))
            source, report = root / "traps.cpp", root / "report.json"
            result = trap.generate(inventory, symbols, source, report)
            self.assertEqual(len(result["functions"]), 2)
            self.assertEqual(len(result["data"]), 4)
            self.assertEqual(len(result["unknown"]), 1)
            generated = source.read_text()
            self.assertTrue(generated.startswith("// PLACEHOLDER(port): diagnostic link; resolve when the undefined list is empty."))
            self.assertIn('__asm__("__ZNK5Thing6updateEi")', generated)
            self.assertIn("__builtin_return_address(0)", generated)
            self.assertIn("std::abort()", generated)
            for raw in ("__ZTV5Thing", "__ZTI5Thing", "__ZN5Thing6sValueE", "_CData", "_unclassified"):
                self.assertNotIn('__asm__("' + raw + '")', generated)

    def test_existing_definition_in_link_input_does_not_get_duplicate_trap(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            inventory, symbols, inputs = (root / name for name in ("inventory.json", "symbols.txt", "inputs.json"))
            symbols.write_text("")
            inventory.write_text(json.dumps(dict(entries=[dict(symbol="Thing::go()", raw_symbols=["__ZN5Thing2goEv"],
                compiled_definitions={"__ZN5Thing2goEv": ["/actual/lib.a[go.o]"]})])))
            inputs.write_text(json.dumps(dict(args=["c++", "/actual/lib.a"])))
            result = trap.generate(inventory, symbols, root / "traps.cpp", root / "report.json", inputs)
            self.assertEqual(len(result["functions"]), 0)
            self.assertEqual(len(result["already_provided"]), 1)

    def test_data_prefix_wins_over_function_type_in_demangled_template(self):
        for display, raw in (("vtable for A<void ()>", "__ZTV1A"), ("guard variable for A::x()::a", "__ZGVthing")):
            self.assertEqual(trap.classify(display, raw, {})[0], "data")

    def test_unsafe_assembler_label_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            inventory, symbols = root / "inventory.json", root / "symbols.txt"
            inventory.write_text(json.dumps(dict(entries=[dict(symbol="Thing::go()", raw_symbols=['bad";'])])))
            symbols.write_text("")
            with self.assertRaises(ValueError):
                trap.generate(inventory, symbols, root / "traps.cpp", root / "report.json")

    def test_explicit_guard_mode_emits_unreadable_storage_and_fault_reporter(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            inventory, symbols = root / "inventory.json", root / "symbols.txt"
            inventory.write_text(json.dumps(dict(entries=[dict(symbol="vtable for Thing", raw_symbols=["__ZTV5Thing"])])))
            symbols.write_text("")
            result = trap.generate(inventory, symbols, root / "traps.cpp", root / "report.json", guard_data=True)
            self.assertEqual(result["data_policy"], "protected-unreadable-storage")
            self.assertEqual(len(result["guarded_data"]), 1)
            code = (root / "traps.cpp").read_text()
            for fragment in ('__asm__("__ZTV5Thing")', 'section("__DATA,__ptguard")', 'PROT_NONE', 'SA_SIGINFO', 'info->si_addr', 'fault PC:', 'backtrace_symbols_fd'):
                self.assertIn(fragment, code)


if __name__ == "__main__":
    unittest.main()
