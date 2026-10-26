#!/usr/bin/env python3
"""Independent default-off wait history over FINAL generated Aurora sources."""
import sys
from pathlib import Path
N='PetariNative::GameWaitHistory'
def patch(text,kind):
    if "#include <petari/game_wait_history.hpp>" in text:raise ValueError("already instrumented")
    def replace(old,new,count=1):
        if text.count(old)!=count:raise ValueError(f'{kind}: expected {count} exact {old!r}, got {text.count(old)}')
        return text.replace(old,new)
    def observed(call,name,expression=False,obj='0',value='0'):
        nonlocal text
        body=('return '+call) if expression else call
        # call includes its semicolon; original code executes exactly once.
        new=f'{N}::invoke({N}::Kind::{name}, {obj}, {value}, [&] {{ {body} }});'
        text=replace(call,new)
    if kind=='aurora':
        observed('g_surface.GetCurrentTexture(&surfaceTexture);','Drawable')
        observed('status = g_surface.Present();','Present')
        observed('g_queue.Submit(1, &buffer);','Submit')
    elif kind=='fifo':
        observed('lock.lock();\n      work();','Fifo')
        # Include try-lock/spin/host release AND later CPU reacquisition as one
        # outer span, while the inner span isolates the original mutex/work.
        anchor='void petari_with_buffer_lock(Work&& work) {'
        text=replace(anchor,anchor+'\n  return '+N+'::invoke('+N+'::Kind::Fifo, 0, 1, [&] {')
        text=replace('  work();\n}', '  work();\n  });\n}')
    elif kind=='worker':
        text=replace('  std::unique_lock lock{m_mutex};', '  std::unique_lock lock{m_mutex, std::defer_lock};\n  '+N+'::invoke('+N+'::Kind::RenderQueue, 0, 1, [&] { lock.lock(); });',3)
        observed('m_notFull.wait(lock, [&] { return m_closed || m_items.size() < m_capacity; });','RenderQueue')
        observed('sync->cv.wait(lock, [&] { return sync->complete; });','RenderSync')
        # Mutex acquisition preceding a wait is separately bounded.
        text=replace('  std::unique_lock lock{sync->mutex};','  std::unique_lock lock{sync->mutex, std::defer_lock};\n  '+N+'::invoke('+N+'::Kind::RenderSync, 0, 1, [&] { lock.lock(); });')
        text=replace('  m_cv.wait(lock, [&] {','  '+N+'::invoke('+N+'::Kind::FrameSlot, 0, 0, [&] { m_cv.wait(lock, [&] {')
        text=replace('    return false;\n  });\n  for (size_t i = 0; i < m_freeSlots.size(); ++i) {',
                     '    return false;\n  }); });\n  for (size_t i = 0; i < m_freeSlots.size(); ++i) {')
    elif kind=='frame':
        old='const size_t frameSlot = acquire_frame_slot();'
        text=replace(old,'const size_t frameSlot = '+N+'::invoke('+N+'::Kind::FrameSlot, 0, 0, [&] { return acquire_frame_slot(); });')
        old='const auto stagingSlot = acquire_mapped_staging_buffer();'
        text=replace(old,'const auto stagingSlot = '+N+'::invoke('+N+'::Kind::Staging, frameSlot, 0, [&] { return acquire_mapped_staging_buffer(); });')
        text=replace('  frame.frameId = g_nextFrameId++;', '  frame.frameId = g_nextFrameId++;\n  PETARI_GAME_WAIT_EVENT('+N+'::Kind::Worker, frame.frameId, frameSlot, 1);')
        # Readback appends efb_frame.inc before geometry/ubershader generation.
        # Instrument its segment hooks here too: this final pass exclusively
        # owns frame instrumentation, and the top-level double-patch guard stays
        # effective over the complete composed translation unit.
        old='uint64_t ticket, PetariNative::GX::SnapshotReady ready) {'
        text=replace(old,old+'\n  PETARI_GAME_WAIT_EVENT('+N+'::Kind::Capture,frame.frameId,ticket,0);')
        text=replace('    return acquire_mapped_staging_buffer();', '    return '+N+'::invoke('+N+'::Kind::Staging, 0, 1, [&] { return acquire_mapped_staging_buffer(); });')
        old='      auto mapReadback = PetariNative::GX::encodeSnapshot(frame.encoder, source, ticket, ready);'
        text=replace(old,'      PETARI_GAME_WAIT_EVENT('+N+'::Kind::Capture,frame.frameId,ticket,1);\n'+old)
        observed('g_queue.Submit(1, &commands);','Submit',obj='frame.frameId',value='ticket')
        observed('completion.get();','RenderSync',obj='frame.frameId',value='ticket')
    else:raise ValueError('unknown kind '+kind)
    return '#include <petari/game_wait_history.hpp>\n'+text
if __name__=='__main__':
    source,out,kind=sys.argv[1:]
    result=patch(Path(source).read_text(),kind)
    dest=Path(out);dest.parent.mkdir(parents=True,exist_ok=True)
    if not dest.exists() or dest.read_text()!=result:dest.write_text(result)
