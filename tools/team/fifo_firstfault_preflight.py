#!/usr/bin/env python3
"""Fail-closed source preflight for a future isolated FIFO diagnostic generator.

This phase deliberately emits NO C++ patch: exact producer-copy provenance is
not yet synchronization-qualified. Exit2 is BLOCKED, never a passing diagnostic.
Reads public source only; never reads FIFO bytes, game data or a running process.
"""
import argparse
import hashlib
import json
from pathlib import Path
import sys

ROLES = ('fifo', 'processor', 'display_list', 'fifo_header')
OWNED_OUTPUT_ROOT = Path.home() / 'MacioGalaxy2-team/scratch-port/solP/fifo-firstfault-prototype-output'
PROFILE_SHA = '66afa66a86800deac0dc427d1071e7f866e71fdc7d560c8c72689cc12e42c43c'


class Rejected(ValueError):
    pass


def require(condition, message):
    if not condition:
        raise Rejected(message)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def unique(text, needle):
    require(text.count(needle) == 1, 'missing/nonunique source anchor: ' + needle)
    return text.index(needle)


def feasibility(texts):
    """Bounded exact source witnesses, not a C++ synchronization proof."""
    fifo, processor, dl, header = (texts[r] for r in ROLES)
    # Slow copy recalculates its destination only AFTER a potentially yielding wait.
    begin = unique(fifo, 'void write_data_grow(const void* data, uint32_t length) {')
    end = unique(fifo, '\nvoid publish() noexcept {')
    require(begin < end, 'grow/publish source order')
    grow = fifo[begin:end]
    unique(grow, 'const auto grow = [data, length] {')
    unique(grow, 'std::memcpy(detail::sBufferData + detail::sBufferSize, data, length);')
    unique(grow, 'detail::sBufferSize = needed;')
    unique(grow, 'petari_with_buffer_lock(grow);')
    unique(fifo, 'const bool released = petari_gx_sync_begin_host_wait() != 0;')
    unique(fifo, 'petari_gx_sync_end_host_wait();')
    unique(fifo, 'sStreamBase = target;')
    unique(fifo, 'sStreamBase = processed;')
    unique(fifo, 'uint64_t petari_aurora_fifo_write_position(void) {')
    # The known processor consumes its immutable mirror OUTSIDE sBufferMutex.
    unique(fifo, 'sPetariMirror.insert(sPetariMirror.end(), from, from + (limit - mirrored));')
    unique(fifo, 'const ProcessResult result = process(sPetariMirror.data() + (processed - sPetariMirrorBase),')
    pb = unique(processor, 'ProcessResult process(const u8* data, u32 size) noexcept {')
    pe = unique(processor, '[[noreturn]] static void handle_draw_overrun(')
    require(pb < pe, 'processor scope order')
    processor = processor[pb:pe]
    unique(processor, 'const u8 cmd = reader.read<u8>();')
    unique(processor, 'u8 opcode = cmd & CP_OPCODE_MASK;')
    unique(processor, '"indexed XF load from unmapped array {}"')
    unique(processor, 'Log.warn("Ignoring nested GX_CMD_CALL_DL");')
    unique(dl, 'void GXCallDisplayList(const void* data, u32 nbytes) {')
    unique(dl, 'aurora::gx::fifo::write_data(data, nbytes);')
    unique(dl, 'aurora::gx::fifo::publish();')
    unique(header, 'inline void write_data(const void* data, const uint32_t length) {')
    unique(header, 'std::memcpy(detail::sBufferData + detail::sBufferSize, data, length);')
    unique(header, 'std::memcpy(detail::sDlBuffer + detail::sDlWritePos, data, length);')
    return {
        'worker_offset_seam': 'processed + reader.offset(), against worker-owned immutable mirror; copy context before each process() call, preserve abort/restart generation',
        'wrapper_only_span': 'REJECTED: pre/post write_position() is not a copy-site witness; grow wait may yield CPU before choosing destination',
        'required_copy_sites': ['fifo_header fast main-stream memcpy', 'fifo.cpp grow closure memcpy under buffer lock', 'fifo_header DL redirection memcpy (recording domain, not main stream)'],
        'current_profile_witnesses_valid': True,
        'synchronization_qualified': False,
        'blocking_requirements': [
            'Bind actual compiled build.ninja edges and generated-header inclusion; matching individual source hashes alone does not bind a binary.',
            'Review all FIFO writer/interrupt serialization and reentrancy against OS CPU baton before adding any producer TLS context; three source snippets do not prove this.',
            'Capture exact stream start/end at actual fast/slow copy site, before publication; wrapper range can include another writer while grow yields.',
            'Choose reviewed bounded sidecar synchronization: slow-copy buffer-lock -> sidecar order, worker snapshot outside buffer lock; prohibit reverse order and allocation/I/O/GX calls while either held.',
            'Specify reset/generation and abort/compaction handling; absolute counters restart at init, display-list recording has a separate position domain.',
            'Review immutable metadata publication and ring eviction so no plain C++ data race or overwritten slot is mistaken for a matching list.',
            'Review private first-trigger flush and bounded raw/state storage; no flush or usable capture hook is emitted by this preflight.',
        ],
        'missing_capture_coverage': ['producer copy-site sidecar', 'raw command ledger', 'preceding primitive VCD/VAT/cache snapshot', 'CP/native-array binding history', 'bounded private raw-byte flush'],
    }


