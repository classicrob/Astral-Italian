import importlib.util
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest


MODULE = Path(__file__).with_name("argorder_sweep.py")
SPEC = importlib.util.spec_from_file_location("argorder_sweep", MODULE)
sweep = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = sweep
SPEC.loader.exec_module(sweep)


class TokenizerTests(unittest.TestCase):
    def test_comments_literals_and_lines(self):
        text = '/* f(a(),b()) */\nfoo("a,b(\\\"", \'x\', R"tag(a(),b())tag", L"wide"); // ++i\n'
        tokens, _ = sweep.tokenize(text)
        self.assertEqual(sum(t.kind == "literal" for t in tokens), 4)
        self.assertEqual(tokens[0].line, 2)
        self.assertEqual(sweep.Scanner(text).scan(), [])

    def test_directive_continuations_and_invocation(self):
        source = '#define DRAW(a,b) f(a,b) \\\n+  + g(a,b)\nDRAW(MR::getRandom(), MR::getRandom());'
        scanner = sweep.Scanner(source)
        sites = scanner.scan()
        self.assertEqual(scanner.directive_count, 1)
        self.assertEqual(len(sites), 1)
        self.assertEqual(sites[0]["line"], 3)

    def test_numbers_do_not_swallow_operators(self):
        tokens, _ = sweep.tokenize('1e-3 + foo() - 0x1.fp+2 + bar()')
        self.assertEqual([t.text for t in tokens if t.kind == "number"], ['1e-3', '0x1.fp+2'])
        self.assertEqual(sum(t.text == '+' for t in tokens), 2)

    def test_delimiter_warning(self):
        scanner = sweep.Scanner('f(a(), b();')
        self.assertTrue(scanner.errors)
        scanner.scan()

    def test_native_alternative_open_braces_preserve_lines(self):
        source = '#ifdef GALAXY_NATIVE\nvoid native() {\n#else\nvoid wii() {\n#endif\nf(rng.rand(), rng.rand());\n}\n'
        sites, metadata = sweep.scan_source(source, 'a.cpp')
        self.assertTrue(metadata['raw_delimiter_warnings'])
        self.assertEqual(metadata['delimiter_warnings'], [])
        self.assertEqual(len(sites), 1)
        self.assertEqual(sites[0]['line'], 6)
        self.assertEqual(len(sites[0]['views']), 2)

    def test_native_view_nested_unknown_guard_and_raw_string(self):
        source = 'auto s = R"(\n#if GALAXY_NATIVE\n)";\n#ifdef OTHER\n#ifndef GALAXY_NATIVE\nwii();\n#else\nnative();\n#endif\n#endif\n'
        native = sweep.native_view(source, True)
        self.assertIn('native();', native)
        self.assertNotIn('wii();', native)
        self.assertIn('#if GALAXY_NATIVE', native)
        self.assertEqual(source.count('\n'), native.count('\n'))

    def test_native_view_elif(self):
        source = '#if !defined(GALAXY_NATIVE)\nwii();\n#elif defined(GALAXY_NATIVE)\nnative();\n#else\nother();\n#endif\n'
        self.assertIn('native();', sweep.native_view(source, True))
        self.assertNotIn('other();', sweep.native_view(source, True))
        self.assertIn('wii();', sweep.native_view(source, False))
        self.assertNotIn('native();', sweep.native_view(source, False))


