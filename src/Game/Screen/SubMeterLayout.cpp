#include "Game/Screen/SubMeterLayout.hpp"
#include "Game/Util.hpp"

namespace {
    static f32 sScreenOffsetX = 60.0f;
    static f32 sScreenOffsetY = -70.0f;
};  // namespace

SubMeterLayout::SubMeterLayout(const char* pName, const char* pLayoutName) : LayoutActor(pName, true) {
    MR::connectToSceneLayout(this);
    initLayoutManager(pLayoutName, 1);
    MR::createAndAddPaneCtrl(this, "Count", 1);
}

void SubMeterLayout::init(const JMapInfoIter& rIter) {
    MR::startPaneAnim(this, "Count", "Count", 0);
    MR::setPaneAnimFrameAndStop(this, "Count", 0.0f, 0);
    appear();
    MR::hideLayout(this);
    mState = State_Hidden;
    mFullLifeFrames = 0;
}

void SubMeterLayout::setLifeRatio(f32 ratio) {
    f32 mult = 128.0f * ratio;
    mLife = mult;
    MR::setPaneAnimFrameAndStop(this, "Count", 0x80 - mLife, 0);
}

void SubMeterLayout::requestDeactivate() {
    MR::hideLayout(this);
    mState = State_Hidden;
}

void SubMeterLayout::requestAppear() {
    MR::showLayout(this);
    MR::startAnim(this, "Appear", nullptr);
    mState = State_Appear;
    control();
}

void SubMeterLayout::requestFrameIn() {
    MR::showLayout(this);
    MR::startAnim(this, "Appear", 0);
    mState = State_Appear;
    control();
}

void SubMeterLayout::requestFrameOut() {
    MR::startAnim(this, "End", 0);
    mState = State_FrameOut;
}

void SubMeterLayout::control() {
    switch (mState) {
    case State_Hidden:
        break;
    case State_Appear:
        if (MR::isAnimStopped(this, 0)) {
            MR::startAnim(this, "Wait", 0);
            mFullLifeFrames = 0;
            mState += 1;
        }
        break;
    case State_Wait:
        if (mLife < 0x40) {
            MR::startAnim(this, "Danger", 0);
            mState = State_Danger;
        }
        break;
    case State_Danger:
        if (mLife > 0x40) {
            MR::startAnim(this, "Wait", 0);
            MR::setAnimRate(this, 1.0f, 0);
            mState = State_Wait;
        } else {
            MR::setAnimRate(this, ((0x40 - mLife) * 1.0f / 32.0f) + 1.0f, 0);
        }
        break;
    case State_FrameOut:
        if (MR::isAnimStopped(this, 0)) {
            MR::hideLayout(this);
            mState = State_Hidden;
        }
        break;
    }

    bool flag = true;

    if (MR::isPlayerInWaterMode()) {
        if (!MR::isPlayerOnWaterSurface()) {
            flag = false;
        }
    } else if (!MR::isOnGroundPlayer()) {
        flag = false;
    }

    if (flag) {
        if (((mState + 0xFE) & 0xFF) <= 1u && mLife == 0x80) {
            if (++mFullLifeFrames == 60) {
                MR::startAnim(this, "End", 0);
                mState = State_AutoHide;
            }
        }
    } else if (mState == State_AutoHide) {
        mState = State_Appear;
        mFullLifeFrames = 0;
        MR::startAnim(this, "Appear", 0);
    }

    if (!MR::isHiddenLayout(this)) {
        TVec2f screenPos;
        MR::calcScreenPosition(&screenPos, *MR::getPlayerPos());
        screenPos.x += ::sScreenOffsetX;
        screenPos.y += ::sScreenOffsetY;
        setTrans(screenPos);
    }
}
