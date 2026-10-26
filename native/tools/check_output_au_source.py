#!/usr/bin/env python3
"""Serialized CPU-only source check. No app/device/GPU/default setter.

Invoke under lockf -t 0 -k m1-heavy.lock. Retains every command/log/executable;
native.lock is acquired nonblocking here. The real app/codegen/measurement is
PORT's server-build + reviewed M1 sequence, not this compile check.
"""
import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument("--out", type=Path, required=True)
args = parser.parse_args()
allowed = Path.home() / "MacioGalaxy2-gamedata/port53/m1g/build-reports"
out = args.out.resolve()
if not out.is_relative_to(allowed.resolve()) or out.exists():
    raise SystemExit("refused existing/out-of-namespace output")
out.mkdir(parents=True)
receipt = {"kind": "CPU-only source compile and fake-device fixtures", "app_or_real_device_run": False,
           "gpu_cache_access": False, "default_device_setter": False, "started_ns": time.time_ns(), "commands": []}


def save():
    (out / "RESULT.json").write_text(json.dumps(receipt, indent=2) + "\n")


def run(argv, label):
    row = {"label": label, "argv": list(map(str, argv)), "start_ns": time.time_ns()}
    receipt["commands"].append(row)
    with (out / (label + ".log")).open("w") as log:
        result = subprocess.run(argv, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, timeout=120)
    row.update(returncode=result.returncode, end_ns=time.time_ns())
    save()
    print(label, result.returncode, flush=True)
    if result.returncode:
        print((out / (label + ".log")).read_text()[-6000:], flush=True)
        raise RuntimeError(label)


try:
    with (Path.home() / "MacioGalaxy2-team/slots/native.lock").open("a+") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        processes = subprocess.check_output(["ps", "-axo", "pid=,comm="], text=True)
        foreign = [line for line in processes.splitlines() if Path(line.split(maxsplit=1)[-1]).name in
                   ("Petari", "ninja", "clang", "clang++", "mwcceppc", "wibo", "ld")]
        receipt["foreign_native_processes"] = foreign
        if foreign:
            raise RuntimeError("foreign native process; no compilation started")
        compiler = subprocess.check_output(["xcrun", "--find", "clang++"], text=True).strip()
        sdk = subprocess.check_output(["xcrun", "--show-sdk-path"], text=True).strip()
        flags = [compiler, "-isysroot", sdk, "-std=c++17", "-O2", "-g", "-UNDEBUG", "-pthread", "-Inative/include", "-Inative/platform/include"]
        frameworks = ["-framework", "AudioToolbox", "-framework", "CoreAudio", "-framework", "IOKit", "-framework", "CoreFoundation"]
        pinned = list((ROOT / "native/audio").glob("output_au*")) + list((ROOT / "native/tests").glob("audio_output*"))
        pinned += list((ROOT / "native/platform/audio").glob("audio_workgroup*.cpp")) + list((ROOT / "native/platform/audio").glob("workgroup_history*.hpp"))
        pinned += [ROOT / "native/audio/sdl_sink.cpp", ROOT / "native/audio/paced_ring.hpp",
                   ROOT / "native/include/petari/audio_output_au.hpp", ROOT / "native/platform/include/petari/platform/audio_workgroup.hpp"]
        def hashes():
            return {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in pinned if p.is_file()}
        receipt["source_hashes"] = hashes()
        for source in ("native/audio/output_au_control.cpp", "native/audio/output_au_render.cpp",
                       "native/audio/output_au_converter.cpp", "native/platform/audio/audio_workgroup_rt.cpp",
                       "native/platform/audio/audio_workgroup.cpp"):
            run(flags + ["-fsyntax-only", source], "syntax-" + Path(source).stem)
        sdl = Path.home() / "MacioGalaxy2-team/scratch-port/gate53/deps/sdl-src-patched/include"
        overlay = Path.home() / "MacioGalaxy2-gamedata/port53/m1g/build-reports/sola-aq-localize-r1/sdl-patched/include"
        base = Path.home() / "MacioGalaxy2-team/scratch-port/solA/build-ov-gx/_deps/sdl-src/include"
        if (sdl / "SDL3/SDL_petari_audio_diag.h").is_file():
            run(flags + ["-I" + str(sdl), "-fsyntax-only", "native/audio/sdl_sink.cpp"], "syntax-sdl_sink")
        elif (overlay / "SDL3/SDL_petari_audio_diag.h").is_file() and (base / "SDL3/SDL_audio.h").is_file():
            run(flags + ["-I" + str(overlay), "-I" + str(base), "-fsyntax-only", "native/audio/sdl_sink.cpp"], "syntax-sdl_sink")
        else:
            receipt["sdl_syntax_pending"] = "local pinned SDL has no 0003 header; PORT prepared SDL required"
        cases = {
            "core": ["native/tests/audio_output_core_tests.cpp"],
            "converter": ["native/tests/audio_output_converter_tests.cpp", "native/audio/output_au_converter.cpp"],
            "render": ["native/tests/audio_output_render_tests.cpp",
                       "native/audio/output_au_render.cpp", "native/platform/audio/audio_workgroup_rt.cpp"],
            "control": ["native/tests/audio_output_control_tests.cpp", "native/audio/output_au_control.cpp",
                        "native/audio/output_au_render.cpp", "native/audio/output_au_converter.cpp"],
        }
        for name, sources in cases.items():
            executable = out / ("petari_audio_output_" + name + "_tests")
            define = ["-DPETARI_OUTPUT_AU_TESTING=1"] if name == "control" else []
            libraries = []
            if name == "render":
                guard = out / "libpetari_audio_output_rt_guard.dylib"
                run(flags + ["-dynamiclib", "native/tests/audio_output_rt_guard.cpp"] + frameworks + ["-Wl,-install_name," + str(guard),
                             "-o", str(guard)], "build-guard")
                libraries = [str(guard)]
            run(flags + define + sources + libraries + frameworks + ["-o", str(executable)], "build-" + name)
            run([str(executable)], "test-" + name)
            receipt[name + "_sha256"] = hashlib.sha256(executable.read_bytes()).hexdigest()
        receipt["passed"] = True
        receipt["sources_unchanged"] = hashes() == receipt["source_hashes"]
        if not receipt["sources_unchanged"]:
            raise RuntimeError("source changed during CPU checks")
except Exception as error:
    receipt["passed"] = False
    receipt["error"] = str(error)
finally:
    receipt["ended_ns"] = time.time_ns()
    save()
raise SystemExit(0 if receipt.get("passed") else 1)
