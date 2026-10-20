#!/usr/bin/env python3
"""Exercise the actual removeExtensionString body against the DOL address-word rules."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def body(source):
    start = source.index("char* removeExtensionString(")
    opening = source.index("{", start)
    # The two preprocessor alternatives each open the same if statement.
    # Select the function's four-space closing brace, not a raw brace count.
    end = source.index("\n    }", opening) + len("\n    }")
    return source[start:end]


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
using u32 = std::uint32_t;
namespace MR { BODY }
static int checks;
static void test(const std::string& path, unsigned capacity) {
    char actual[64], expected[64];
    std::snprintf(expected, capacity, "%s", path.c_str());
    int dot = -1, slash = -1;
    for (int i = 0; expected[i]; ++i) {
        if (expected[i] == '.') dot = i;
        if (expected[i] == '/') slash = i;
    }
    // DOL 8005F4E4..F508: CMPLW, ADDI 1, CMPLW, optional STB 0.
    const std::uint32_t base = 0x80004000u;
    const auto dotAddress = dot < 0 ? 0u : base + dot;
    const auto slashAddress = slash < 0 ? 0u : base + slash;
    if (!(dotAddress < slashAddress || slashAddress + 1u == dotAddress) && dotAddress)
        expected[dot] = 0;
    char* result = MR::removeExtensionString(actual, capacity, path.c_str());
    const int resultOffset = result ? int(result - actual) : -1;
    if (std::strcmp(actual, expected) || resultOffset != slash) {
        std::fprintf(stderr, "path=%s capacity=%u result=%s expected=%s offset=%d/%d\n",
                     path.c_str(), capacity, actual, expected, resultOffset, slash);
        std::exit(1);
    }
    ++checks;
}
static void enumerate(std::string text, int remaining) {
    for (unsigned size = 1; size <= text.size() + 2; ++size) test(text, size);
    if (!remaining) return;
    for (char ch : std::string("a/.")) enumerate(text + ch, remaining - 1);
}
int main() {
    for (const char* path : {"", "file", "file.arc", ".hidden", "/.hidden", "dir/file",
                             "dir.with.dot/file", "a/b.c.d", "a/", "a/..", "a/.x.y"})
        for (unsigned size = 1; size <= std::strlen(path) + 2; ++size) test(path, size);
    enumerate("", 6);
    std::printf("%d production StringUtil/DOL-address cases passed\n", checks);
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="clang++")
    args = parser.parse_args()
    path = ROOT / "src/Game/Util/StringUtil.cpp"
    source = path.read_text()
    results = []
    with tempfile.TemporaryDirectory(prefix="smg2-string-ubsan-") as temp:
        temp = Path(temp)
        cpp = temp / "test.cpp"
        cpp.write_text(HARNESS.replace("BODY", body(source)))
        for native in (True, False):
            exe = temp / ("native" if native else "legacy-negative")
            command = [args.compiler, "-std=c++17", "-O1", "-fsanitize=address,undefined,pointer-overflow",
                       "-fno-sanitize-recover=all", "-fno-omit-frame-pointer", str(cpp), "-o", str(exe)]
            if native:
                command.insert(1, "-DGALAXY_NATIVE=1")
            subprocess.run(command, check=True, capture_output=True, text=True)
            result = subprocess.run([str(exe)], capture_output=True, text=True)
            if native:
                assert result.returncode == 0, result.stderr
            else:
                assert result.returncode != 0 and "null pointer" in result.stderr, result.stderr
            results.append({"native": native, "exit": result.returncode,
                            "stdout": result.stdout, "stderr": result.stderr})
    print(json.dumps({"source_sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
                      "results": results}, indent=2))


if __name__ == "__main__":
    main()