def validate_profile(data):
    require(sha(data) == PROFILE_SHA, 'effective-source profile hash mismatch')
    profile = json.loads(data)
    require(set(profile) == {'schema', 'scope', 'hashes', 'production_acceptance'}, 'profile schema')
    require(profile['schema'] == 1 and profile['production_acceptance'] is False, 'profile status')
    require(set(profile['hashes']) == set(ROLES), 'profile role closure')
    return profile


def confined_output(root, sources):
    require(not root.is_symlink(), 'output symlink')
    root = root.resolve()
    allowed = OWNED_OUTPUT_ROOT.absolute()
    require(allowed.resolve() == allowed, 'owned output root has symlink ancestry')
    require(root != allowed and allowed in root.parents, 'output outside fixed P-owned scratch root')
    require(not root.exists(), 'output must be a fresh nonexistent private directory')
    for source in sources:
        source = source.resolve()
        require(root != source and root not in source.parents, 'output contains input source')
    # All emitted files are fixed names below this newly created directory.
    return root


def run(args):
    profile = validate_profile(Path(__file__).with_name('fifo_firstfault_effective_v1.json').read_bytes())
    paths = {role: Path(getattr(args, role)).resolve(strict=True) for role in ROLES}
    require(len(set(paths.values())) == len(ROLES), 'source roles alias')
    require(all(p.is_file() for p in paths.values()), 'regular source files required')
    raw = {role: path.read_bytes() for role, path in paths.items()}
    for role, data in raw.items():
        require(sha(data) == profile['hashes'][role], 'effective source hash mismatch: ' + role)
    evidence = feasibility({role: data.decode('utf-8') for role, data in raw.items()})
    output = confined_output(Path(args.output_root), paths.values())
    report = {
        'schema': 1, 'status': 'BLOCKED_SYNCHRONIZATION_REVIEW_REQUIRED',
        'patch_emitted': False, 'capture_ready': False, 'production_acceptance': False,
        'profile_sha256': PROFILE_SHA,
        'sources': {r: {'path': str(paths[r]), 'sha256': sha(raw[r])} for r in ROLES},
        'feasibility': evidence,
        'preserved_operations': 'No source is written; parser cursor/publication/state/fatal operations unchanged. Default OFF vacuously; no instrumentation exists yet.',
    }
    output.mkdir(parents=True, mode=0o700)
    path = output / 'preflight.json'
    path.write_text(json.dumps(report, indent=2) + '\n')
    path.chmod(0o600)
    print(json.dumps({'status': report['status'], 'report': str(path), 'patch_emitted': False}))
    return 2


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for role in ROLES:
        parser.add_argument('--' + role.replace('_', '-'), required=True)
    parser.add_argument('--output-root', required=True)
    args = parser.parse_args()
    try:
        return run(args)
    except (Rejected, OSError, UnicodeError, json.JSONDecodeError) as error:
        print('REJECTED: ' + str(error), file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
