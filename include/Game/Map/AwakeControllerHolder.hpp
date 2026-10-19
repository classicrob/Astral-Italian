#pragma once

#include "Game/NameObj/NameObj.hpp"
#include "Game/Util/Array.hpp"

class AwakeController;
class JMapInfoIter;
class SwitchEventListener;

class AwakeControllerHolder : public NameObj {
public:
    /// @brief Creates a new `AwakeControllerHolder`.
    AwakeControllerHolder();
    virtual ~AwakeControllerHolder();

    virtual void movement();

    void add(NameObj*, const JMapInfoIter&, SwitchEventListener*);
    void initSync();

private:
    /* 0x14 */ MR::Vector< MR::FixedArray< AwakeController*, 1024 > > mAwakeControl;
};
