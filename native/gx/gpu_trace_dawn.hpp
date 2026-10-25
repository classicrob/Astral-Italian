#pragma once
#include <petari/gpu_trace.hpp>
#include <webgpu/webgpu_cpp.h>

namespace PetariNative::GpuTrace {
// Caller already passed the cached flag. Callback registration is trace-on only.
inline void observeSubmission(const wgpu::Queue& queue, Context c) {
    run->pending.fetch_add(1, std::memory_order_relaxed);
    emit("completion_observer_register_begin", c);
    queue.OnSubmittedWorkDone(wgpu::CallbackMode::AllowSpontaneous,
        [c](wgpu::QueueWorkDoneStatus status, wgpu::StringView) {
            PETARI_GPU_TRACE(emit("gpu_queue_done_observed", c,
                status == wgpu::QueueWorkDoneStatus::Success ? 0 : 1));
            run->pending.fetch_sub(1, std::memory_order_relaxed);
        });
    emit("completion_observer_register_end", c);
}
}
