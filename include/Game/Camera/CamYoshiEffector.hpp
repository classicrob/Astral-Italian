#pragma once

#include <revolution/types.h>

class CameraMan;

// Adjusts camera distance to preserve framing while Yoshi dashes.
class CamYoshiEffector {
public:
    CamYoshiEffector();

    void update(CameraMan*);
};
