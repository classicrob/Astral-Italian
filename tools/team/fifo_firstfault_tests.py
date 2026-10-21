"""Lightweight source-generator rejection tests. No C++ build or game runtime."""
import contextlib
import io
import json
from pathlib import Path
import sys
import tempfile
import types
import unittest
from unittest.mock import patch

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
import fifo_firstfault_preflight as ff

# Deliberately synthetic source witnesses; never presented as effective-source proof.
FIXTURE = {
    'fifo': '\n'.join([
        'void write_data_grow(const void* data, uint32_t length) {',
        'const auto grow = [data, length] {',
        'std::memcpy(detail::sBufferData + detail::sBufferSize, data, length);',
        'detail::sBufferSize = needed;', 'petari_with_buffer_lock(grow);',
        '\nvoid publish() noexcept {',
        'const bool released = petari_gx_sync_begin_host_wait() != 0;',
        'petari_gx_sync_end_host_wait();', 'sStreamBase = target;',
        'sStreamBase = processed;', 'uint64_t petari_aurora_fifo_write_position(void) {',
        'sPetariMirror.insert(sPetariMirror.end(), from, from + (limit - mirrored));',
        'const ProcessResult result = process(sPetariMirror.data() + (processed - sPetariMirrorBase),']),
    'processor': '\n'.join(['ProcessResult process(const u8* data, u32 size) noexcept {',
        'const u8 cmd = reader.read<u8>();', 'u8 opcode = cmd & CP_OPCODE_MASK;',
        '"indexed XF load from unmapped array {}"',
        'Log.warn("Ignoring nested GX_CMD_CALL_DL");', '[[noreturn]] static void handle_draw_overrun(']),
    'display_list': '\n'.join(['void GXCallDisplayList(const void* data, u32 nbytes) {',
        'aurora::gx::fifo::write_data(data, nbytes);', 'aurora::gx::fifo::publish();']),
    'fifo_header': '\n'.join(['inline void write_data(const void* data, const uint32_t length) {',
        'std::memcpy(detail::sBufferData + detail::sBufferSize, data, length);',
        'std::memcpy(detail::sDlBuffer + detail::sDlWritePos, data, length);']),
}