class ExpressionTests(unittest.TestCase):
    def sites(self, source, cls=None):
        findings = sweep.Scanner(source).scan()
        return findings if cls is None else [f for f in findings if f['class'] == cls]

    def test_rng_nested_parentheses(self):
        sites = self.sites('f((MR::getRandom(-1.f, 1.f)), 2 * MR::getRandom());', 'A')
        self.assertEqual(len(sites), 1)
        self.assertEqual(sites[0]['severity'], 'HIGH')
        self.assertEqual(sites[0]['effectful_operands'], 2)

    def test_two_calls_in_one_argument_are_not_two_arguments(self):
        self.assertFalse(self.sites('f(a() + b(), 1);', 'A'))
        self.assertEqual(len(self.sites('f(a() + b(), 1);', 'B')), 1)

    def test_constructor(self):
        sites = self.sites('TVec3f v(rng.get_r_f(), rng.get_r_f(), rng.get_r_f());', 'A')
        self.assertEqual(len(sites), 1)
        self.assertEqual(sites[0]['severity'], 'HIGH')
        self.assertEqual(len(self.sites('TVec3f v(MR::getRandom(), MR::getRandom());', 'A')), 1)

    def test_nested_templates_and_relational(self):
        source = 'f<std::pair<int, std::vector<float>>>(g<int>(rng.rand()), rng.rand());'
        sites = self.sites(source, 'A')
        self.assertEqual(len(sites), 1)
        self.assertEqual(sites[0]['callee_or_operator'], 'f')
        sites = self.sites('f(a() < b(), c() > d());')
        self.assertEqual(sum(f['class'] == 'B' for f in sites), 2)

    def test_template_constructor(self):
        source = 'JGeometry::TVec3<f32> rnd(e->get_r_zp(), e->get_r_zp(), e->get_r_zp());'
        self.assertEqual(len(self.sites(source, 'A')), 1)

    def test_direct_operator_call(self):
        sites = self.sites('operator+(rng.rand(), rng.rand());', 'A')
        self.assertEqual(len(sites), 1)
        self.assertEqual(sites[0]['callee_or_operator'], 'operator+')
        for op in ('<<', '>>', '()', '[]', '++', '+='):
            with self.subTest(op=op):
                sites = self.sites(f'obj.operator{op}(rng.rand(), rng.rand());', 'A')
                self.assertEqual(len(sites), 1)
                self.assertEqual(sites[0]['callee_or_operator'], 'operator' + op)

    def test_indirect_call(self):
        self.assertEqual(len(self.sites('(*fp)(s.read(), s.read());', 'A')), 1)

    def test_binary_precedence(self):
        sites = self.sites('return a() + b() * c();', 'B')
        self.assertEqual({f['callee_or_operator'] for f in sites}, {'+', '*'})
        multiply = next(f for f in sites if f['callee_or_operator'] == '*')
        self.assertEqual(multiply['expression'], 'b() * c()')

    def test_binary_class_separate_from_arguments(self):
        sites = self.sites('f(rng.rand(), rng.rand()) + g(rng.rand(), rng.rand());')
        self.assertEqual(sum(f['class'] == 'A' for f in sites), 2)
        self.assertEqual(sum(f['class'] == 'B' for f in sites), 1)

    def test_braces_are_ordered_but_nested_calls_are_not(self):
        self.assertFalse(self.sites('TVec3f v{rng.rand(), rng.rand(), rng.rand()};'))
        sites = self.sites('T v{f(rng.rand(), rng.rand()), 2};', 'A')
        self.assertEqual(len(sites), 1)
        self.assertEqual(sites[0]['callee_or_operator'], 'f')

    def test_braced_argument_against_other_argument(self):
        self.assertEqual(len(self.sites('f(T{rng.rand(), rng.rand()}, rng.rand());', 'A')), 1)

    def test_sequenced_operators(self):
        for op in ('&&', '||', ',', '=', '+=', '<<', '>>'):
            with self.subTest(op=op):
                self.assertFalse(self.sites(f'a() {op} b();', 'B'))
        self.assertFalse(self.sites('q() ? a() : b();', 'B'))

    def test_comma_inside_one_argument(self):
        self.assertFalse(self.sites('f((rng.rand(), rng.rand()), 0);'))

    def test_unary_and_postfix_mutations(self):
        self.assertFalse(self.sites('f(*ptr, -value, &x);'))
        sites = self.sites('f(++i, j--);', 'A')
        self.assertEqual(len(sites), 1)
        self.assertEqual(sites[0]['severity'], 'HIGH')
        self.assertEqual(len(self.sites('i++ + j++;', 'B')), 1)

    def test_assignment_as_argument_effect(self):
        sites = self.sites('f((x = 1), (y += 2));', 'A')
        self.assertEqual(len(sites), 1)
        self.assertEqual(sites[0]['severity'], 'HIGH')

    def test_deferred_lambda_body(self):
        self.assertFalse(self.sites('f([] { return rng.rand(); }, rng.rand());', 'A'))
        sites = self.sites('f([] { g(rng.rand(), rng.rand()); }, 0);', 'A')
        self.assertEqual(len(sites), 1)
        self.assertEqual(sites[0]['callee_or_operator'], 'g')

    def test_lambda_capture_runs(self):
        sites = self.sites('f([x = rng.rand()] { return x; }, rng.rand());', 'A')
        self.assertEqual(len(sites), 1)

    def test_immediately_invoked_lambda(self):
        sites = self.sites('f([] { return rng.rand(); }(), rng.rand());', 'A')
        self.assertTrue(any(f['callee_or_operator'] == 'f' for f in sites))

    def test_unevaluated_operands(self):
        for name in ('sizeof', 'decltype', 'noexcept', 'alignof'):
            with self.subTest(name=name):
                self.assertFalse(self.sites(f'{name}(f(rng.rand(), rng.rand()));'))
                self.assertFalse(self.sites(f'f({name}(rng.rand()), rng.rand());', 'A'))

    def test_function_default_arguments_are_declarations(self):
        self.assertFalse(self.sites('void f(int x = rng.rand(), int y = rng.rand());'))
        self.assertFalse(self.sites('void f(int x = rng.rand(), int y = rng.rand()) {}'))
        self.assertFalse(self.sites('Foo::Foo(int x = rng.rand(), int y = rng.rand()) : value(x) {}'))

    def test_function_body_still_scanned(self):
        self.assertEqual(len(self.sites('void f(int x) { g(s.read(), s.read()); }', 'A')), 1)

    def test_purity_is_explicit(self):
        self.assertFalse(self.sites('f(MR::getPlayerPos(), MR::getPlayerGravity());'))
        sites = self.sites('f(getSomething(), getSomething());', 'A')
        self.assertEqual(sites[0]['severity'], 'MEDIUM')
        self.assertEqual(self.sites('f(e->get_r_zp(), e->get_r_zp());', 'A')[0]['severity'], 'HIGH')
        self.assertFalse(self.sites('f(e->getScaleRndm(), e->getLifetimeRndm());'))
        self.assertEqual(sweep.classify_call('other.getMysteryRndm'), 'unknown/stateful')

    def test_empty_definition_and_large_ordered_aggregate(self):
        source = 'void f() { f(a(), b()); } int data[] = {' + ','.join('1' for _ in range(2000)) + '};'
        self.assertEqual(len(self.sites(source, 'A')), 1)

    def test_separate_statements_do_not_mix(self):
        self.assertFalse(self.sites('a(); b(); ++i; --j;'))
        self.assertFalse(self.sites('if (rng.rand()) x = 2 + pure();', 'B'))

    def test_named_temporaries_do_not_reappear(self):
        source = 'float z = rng.rand(); float y = rng.rand(); TVec3f v(rng.rand(), y, z);'
        self.assertFalse(self.sites(source))

    def test_stream_binary(self):
        sites = self.sites('return stream.readU16() + stream.readU16();', 'B')
        self.assertEqual(len(sites), 1)
        self.assertEqual(sites[0]['severity'], 'HIGH')

    def test_c_assignment_and_shift_are_conservative_candidates(self):
        scanner = sweep.Scanner('*dst++ = *src++; a() << b();', 'src/RVLFaceLib/a.c')
        self.assertEqual({f['callee_or_operator'] for f in scanner.scan()}, {'=', '<<'})

    def test_reviewed_reads_and_scoped_exclusion(self):
        self.assertIsNone(sweep.classify_call('PetariNative::readU16BE'))
        self.assertIsNone(sweep.classify_call('readReg', 'src/JSystem/JAudio2/JASSeqParser.cpp'))
        self.assertEqual(sweep.classify_call('readReg', 'src/Game/System/Test.cpp'), 'stream/iterator')

    def test_c_style_casts_are_not_indirect_calls(self):
        for cast in ('s32', 'UINT8 *', 'Foo const &'):
            with self.subTest(cast=cast):
                scanner = sweep.Scanner(f'*p++ = ({cast})(x * y);', 'a.c')
                self.assertFalse(scanner.scan())
        self.assertEqual(len(self.sites('(*fp)(s.read(), s.read());', 'A')), 1)


