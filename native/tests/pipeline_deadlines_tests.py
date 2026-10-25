#!/usr/bin/env python3
"""PFD1 validation, PSD1/PSD2 compatibility and deterministic round trips."""
from pathlib import Path
import importlib.util
import lzma
import os
import sqlite3
import struct
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'native/tools'))
import pipeline_deadlines as deadlines
import seedpack

ROWS = [(0, 5, 1, 2, b'cc', 0), (1, -5, 13, 3, b'abc', 82), (1, 11, 13, 3, b'def', 83)]
FRAMES = {(-5) & deadlines.MASK: 0, 11: 1}


def database(path, rows):
    with sqlite3.connect(path) as db:
        db.executescript(seedpack.SCHEMA)
        db.execute('INSERT INTO aurora_schema VALUES(1)')
        db.executemany('INSERT INTO pipeline_cache VALUES(?,?,?,?,?,?)', rows)


class Sidecars(unittest.TestCase):
    def setUp(self):
        self.data = deadlines.encode('A', ROWS, FRAMES)

    def test_valid(self):
        self.assertEqual(deadlines.decode(self.data, 'A', ROWS), FRAMES)
        self.assertEqual(deadlines.fingerprint(ROWS), deadlines.fingerprint(list(reversed(ROWS))))

    def test_every_truncation(self):
        for n in range(len(self.data)):
            with self.assertRaises(ValueError): deadlines.decode(self.data[:n], 'A', ROWS)

    def test_trailing_bytes(self):
        with self.assertRaises(ValueError): deadlines.decode(self.data + b'x', 'A', ROWS)

    def test_wrong_version(self):
        for at in (0, 4, 6, 10):
            bad = bytearray(self.data); bad[at] ^= 1
            with self.assertRaises(ValueError): deadlines.decode(bad, 'A', ROWS)

    def test_binding(self):
        with self.assertRaises(ValueError): deadlines.decode(self.data, 'B', ROWS)
        changed = list(ROWS); changed[1] = (*ROWS[1][:-1], 84)
        with self.assertRaises(ValueError): deadlines.decode(self.data, 'A', changed)

    def test_unknown_duplicate_frame(self):
        for replacement in (struct.pack('<QI', 999, 0), struct.pack('<QI', 11, 0), struct.pack('<QI', (-5) & deadlines.MASK, 0xFFFFFFFF)):
            with self.assertRaises(ValueError): deadlines.decode(self.data[:-12] + replacement, 'A', ROWS)

    def test_pack(self):
        with tempfile.TemporaryDirectory() as directory:
            tmp = Path(directory); seeds = tmp / 'seeds'; seeds.mkdir()
            database(seeds / '__global__.db', ROWS); database(seeds / 'A.db', ROWS)
            old = str(tmp / 'v13.xz'); seedpack.pack(str(seeds), old)
            self.assertEqual(lzma.decompress(Path(old).read_bytes())[:4], b'PSD1')
            (seeds / 'A.pfd').write_bytes(self.data)
            new = str(tmp / 'v14.xz'); seedpack.pack(str(seeds), new)
            before = Path(new).read_bytes(); seedpack.pack(str(seeds), new)
            self.assertEqual(before, Path(new).read_bytes())
            self.assertEqual(lzma.decompress(before)[:4], b'PSD2')
            self.assertEqual(seedpack.check(new), [])
            out = tmp / 'expanded'; seedpack.expand(new, str(out))
            self.assertEqual(seedpack.verify(str(seeds), str(out)), 0)
            self.assertEqual(sorted(seedpack.database_rows(str(out / 'A.db'))), sorted(ROWS))
            (out / 'A.pfd').write_bytes(b'wrong')
            self.assertEqual(seedpack.local_state(str(out))[0], 'modified')
            self.assertEqual(seedpack.verify(str(seeds), str(out)), 1)
            with self.assertRaises(ValueError): seedpack.pack(str(out), str(tmp / 'bad.xz'))

    def legacy_expansion(self, tmp):
        seeds = tmp / 'source'; seeds.mkdir()
        database(seeds / '__global__.db', ROWS); database(seeds / 'A.db', ROWS)
        (seeds / 'A.pfd').write_bytes(self.data)
        pack_dir = tmp / 'packs'; pack_dir.mkdir(); pack = pack_dir / 'pipeline-seeds.v14.xz'
        seedpack.pack(str(seeds), str(pack)); out = tmp / 'expanded'
        seedpack.expand(str(pack), str(out))
        legacy = out / ('A' + seedpack.LEGACY_DEADLINE_SUFFIX)
        (out / 'A.pfd').rename(legacy)
        import json
        marker_path = out / seedpack.MARKER; marker = json.loads(marker_path.read_text())
        marker['deadline_sidecars'] = {legacy.name: marker['deadline_sidecars']['A.pfd']}
        marker_path.write_text(json.dumps(marker))
        return pack_dir, out, legacy

    def test_ensure_migrates_unchanged_generated_legacy_expansion(self):
        with tempfile.TemporaryDirectory() as directory:
            packs, out, legacy = self.legacy_expansion(Path(directory))
            self.assertEqual(seedpack.local_state(str(out))[0], 'expansion')
            seedpack.ensure(str(packs), str(out))
            self.assertFalse(legacy.exists()); self.assertEqual((out/'A.pfd').read_bytes(), self.data)
            self.assertEqual(seedpack.local_state(str(out))[0], 'expansion')

    def test_modified_legacy_expansion_is_never_overwritten(self):
        with tempfile.TemporaryDirectory() as directory:
            packs, out, legacy = self.legacy_expansion(Path(directory)); legacy.write_bytes(b'changed')
            self.assertEqual(seedpack.local_state(str(out))[0], 'modified')
            seedpack.ensure(str(packs), str(out))
            self.assertEqual(legacy.read_bytes(), b'changed'); self.assertFalse((out/'A.pfd').exists())
            with self.assertRaisesRegex(ValueError, 'legacy deadline filenames'):
                seedpack.pack(str(out), str(Path(directory)/'bad.xz'))

    def test_identity_data_change(self):
        spec = importlib.util.spec_from_file_location('identity', ROOT / 'native/gx/pipeline_cache_identity.py')
        identity = importlib.util.module_from_spec(spec); spec.loader.exec_module(identity)
        with tempfile.TemporaryDirectory() as directory:
            tmp = Path(directory); root, aurora = tmp / 'root', tmp / 'aurora'
            for name in ['native/gx/CMakeLists.txt', 'native/app/CMakeLists.txt', 'native/cmake/GraphicsPresentation.cmake']:
                p = root / name; p.parent.mkdir(parents=True, exist_ok=True); p.write_bytes(b'code')
            for name in ['lib/gx/pipeline.hpp', 'lib/gfx/clear.hpp', 'lib/gfx/pipeline_cache.cpp', 'lib/webgpu/gpu_cache.cpp']:
                p = aurora / name; p.parent.mkdir(parents=True, exist_ok=True); p.write_bytes(b'schema')
            pack = root / 'native/data/pipeline-seeds.v14.xz'; pack.parent.mkdir(); pack.write_bytes(b'one')
            first = identity.identity(root, aurora); pack.write_bytes(b'two')
            self.assertNotEqual(first, identity.identity(root, aurora))


