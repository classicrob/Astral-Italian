#!/usr/bin/env python3
"""Generate diagnostic function traps and optional unreadable guarded data ranges."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shlex
import subprocess

DATA_PREFIXES = ("vtable for ", "VTT for ", "typeinfo for ", "typeinfo name for ", "guard variable for ")


def symbol_kinds(path):
    kinds = {}
    for line in path.read_text().splitlines():
        match = re.match(r"(\S+)\s*=.*?//.*?\btype:(function|object)\b", line)
        if match:
            kinds[match[1]] = match[2]
    return kinds


def classify(display, raw, kinds):
    name = raw[1:] if raw.startswith("_") else raw
    if display.startswith(DATA_PREFIXES) or name.startswith(("_ZTV", "_ZTT", "_ZTI", "_ZTS", "_ZGV")):
        return "data", "vtable/RTTI/guard storage"
    if kinds.get(name) == "object":
        return "data", "symbol-kind metadata: object"
    if name.startswith("_Z"):
        if re.search(r"\)(?:(?: const| volatile| noexcept| &| &&))*$", display):
            return "function", "demangled function signature"
        return "data", "mangled C++ symbol without a function signature"
    if kinds.get(name) == "function":
        return "function", "symbol-kind metadata: function"
    return "unknown", "no positive function evidence; no definition generated"


def generate(inventory, symbols, output, report, inputs=None, extra_kinds=None, guard_data=False, collect_functions=False):
    payload = json.loads(inventory.read_text())
    kinds = symbol_kinds(symbols)
    if extra_kinds is not None:
        declarations = json.loads(extra_kinds.read_text())
        if declarations.get("schema_version") != 1:
            raise ValueError("unsupported symbol-kind declaration schema")
        for name, evidence in declarations["symbols"].items():
            if evidence["kind"] not in ("function", "object") or not evidence.get("declaration"):
                raise ValueError("symbol-kind declarations require a kind and source evidence")
            if name in kinds and kinds[name] != evidence["kind"]:
                raise ValueError("conflicting symbol-kind evidence: " + name)
            kinds[name] = evidence["kind"]
    paths = []
    if inputs is not None:
        paths = [arg for arg in json.loads(inputs.read_text())["args"] if arg.endswith((".o", ".a"))]
    result = dict(schema_version=1, diagnostic_only=True, inventory=str(inventory),
                  inventory_sha256=hashlib.sha256(inventory.read_bytes()).hexdigest(),
                  scope=payload.get("scope"), functions=[], data=[], unknown=[], already_provided=[])
    result["extra_symbol_kinds"] = str(extra_kinds) if extra_kinds else None
    result["data_policy"] = "protected-unreadable-storage" if guard_data else "undefined"
    result["function_policy"] = "collect-zero" if collect_functions else "abort"
    seen = set()
    for entry in payload["entries"]:
        raw_names = entry.get("raw_symbols", [])
        if not raw_names:
            result["unknown"].append(dict(symbol=entry["symbol"], raw=None, owner=entry.get("owner"),
                                          reason="inventory has no raw assembler symbol"))
        for raw in raw_names:
            if raw in seen:
                continue
            seen.add(raw)
            if not re.fullmatch(r"[A-Za-z_.$][A-Za-z0-9_.$]*", raw):
                raise ValueError("unsupported raw assembler symbol: " + raw)
            row = dict(symbol=entry["symbol"], raw=raw, owner=entry.get("owner"))
            providers = entry.get("compiled_definitions", {}).get(raw, [])
            if any(location == path or location.startswith((path + "(", path + "[")) for location in providers for path in paths):
                row["reason"] = "definition exists in actual link inputs; do not create a duplicate"
                result["already_provided"].append(row)
                continue
            kind, reason = classify(entry["symbol"], raw, kinds)
            row["reason"] = reason
            result["functions" if kind == "function" else kind].append(row)
    for key in ("functions", "data", "unknown", "already_provided"):
        result[key].sort(key=lambda row: (row["raw"] or "", row["symbol"]))
        for row in result[key]:
            row["display_name"] = row["raw"][1:] if row["raw"] and row["symbol"] == row["raw"] and row["raw"].startswith("_") else row["symbol"]
    lines = ["// PLACEHOLDER(port): diagnostic link; resolve when the undefined list is empty.",
             "#include <cstdio>", "#include <cstdlib>", "",
             '__attribute__((constructor(100))) static void petariTrapModeNotice() {',
             '    std::fputs("PLACEHOLDER(port): diagnostic link active; no gameplay qualification\\n", stderr);',
             '    std::fflush(stderr);', '}', '']
    if collect_functions:
        lines += ['#include <atomic>', '#include <cstdint>',
                  'extern "C" std::uint64_t petari_headless_source_frame() __attribute__((weak_import));',
                  'static std::atomic<unsigned long long> petariCollectSequence{0};',
                  'static void petariCollect(const char* name, void* caller) {',
                  '    const auto frame = petari_headless_source_frame ? petari_headless_source_frame() : UINT64_MAX;',
                  '    const auto sequence = petariCollectSequence.fetch_add(1, std::memory_order_relaxed);',
                  '    std::fprintf(stderr, "PLACEHOLDER(port): collect sequence=%llu frame=%llu symbol=%s caller=%p\\n", sequence, static_cast<unsigned long long>(frame), name, caller);',
                  '    std::fflush(stderr);', '}', '']
    for index, row in enumerate(result["functions"]):
        identifier = f"petari_undefined_trap_{index}"
        if collect_functions:
            lines += [f'extern "C" __attribute__((noinline)) std::uintptr_t {identifier}() __asm__({json.dumps(row["raw"])});',
                      f'extern "C" __attribute__((noinline)) std::uintptr_t {identifier}() {{',
                      '    static std::atomic_flag seen = ATOMIC_FLAG_INIT;',
                      f'    if (!seen.test_and_set(std::memory_order_relaxed)) petariCollect({json.dumps(row["display_name"])}, __builtin_extract_return_addr(__builtin_return_address(0)));',
                      '#if defined(__aarch64__)',
                      '    __asm__ volatile("mov x1, xzr\\nmovi v0.16b, #0\\nmovi v1.16b, #0\\nmovi v2.16b, #0\\nmovi v3.16b, #0" ::: "x1", "v0", "v1", "v2", "v3");',
                      '#endif', '    return 0;', '}', '']
            continue
        lines += [f'extern "C" [[noreturn]] __attribute__((noinline)) void {identifier}() __asm__({json.dumps(row["raw"])});',
                  f'extern "C" [[noreturn]] __attribute__((noinline)) void {identifier}() {{',
                  '    void* caller = __builtin_extract_return_addr(__builtin_return_address(0));',
                  f'    std::fprintf(stderr, "PLACEHOLDER(port): undefined %s\\nreturn address: %p\\n", {json.dumps(row["display_name"])}, caller);',
                  '    std::fflush(stderr);', '    std::abort();', '}', '']
    guarded = [row for row in result["data"] + result["unknown"] if row["raw"]] if guard_data else []
    result["guarded_data"] = guarded
    if guarded:
        lines += ['#include <cstddef>', 'struct TrapGuard { const char* name; const unsigned char* start; std::size_t size; };', 'extern "C" {']
        for index, row in enumerate(guarded):
            lines.append(f'alignas(65536) unsigned char petari_guard_{index}[65536] __asm__({json.dumps(row["raw"])}) __attribute__((section("__DATA,__ptguard"), used)) = {{}};')
        lines += ['}', 'static const TrapGuard trapGuards[] = {']
        for index, row in enumerate(guarded):
            lines.append(f'    {{{json.dumps(row["display_name"])}, petari_guard_{index}, sizeof(petari_guard_{index})}},')
        lines += ['};', (Path(__file__).resolve().parents[1] / 'app/headless/trap_guard_runtime.inc').read_text()]
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines) + "\n")
    report.write_text(json.dumps(result, indent=2) + "\n")
    print(f"diagnostic traps: {len(result['functions'])} functions; {len(result['data'])} data/RTTI/vtables, {len(result['unknown'])} unknown; {len(guarded)} PROT_NONE ranges", flush=True)
    return result


def link(inputs, objects, output, log, status):
    manifest = json.loads(inputs.read_text())
    original = manifest["args"]
    args, index = [], 0
    while index < len(original):
        arg = original[index]
        index += 1
        if arg == "-o":
            index += 1
        else:
            args.append(arg)
    args += [str(path.resolve()) for path in objects] + ["-o", str(output.resolve())]
    output.unlink(missing_ok=True)
    Path(str(output) + ".diagnostic.json").unlink(missing_ok=True)
    response = output.with_suffix(".link.rsp")
    response.write_text("\n".join(shlex.quote(arg) for arg in args[1:]) + "\n")
    with log.open("w") as out:
        code = subprocess.run([args[0], "@" + str(response.resolve())], cwd=manifest["cwd"], stdout=out, stderr=subprocess.STDOUT).returncode
    status.write_text(json.dumps(dict(diagnostic_only=True, command=args, cwd=manifest["cwd"],
                                     exit_status=code, output=str(output), linked=code == 0), indent=2) + "\n")
    if code == 0:
        Path(str(output) + ".diagnostic.json").write_text(json.dumps(dict(diagnostic_only=True, inputs=str(inputs)), indent=2) + "\n")
    if code:
        print(log.read_text(errors="replace")[-12000:])
    return code


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    gen = sub.add_parser("generate")
    gen.add_argument("--inventory", type=Path, required=True)
    gen.add_argument("--symbols", type=Path, required=True)
    gen.add_argument("--inputs", type=Path)
    gen.add_argument("--extra-kinds", type=Path)
    gen.add_argument("--guard-data", action="store_true", help="diagnostic unreadable PROT_NONE storage; no usable data values")
    gen.add_argument("--collect-functions", action="store_true", help="diagnostic first-call logging and zero scalar return; data remains protected")
    gen.add_argument("--output", type=Path, required=True)
    gen.add_argument("--report", type=Path, required=True)
    run = sub.add_parser("link")
    run.add_argument("--inputs", type=Path, required=True)
    run.add_argument("--object", type=Path, action="append", required=True)
    run.add_argument("--output", type=Path, required=True)
    run.add_argument("--log", type=Path, required=True)
    run.add_argument("--status", type=Path, required=True)
    args = parser.parse_args()
    if args.command == "generate":
        generate(args.inventory, args.symbols, args.output, args.report, args.inputs, args.extra_kinds, args.guard_data, args.collect_functions)
        return 0
    return link(args.inputs, args.object, args.output, args.log, args.status)


if __name__ == "__main__":
    raise SystemExit(main())
