// Appended to recording.cpp, whose process-wide recorder is the same object
// pipeline_ref() reads. GX work can run on a different native game thread from
// begin_frame(); a thread_local copy of the frame ID is not authoritative here.
namespace aurora::gfx {
uint64_t petari_pipeline_recording_frame_id() noexcept {
  return g_recorder.active() ? g_recorder.frame().frameId : UINT64_MAX;
}
} // namespace aurora::gfx
