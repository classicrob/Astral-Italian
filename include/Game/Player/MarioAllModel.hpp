#pragma once

#include <revolution.h>

class J3DModel;
class LiveActor;
class MarioActor;

/// @brief Holds Mario's per-mode models (normal, fire, yoshi, cloud, rock, ...). Layout not recovered.
class MarioAllModel {
public:
    void registerModel(s32, LiveActor*, bool, bool);
    J3DModel* getJ3DModel() const;

    // SMG2-only (original names unknown; named by address)
    void fn_803E7880(s32);
    void fn_803E7B80();
    void fn_803E7BB0(MarioActor*);
};
