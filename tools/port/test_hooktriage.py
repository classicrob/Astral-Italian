import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parent))
import hooktriage as tri


def group(text, path="src/Game/Util/Sample.cpp", kind="manual", line=1):
    return {"text": text, "path": path, "kind": kind, "line": line, "function": "stale"}


class Fixture(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)

    def write(self, path, text):
        destination = self.root / path
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_text(text)
        return destination

    def overlay(self, source="void Sample::run() { oldCall(); }\n", path="src/Game/Util/Sample.cpp"):
        self.write(path, source)
        return tri.Overlay(self.root)

    def decision(self, text, source="void Sample::run() { oldCall(); }\n", path="src/Game/Util/Sample.cpp"):
        overlay = self.overlay(source, path)
        target = overlay.locate(path, "Sample::run")
        return tri.decide(group(text, path), overlay, target, "Sample::run")


class LocatorTests(Fixture):
    def test_namespace_and_nested_inline_class_definitions(self):
        source = 'namespace MR { void run() {} }\nnamespace J { struct V { float dot(int a) { return a; } }; }'
        self.assertEqual({f["name"] for f in tri.functions(source)}, {"MR::run", "J::V::dot"})

    def test_alternative_signatures_do_not_create_giant_enclosing_functions(self):
        source = '#ifdef FLAG\nvoid C::zero() {\n#else\nvoid C::zero() {\n#endif\n}\nvoid C::next() {}\n'
        spans = tri.functions(source)
        self.assertTrue(any(f["name"] == "C::next" for f in spans))
        self.assertTrue(all(f["end_line"] <= 6 for f in spans if f["name"] == "C::zero"))

    def test_comments_strings_and_calls_are_not_definitions(self):
        source = '// void Bad::fn() {}\nconst char* s="void Bad::fn() {}";\nvoid Good::fn() { other(); if (ok()) {} }'
        self.assertEqual([f["name"] for f in tri.functions(source)], ["Good::fn"])

    def test_constructor_initializer_list(self):
        source = 'C::C() : a(1), b{2} {\n work();\n}\n'
        spans = tri.functions(source)
        self.assertEqual([f['name'] for f in spans], ['C::C'])
        self.assertEqual(spans[0]['end_line'], 3)
        self.assertIn('C::C', {f['name'] for f in tri.functions('class C { public: C() {} };')})

    def test_merged_player_inc(self):
        overlay = self.overlay('void Mario::tryJump() { oldCall(); }\n', 'src/Game/Player/MarioSMG2_jump.inc')
        target = overlay.locate('src/Game/Player/MarioJump.cpp', 'Mario::tryJump')
        self.assertEqual(target["path"], 'src/Game/Player/MarioSMG2_jump.inc')
        self.assertEqual(target["line"], 1)

    def test_signature_selects_correct_overload(self):
        path = "src/Game/Util/Sample.cpp"
        overlay = self.overlay('void Sample::run(float* p) {}\nvoid Sample::run(int* p) {}\n')
        target = overlay.locate(path, "Sample::run", signature="Sample::run(int* p)")
        self.assertEqual(target["line"], 2)

    def test_header_classes_do_not_share_a_bare_method_target(self):
        path = 'include/Game/Util/Sample.hpp'
        overlay = self.overlay('struct A { int dot() { return 1; } };\nstruct B { int dot() { return 2; } };\n', path)
        self.assertEqual(overlay.locate(path, 'B::dot')["line"], 2)
        self.assertIsNone(overlay.locate('src/missing.cpp', 'dot'))

    def test_specialized_template_class_scope(self):
        source = 'namespace J { template<> struct V<float> { int dot() { return 1; } }; }'
        self.assertIn('J::V::dot', {f['name'] for f in tri.functions(source)})

    def test_source_absence_does_not_count_comments_literals_or_includes(self):
        overlay = self.overlay('#include "Absent.hpp"\n// Absent\nconst char* name="Absent";\nvoid Sample::run() {}')
        self.assertFalse(overlay.has_class_definitions('Absent'))

    def test_base_context_overrides_stale_force_match_label(self):
        reference = tri.Reference(self.root, 'base')
        with patch.object(tri.Reference, 'read', return_value='void Old_FORCE_MATCH() {}\nvoid Real::run() {\n oldCall();\n}\n'):
            self.assertEqual(reference.enclosing('a.cpp', 3, 'Old_FORCE_MATCH'), 'Real::run')
            self.assertEqual(reference.enclosing('a.cpp', 6, 'Old_FORCE_MATCH'), '')

    def test_new_observer_method_anchors_existing_class_file(self):
        overlay = self.overlay('void Sample::run() {}\n')
        target = overlay.locate('src/Game/Util/Sample.cpp', 'Sample::missing',
                                new='void Sample::nativeObserve() { publish(); }')
        self.assertEqual(target['precision'], 'insertion')
        self.assertEqual(target['line'], 1)
        target = overlay.locate('src/Game/Util/Sample.cpp', 'Sample::run',
                                new='void Sample::nativeObserve() { publish(); }')
        self.assertEqual(target['precision'], 'insertion')

    def test_branch_view_preserves_offsets_and_skips_wii_branch(self):
        source = '#ifdef __MWERKS__\nasmStuff();\n#else\nscalar();\n#endif\n'
        active = tri.branch_view(source, True, False)
        self.assertEqual(len(source), len(active))
        self.assertEqual(source.index('scalar'), active.index('scalar'))
        self.assertNotIn('asmStuff', active)


