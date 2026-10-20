# M1 GX build with upstream LLVM 20.1.8

This opt-in toolchain builds the native GX application on arm64 macOS 15.1
with Xcode 16.1 / SDK 15.1. It leaves the existing presets and system toolchain
unchanged. Apple clang 16's supplied libc++ lacks the required `std::jthread`.

## Source prerequisite

Validation used **af3a24a8338bd9aa37a139a87afa4b0fc43c62ee +
1048d9554553e3e5f966d3bf24089ebf0a0fe71e, scratch only**.
The latter removes a duplicate native `JAIStreamAramMgr` destructor from
`AudSystem.cpp`; its canonical provider is `JAIStreamDataMgr.cpp`. That separate
source fix must be present for the full app to link. It is not part of this
toolchain change. No duplicate-symbol suppression is used.

## Install the official release in the user account

Use the [official LLVM 20.1.8 release](https://github.com/llvm/llvm-project/releases/tag/llvmorg-20.1.8),
asset `LLVM-20.1.8-macOS-ARM64.tar.xz`. No installer, sudo, or package-manager
installation is needed. The verified archive SHA-256 is:

```text
a9a22f450d35f1f73cd61ab6a17c6f27d8f6051d56197395c1eb397f0c9bbec4
```

```sh
mkdir -p "$HOME/toolchains/downloads"
curl -fL --retry 3 \
  https://github.com/llvm/llvm-project/releases/download/llvmorg-20.1.8/LLVM-20.1.8-macOS-ARM64.tar.xz \
  -o "$HOME/toolchains/downloads/LLVM-20.1.8-macOS-ARM64.tar.xz"
curl -fL --retry 3 \
  https://github.com/llvm/llvm-project/releases/download/llvmorg-20.1.8/LLVM-20.1.8-macOS-ARM64.tar.xz.jsonl \
  -o "$HOME/toolchains/downloads/LLVM-20.1.8-macOS-ARM64.tar.xz.jsonl"
LC_ALL=C shasum -a 256 "$HOME/toolchains/downloads/LLVM-20.1.8-macOS-ARM64.tar.xz"
gh attestation verify "$HOME/toolchains/downloads/LLVM-20.1.8-macOS-ARM64.tar.xz" \
  --bundle "$HOME/toolchains/downloads/LLVM-20.1.8-macOS-ARM64.tar.xz.jsonl" \
  --repo llvm/llvm-project \
  --cert-identity https://github.com/llvm/llvm-project/.github/workflows/release-binaries.yml@refs/tags/llvmorg-20.1.8 \
  --source-ref refs/tags/llvmorg-20.1.8 \
  --source-digest 87f0227cb60147a26a1eeb4fb06e3b505e9c7261
# Continue only after the checksum matches and signature verification succeeds.
nice -n 19 tar -xJf "$HOME/toolchains/downloads/LLVM-20.1.8-macOS-ARM64.tar.xz" \
  -C "$HOME/toolchains"
```

Both checksum and Sigstore verification passed during validation. The clang
binary SHA-256 was
`44698f93798de0177959f17941dbd889ecd802fabd88124dbe86f5cfd93bc1ba`;
its reported source revision was `87f0227cb60147a26a1eeb4fb06e3b505e9c7261`.

## Probe the actual library before building

Create a private `jthread.cpp` outside the checkout:

```cpp
#include <atomic>
#include <thread>
#include <stop_token>
#include <cstdio>
#include <version>
#ifndef __cpp_lib_jthread
#error std::jthread is missing
#endif
int main() {
    std::atomic<bool> entered{false}, stopped{false};
    {
        std::jthread worker([&](std::stop_token token) {
            entered = true;
            while (!token.stop_requested()) std::this_thread::yield();
            stopped = true;
        });
        while (!entered.load()) std::this_thread::yield();
        if (!worker.request_stop()) return 2;
    }
    std::printf("libc++=%d jthread=%ld stop+join=%d\n",
                _LIBCPP_VERSION, long(__cpp_lib_jthread), int(stopped.load()));
    return stopped ? 0 : 3;
}
```

```sh
M1_LLVM_ROOT="$HOME/toolchains/LLVM-20.1.8-macOS-ARM64"
nice -n 19 "$M1_LLVM_ROOT/bin/clang++" -std=c++20 -arch arm64 \
  -mmacosx-version-min=15.0 -isysroot "$(xcrun --show-sdk-path)" \
  -nostdinc++ -isystem "$M1_LLVM_ROOT/include/c++/v1" -nostdlib++ \
  -fuse-ld=lld -Wl,-no_exported_symbols jthread.cpp \
  "$M1_LLVM_ROOT/lib/libc++.a" "$M1_LLVM_ROOT/lib/libc++abi.a" \
  "$M1_LLVM_ROOT/lib/libunwind.a" -o jthread
nice -n 19 ./jthread
otool -L ./jthread
otool -l ./jthread
nice -n 19 env DYLD_PRINT_LIBRARIES=1 ./jthread
```

Expected output: `libc++=200100 jthread=201911 stop+join=1`.
Check that no upstream C++ runtime dylibs are loaded. Apple frameworks may
still load Apple's libc++; the executable keeps its own static C++ symbols private.

## Configure and build in a fresh directory

Coordinate the host's shared `~/MacioGalaxy2-team/slots/native.lock` before
compilation or runtime work. A free lock alone does not supersede the team queue.
Use nice 19 and two build jobs on the M1. In coordinated sessions, record the
host, toolchain, source, phase, output path, PID, and resume action before the
lock/long job and update that state after spawn and completion.

```sh
nice -n 19 cmake -S . -B build/m1-llvm20 -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/native/cmake/M1LLVM.cmake" \
  -DCMAKE_BUILD_TYPE=Release -DPETARI_SANITIZERS=OFF \
  -DPETARI_BUILD_GX_PROBE=ON -DPETARI_BUILD_HEADLESS=OFF \
  -DBUILD_TESTING=ON -DAURORA_SDL3_PROVIDER=vendor \
  -DCMAKE_DISABLE_FIND_PACKAGE_absl=ON -DCMAKE_DISABLE_FIND_PACKAGE_fmt=ON
nice -n 19 cmake --build build/m1-llvm20 --target petari --parallel 2
```

The two package-discovery switches rebuild the C++ dependencies against the
selected headers/runtime. Other dependency choices retain Aurora's defaults.
Validation reused already installed C dependencies (libpng, FreeType, zstd);
no Homebrew installation or system changes were performed. Downloads may be
prepared before the build slot. The validation run used private source overrides
for the same pinned Aurora, Dawn, Abseil, fmt, SDL, and ImGui revisions.

The toolchain defaults to `~/toolchains/LLVM-20.1.8-macOS-ARM64`; override
`PETARI_M1_LLVM_ROOT` to relocate that exact release. It selects C, C++, assembly,
Objective-C and Objective-C++ compilers explicitly. It uses upstream C++ headers
with `-nostdinc++`, disables implicit C++ library selection with `-nostdlib++`,
and links the matching libc++, libc++abi, and libunwind archives.

## Validation and limits

- Real stop-token / join probe passed; the installed release's headers and
  libraries were exercised rather than inferred from the compiler version.
- Full GX compilation and linking succeeded with the separate ODR prerequisite.
  Additional duplicate-symbol failures after that fix: none.
- `petari_platform_allocation_tests` built and `ctest -R
  '^native_platform_allocation$' --output-on-failure` passed (1/1, 0.51 s)
  under the same static toolchain.
- The executable targets minOS 15.0 / SDK 15.1. The downloaded LLVM runtime
  libraries declare minOS 14.0 / SDK 14.5. The pinned Dawn package's 644 objects
  declare minOS 12.0 / SDK 15.5; they linked and ran with the installed SDK 15.1.
- Loader/CLI validation prints usage and exits 2 for `--help` (this app treats
  it as an unrecognized argument). That checks loading, not gameplay.
- Isolated `PETARI_SMOKE=title` startup initialized a Metal adapter and passed
  at frame 992: file select reached after one title press; process exit 0.
  This validates startup/title flow, not a full gameplay or Dolphin comparison.

Apple ld failed when loading this release's libLTO, so the toolchain selects
the matching `ld64.lld`. Dynamic upstream libc++ plus the system libc++ caused
a locale/iostream invalid-free abort at catalog loading. Static archives alone
still allowed weak stream vtables to coalesce across images. The executable-only
`-no_exported_symbols` setting prevents that coalescing; duplicate symbols remain
errors. This configuration is for the application executable, not a tested
interface for C++ plugins or shared C++ objects.

Runtime checks used the approved extracted disc outside git and a newly created
private test profile/cache, outside git. No disc data or user saves/profile are
included in this change. The app still depends on the host's selected C libraries
and Apple frameworks; this is a local build recipe, not a distributable bundle.
