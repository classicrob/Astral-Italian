"""Asset-free pin, pristine-patch and optional real Mach-O/loader checks."""
import argparse
import hashlib
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
import urllib.request

NATIVE = Path(__file__).resolve().parents[1]
MODULE = NATIVE / 'cmake/PadDependencies.cmake'
SDL_URL = 'https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.4.10.tar.gz'
LIBUSB_SHA256 = 'fea36f34f9156400209595e300840767ab1a385ede1dc7ee893015aea9c6dbaf'
SDL_SHA256 = '0dc11d980ba17250200718fa4e28011da293f27ed92f92203afffe396811f307'
PARSER = argparse.ArgumentParser()
PARSER.add_argument('--sdl-archive', type=Path)
PARSER.add_argument('--executable', type=Path)
PARSER.add_argument('--library', type=Path)
PARSER.add_argument('--receipts', type=Path)
ARGS, UNITTEST_ARGS = PARSER.parse_known_args()


def run(command, **kwargs):
    result = subprocess.run([str(part) for part in command], text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            timeout=120, **kwargs)
    if result.returncode:
        raise AssertionError(f'{command}: exit {result.returncode}\n{result.stdout}')
    return result.stdout


class LibUSBBundleTests(unittest.TestCase):
    def test_pinned_release_and_recipe(self):
        module = MODULE.read_text()
        self.assertIn(f'set(PETARI_LIBUSB_SHA256 "{LIBUSB_SHA256}")', module)
        self.assertIn('https://github.com/libusb/libusb/releases/download/v1.0.30/libusb-1.0.30.tar.bz2', module)
        self.assertIn('refs/tags/release-3.4.10', module)
        recipe = (NATIVE / 'cmake/libusb/CMakeLists.txt').read_text()
        self.assertIn('URL_HASH "SHA256=${PETARI_LIBUSB_SHA256}"', recipe)
        self.assertIn('--disable-static --enable-shared', recipe)
        self.assertIn('@rpath/libusb-1.0.0.dylib', recipe)
        for option in ('SDL_HIDAPI_LIBUSB', 'SDL_HIDAPI_LIBUSB_SHARED'):
            self.assertIn(f'set({option} ON CACHE BOOL', module)

    def test_all_patches_apply_to_pristine_release_and_are_idempotent(self):
        patches = sorted((NATIVE / 'gx/sdl-patches').glob('*.patch'))
        self.assertTrue(patches)
        with tempfile.TemporaryDirectory(prefix='petari-libusb-patches-') as directory:
            root = Path(directory)
            archive = ARGS.sdl_archive
            if archive is None:
                archive = root / 'SDL.tar.gz'
                with urllib.request.urlopen(SDL_URL, timeout=60) as response, archive.open('wb') as output:
                    shutil.copyfileobj(response, output)
            self.assertEqual(hashlib.sha256(archive.read_bytes()).hexdigest(), SDL_SHA256)
            source = root / 'source'
            source.mkdir()
            run(['tar', '-xf', archive, '-C', source, '--strip-components=1'])
            self.assertNotIn('@executable_path/../Frameworks/libusb-1.0.0.dylib',
                             (source / 'src/misc/SDL_libusb.c').read_text())
            for patch in patches:
                run(['git', 'apply', '--check', patch], cwd=source)
                run(['git', 'apply', patch], cwd=source)
            # Start fresh for the production patch driver: later patches may
            # change earlier patches' context, so record stamps as each applies.
            shutil.rmtree(source)
            source.mkdir()
            run(['tar', '-xf', archive, '-C', source, '--strip-components=1'])
            command = ['cmake', f'-DSDL_SOURCE_DIR={source}',
                       f'-DPATCH_DIR={NATIVE / "gx/sdl-patches"}',
                       '-P', NATIVE / 'cmake/ApplySDLPatches.cmake']
            # Apply the production series, then exercise the stamped path.
            run(command)
            run(command)
            for patch in patches:
                self.assertEqual((source / '.petari-patches' / (patch.name + '.sha256')).read_text(),
                                 hashlib.sha256(patch.read_bytes()).hexdigest())
            # A later controller patch can be dropped in without changing the
            # recipe, and existing stamps must not prevent its application.
            patch_dir = root / 'patches'
            patch_dir.mkdir()
            for patch in patches:
                shutil.copyfile(patch, patch_dir / patch.name)
            (patch_dir / '9999-test-drop-in.patch').write_text(
                '--- /dev/null\n+++ b/petari-drop-in-proof.txt\n@@ -0,0 +1 @@\n+drop-in applied\n')
            run(['cmake', f'-DSDL_SOURCE_DIR={source}', f'-DPATCH_DIR={patch_dir}',
                 '-P', NATIVE / 'cmake/ApplySDLPatches.cmake'])
            self.assertEqual((source / 'petari-drop-in-proof.txt').read_text(), 'drop-in applied\n')

    @unittest.skipUnless(sys.platform == 'darwin' and ARGS.executable and ARGS.library,
                         'pass --executable and --library for the macOS build proof')
    def test_real_bundle_dependencies_and_clean_environment(self):
        executable, library = ARGS.executable.resolve(), ARGS.library.absolute()
        self.assertFalse(library.is_symlink())
        dependencies = run(['otool', '-L', executable])
        self.assertNotIn('libusb', '\n'.join(dependencies.splitlines()[1:]).lower())
        library_dependencies = run(['otool', '-L', library])
        self.assertNotIn('/opt/homebrew', library_dependencies)
        self.assertNotIn('/usr/local', library_dependencies)
        identity = run(['otool', '-D', library])
        self.assertIn('@rpath/libusb-1.0.0.dylib', identity)
        architecture = run(['lipo', '-archs', library])
        self.assertIn('arm64', architecture)
        rpaths = run(['otool', '-l', executable])
        self.assertIn('path @executable_path/../Frameworks', rpaths)
        resources = executable.parents[1] / 'Resources/licenses'
        self.assertEqual((resources / 'COPYING').read_bytes(), (NATIVE / 'licenses/libusb-COPYING').read_bytes())
        self.assertEqual((resources / 'THIRD_PARTY_NOTICES.txt').read_bytes(),
                         (NATIVE / 'licenses/THIRD_PARTY_NOTICES.txt').read_bytes())
        run(['codesign', '--verify', '--strict', library])
        run(['codesign', '--verify', '--strict', executable.parents[2]])
        present = run(['/usr/bin/env', '-i', executable, '--expect-loaded'], env={})
        loaded = next(line.removeprefix('libusb loaded from: ') for line in present.splitlines()
                      if line.startswith('libusb loaded from: '))
        self.assertEqual(Path(loaded).resolve(), library.resolve())
        self.assertIn('result: PASS', present)
        away = library.with_suffix('.dylib.away')
        self.assertFalse(away.exists())
        library.rename(away)
        try:
            absent = run(['/usr/bin/env', '-i', executable, '--expect-absent'], env={})
            self.assertIn('libusb not loaded; HID enumeration completed without libusb', absent)
            self.assertIn('result: PASS', absent)
        finally:
            away.rename(library)
        if ARGS.receipts:
            ARGS.receipts.mkdir(parents=True, exist_ok=True)
            (ARGS.receipts / 'present.txt').write_text(
                f'Command: /usr/bin/env -i {executable} --expect-loaded\n' + present)
            (ARGS.receipts / 'absent.txt').write_text(
                'Bundled dylib moved aside for this run, then restored.\n'
                f'Command: /usr/bin/env -i {executable} --expect-absent\n' + absent)
            (ARGS.receipts / 'macho.txt').write_text(
                'otool -L executable (libusb must be absent):\n' + dependencies +
                '\notool -L libusb (no Homebrew/local dependencies):\n' + library_dependencies +
                '\notool -D libusb:\n' + identity + '\nlipo -archs libusb:\n' + architecture +
                '\nrpath: @executable_path/../Frameworks\n'
                'COPYING and THIRD_PARTY_NOTICES.txt: match repository copies\n'
                'codesign --verify --strict: dylib and check.app passed\n')


if __name__ == '__main__':
    unittest.main(argv=[sys.argv[0], *UNITTEST_ARGS])
