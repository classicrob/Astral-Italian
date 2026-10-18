#pragma once

#include <revolution.h>

class HitSensor;

class SensorGroup {
public:
    SensorGroup(int, const char*);

    void add(HitSensor*);
    void remove(HitSensor*);
    void clear() const;

    /* 0x00 */ s32 mMaxSensors;
    /* 0x04 */ s32 mSensorCount;
    /* 0x08 */ HitSensor** mSensors;
};
