#pragma once
#include "Game/Screen/WipeLayoutBase.hpp"
class WipeMario : public WipeLayoutBase {
public:
    WipeMario();
    virtual ~WipeMario();
    virtual void init(const JMapInfoIter&);
    virtual void wipe(s32);
    virtual void forceClose();
    virtual void forceOpen();
    virtual bool isOpen() const;
    virtual bool isClose() const;
    virtual bool isWipeIn() const;
    virtual bool isWipeOut() const;
    virtual s32 getWipeType() const;
    void exeWipeOut();
    void exeWipeIn();
    s32 mWipeFrames;
};
