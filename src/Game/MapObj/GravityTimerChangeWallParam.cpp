#include <revolution/types.h>

// Real source storage for the original wall timing parameters.
// Target 807DD6F0 is f32 8.0 followed by four alignment bytes.
extern const f32 lbl_807DD6F0 = 8.0f;
// Target 807E3098..807E30A0 is two separate read-only .sbss2 zeros.
extern const f32 lbl_807E3098 = 0.0f;
extern const s32 lbl_807E309C = 0;
