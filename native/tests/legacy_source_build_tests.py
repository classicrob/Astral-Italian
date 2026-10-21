"""Compile an asset-free fixture through the production legacy-source build rules."""
import contextlib
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time
import unittest

NATIVE = Path(__file__).resolve().parents[1]
MODULE = NATIVE / 'cmake/LegacySources.cmake'
ENCODER = NATIVE / 'tools/encode_legacy_sources.py'


class LegacyBuildTests(unittest.TestCase):
    def exercise(self, generator, standalone=False, module=MODULE, negative=False):
        label = generator.replace(' ', '-') + ('-standalone' if standalone else '-root')
        if negative:
            label += '-negative'
        evidence = os.environ.get('PETARI_LEGACY_BUILD_TEST_ROOT')
        if evidence:
            root = Path(evidence) / label
            root.mkdir(parents=True)
            scope = contextlib.nullcontext(str(root))
        else:
            scope = tempfile.TemporaryDirectory(prefix='petari-legacy-build-')
        with scope as directory:
            root = Path(directory)
            for name in ('src', 'include', 'native/tools', 'native/cmake', 'native/include/petari'):
                (root / name).mkdir(parents=True, exist_ok=True)
            shutil.copyfile(module, root / 'native/cmake/LegacySources.cmake')
            shutil.copyfile(ENCODER, root / 'native/tools/encode_legacy_sources.py')
            for name in ('utf16.h', 'msl_stdio.h'):
                (root / 'native/include/petari' / name).write_text('')
            original = root / 'src/probe.cpp'
            original.write_text('#include "value.hpp"\nint value() { return VALUE + 1; }\n'
                                'const char* label() { return "星"; }\n')
            (root / 'src/untouched.cpp').write_text('int untouched() { return 100; }\n')
            header = root / 'include/value.hpp'
            header.write_text('#define VALUE 10\n')
            (root / 'native/main.cpp').write_text('#include <cstdio>\nint value();\n'
                                                  'int main() { std::printf("%d\\n", value()); }\n')
            project = root / 'standalone' if standalone else root
            project.mkdir(exist_ok=True)
            commands = (
                f'add_executable(probe "{original}" "{root}/src/untouched.cpp" "{root}/native/main.cpp")\n'
                'petari_encode_target_sources(probe)\n'
                f'target_include_directories(probe PRIVATE "${{PETARI_LEGACY_ROOT}}/include" "{root}/native/include")\n')
            setup = ('cmake_minimum_required(VERSION 3.25)\nproject(LegacyDependencyProbe LANGUAGES CXX)\n'
                     f'include("{root}/native/cmake/LegacySources.cmake")\n')
            if standalone:
                consumer = root / 'consumer'
                consumer.mkdir()
                (consumer / 'CMakeLists.txt').write_text(commands)
                setup += f'add_subdirectory("{consumer}" consumer)\n'
            else:
                setup += commands
            (project / 'CMakeLists.txt').write_text(setup)
            build = root / 'build'
            executable = build / 'consumer/probe' if standalone else build / 'probe'
            logs = []

            def run(command):
                result = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                        timeout=20)
                logs.append({'command': command, 'exit': result.returncode, 'output': result.stdout})
                (root / 'commands.json').write_text(json.dumps(logs, indent=2) + '\n')
                self.assertEqual(result.returncode, 0, result.stdout)
                return result.stdout

            run(['cmake', '-S', str(project), '-B', str(build), '-G', generator])
            run(['cmake', '--build', str(build), '--parallel', '1'])
            generated = build / 'legacy-sjis/src/probe.cpp'
            self.assertNotIn('星', generated.read_text())
            obj = next(build.rglob('probe.cpp.o'))
            unrelated = next(build.rglob('untouched.cpp.o'))
            self.assertEqual(run([str(executable)]).strip(), '11')

            def rebuild(expected, change, unrelated_changes=False):
                before = obj.stat().st_mtime_ns
                other = unrelated.stat().st_mtime_ns
                time.sleep(1.05 if generator == 'Unix Makefiles' else 0.03)
                change()
                output = run(['cmake', '--build', str(build), '--parallel', '1'])
                self.assertNotIn('Configuring done', output)
                self.assertEqual(run([str(executable)]).strip(), str(expected), output)
                self.assertGreater(obj.stat().st_mtime_ns, before, output)
                if not unrelated_changes:
                    self.assertEqual(unrelated.stat().st_mtime_ns, other, output)

            if negative:
                before = obj.stat().st_mtime_ns
                time.sleep(1.05 if generator == 'Unix Makefiles' else 0.03)
                original.write_text(original.read_text().replace('VALUE + 1', 'VALUE + 2'))
                run(['cmake', '--build', str(build), '--parallel', '1'])
                self.assertIn('VALUE + 2', generated.read_text())
                self.assertGreater(generated.stat().st_mtime_ns, before)
                self.assertEqual(obj.stat().st_mtime_ns, before)
                self.assertEqual(run([str(executable)]).strip(), '11')
                print('NEGATIVE CONTROL: old rule regenerated source but kept stale object and value11', flush=True)
                return

            rebuild(11, lambda: original.touch())
            rebuild(12, lambda: original.write_text(original.read_text().replace('VALUE + 1', 'VALUE + 2')))
            rebuild(22, lambda: header.write_text('#define VALUE 20\n'))
            rebuild(22, generated.unlink)
            rebuild(22, (build / 'legacy-sjis/include/value.hpp').unlink)
            encoder = root / 'native/tools/encode_legacy_sources.py'
            rebuild(22, encoder.touch, unrelated_changes=True)
            before = (obj.stat().st_mtime_ns, unrelated.stat().st_mtime_ns)
            run(['cmake', '--build', str(build), '--parallel', '1'])
            self.assertEqual((obj.stat().st_mtime_ns, unrelated.stat().st_mtime_ns), before)
            print(json.dumps({'generator': generator, 'standalone': standalone,
                              'touch_edit_header_delete_encoder_and_noop': 'PASS',
                              'object_sha256': hashlib.sha256(obj.read_bytes()).hexdigest()}), flush=True)

    @unittest.skipUnless(shutil.which('ninja'), 'Ninja is not installed')
    def test_ninja(self):
        self.exercise('Ninja')

    def test_makefiles(self):
        self.exercise('Unix Makefiles')

    @unittest.skipUnless(shutil.which('ninja'), 'Ninja is not installed')
    def test_standalone_ninja(self):
        self.exercise('Ninja', standalone=True)

    def test_standalone_makefiles(self):
        self.exercise('Unix Makefiles', standalone=True)


if __name__ == '__main__':
    if len(sys.argv) == 3 and sys.argv[1] == '--negative-control':
        LegacyBuildTests().exercise('Ninja', module=Path(sys.argv[2]), negative=True)
    else:
        unittest.main()