class DecisionTests(Fixture):
    def test_missing_filename_alone_is_not_drop_evidence(self):
        overlay = self.overlay()
        result = tri.decide(group('+nativeCall();', 'src/Game/Enemy/Absent.cpp', 'missing'), overlay, None, 'Absent::run')
        self.assertEqual(result['decision'], 'unsure')

    def test_observation_with_cast_and_nullptr_is_native_behavior(self):
        result = self.decision('+#ifdef GALAXY_NATIVE\n+MR::Native::publishActor(static_cast<int>(value), nullptr);\n+#endif')
        self.assertEqual(result['decision'], 'port-native')

    def test_pointer_width_and_dependent_lookup_are_clang(self):
        for text in ('-u32 p;\n+uintptr_t p;', '-set<T>(a);\n+this->template set<T>(a);'):
            with self.subTest(text=text):
                self.assertEqual(self.decision(text)['decision'], 'port-clang')

    def test_feature_comment_does_not_drop_core_diagnostic(self):
        result = self.decision('+#ifdef GALAXY_NATIVE\n+// PhotoMode also uses this diagnostic.\n+MR::Native::publishActor();\n+#endif')
        self.assertEqual(result['decision'], 'port-native')

    def test_deferred_mod_includes_and_qualified_calls(self):
        for text in ('+#include "Game/Player/GenericFigure.hpp"',
                     '+PetariNative::Odyssey::roll();', '+petari_mod_enabled(2);'):
            with self.subTest(text=text):
                self.assertEqual(self.decision(text)['decision'], 'drop-smg1')

    def test_feature_helper_callers_are_deferred_but_mixed_groups_are_not_dropped(self):
        overlay = self.overlay()
        target = overlay.locate('src/Game/Util/Sample.cpp', 'Sample::run')
        for text, expected in [('+if (::isDecide()) {', 'drop-smg1'),
                               ('+if (::isDecide()) { coreCall(); }', 'unsure'),
                               ('+isDecide();\n+counter += 1;', 'unsure')]:
            result = tri.decide(group(text), overlay, target, 'Sample::run', {'isDecide'})
            self.assertEqual(result['decision'], expected)

    def test_substantive_replacement_already_present(self):
        result = self.decision('-oldCall();\n+newCall(1, 2, 3);', 'void Sample::run() { newCall(1, 2, 3); }')
        self.assertEqual(result['decision'], 'drop-fixed')

    def test_brace_alone_cannot_prove_fixed(self):
        self.assertNotEqual(self.decision('+}')['decision'], 'drop-fixed')

    def test_wii_only_replacement_does_not_prove_native_fixed(self):
        source = 'void Sample::run() {\n#ifdef GALAXY_NATIVE\noldCall();\n#else\nnewCall(1, 2, 3);\n#endif\n}'
        self.assertNotEqual(self.decision('-oldCall();\n+newCall(1, 2, 3);', source)['decision'], 'drop-fixed')

    def test_reviewed_hash_fix_requires_current_overlay_evidence(self):
        header = 'include/Game/Util/HashUtil.hpp'
        text = '-    /* 0x08 */ u32* _8;\n+    /* 0x08 */ Value* _8;\n'
        self.write('src/Game/Util/HashUtil.cpp', 'new uintptr_t[cnt]; uintptr_t swapArray[1]; uintptr_t* a2;')
        overlay = self.overlay('uintptr_t* mPayloads; void searchPtr(); void addPtr();', header)
        target = overlay.locate(header, '')
        result = tri.decide(group(text, header), overlay, target, '')
        self.assertEqual(result['decision'], 'drop-fixed')
        self.assertEqual(result['method'], 'reviewed')
        self.assertTrue(result['evidence'])
        self.assertIsNone(tri.reviewed(group(text + '+unrelatedChange();\n', header), overlay, target, ''))
        self.write(header, 'u32* mPayloads;')
        overlay = tri.Overlay(self.root)
        self.assertNotEqual(tri.decide(group(text, header), overlay, overlay.locate(header, ''), '')['decision'], 'drop-fixed')

    def test_feature_absence_rule_rechecks_class_implementation(self):
        overlay = self.overlay()
        g = group('+nativeCall();', 'src/Game/Demo/PrologueDirector.cpp', 'missing')
        self.assertEqual(tri.decide(g, overlay, None, 'PrologueDirector::run')['decision'], 'drop-smg1')
        self.write(g['path'], 'void PrologueDirector::run() {}')
        overlay = tri.Overlay(self.root)
        self.assertNotEqual(tri.decide(g, overlay, None, 'PrologueDirector::run')['decision'], 'drop-smg1')


