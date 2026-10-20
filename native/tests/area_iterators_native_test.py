#!/usr/bin/env python3
"""Actual Array.hpp and extracted AreaObjMgr::find_in, including opaque predicate effects."""
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
HARNESS = r'''
#include <revolution.h>
#ifdef TEST_OLD_ARRAY
#undef GALAXY_NATIVE
#endif
#include "Game/Util/Array.hpp"
#ifndef GALAXY_NATIVE
#define GALAXY_NATIVE 1
#endif
#include <petari/game_compat.hpp>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <vector>
struct TVec3f { float x, y, z; };
static std::vector<int> trace;
struct AreaObj {
    int id = 0;
    bool inside = false;
    std::function<void()> effect;
    virtual bool isInVolume(const TVec3f&) const {
        trace.push_back(id);
        if (effect) effect();
        return inside;
    }
};
struct AreaObjMgr {
    MR::Vector<MR::AssignableArray<AreaObj*>> mArray;
    AreaObj* find_in(const TVec3f&) const;
};
#ifdef TEST_OLD_AREA
#undef GALAXY_NATIVE
#endif
BODY
static int checks;
static void check(bool ok) { ++checks; if (!ok) std::abort(); }
int main() {
    MR::AssignableArray<int> unallocated;
    const auto& constArray = unallocated;
    check(unallocated.begin() == nullptr && unallocated.end() == nullptr);
    check(constArray.begin() == nullptr && constArray.end() == nullptr);
    MR::Vector<MR::AssignableArray<int>> empty;
    const auto& constEmpty = empty;
    check(empty.begin() == nullptr && empty.end() == nullptr);
    check(constEmpty.begin() == nullptr && constEmpty.end() == nullptr);
    MR::Vector<MR::FixedArray<int,4>> fixed;
    check(fixed.begin() == fixed.end());
    fixed.push_back(7); fixed.push_back(9);
    const auto& constFixed = fixed;
    check(fixed.end() - fixed.begin() == 2 && constFixed.end()[-1] == 9);
    AreaObjMgr noStorage;
    check(noStorage.find_in({}) == nullptr);
    for (int count = 0; count <= 8; ++count) {
        for (int mask = 0; mask < (1 << count); ++mask) {
            AreaObj objects[8]; AreaObjMgr mgr; mgr.mArray.init(8);
            AreaObj* expected = nullptr; std::vector<int> expectedTrace;
            for (int i=0; i<count; ++i) {
                objects[i].id=i; objects[i].inside=(mask & (1 << i)) != 0;
                mgr.mArray.push_back(&objects[i]);
            }
            for (int i=count; i>0;) {
                --i; expectedTrace.push_back(i);
                if (objects[i].inside) {expected=&objects[i]; break;}
            }
            trace.clear();
            check(mgr.find_in({}) == expected && trace == expectedTrace);
        }
    }
    AreaObj objects[4], replacement; AreaObjMgr mgr; mgr.mArray.init(4);
    for (int i=0;i<4;++i) { objects[i].id=i; mgr.mArray.mArray.mArr[i]=&objects[i]; }
    mgr.mArray.mCount=3;
    objects[2].inside=true;
    objects[2].effect=[&] {mgr.mArray.mCount=2;};
    // Target reloads end after the predicate, so a matching cell at new end is excluded.
    check(mgr.find_in({}) == nullptr);
    mgr.mArray.mCount=3;
    objects[2].effect=[&] {mgr.mArray[2]=&replacement;};
    check(mgr.find_in({}) == &replacement);  // result cell is reloaded after the predicate
    mgr.mArray[2]=&objects[2]; objects[2].inside=false;
    objects[2].effect=[&] {mgr.mArray.mCount=0;};
    trace.clear();
    // Search keeps its original range; final end comparison observes the new one.
    check(mgr.find_in({}) == &objects[3] && trace == std::vector<int>({2,1,0}));
    std::printf("%d production array/reverse-search checks passed\n", checks);
}
'''


def main():
    source = ROOT / "src/Game/AreaObj/AreaObj.cpp"
    text = source.read_text()
    start = text.index("AreaObj* AreaObjMgr::find_in(")
    end = text.index("\n}", start) + len("\n}")
    results = []
    with tempfile.TemporaryDirectory(prefix="smg2-area-ubsan-") as temp:
        temp = Path(temp)
        cpp = temp / "test.cpp"
        cpp.write_text(HARNESS.replace("BODY", text[start:end]))
        for mode in ("native", "TEST_OLD_ARRAY", "TEST_OLD_AREA"):
            exe = temp / mode
            command = ["clang++", "-std=c++17", "-DGALAXY_NATIVE=1", "-O1",
                       "-fsanitize=address,undefined,pointer-overflow", "-fno-sanitize-recover=all",
                       "-fno-omit-frame-pointer", "-Wno-deprecated-declarations", "-Wno-register"]
            for directory in ("native/include", "include", "libs/RVL_SDK/include",
                              "libs/JSystem/include", "libs/nw4r/include"):
                command += ["-I", str(ROOT / directory)]
            if mode != "native":
                command += ["-D" + mode + "=1"]
            subprocess.run(command + [str(cpp), "-o", str(exe)], check=True, capture_output=True, text=True)
            result = subprocess.run([str(exe)], capture_output=True, text=True)
            if mode == "native":
                assert result.returncode == 0, result.stderr
            else:
                assert result.returncode != 0 and "null pointer" in result.stderr, result.stderr
            results.append({"mode": mode, "exit": result.returncode,
                            "stdout": result.stdout, "stderr": result.stderr})
    print(json.dumps({"source_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
                      "array_sha256": hashlib.sha256((ROOT / "include/Game/Util/Array.hpp").read_bytes()).hexdigest(),
                      "results": results}, indent=2))


if __name__ == "__main__":
    main()
