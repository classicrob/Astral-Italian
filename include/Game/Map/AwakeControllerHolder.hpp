#pragma once

#include "Game/NameObj/NameObj.hpp"
#include "Game/Util/Array.hpp"

class AreaObj;
class AwakeController;
class LiveActor;
class JMapInfoIter;
class SwitchEventListener;

class AwakeControllerHolder : public NameObj {
public:
    /// @brief Creates a new `AwakeControllerHolder`.
    AwakeControllerHolder();

    virtual void movement();

    void add(NameObj*, const JMapInfoIter&, SwitchEventListener*);
    void initSync();

private:
    /* 0x14 */ MR::Vector< MR::FixedArray< AwakeController*, 1024 > > mAwakeControl;
};

namespace AwakeControlFunc {
    void initSyncSleepController();
    void addAwakeControl(AreaObj*, const JMapInfoIter&);
    void addAwakeControl(LiveActor*, const JMapInfoIter&);
};  // namespace AwakeControlFunc
