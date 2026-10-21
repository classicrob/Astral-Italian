#!/usr/bin/env python3
"""SCRATCH ONLY. Transform generated/copied Aurora input, never the shared checkout.
Usage: patch_wm_diag.py SOURCE OUTPUT KIND
Run after the normal native patches. Anchors fail closed if a pinned source changed.
"""
from pathlib import Path
import sys
source, output = map(Path, sys.argv[1:3])
kind = sys.argv[3]
s = source.read_text()
def replace(old, new):
    global s
    if s.count(old) != 1:
        raise SystemExit(f'WM diagnostic anchor mismatch ({kind}): {old!r}, count={s.count(old)}')
    s = s.replace(old, new)
if kind == 'GXDispList.cpp':
    replace('void GXCallDisplayList(const void* data, u32 nbytes) {', '''void GXCallDisplayList(const void* data, u32 nbytes) {
  const auto wmSeq = PetariWmDiag::begin(data, nbytes, aurora::gx::fifo::wm_diag_offset(), aurora::gx::fifo::in_display_list());''')
    replace('  aurora::gx::fifo::write_data(data, nbytes);', '''  if (!aurora::gx::fifo::in_display_list())
    PetariWmDiag::span(wmSeq, aurora::gx::fifo::wm_diag_offset(), nbytes);
  aurora::gx::fifo::write_data(data, nbytes);''')
    s = 'namespace aurora::gx::fifo { uint64_t wm_diag_offset(); }\n' + s
elif kind == 'fifo.cpp':
    if '    const ProcessResult result = process(sPetariMirror.data() + (processed - sPetariMirrorBase),' in s:
        replace('    const ProcessResult result = process(sPetariMirror.data() + (processed - sPetariMirrorBase),', '''    if (PetariWmDiag::enabled()) PetariWmDiag::workerBase = processed;
    const ProcessResult result = process(sPetariMirror.data() + (processed - sPetariMirrorBase),''')
    else:
        replace('      result = process(detail::sBufferData + start, size);', '''      if (PetariWmDiag::enabled()) PetariWmDiag::workerBase = processed;
      result = process(detail::sBufferData + start, size);''')
    replace('uint32_t get_buffer_size() { return detail::sBufferSize; }', '''uint32_t get_buffer_size() { return detail::sBufferSize; }
uint64_t wm_diag_offset() { return sStreamBase + detail::sBufferSize; }''')
elif kind == 'command_processor.cpp':
    replace('    const bool le = reader.read<u8>() == 1;', """    const bool le = reader.read<u8>() == 1;
    PetariWmDiag::baseWrite(attrIdx, reinterpret_cast<const void*>(arrayAddr), arraySize, le);""")
    replace('    const u8 cmd = reader.read<u8>();\n    u8 opcode = cmd & CP_OPCODE_MASK;', '''    PetariWmDiag::command(reader.offset(), reader.data() + reader.offset(), reader.remaining());
    const u8 cmd = reader.read<u8>();
    u8 opcode = cmd & CP_OPCODE_MASK;''')
    replace('      handle_cp(addr, reader.read<u32>());', '''      const u32 wmValue = reader.read<u32>();
      PetariWmDiag::cpWrite(addr, wmValue);
      handle_cp(addr, wmValue);''')
    replace('      Log.warn("Ignoring nested GX_CMD_CALL_DL");', '''      PetariWmDiag::fail("masked-CALL_DL (inspect raw command)");
      Log.warn("Ignoring nested GX_CMD_CALL_DL");''')
    replace('      AURORA_ASSERT(array.data != nullptr, "indexed XF load from unmapped array {}", arrayType);', '''      if (PetariWmDiag::enabled() && (!array.data || srcOffset > array.size || srcSize > array.size - srcOffset)) {
        PetariWmDiag::fail("indexed-XF-range");
        std::fprintf(stderr, "[wm-xf] raw=%02x masked=%02x array=%u index=%u dst=%03x words=%u offset=%u size=%u\\n",
            cmd, opcode, arrayType, srcArrayIdx, dstAddr, len, srcOffset, srcSize);
        for (unsigned wmAttr = GX_POS_MTX_ARRAY; wmAttr <= GX_LIGHT_ARRAY; ++wmAttr) {
          const auto& wmArray = g_gxState.arrays[wmAttr];
          std::fprintf(stderr, "[wm-array] attr=%u pointer=%p size=%u stride=%u le=%d\\n",
              wmAttr, wmArray.data, wmArray.size, wmArray.stride, wmArray.le);
        }
      }
      AURORA_ASSERT(array.data != nullptr, "indexed XF load from unmapped array {}", arrayType);''')
    replace('  u32 totalVtxBytes = vtxCount * vtxSize;', '''  u32 totalVtxBytes = vtxCount * vtxSize;
  if (PetariWmDiag::enabled()) PetariWmDiag::lastDraw = {PetariWmDiag::commandOffset, unsigned(fmt), vtxCount, vtxSize, totalVtxBytes, unsigned(reader.remaining())};''')
    replace('  if (totalVtxBytes > reader.remaining())\n    UNLIKELY { handle_draw_overrun(totalVtxBytes, reader); }', '''  if (totalVtxBytes > reader.remaining())
    UNLIKELY { PetariWmDiag::fail("draw-overrun"); handle_draw_overrun(totalVtxBytes, reader); }''')
    replace('        FATAL("command_processor: unknown opcode 0x{:02X} at pos {}", cmd, reader.offset() - 1);', '''        PetariWmDiag::fail("unknown-opcode");
        FATAL("command_processor: unknown opcode 0x{:02X} at pos {}", cmd, reader.offset() - 1);''')
else:
    raise SystemExit('Unknown source kind: ' + kind)
s = '#include <petari/wm_diag.hpp>\n' + s
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(s)