class ReportTests(Fixture):
    def test_stable_ids_handle_duplicate_text_and_line_shifts(self):
        report = {'files': [{'path': 'x.cpp', 'groups': [group('+}', line=1), group('+}', line=9), group('+ignored', kind='clean')]}]}
        original = list(tri.groups(report))
        for g in report['files'][0]['groups']:
            g['line'] += 30
        shifted = list(tri.groups(report))
        self.assertEqual([g['id'] for g in original], [g['id'] for g in shifted])
        self.assertEqual(len({g['id'] for g in original}), 2)
        report['files'][0]['groups'][0]['text'] = '+changed();'
        self.assertNotEqual(next(tri.groups(report))['id'], original[0]['id'])

    def test_complete_run_lane_map_targets_ledger_and_relocated_functions(self):
        self.write('src/Game/Player/MarioSMG2_test.inc', 'void Mario::run() { oldCall(); }\n')
        lanes = self.write('lanes.py', 'def lane_of(path):\n    return "astra" if "/Player/" in path else "core"\n')
        input_report = {'macio_base': 'base', 'files': [{'path': 'src/Game/Player/MarioJump.cpp', 'groups': [
            group('-oldCall();\n+this->template set<T>(x);', kind='missing'),
            group('+#ifdef GALAXY_NATIVE\n+MR::Native::publishActor();\n+#endif', kind='manual'),
            group('+ignored', kind='clean')]}]}
        report_path = self.write('report.json', json.dumps(input_report))
        with patch.object(tri.Reference, 'read', return_value='void Mario::run() { oldCall(); }\n'):
            result = tri.triage(report_path, self.root, self.root, lanes)
        entries = result['groups']
        self.assertEqual(len(entries), 2)
        self.assertEqual({e['decision'] for e in entries}, {'port-clang', 'port-native'})
        self.assertTrue(all(e['lane'] == 'astra' for e in entries))
        for entry in entries:
            target = entry['target']
            self.assertEqual(target['path'], 'src/Game/Player/MarioSMG2_test.inc')
            self.assertEqual(target['line'], 1)
            self.assertNotIn('\n', entry['reason'])
        output = tri.render(result)
        self.assertIn('Totals (2 groups)', output)
        self.assertTrue(all(e['id'] in output for e in entries))
        self.assertIn('MarioSMG2_test.inc:1', output)
        self.assertIn('input_sha256', result)

    def test_deferred_helper_dependency_is_recovered_from_input_feature_group(self):
        self.write('src/Game/Util/Sample.cpp', 'void Sample::run() { oldCall(); }')
        lanes = self.write('lanes.py', 'def lane_of(path):\n    return "core"\n')
        report = {'macio_base': 'base', 'files': [{'path': 'src/Game/Util/Sample.cpp', 'groups': [
            group('+bool isDecide() { return CutsceneSkip::isActive(); }', kind='feature'),
            group('-oldCall();\n+if (::isDecide()) {', kind='unconditional')]}]}
        report_path = self.write('report.json', json.dumps(report))
        with patch.object(tri.Reference, 'read', return_value='void Sample::run() { oldCall(); }'):
            result = tri.triage(report_path, self.root, self.root, lanes)
        self.assertEqual(result['groups'][0]['decision'], 'drop-smg1')
        self.assertEqual(result['deferred_helpers']['src/Game/Util/Sample.cpp'], ['isDecide'])


if __name__ == '__main__':
    unittest.main()
