Actual local run on 2026-10-22 (America/New_York), on the Apple M1 Max MacBook arm64,
macOS 15.1 (24B5070a), CMake 3.27.7, Ninja, Apple clang 16.0.0.

Commands used (with `LC_ALL=C` for the build/test commands):

```sh
cmake -S native/tools/libusb_load_check -B /tmp/pad-sol-a-libusb/check-build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0
cmake --build /tmp/pad-sol-a-libusb/check-build --parallel 4
ctest --test-dir /tmp/pad-sol-a-libusb/check-build --output-on-failure
python3 native/tests/libusb_bundle_tests.py --sdl-archive /tmp/pad-sol-a-libusb/sdl.tar.gz \
  --executable /tmp/pad-sol-a-libusb/check-build/check.app/Contents/MacOS/check \
  --library /tmp/pad-sol-a-libusb/check-build/check.app/Contents/Frameworks/libusb-1.0.0.dylib \
  --receipts native/tests/receipts/libusb_bundle_20261022 -v
cmake -S . -B /tmp/pad-sol-a-libusb/root-build -G Ninja \
  -DPETARI_BUILD_GX_PROBE=OFF -DPETARI_BUILD_APP=OFF
ctest --test-dir /tmp/pad-sol-a-libusb/root-build \
  -R '^native_(libusb_bundle|graphics_dependency_build)$' --output-on-failure
```

`present.txt` and `absent.txt` contain the actual `env -i` runs. There were no
DYLD variables. The test moved the bundled dylib aside for the absent run and
restored it afterwards. Both runs enumerated successfully (zero devices) without
a crash. The present run's dyld image path is inside the bundle's Frameworks.

`macho.txt` records actual dependency/install-name/architecture checks, plus
successful signature, licence-payload and rpath verification. Both the libusb
dylib and check executable have deployment target 13.0; libusb is arm64.
`dependency-paths.tsv` records the exact libusb and applied SDL patch hashes.
`validation.txt` contains the standalone Python and CTest summaries.

The mini bundle is named `check.app`, with executable `Contents/MacOS/check`.
It uses the same PadDependencies.cmake, patched static SDL, libusb recipe,
bundle packaging function and nested ad-hoc signing as Petari. Full GX/Petari
was not built: this M1 has clang 16, while GX requires clang 21. Petari-specific
linkage and final bundle were therefore verified only through these shared
rules in the standalone project. No controller compatibility claim is made.

The libusb release archive passed the pinned SHA256 check in ExternalProject.
SDL patches were generated from a fresh release-3.4.10 tarball extraction; tests
apply every patch to another pristine extraction, rerun the production driver,
and verify discovery of an additional patch. Repeated configure/build completed
without rebuilding unchanged dependencies.

Scratch builds, downloaded tarballs and extractions were deleted after
verification; only these small receipts remain. No server Mac was used.
