#pragma once

#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>

class J3DAnmBase;
class J3DModelData;
class ResTable;
class ResourceHolder;

class AnmPlayerBase {
public:
    AnmPlayerBase(const ResTable*);

    virtual void changeAnimation(J3DAnmBase*);
    virtual void stopAnimation();

    void update();
    void reflectFrame();
    void clearUpdateSkipFlag();
    void start(const char*);
    void stop();
    bool isPlaying(const char*) const;
    // PLACEHOLDER(core): unnamed fn_80069450 (AnmPlayerBase unit): stores the float 1.0 (lbl_807D7290) to frame-ctrl+0xC (resume rate); resolve when named
    void fn_80069450();
    bool isStop() const NO_INLINE {
        return mAnmRes == nullptr || static_cast< s32 >(mFrameCtrl.checkState(1)) == 1 || mFrameCtrl.getRate() == 0.0f;
    }

    /* 0x4 */ J3DAnmBase* mAnmRes;
    /* 0x8 */ const ResTable* mResTable;
    /* 0xC */ J3DFrameCtrl mFrameCtrl;
    // PLACEHOLDER(core): SMG2 AnmPlayerBase flag at 0x20 (0 in ctor, set to 1 by start(), update() skips frame advance when nonzero); semantics unconfirmed, resolve when its other writers are found
    /* 0x20 */ bool _20;
};

class MaterialAnmPlayerBase : public AnmPlayerBase {
public:
    MaterialAnmPlayerBase(const ResTable*, J3DModelData*);

    virtual void attach(J3DAnmBase*, J3DModelData*) = 0;
    virtual void detach(J3DAnmBase*, J3DModelData*) = 0;

    void beginDiff();
    void endDiff();

    /* 0x24 */ J3DModelData* mModelData;
};