class PreflightTests(unittest.TestCase):
    def test_pinned_profile(self):
        p = Path(ff.__file__).with_name('fifo_firstfault_effective_v1.json')
        self.assertEqual(set(ff.validate_profile(p.read_bytes())['hashes']), set(ff.ROLES))

    def test_changed_profile_rejects(self):
        p = Path(ff.__file__).with_name('fifo_firstfault_effective_v1.json')
        with self.assertRaises(ff.Rejected): ff.validate_profile(p.read_bytes() + b' ')

    def test_unique_anchor_rejects_missing(self):
        with self.assertRaises(ff.Rejected): ff.unique('source', 'absent')

    def test_unique_anchor_rejects_duplicate(self):
        with self.assertRaises(ff.Rejected): ff.unique('one one', 'one')

    def test_feasibility_never_claims_capture_ready(self):
        r = ff.feasibility(FIXTURE)
        self.assertFalse(r['synchronization_qualified'])
        self.assertGreaterEqual(len(r['blocking_requirements']), 6)
        self.assertIn('REJECTED', r['wrapper_only_span'])

    def test_each_critical_anchor_negative(self):
        for role, old in [('fifo', 'petari_with_buffer_lock(grow);'),
                          ('fifo', 'petari_gx_sync_end_host_wait();'),
                          ('fifo', 'sStreamBase = target;'),
                          ('processor', 'const u8 cmd = reader.read<u8>();'),
                          ('processor', 'u8 opcode = cmd & CP_OPCODE_MASK;'),
                          ('display_list', 'aurora::gx::fifo::publish();'),
                          ('fifo_header', 'std::memcpy(detail::sDlBuffer + detail::sDlWritePos, data, length);')]:
            with self.subTest(role=role, anchor=old):
                d = dict(FIXTURE); d[role] = d[role].replace(old, '// removed')
                with self.assertRaises(ff.Rejected): ff.feasibility(d)

    def test_duplicate_inside_processor_rejects(self):
        d = dict(FIXTURE)
        d['processor'] = d['processor'].replace('u8 opcode =', 'const u8 cmd = reader.read<u8>();\nu8 opcode =')
        with self.assertRaises(ff.Rejected): ff.feasibility(d)

    def test_helper_after_process_does_not_alias_process_anchor(self):
        d = dict(FIXTURE); d['processor'] += '\nconst u8 cmd = reader.read<u8>();'
        self.assertFalse(ff.feasibility(d)['synchronization_qualified'])

    def test_grow_order_negative(self):
        d = dict(FIXTURE); d['fifo'] = d['fifo'].replace('\nvoid publish() noexcept {', '')
        d['fifo'] = '\nvoid publish() noexcept {\n' + d['fifo']
        with self.assertRaises(ff.Rejected): ff.feasibility(d)

    def test_output_existing_negative(self):
        with tempfile.TemporaryDirectory() as td, patch.object(ff, 'OWNED_OUTPUT_ROOT', Path(td).resolve()):
            p = Path(td).resolve() / 'existing'; p.mkdir()
            with self.assertRaises(ff.Rejected): ff.confined_output(p, [])

    def test_output_outside_owned_negative(self):
        with tempfile.TemporaryDirectory() as td, patch.object(ff, 'OWNED_OUTPUT_ROOT', Path(td).resolve() / 'owned'):
            with self.assertRaises(ff.Rejected): ff.confined_output(Path(td).resolve() / 'elsewhere', [])

    def test_output_symlink_negative(self):
        with tempfile.TemporaryDirectory() as td, patch.object(ff, 'OWNED_OUTPUT_ROOT', Path(td).resolve()):
            p = Path(td).resolve() / 'link'; p.symlink_to(Path(td).resolve() / 'target')
            with self.assertRaises(ff.Rejected): ff.confined_output(p, [])

    def test_output_containing_source_negative(self):
        with tempfile.TemporaryDirectory() as td, patch.object(ff, 'OWNED_OUTPUT_ROOT', Path(td).resolve()):
            with self.assertRaises(ff.Rejected): ff.confined_output(Path(td).resolve() / 'out', [Path(td).resolve() / 'out/input'])

    def make_sources(self, root):
        paths = {}
        for role, text in FIXTURE.items():
            p = root / (role + '.cpp'); p.write_text(text); paths[role] = p
        profile = {'hashes': {r: ff.sha(p.read_bytes()) for r, p in paths.items()}}
        return paths, profile

    def test_full_synthetic_preflight_blocked_and_inputs_unchanged(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td).resolve(); paths, profile = self.make_sources(root)
            args = types.SimpleNamespace(**paths, output_root=str(root / 'out'))
            before = {r: p.read_bytes() for r, p in paths.items()}
            with patch.object(ff, 'validate_profile', return_value=profile), patch.object(ff, 'OWNED_OUTPUT_ROOT', root), contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(ff.run(args), 2)
            report = json.loads((root / 'out/preflight.json').read_text())
            self.assertFalse(report['patch_emitted']); self.assertFalse(report['capture_ready'])
            self.assertFalse(report['production_acceptance'])
            self.assertEqual([p.name for p in (root / 'out').iterdir()], ['preflight.json'])
            self.assertEqual(before, {r: p.read_bytes() for r, p in paths.items()})

    def test_effective_hash_change_negative_no_output(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td).resolve(); paths, profile = self.make_sources(root)
            paths['fifo'].write_text('changed')
            args = types.SimpleNamespace(**paths, output_root=str(root / 'out'))
            with patch.object(ff, 'validate_profile', return_value=profile), patch.object(ff, 'OWNED_OUTPUT_ROOT', root):
                with self.assertRaisesRegex(ff.Rejected, 'effective source hash mismatch'): ff.run(args)
            self.assertFalse((root / 'out').exists())

    def test_source_role_alias_negative(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td).resolve(); paths, profile = self.make_sources(root); paths['processor'] = paths['fifo']
            args = types.SimpleNamespace(**paths, output_root=str(root / 'out'))
            with patch.object(ff, 'validate_profile', return_value=profile):
                with self.assertRaisesRegex(ff.Rejected, 'source roles alias'): ff.run(args)


if __name__ == '__main__': unittest.main()
