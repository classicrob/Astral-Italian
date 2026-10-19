#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void petari_headless_wait_frame(void);
void petari_headless_wait_retrace(void);
int petari_kpad_replay_hook_version(void);
#ifdef __cplusplus
}
#endif
