#!/usr/bin/env python3
"""Compose default-off boundary hooks after the existing Aurora patches.
Pure source transformation; does not compile, initialize graphics or run apps.
"""
from pathlib import Path
import sys


def patch(text, kind):
    if "#include <petari/gpu_trace.hpp>" in text:
        raise ValueError("GPU trace source already patched")
    def replace(old, new, count=1):
        nonlocal text
        actual = text.count(old)
        if actual != count:
            raise ValueError(f'{kind}: expected {count}, got {actual}: {old!r}')
        text = text.replace(old, new)
    text = '#include <petari/gpu_trace.hpp>\n' + text
    if kind == 'frame':
        replace('  frame.frameIndex = g_frameIndex;', '  frame.frameIndex = g_frameIndex;\n'
                '  PETARI_GPU_TRACE(PetariNative::GpuTrace::Context c{}; c.packet = frame.frameId;\n'
                '      c.detail = frame.frameIndex; PetariNative::GpuTrace::emit("packet_open", c));')
        replace('  const uint64_t frameId = frame.frameId;', '  const uint64_t frameId = frame.frameId;\n'
                '  PETARI_GPU_TRACE(PetariNative::GpuTrace::Context c{}; c.packet = frameId;\n'
                '      PetariNative::GpuTrace::emit("packet_close", c));')
    elif kind == 'worker':
        # The existing queue already carries immutable frameId/type/passIndex.
        # Apply to worker, recursive worker-inline and pre-initialize inline paths.
        def begin(item):
            return ('PETARI_GPU_TRACE(PetariNative::GpuTrace::Context c{}; c.packet = '+item+'.frameId;\n'
                    '      c.detail = static_cast<unsigned>('+item+'.type); c.subject = '+item+'.passIndex;\n'
                    '      PetariNative::GpuTrace::beginWork(c));\n')
        replace('      item->work();', '      '+begin('(*item)')+'      item->work();\n'
                '      PETARI_GPU_TRACE(PetariNative::GpuTrace::endWork());')
        replace('      item.work();', '      '+begin('item')+'      item.work();\n'
                '      PETARI_GPU_TRACE(PetariNative::GpuTrace::endWork());', 2)
        replace('void enqueue(QueueItem item) {', 'void enqueue(QueueItem item) {\n'
                '  PETARI_GPU_TRACE(PetariNative::GpuTrace::Context c{}; c.packet = item.frameId;\n'
                '      c.detail = static_cast<unsigned>(item.type); c.subject = item.passIndex;\n'
                '      PetariNative::GpuTrace::emit("worker_enqueue", c));')
    elif kind == 'aurora':
        text = '#include "gpu_trace_dawn.hpp"\n' + text
        for call, name in [('g_surface.GetCurrentTexture(&surfaceTexture);', 'drawable_acquire'),
                           ('status = g_surface.Present();', 'present')]:
            replace(call, 'PETARI_GPU_TRACE(PetariNative::GpuTrace::emit("'+name+'_begin", PetariNative::GpuTrace::workerContext));\n'
                    '          '+call+'\n'
                    '          PETARI_GPU_TRACE(PetariNative::GpuTrace::emit("'+name+'_end", PetariNative::GpuTrace::workerContext));')
        replace('      g_queue.Submit(1, &buffer);',
                '      PETARI_GPU_TRACE(PetariNative::GpuTrace::workerContext.submission = PetariNative::GpuTrace::nextId();\n'
                '          PetariNative::GpuTrace::emit("submit_begin", PetariNative::GpuTrace::workerContext));\n'
                '      g_queue.Submit(1, &buffer);\n'
                '      PETARI_GPU_TRACE(PetariNative::GpuTrace::emit("submit_return", PetariNative::GpuTrace::workerContext);\n'
                '          PetariNative::GpuTrace::observeSubmission(g_queue, PetariNative::GpuTrace::workerContext));')
        replace('    const auto buffer = encoder.Finish(&cmdBufDescriptor);',
                '    PETARI_GPU_TRACE(PetariNative::GpuTrace::emit("encoder_finish_begin", PetariNative::GpuTrace::workerContext));\n'
                '    const auto buffer = encoder.Finish(&cmdBufDescriptor);\n'
                '    PETARI_GPU_TRACE(PetariNative::GpuTrace::emit("encoder_finish_end", PetariNative::GpuTrace::workerContext));')
    else:
        raise ValueError(f'unknown trace patch kind: {kind}')
    return text


if __name__ == '__main__':
    source, output, kind = sys.argv[1:]
    result = patch(Path(source).read_text(), kind)
    destination = Path(output)
    destination.parent.mkdir(parents=True, exist_ok=True)
    if not destination.exists() or destination.read_text() != result:
        destination.write_text(result)