class CorpusTests(unittest.TestCase):
    root = Path(os.environ.get('ARGORDER_CORPUS_ROOT', MODULE.resolve().parents[2]))

    def test_fixed_game_rng_sites(self):
        paths = ['src/Game/Enemy/KirairaChain.cpp', 'src/Game/Map/Flag.cpp',
                 'src/Game/Map/FishGroup.cpp', 'src/Game/MapObj/StarPiece.cpp',
                 'src/Game/Map/WaterPlant.cpp', 'src/Game/Util/MathUtil.cpp']
        for path in paths:
            with self.subTest(path=path):
                findings = sweep.Scanner((self.root / path).read_text(), path).scan()
                repeated_rng = [f for f in findings if sum(e['kind'] == 'RNG' for e in f['effects']) > 1]
                self.assertEqual(repeated_rng, [])

    def test_jpa_init_c_current_and_model(self):
        relative = 'src/JSystem/JParticle/JPAParticle.cpp'
        for root in (self.root, Path.home() / 'MacioGalaxy'):
            path = root / relative
            if not path.is_file():
                if root == self.root:
                    self.fail(f'missing required corpus file {path}')
                continue
            source = path.read_text()
            start = source.index('void JPABaseParticle::init_c(')
            sites = sweep.Scanner(source[start:], relative).scan()
            constructors = [f for f in sites if f['class'] == 'A' and
                            f['callee_or_operator'] in {'rnd', 'base_vel'}]
            self.assertEqual(len(constructors), 2)
            self.assertTrue(all(f['severity'] == 'HIGH' for f in constructors))

    def test_json_render_and_deduplicated_inputs(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            folder = root / 'src/Game/Util'
            folder.mkdir(parents=True)
            (folder / 'a.cpp').write_text('f(s.read(), s.read());')
            (folder / 'b.inc').write_text('f(++i, --j);')
            report = sweep.sweep(root, ['src/Game', 'src/Game/Util/a.cpp'])
            self.assertEqual(len(report['files']), 2)
            self.assertEqual(len(report['findings']), 2)
            self.assertIn('| src/Game/Util | core | 2 | 2 | 0 | 0 | 0 |', sweep.render(report))
            self.assertEqual(json.loads(json.dumps(report))['schema_version'], 1)


if __name__ == '__main__':
    unittest.main()
