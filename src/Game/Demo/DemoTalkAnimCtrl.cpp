#include "Game/Demo/DemoTalkAnimCtrl.hpp"
#include "Game/Util.hpp"
#include "Game/Demo/DemoExecutor.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/Demo/DemoSubPartKeeper.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/System/ResourceInfo.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <cstdio>

namespace {
    // sBckCtrlData
};  // namespace

DemoTalkAnimCtrl::DemoTalkAnimCtrl(LiveActor* pActor, const char* a3, const char* a4)
    : NerveExecutor(a3), mActor(pActor), _C(a3), _10(a4), _14(nullptr), _18(""), _1C(""), mCameraInfo(nullptr), _24(0), _28(0), _2C(0), _30(0),
      _34(true), _35(false), _36(false), _38(0), _3C(0), _40(0), _44(0), _48(false), mHaveCamera(false), _4A(false), mHaveBtk(false), mHaveBpk(false),
      mHaveBtp(false), mHaveBrk(false), mHaveBva(false) {
}

void DemoTalkAnimCtrl::initForScene(const char* a1, const char* a2, const JMapInfoIter& rIter) {
    char buf[0x100];
    snprintf(buf, sizeof(buf), "%s.canm", a2);
    initForActor(a1);
    _35 = true;
    _34 = _10 == nullptr;
    mHaveCamera = MR::getResourceHolder(mActor)->mFileInfoTable->isExistRes(buf);
    _48 = MR::isExistBck(MR::getPlayerDemoActor(), a1);
    if (mHaveCamera) {
        _1C = a2;
        mCameraInfo = MR::createActorCameraInfo(rIter);
        MR::initAnimCamera(mActor, mCameraInfo, _1C);
    }
}


namespace {
    s32 calcDemoPartStep(const char* pExecutorName, const char* pMainPartName) {
        DemoExecutor* exec = DemoFunction::findDemoExecutor(pExecutorName);

        DemoSubPartInfo* subpart;
        DemoSubPartKeeper* subpartkeeper = exec->mSubPartKeeper;
        for (int i = 0; i < subpartkeeper->mNumSubPartInfos; i++) {
            subpart = &subpartkeeper->mSubPartInfos[i];
            if (MR::isEqualString(pMainPartName, subpart->mMainPartName) && MR::isEqualSubString(subpart->mSubPartName, "会話アニメループ")) {
                return subpart->mMainPartStep;
            }
        }
        return 0;
    }
};  // namespace


void DemoTalkAnimCtrl::initForActor(const char* a1) {
    _35 = false;
    _34 = true;
    mHaveCamera = false;
    _48 = false;
    _18 = a1;
    _14 = nullptr;
    _24 = 0;
    _28 = 0;
    _2C = 0;
    _30 = 0;
    _36 = false;
    mCameraInfo = nullptr;
    _1C = "";
    _4A = MR::isExistBck(mActor, a1);
    mHaveBtk = MR::isExistBtk(mActor, _18);
    mHaveBpk = MR::isExistBpk(mActor, _18);
    mHaveBtp = MR::isExistBtp(mActor, _18);
    mHaveBrk = MR::isExistBrk(mActor, _18);
    mHaveBva = MR::isExistBva(mActor, _18);
}


bool DemoTalkAnimCtrl::updateDemo() {
    if (!_34) {
        return false;
    }

    const char* name = MR::getCurrentDemoPartNameMain(_C);
    if (_14 == nullptr) {
        startAnim();
        _14 = "";
    }

    bool ok = MR::isDemoPartFirstStep(name);
    if (MR::isEqualStringCase(_14, "")) {
        ok = MR::isDemoPartStep(name, 1);
    }

    if (ok) {
        s32 totalsteps = MR::getDemoPartTotalStep(name);
        BckCtrlData bck;
        bck.mLoopMode = 0xFF;
        _36 = MR::isDemoPartTalk(name);
        _14 = name;
        createBckCtrlData(&bck, totalsteps);
        updateAnim(bck);
        if (_48) {
            MR::startBckPlayer(_18, bck);
        }
        _28 = 0;
        _2C = 0;
        _24 += totalsteps;
    }

    if (_35 && _36) {
        updatePause();
    }
    if (mHaveCamera) {
        updateCamera();
    }

    if (DemoFunction::isDemoSuspendOrLastPartLastStep()) {
        if (mHaveCamera) {
            endCamera();
        }
        return false;
    } else {
        if (!DemoFunction::isPauseTimeKeepDemo(_C)) {
            _30 += 1;
            _28 += 1;
        }
        _2C += 1;
        return true;
    }
}






void DemoTalkAnimCtrl::setupStartDemoPart(const char* pPartName) {
    if (_10 != nullptr) {
        _34 = MR::isEqualStringCase(pPartName, _10);
    }
}
