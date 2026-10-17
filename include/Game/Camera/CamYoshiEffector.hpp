#pragma once

#include <revolution/types.h>

class CameraMan;

// SMG2-only camera effector (name unverified: target symbols fn_8010E220 ctor / fn_8010E230 update, empty 1-byte class)
class CamYoshiEffector {
public:
    CamYoshiEffector();

    void update(CameraMan*);
};
