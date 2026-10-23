"""Asset-free link and pinned-provider regressions using production CMake rules."""
import contextlib
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

MODULE = Path(__file__).resolve().parents[1] / 'cmake/GraphicsDependencies.cmake'


@unittest.skipUnless(sys.platform == 'darwin' and shutil.which('ninja'), 'macOS and Ninja required')
class GraphicsDependencyBuildTests(unittest.TestCase):
    def exercise(self, label, link=True, wrong=None, pad=False):
        evidence = os.environ.get('PETARI_GRAPHICS_BUILD_TEST_ROOT')
        if evidence:
            root = Path(evidence) / label
            root.mkdir(parents=True)
            scope = contextlib.nullcontext(str(root))
        else:
            scope = tempfile.TemporaryDirectory(prefix='petari-graphics-build-')
        with scope as directory:
            root = Path(directory)
            (root / 'audio.cpp').write_text('#include <CoreAudio/CoreAudio.h>\n'
                'using Query = decltype(&AudioObjectGetPropertyData);\n'
                'Query volatile query = &AudioObjectGetPropertyData;\n'
                'int probe() { return query ? 0 : 1; }\n')
            (root / 'main.cpp').write_text('int probe(); int main() { return probe(); }\n')
            (root / 'sdl.cpp').write_text('int fake_sdl() { return 0; }\n')
            lines = ['cmake_minimum_required(VERSION 3.25)', 'project(AudioDependencyProbe LANGUAGES CXX)',
                     f'include("{MODULE}")', 'set(AURORA_SDL3_PROVIDER system CACHE STRING "" FORCE)',
                     'petari_configure_graphics_dependencies()',
                     'if(NOT AURORA_SDL3_PROVIDER STREQUAL "vendor" OR NOT AURORA_SDL3_LINKAGE STREQUAL "static")',
                     'message(FATAL_ERROR "provider not pinned")', 'endif()']
            for name in ('aurora', 'dawn_prebuilt', 'imgui', 'SDL', 'tracy'):
                source = root / name
                source.mkdir()
                body = ''
                if name == 'SDL':
                    if wrong == 'imported':
                        body = 'add_library(SDL3::SDL3 SHARED IMPORTED GLOBAL)\n'
                    elif wrong == 'target-dir':
                        child = source / 'foreign'; child.mkdir()
                        (child / 'CMakeLists.txt').write_text(f'add_library(SDL3-static STATIC "{root}/sdl.cpp")\nadd_library(SDL3::SDL3 ALIAS SDL3-static)\n')
                        body = 'add_subdirectory(foreign)\n'
                    else:
                        body = f'add_library(SDL3-static STATIC "{root}/sdl.cpp")\nadd_library(SDL3::SDL3 ALIAS SDL3-static)\n'
                (source / 'CMakeLists.txt').write_text(body)
                lines.extend([f'set(FETCHCONTENT_SOURCE_DIR_{name.upper()} "{source}" CACHE PATH "" FORCE)',
                              f'FetchContent_Declare({name} SOURCE_DIR "{source}")'])
                if not (wrong == 'missing' and name == 'tracy'):
                    lines.append(f'FetchContent_MakeAvailable({name})')
            if wrong == 'outside':
                lines.append(f'set(FETCHCONTENT_SOURCE_DIR_IMGUI "{root}/elsewhere")')
            if pad:
                cmake = root / 'aurora/cmake'
                cmake.mkdir()
                (cmake / 'AuroraDependencyVersions.cmake').write_text(
                    '_aurora_dependency_version(AURORA_SDL3_REF "refs/tags/release-3.4.10" "SDL pin")\n')
                library = root / 'libusb-1.0.0.dylib'
                library.touch()
                patch = root / '0001.patch'
                patch.write_text('fixture patch identity\n')
                stamp = root / 'SDL/.petari-patches/0001.patch.sha256'
                stamp.parent.mkdir()
                stamp.write_text('wrong' if wrong == 'patch-hash' else hashlib.sha256(patch.read_bytes()).hexdigest())
                ref = 'wrong-ref' if wrong == 'sdl-pin' else 'refs/tags/release-3.4.10'
                lines.extend(['set(PETARI_SDL3_REF "refs/tags/release-3.4.10")',
                              f'set(AURORA_SDL3_REF "{ref}")',
                              f'set(PETARI_LIBUSB_LIBRARY "{library}")',
                              f'set(LibUSB_LIBRARY "{library}")',
                              'set(PETARI_LIBUSB_VERSION 1.0.30)',
                              'set(PETARI_LIBUSB_SHA256 fixture-sha)',
                              'set(SDL_HIDAPI_LIBUSB ON)', 'set(SDL_HIDAPI_LIBUSB_SHARED ON)',
                              f'set(PETARI_SDL_PATCHES "{patch}")'])
                config = '#define HAVE_LIBUSB 1\n'
                if wrong != 'sdl-dynamic':
                    config += '#define SDL_LIBUSB_DYNAMIC "libusb-1.0.dylib"\n'
                lines.extend(['FetchContent_GetProperties(SDL BINARY_DIR sdl_binary)',
                              'file(MAKE_DIRECTORY "${sdl_binary}/include-config-release/build_config")',
                              f'file(WRITE "${{sdl_binary}}/include-config-release/build_config/SDL_build_config.h" [=[{config}]=])'])
            lines.extend(['petari_record_graphics_dependencies("${CMAKE_BINARY_DIR}/dependencies.tsv")',
                          'add_library(audio STATIC audio.cpp)', 'add_executable(probe main.cpp)',
                          'target_link_libraries(probe PRIVATE audio)'])
            if link:
                lines.append('petari_link_audio_frameworks(audio)')
            (root / 'CMakeLists.txt').write_text('\n'.join(lines) + '\n')
            commands = []
            def run(command):
                result = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=20)
                commands.append({'command':command,'exit':result.returncode,'output':result.stdout})
                (root / 'commands.json').write_text(json.dumps(commands,indent=2)+'\n')
                return result
            build = root / 'build'
            result = run(['cmake', '-S', str(root), '-B', str(build), '-G', 'Ninja'])
            if wrong:
                self.assertNotEqual(result.returncode,0,result.stdout)
                expected = {'sdl-pin': 'AURORA_SDL3_REF', 'patch-hash': 'SDL patch hash differs',
                            'sdl-dynamic': 'SDL did not enable dynamic libusb'}.get(
                    wrong, 'Pinned' if wrong in ('outside','missing') else 'SDL3 must be')
                self.assertIn(expected, result.stdout)
                return
            self.assertEqual(result.returncode,0,result.stdout)
            result = run(['cmake','--build',str(build),'--parallel','1'])
            if not link:
                self.assertNotEqual(result.returncode,0,result.stdout)
                self.assertIn('_AudioObjectGetPropertyData',result.stdout)
                return
            self.assertEqual(result.returncode,0,result.stdout)
            result = run([str(build / 'probe')])
            self.assertEqual(result.returncode,0,result.stdout)
            self.assertEqual(len((build/'dependencies.tsv').read_text().splitlines()),7 if pad else 5)

    def test_explicit_framework_propagates_from_static_audio(self):
        self.exercise('positive')

    def test_missing_framework_real_link_negative(self):
        self.exercise('negative',link=False)

    def test_dependency_outside_configured_pin_rejected(self):
        self.exercise('outside',wrong='outside')

    def test_imported_system_sdl_rejected(self):
        self.exercise('imported',wrong='imported')

    def test_sdl_target_outside_pinned_directory_rejected(self):
        self.exercise('target-dir',wrong='target-dir')

    def test_unpopulated_dependency_rejected(self):
        self.exercise('missing',wrong='missing')

    def test_pad_dependency_record(self):
        self.exercise('pad', pad=True)

    def test_changed_aurora_sdl_ref_rejected(self):
        self.exercise('sdl-pin', pad=True, wrong='sdl-pin')

    def test_unapplied_patch_hash_rejected(self):
        self.exercise('patch-hash', pad=True, wrong='patch-hash')

    def test_sdl_silently_disabling_dynamic_libusb_rejected(self):
        self.exercise('sdl-dynamic', pad=True, wrong='sdl-dynamic')


if __name__ == '__main__':
    unittest.main()