def cpp(cxx):
    source = r'''
#include "pipeline_deadline_manifest.hpp"
#include <cstdio>
int main(int argc, char** argv) {
  const unsigned char clear[]{'c','c'}, a[]{'a','b','c'}, b[]{'d','e','f'};
  std::vector<PetariDeadlines::Row> rows{{0,5,1,clear,0},{1,UINT64_MAX-4,13,a,82},{1,11,13,b,83}};
  auto good = PetariDeadlines::load(std::filesystem::path(argv[1]) / "valid.pfd", "A", rows);
  if (!good.valid || good.frames.at(11)!=1 || good.frames.at(UINT64_MAX-4)!=0) return 1;
  if (PetariDeadlines::load(std::filesystem::path(argv[1]) / "absent.pfd", "A", rows).valid) return 2;
  if (PetariDeadlines::load(std::filesystem::path(argv[1]) / "valid.pfd", "B", rows).valid) return 3;
  sqlite3* db = nullptr;
  if (sqlite3_open((std::filesystem::path(argv[1]) / "rows.db").string().c_str(), &db) != SQLITE_OK) return 5;
  auto fromSql = PetariDeadlines::loadManifest(db, std::filesystem::path(argv[1]) / "valid.pfd", "A");
  if (!fromSql.valid || fromSql.frames != good.frames) return 6;
  sqlite3_exec(db, "UPDATE pipeline_cache SET first_frame_used=84 WHERE hash=-5", nullptr, nullptr, nullptr);
  if (PetariDeadlines::loadManifest(db, std::filesystem::path(argv[1]) / "valid.pfd", "A").valid) return 7;
  sqlite3_exec(db, "UPDATE pipeline_cache SET config_size=9 WHERE hash=-5", nullptr, nullptr, nullptr);
  if (PetariDeadlines::loadManifest(db, std::filesystem::path(argv[1]) / "valid.pfd", "A").valid) return 8;
  sqlite3_close(db);
  for (unsigned i=0; i<49; ++i) {
    auto result=PetariDeadlines::load(std::filesystem::path(argv[1]) / (std::to_string(i)+".pfd"), "A", rows);
    if (result.valid || !result.frames.empty()) return 4;
  }
  std::puts("Native PFD1 decode, Python fingerprint parity and fail-closed empty fallback PASS");
}
'''
    with tempfile.TemporaryDirectory() as directory:
        tmp = Path(directory); data = deadlines.encode('A', ROWS, FRAMES)
        assert len(data) == 49
        (tmp / 'valid.pfd').write_bytes(data)
        database(tmp / 'rows.db', ROWS)
        for n in range(len(data)): (tmp / f'{n}.pfd').write_bytes(data[:n])
        src, binary = tmp / 'test.cpp', tmp / 'test'; src.write_text(source)
        cmd = [cxx, '-std=c++20', '-O2', '-I', str(ROOT / 'native/gx')]
        if sys.platform == 'darwin':
            cmd += ['-isysroot', subprocess.check_output(['xcrun', '--show-sdk-path'], text=True).strip()]
        subprocess.run(cmd + [str(src), '-lsqlite3', '-o', str(binary)], check=True)
        subprocess.run([str(binary), str(tmp)], check=True)


if __name__ == '__main__':
    if len(sys.argv) == 3 and sys.argv[1] == '--cxx': cpp(sys.argv[2])
    else: unittest.main()
