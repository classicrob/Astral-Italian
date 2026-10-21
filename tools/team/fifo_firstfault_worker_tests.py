"""Lightweight generator/negative checks; NO C++ compile or runtime claims."""
from pathlib import Path
import re
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.dont_write_bytecode=True
sys.path.insert(0,str(Path(__file__).resolve().parent))
import fifo_firstfault_worker_generate as g

FIXTURE='''ProcessResult process(const u8* data, u32 size) noexcept {
  ByteReader reader{{data, size}};
    const u8 cmd = reader.read<u8>();
      handle_cp(addr, reader.read<u32>());
      Log.warn("Ignoring nested GX_CMD_CALL_DL");
      AURORA_ASSERT(array.data != nullptr, "indexed XF load from unmapped array {}", arrayType);
}
[[noreturn]] static void handle_draw_overrun(size_t totalVtxBytes, const ByteReader& reader) noexcept {
}
static void draw_prim(GXPrimitive prim, GXVtxFmt fmt, u16 vtxCount, ByteReader& reader) noexcept {
  ZoneScoped;
  u32 totalVtxBytes = vtxCount * vtxSize;
    reader.skip(totalVtxBytes);
}
  } else if (subCmd == GX_AURORA_LOAD_TEXOBJ) {
}
'''

class WorkerTests(unittest.TestCase):
    def template(self):return Path(g.__file__).with_name('fifo_firstfault_worker_capture.hpp.in').read_text()
    def test_template_pin(self):self.assertEqual(g.sha(self.template().encode()),g.TEMPLATE_SHA)
    def test_original_bytes_preserved(self):self.assertEqual(g.strip_hooks(g.transform(FIXTURE,Path('/owned/capture.hpp'))),FIXTURE)
    def test_exact_reader_operations_preserved(self):
        s=g.transform(FIXTURE,Path('/owned/capture.hpp'))
        for op in ['reader.read<u8>()','reader.read<u32>()','reader.skip(totalVtxBytes)']:
            self.assertEqual(s.count(op),FIXTURE.count(op))
    def test_each_missing_anchor_rejects(self):
        for anchor in ['  ByteReader reader{{data, size}};\n','    const u8 cmd = reader.read<u8>();\n','      handle_cp(addr, reader.read<u32>());\n','      Log.warn("Ignoring nested GX_CMD_CALL_DL");','      AURORA_ASSERT(array.data != nullptr, "indexed XF load from unmapped array {}", arrayType);','  u32 totalVtxBytes = vtxCount * vtxSize;\n','    reader.skip(totalVtxBytes);\n','  } else if (subCmd == GX_AURORA_LOAD_TEXOBJ) {']:
            with self.subTest(anchor=anchor),self.assertRaises(ValueError):g.transform(FIXTURE.replace(anchor,''),Path('/owned/a.hpp'))
    def test_duplicate_inside_process_rejects(self):
        s=FIXTURE.replace('    const u8 cmd = reader.read<u8>();','    const u8 cmd = reader.read<u8>();\n    const u8 cmd = reader.read<u8>();')
        with self.assertRaises(ValueError):g.transform(s,Path('/owned/a.hpp'))
    def test_duplicate_helper_outside_process_is_not_alias(self):
        s=FIXTURE+'\n    const u8 cmd = reader.read<u8>();\n';self.assertEqual(g.strip_hooks(g.transform(s,Path('/owned/a.hpp'))),s)
    def test_instrumented_input_rejects(self):
        with self.assertRaises(ValueError):g.transform(g.transform(FIXTURE,Path('/owned/a.hpp')),Path('/owned/b.hpp'))
    def test_no_stdio_formatter_or_cpp_allocator(self):
        s=self.template()
        for word in ['std::fprintf','snprintf','std::string','fmt::','backtrace','new ','malloc(','fsync(']:self.assertNotIn(word,s)
    def test_format_vocabulary_is_complete(self):
        supported={'%s','%llu','%zu','%u','%x','%02x','%llx'}
        for literal in re.findall(r'append\("([^"\n]*)"',self.template()):
            self.assertTrue(set(re.findall(r'%[a-z0-9]+',literal))<=supported,literal)
    def test_default_off(self):
        s=self.template();self.assertIn('if (!flag || std::strcmp(flag,"1")!=0) return false;',s)
    def test_private_exclusive_and_bounded_io(self):
        s=self.template()
        for required in ['O_EXCL','O_NOFOLLOW','::fchmod(state.fd,0600)','(st.st_mode&0777)==0700','attempt<15','MaxOutput = 512 * 1024']:
            self.assertIn(required,s)
    def test_borrowed_pointer_cleared(self):self.assertIn('state.data=nullptr;state.size=0;state.active=false;',self.template())
    def test_primitive_eviction_is_explicit(self):
        s=self.template();self.assertIn('if(state.primitives>=state.primitive.size())state.limits|=RingLoss;',s)
        self.assertIn('totals commands=%llu bindings=%llu primitives=%llu capacities=64,32,2',s)
    def test_bounded_extra_reads(self):
        s=self.template()
        for required in ['pos>state.size','end>=4 && end<=state.size','fmt>=gx.vtxFmts.size()','b.attr<gx.arrays.size()','end-begin>65536']:
            self.assertIn(required,s)
        self.assertNotIn('srcData',s)
    def test_first_trigger_latches_before_flush(self):
        s=self.template();self.assertIn('state.warning=true;flush(',s);self.assertIn('state.fault=true;',s)
    def test_fresh_output_outside_root_rejects(self):
        with tempfile.TemporaryDirectory() as td:
            r=Path(td).resolve()
            with self.assertRaises(ValueError):g.fresh(r/'other',r/'owned')
    def test_existing_output_rejects(self):
        with tempfile.TemporaryDirectory() as td:
            r=Path(td).resolve();d=r/'existing';d.mkdir()
            with self.assertRaises(ValueError):g.fresh(d,r)
    def test_symlink_output_rejects(self):
        with tempfile.TemporaryDirectory() as td:
            r=Path(td).resolve();d=r/'link';d.symlink_to(r/'elsewhere')
            with self.assertRaises(ValueError):g.fresh(d,r)

if __name__=='__main__':unittest.main()
