#!/usr/bin/env python3
"""Checks for first-error parsing and native audit aggregation."""

import json
from pathlib import Path
import tempfile
import unittest

from classify_audit import classify, first_error, normalize_message, read_reports, render


class ClassifyAuditTests(unittest.TestCase):
    def test_primary_classes(self):
        cases = {
            "invalid output constraint '=f' in asm": "inline asm / asm function",
            "unknown type name '__asm'": "inline asm / asm function",
            "use of undeclared identifier '__frsqrte'": "Wii intrinsic",
            "cast from pointer to smaller type 'u32' loses information": "pointer to integer (64-bit hazard)",
            "unknown type name 'MissingType'": "unknown type / identifier",
            "use of undeclared identifier 'get'": "unknown type / identifier",
            "no template named 'MissingTemplate'": "unknown type / identifier",
            "'missing/header.h' file not found": "missing header",
            "'f' marked 'override' but does not override any member functions": "override / virtual mismatch",
            "no matching function for call to 'loadToMainRAM'": "ambiguous / overload",
            "templates must have C++ linkage": "template",
            "explicit specialization of 'push_back' after instantiation": "template",
            "__declspec attributes are not enabled": "MWCC-only syntax",
            "non-void function does not return a value": "other",
        }
        for message, expected in cases.items():
            with self.subTest(message=message):
                self.assertEqual(classify(message)[0], expected)
        self.assertEqual(classify("use of undeclared identifier 'get'")[1], "get")

    def test_first_error_ignores_warnings_and_notes(self):
        diagnostics = (
            "In file included from /tree/src/Game/Test.cpp:1:\n"
            "/tree/include/Test.h:4:2: warning: old construct\n"
            "/tree/build/audit-sjis/libs/Test.h:12:8: error: use of undeclared identifier 'get'\n"
            "/tree/include/Test.h:13:1: note: declared here\n"
            "/tree/include/Test.h:15:1: error: another error\n"
        )
        error = first_error(diagnostics, "/tree")
        self.assertEqual(error["location"], "libs/Test.h:12")
        self.assertEqual(error["symbol"], "get")
        self.assertEqual(error["message"], "use of undeclared identifier 'get'")

    def test_fatal_and_driver_diagnostics(self):
        error = first_error("/tree/a.h:3:5: fatal error: 'b.h' file not found\n", "/tree")
        self.assertEqual(error["class"], "missing header")
        self.assertEqual(first_error("clang++: error: invalid argument\n", "")["message"], "invalid argument")
        self.assertEqual(normalize_message("failed at /tmp/header.h:42:3"), "failed at <path>:<line>")

    def test_shared_header_counts_and_empty_groups(self):
        report = {"root": "/tree", "paths": ["src/Game", "src/RVL_SDK"], "results": [
            {"source": "src/Game/Util/A.cpp", "passed": False,
             "diagnostics": "/tree/include/Common.h:5:2: error: unknown type name 'T'\n"},
            {"source": "src/Game/MapObj/B.cpp", "passed": False,
             "diagnostics": "/tree/include/Common.h:5:2: error: unknown type name 'T'\n"},
            {"source": "src/Game/Util/C.cpp", "passed": True, "diagnostics": ""},
        ]}
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "audit.json"
            path.write_text(json.dumps(report))
            reports, results = read_reports([path])
            overlay = {**results, "src/nw4r/lyt/New.cpp": {"source": "src/nw4r/lyt/New.cpp", "passed": True}}
            markdown = render(reports, results, overlay=overlay)
            self.assertIn("| src/Game/Util | core | 2 | 1 | 1 |", markdown)
            self.assertIn("| src/Game/MapObj | objects | 1 | 0 | 1 |", markdown)
            self.assertIn("| src/RVL_SDK | m1 | 0 | 0 | 0 |", markdown)
            self.assertIn("| 1 | 2 | include/Common.h:5 |", markdown)
            self.assertIn("With transferred hooks", markdown)
            self.assertIn("With hooks top 3 first-error classes", markdown)
            self.assertIn("| src/nw4r/lyt | m1 | 0 | 0 | 0 | — | 1/1 |", markdown)
            with self.assertRaisesRegex(ValueError, "duplicate source"):
                read_reports([path, path])


if __name__ == "__main__":
    unittest.main()
