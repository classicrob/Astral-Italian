#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Animation/BckCtrl.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/AudioLib/AudAnmSoundObject.hpp"
#include "Game/LiveActor/ActorAnimKeeper.hpp"
#include "Game/LiveActor/ActorActionKeeper.hpp"
#include "Game/LiveActor/ActorSoundHolder.hpp"
#include "Game/LiveActor/ActorLightCtrl.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/ClippingActorHolder.hpp"
#include "Game/LiveActor/ClippingDirector.hpp"
#include "Game/LiveActor/DisplayListMaker.hpp"
#include "Game/LiveActor/EffectKeeper.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/LiveActorGroupArray.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/MirrorActor.hpp"
#include "Game/LiveActor/MirrorCamera.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Map/Flag.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Map/LightFunction.hpp"
#include "Game/NameObj/NameObjExecuteHolder.hpp"
#include "Game/NameObj/NameObjFinder.hpp"
#include "Game/Player/GroupChecker.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/CollisionPartsFilter.hpp"
#include "Game/Util/FurMulti.hpp"
#include <cstdio>

namespace {
    void changeBckForEffectKeeper(const LiveActor* pActor) NO_INLINE {
        EffectKeeper* keeper = pActor->mEffectKeeper;

        if (keeper != nullptr) {
            keeper->changeBck();
        }
    }
    void resetSoundSyncBck(const LiveActor* pActor) NO_INLINE {
        if (pActor->mSoundHolder) pActor->mSoundHolder->resetSyncBck();
    }
};  // namespace

namespace MR {

    bool isExistAnim(const LiveActor* pActor, const char* pName) {
        if (isExistBck(pActor, pName)) {
            return true;
        }

        if (isExistBtk(pActor, pName)) {
            return true;
        }

        if (isExistBrk(pActor, pName)) {
            return true;
        }

        if (isExistBtp(pActor, pName)) {
            return true;
        }

        if (isExistBpk(pActor, pName)) {
            return true;
        }

        return isExistBva(pActor, pName);
    }

    bool isExistBck(const LiveActor* pActor, const char* pName) {
        if (pName == nullptr) {
            return getResourceHolder(pActor)->mMotionResTable->mCount != 0;
        }

        ModelManager* manager = pActor->mModelManager;
        if (manager) {
            XanimePlayer* player = manager->mXanimePlayer;
            if (player && player->isExistAnimation(pName)) return true;
        }
        return false;
    }

    bool isExistBtk(const LiveActor* pActor, const char* pName) {
        if (pName == nullptr) {
            return getResourceHolder(pActor)->mBtkResTable->mCount != 0;
        }

        return getResourceHolder(pActor)->mBtkResTable->isExistRes(pName);
    }

    bool isExistBrk(const LiveActor* pActor, const char* pName) {
        if (pName == nullptr) {
            return getResourceHolder(pActor)->mBrkResTable->mCount != 0;
        }

        return getResourceHolder(pActor)->mBrkResTable->isExistRes(pName);
    }

    bool isExistBtp(const LiveActor* pActor, const char* pName) {
        if (pName == nullptr) {
            return getResourceHolder(pActor)->mBtpResTable->mCount != 0;
        }

        return getResourceHolder(pActor)->mBtpResTable->isExistRes(pName);
    }

    bool isExistBpk(const LiveActor* pActor, const char* pName) {
        if (pName == nullptr) {
            return getResourceHolder(pActor)->mBpkResTable->mCount != 0;
        }

        return getResourceHolder(pActor)->mBpkResTable->isExistRes(pName);
    }

    bool isExistBva(const LiveActor* pActor, const char* pName) {
        if (pName == nullptr) {
            return getResourceHolder(pActor)->mBvaResTable->mCount != 0;
        }

        return getResourceHolder(pActor)->mBvaResTable->isExistRes(pName);
    }


    bool startAnimIfExistAll(const LiveActor* pActor, const char* pName) {
        bool isStarted = false;

        if (startBckIfExist(pActor, pName)) {
            isStarted = true;
        }

        if (startBtkIfExist(pActor, pName)) {
            isStarted = true;
        }

        if (startBrkIfExist(pActor, pName)) {
            isStarted = true;
        }

        if (startBpkIfExist(pActor, pName)) {
            isStarted = true;
        }

        if (startBtpIfExist(pActor, pName)) {
            isStarted = true;
        }

        if (startBvaIfExist(pActor, pName)) {
            isStarted = true;
        }

        return isStarted;
    }

    void startAction(const LiveActor* pActor, const char* pName) {
        if (pActor->mSoundHolder) {
            pActor->mSoundHolder->startAction(pName);
            pActor->mSoundHolder->resetSyncBck();
        }
        if (!pActor->mActionKeeper || !pActor->mActionKeeper->startAction(pName)) {
            startAnimIfExistAll(pActor, pName);
        }
    }

    void startActionAtRandomFrame(LiveActor* pActor, const char* pName) {
        startAction(pActor, pName);
        f32 frame = 0.0f;
        if (isBckPlayingIfExist(pActor, pName)) {
            s16 end = pActor->mModelManager->getBckCtrl()->getEnd();
            frame = static_cast<s32>(end * getRandom());
        }
        else if (isBtkPlayingIfExist(pActor, pName)) {
            s16 end = pActor->mModelManager->getBtkCtrl()->getEnd();
            frame = static_cast<s32>(end * getRandom());
        }
        else if (isBrkPlayingIfExist(pActor, pName)) {
            s16 end = pActor->mModelManager->getBrkCtrl()->getEnd();
            frame = static_cast<s32>(end * getRandom());
        }
        else if (isBtpPlayingIfExist(pActor, pName)) {
            s16 end = pActor->mModelManager->getBtpCtrl()->getEnd();
            frame = static_cast<s32>(end * getRandom());
        }
        else if (isBpkPlayingIfExist(pActor, pName)) {
            s16 end = pActor->mModelManager->getBpkCtrl()->getEnd();
            frame = static_cast<s32>(end * getRandom());
        }
        else if (isBvaPlayingIfExist(pActor, pName)) {
            s16 end = pActor->mModelManager->getBvaCtrl()->getEnd();
            frame = static_cast<s32>(end * getRandom());
        }
        if (pActor->mActionKeeper && pActor->mActionKeeper->mAnimKeeper &&
            pActor->mActionKeeper->mAnimKeeper->trySetAllFrame(frame)) return;
        if (isBckPlayingIfExist(pActor, pName)) setBckFrame(pActor, frame);
        if (isBtkPlayingIfExist(pActor, pName)) pActor->mModelManager->getBtkCtrl()->setFrame(frame);
        if (isBrkPlayingIfExist(pActor, pName)) pActor->mModelManager->getBrkCtrl()->setFrame(frame);
        if (isBtpPlayingIfExist(pActor, pName)) pActor->mModelManager->getBtpCtrl()->setFrame(frame);
        if (isBpkPlayingIfExist(pActor, pName)) pActor->mModelManager->getBpkCtrl()->setFrame(frame);
        if (isBvaPlayingIfExist(pActor, pName)) pActor->mModelManager->getBvaCtrl()->setFrame(frame);
    }

    bool tryStartActionNotPlaying(const LiveActor* pActor, const char* pName) {
        if (!isAnyAnimPlaying(pActor, pName)) {
            startAction(pActor, pName);
            return true;
        }
        return false;
    }

    bool tryStartAllAnim(const LiveActor* pActor, const char* pName) {
        if (!isExistAnimInKeeper(pActor, pName)) return false;
        startAction(pActor, pName);
        return true;
    }

    bool isActionEnd(const LiveActor* pActor) {
        if (isExistBck(pActor, nullptr)) return pActor->mModelManager->isBckStopped();
        if (isExistBtk(pActor, nullptr)) return pActor->mModelManager->isBtkStopped();
        if (isExistBrk(pActor, nullptr)) return pActor->mModelManager->isBrkStopped();
        if (isExistBpk(pActor, nullptr)) return pActor->mModelManager->isBpkStopped();
        if (isExistBtp(pActor, nullptr)) return pActor->mModelManager->isBtpStopped();
        if (isExistBva(pActor, nullptr)) return pActor->mModelManager->isBvaStopped();
        return false;
    }

    void startBck(const LiveActor* pActor, const char* pBckName, const char* pBrkName) {
        pActor->mModelManager->startBck(pBckName, pBrkName);
        ::changeBckForEffectKeeper(pActor);
        ::resetSoundSyncBck(pActor);
        startBas(pActor, pBckName, false, 0.0f, 0.0f);
    }

    void startBckWithInterpole(const LiveActor* pActor, const char* pBckName, s32 interpole) {
        pActor->mModelManager->startBckWithInterpole(pBckName, interpole);
        startBas(pActor, pBckName, false, 0.0f, 0.0f);
        ::changeBckForEffectKeeper(pActor);
        ::resetSoundSyncBck(pActor);
    }

    void startBckNoInterpole(const LiveActor* pActor, const char* pBckName) {
        pActor->mModelManager->startBckWithInterpole(pBckName, 0);
        startBas(pActor, pBckName, false, 0.0f, 0.0f);
        ::changeBckForEffectKeeper(pActor);
        ::resetSoundSyncBck(pActor);
    }

    void startBckAtFirstStep(const LiveActor* pActor, const char* pBckName) {
        if (!isFirstStep(pActor)) {
            return;
        }

        pActor->mModelManager->startBck(pBckName, nullptr);
        ::changeBckForEffectKeeper(pActor);
        ::resetSoundSyncBck(pActor);
        startBas(pActor, pBckName, false, 0.0f, 0.0f);
    }

    bool tryStartBck(const LiveActor* pActor, const char* pBckName, const char* pBrkName) {
        if (!isBckPlaying(pActor->mModelManager->mXanimePlayer, pBckName)) {
            pActor->mModelManager->startBck(pBckName, pBrkName);
            ::changeBckForEffectKeeper(pActor);
        ::resetSoundSyncBck(pActor);
            startBas(pActor, pBckName, false, 0.0f, 0.0f);
            return true;
        }

        return false;
    }

    bool tryStartBckAndBtp(const LiveActor* pActor, const char* pBckName, const char* pBtpName) {
        if (tryStartBck(pActor, pBckName, pBtpName)) {
            if (isExistBtp(pActor, pBckName)) {
                pActor->mModelManager->startBtp(pBckName);
            }

            return true;
        }

        return false;
    }

    void setAllAnimFrame(const LiveActor* pActor, const char* pName, f32 frame) {
        if (isExistBck(pActor, pName)) {
            setBckFrame(pActor, frame);
        }

        if (isExistBtk(pActor, pName)) {
            pActor->mModelManager->getBtkCtrl()->setFrame(frame);
        }

        if (isExistBpk(pActor, pName)) {
            pActor->mModelManager->getBpkCtrl()->setFrame(frame);
        }

        if (isExistBtp(pActor, pName)) {
            pActor->mModelManager->getBtpCtrl()->setFrame(frame);
        }

        if (isExistBrk(pActor, pName)) {
            pActor->mModelManager->getBrkCtrl()->setFrame(frame);
        }

        if (isExistBva(pActor, pName)) {
            pActor->mModelManager->getBvaCtrl()->setFrame(frame);
        }
    }

    void setAllAnimFrameAndStop(const LiveActor* pActor, const char* pName, f32 frame) {
        if (isExistBck(pActor, pName)) {
            setBckFrameAndStop(pActor, frame);
        }

        if (isExistBtk(pActor, pName)) {
            setBtkFrameAndStop(pActor, frame);
        }

        if (isExistBpk(pActor, pName)) {
            setBpkFrameAndStop(pActor, frame);
        }

        if (isExistBtp(pActor, pName)) {
            setBtpFrameAndStop(pActor, frame);
        }

        if (isExistBrk(pActor, pName)) {
            setBrkFrameAndStop(pActor, frame);
        }

        if (isExistBva(pActor, pName)) {
            setBvaFrameAndStop(pActor, frame);
        }
    }

    void setAllAnimFrameAtEnd(const LiveActor* pActor, const char* pName) {
        if (isExistBck(pActor, pName)) {
            setBckFrame(pActor, pActor->mModelManager->getBckCtrl()->getEnd());
        }

        if (isExistBtk(pActor, pName)) {
            setBtkFrame(pActor, pActor->mModelManager->getBtkCtrl()->getEnd());
        }

        if (isExistBpk(pActor, pName)) {
            setBpkFrame(pActor, pActor->mModelManager->getBpkCtrl()->getEnd());
        }

        if (isExistBtp(pActor, pName)) {
            setBtpFrame(pActor, pActor->mModelManager->getBtpCtrl()->getEnd());
        }

        if (isExistBrk(pActor, pName)) {
            setBrkFrame(pActor, pActor->mModelManager->getBrkCtrl()->getEnd());
        }

        if (isExistBva(pActor, pName)) {
            setBvaFrame(pActor, pActor->mModelManager->getBvaCtrl()->getEnd());
        }
    }

    bool isAnyAnimPlaying(const LiveActor* pActor, const char* pName) {
        ActorAnimKeeper* pAnimKeeper;
        if (pActor->mActionKeeper == nullptr || (pAnimKeeper = pActor->mActionKeeper->mAnimKeeper) == nullptr) {
            if (isExistBck(pActor, nullptr)) {
                return isBckPlaying(pActor->mModelManager->mXanimePlayer, pName);
            }

            if (isExistBtk(pActor, nullptr)) {
                return pActor->mModelManager->isBtkPlaying(pName);
            }

            if (isExistBrk(pActor, nullptr)) {
                return pActor->mModelManager->isBrkPlaying(pName);
            }

            if (isExistBpk(pActor, nullptr)) {
                return pActor->mModelManager->isBpkPlaying(pName);
            }

            if (isExistBtp(pActor, nullptr)) {
                return pActor->mModelManager->isBtpPlaying(pName);
            }

            if (isExistBva(pActor, nullptr)) {
                return pActor->mModelManager->isBvaPlaying(pName);
            }

            return false;
        }

        return pAnimKeeper->isAnyAnimPlaying(pName);
    }

    bool isExistAnimInKeeper(const LiveActor* pActor, const char* pName) {
        if (pActor->mActionKeeper == nullptr || pActor->mActionKeeper->mAnimKeeper == nullptr) {
            return isExistAnim(pActor, pName);
        }
        return pActor->mActionKeeper->mAnimKeeper->isExistAnim(pName);
    }

    bool isAnyAnimStopped(const LiveActor* pActor, const char* pName) {
        if (isExistBck(pActor, pName) && isBckStopped(pActor)) {
            return true;
        }

        if (isExistBtk(pActor, pName) && isBtkStopped(pActor)) {
            return true;
        }

        if (isExistBpk(pActor, pName) && isBpkStopped(pActor)) {
            return true;
        }

        if (isExistBtp(pActor, pName) && isBtpStopped(pActor)) {
            return true;
        }

        if (isExistBrk(pActor, pName) && isBrkStopped(pActor)) {
            return true;
        }

        if (isExistBva(pActor, pName) && isBvaStopped(pActor)) {
            return true;
        }

        return false;
    }

    bool isAnyAnimOneTimeAndStopped(const LiveActor* pActor, const char* pName) {
        if (isExistBck(pActor, pName) && isBckStopped(pActor)) {
            return true;
        }

        if (isExistBtk(pActor, pName) && isBtkStopped(pActor)) {
            return true;
        }

        if (isExistBpk(pActor, pName) && isBpkStopped(pActor)) {
            return true;
        }

        if (isExistBtp(pActor, pName) && isBtpStopped(pActor)) {
            return true;
        }

        if (isExistBrk(pActor, pName) && isBrkStopped(pActor)) {
            return true;
        }

        if (isExistBva(pActor, pName) && isBvaStopped(pActor)) {
            return true;
        }

        return false;
    }

    bool isBckStopped(const LiveActor* pActor) {
        return pActor->mModelManager->isBckStopped();
    }

    bool isBtkStopped(const LiveActor* pActor) {
        return pActor->mModelManager->isBtkStopped();
    }

    bool isBrkStopped(const LiveActor* pActor) {
        return pActor->mModelManager->isBrkStopped();
    }

    bool isBtpStopped(const LiveActor* pActor) {
        return pActor->mModelManager->isBtpStopped();
    }

    bool isBpkStopped(const LiveActor* pActor) {
        return pActor->mModelManager->isBpkStopped();
    }

    bool isBvaStopped(const LiveActor* pActor) {
        return pActor->mModelManager->isBvaStopped();
    }

    bool isBckOneTimeAndStopped(const LiveActor* pActor) {
        return pActor->mModelManager->isBckStopped();
    }

    bool isBtkOneTimeAndStopped(const LiveActor* pActor) {
        return pActor->mModelManager->isBtkStopped();
    }

    bool isBrkOneTimeAndStopped(const LiveActor* pActor) {
        return pActor->mModelManager->isBrkStopped();
    }

    bool isBckLooped(const LiveActor* pActor) {
        return (pActor->mModelManager->getBckCtrl()->mState & 0x2) != 0;
    }

    bool checkPassBckFrame(const LiveActor* pActor, f32 f) {
        return pActor->mModelManager->getBckCtrl()->checkPass(f) == 1;
    }

    void stopAllAnim(LiveActor* pActor) {
        if (pActor->mModelManager != nullptr) {
            if (getPlayingBckName(pActor) != nullptr) {
                pActor->mModelManager->getBckCtrl()->setRate(0.0f);
            }

            if (isExistBva(pActor, nullptr)) {
                pActor->mModelManager->stopBva();
            }

            if (isExistBpk(pActor, nullptr)) {
                pActor->mModelManager->stopBpk();
            }

            if (isExistBtp(pActor, nullptr)) {
                pActor->mModelManager->stopBtp();
            }

            if (isExistBrk(pActor, nullptr)) {
                pActor->mModelManager->stopBrk();
            }

            if (isExistBtk(pActor, nullptr)) {
                pActor->mModelManager->stopBtk();
            }
        }
    }

    void resumeAllAnim(LiveActor* pActor) {
        if (pActor->mModelManager) {
            if (getPlayingBckName(pActor)) pActor->mModelManager->getBckCtrl()->setRate(1.0f);
            if (isExistBva(pActor, nullptr)) pActor->mModelManager->resumeBva();
            if (isExistBpk(pActor, nullptr)) pActor->mModelManager->resumeBpk();
            if (isExistBtp(pActor, nullptr)) pActor->mModelManager->resumeBtp();
            if (isExistBrk(pActor, nullptr)) pActor->mModelManager->resumeBrk();
            if (isExistBtk(pActor, nullptr)) pActor->mModelManager->resumeBtk();
        }
    }

    void setBckFrameAtRandom(const LiveActor* pActor) {
        s16 end = pActor->mModelManager->getBckCtrl()->getEnd();
        s32 randomFrame = end * MR::getRandom();

        setBckFrame(pActor, randomFrame);
    }

    void setBtkFrameAtRandom(const LiveActor* pActor) {
        s32 randomFrame = pActor->mModelManager->getBtkCtrl()->getEnd() * MR::getRandom();

        pActor->mModelManager->getBtkCtrl()->mFrame = randomFrame;
    }

    void setBckFrameAndStop(const LiveActor* pActor, f32 frame) {
        setBckFrame(pActor, frame);
        pActor->mModelManager->getBckCtrl()->setRate(0.0f);
    }

    void setBtkFrameAndStop(const LiveActor* pActor, f32 frame) {
        pActor->mModelManager->getBtkCtrl()->setFrame(frame);
        pActor->mModelManager->getBtkCtrl()->setRate(0.0f);
    }

    void setBrkFrameAndStop(const LiveActor* pActor, f32 frame) {
        pActor->mModelManager->getBrkCtrl()->setFrame(frame);
        pActor->mModelManager->getBrkCtrl()->setRate(0.0f);
    }

    void setBtpFrameAndStop(const LiveActor* pActor, f32 frame) {
        pActor->mModelManager->getBtpCtrl()->setFrame(frame);
        pActor->mModelManager->getBtpCtrl()->setRate(0.0f);
    }

    void setBpkFrameAndStop(const LiveActor* pActor, f32 frame) {
        pActor->mModelManager->getBpkCtrl()->setFrame(frame);
        pActor->mModelManager->getBpkCtrl()->setRate(0.0f);
    }

    void setBvaFrameAndStop(const LiveActor* pActor, f32 frame) {
        pActor->mModelManager->getBvaCtrl()->setFrame(frame);
        pActor->mModelManager->getBvaCtrl()->setRate(0.0f);
    }

    void startBckAndSetFrameAndStop(const LiveActor* pActor, const char* pBckName, f32 frame, const char* pInterpole) {
        startBck(pActor, pBckName, pInterpole);
        setBckFrame(pActor, frame);
        pActor->mModelManager->getBckCtrl()->setRate(0.0f);
    }

    void setBckFrameEndAndStop(const LiveActor* pActor) {
        setBckFrame(pActor, pActor->mModelManager->getBckCtrl()->getEnd());
        pActor->mModelManager->getBckCtrl()->setRate(0.0f);
    }

    void setBrkFrameEndAndStop(const LiveActor* pActor) {
        pActor->mModelManager->getBrkCtrl()->setFrame(pActor->mModelManager->getBrkCtrl()->getEnd());
        pActor->mModelManager->getBrkCtrl()->setRate(0.0f);
    }

    void startBtkAndSetFrameAndStop(const LiveActor* pActor, const char* pBtkName, f32 frame) {
        startBtk(pActor, pBtkName);
        setBtkFrameAndStop(pActor, frame);
    }

    void startBrkAndSetFrameAndStop(const LiveActor* pActor, const char* pBrkName, f32 frame) {
        startBrk(pActor, pBrkName);
        setBrkFrameAndStop(pActor, frame);
    }

    void startBtpAndSetFrameAndStop(const LiveActor* pActor, const char* pBtpName, f32 frame) {
        startBtp(pActor, pBtpName);
        setBtpFrameAndStop(pActor, frame);
    }

    void startBvaAndSetFrameAndStop(const LiveActor* pActor, const char* pBvaName, f32 frame) {
        startBva(pActor, pBvaName);
        setBvaFrameAndStop(pActor, frame);
    }

    void startBtk(const LiveActor* pActor, const char* pBtkName) {
        pActor->mModelManager->startBtk(pBtkName);
    }

    void startBrk(const LiveActor* pActor, const char* pBrkName) {
        pActor->mModelManager->startBrk(pBrkName);
    }

    void startBtp(const LiveActor* pActor, const char* pBtpName) {
        pActor->mModelManager->startBtp(pBtpName);
    }

    void startBpk(const LiveActor* pActor, const char* pBpkName) {
        pActor->mModelManager->startBpk(pBpkName);
    }

    void startBva(const LiveActor* pActor, const char* pBvaName) {
        pActor->mModelManager->startBva(pBvaName);
    }

    bool startBckIfExist(const LiveActor* pActor, const char* pBckName) {
        if (pActor->mModelManager && pActor->mModelManager->mXanimePlayer && pActor->mModelManager->mXanimePlayer->isExistAnimation(pBckName)) {
            pActor->mModelManager->startBck(pBckName, nullptr);
            ::changeBckForEffectKeeper(pActor);
        ::resetSoundSyncBck(pActor);
            startBas(pActor, pBckName, false, 0.0f, 0.0f);
            return true;
        }

        return false;
    }

    bool startBtkIfExist(const LiveActor* pActor, const char* pBtkName) {
        if (getResourceHolder(pActor)->mBtkResTable->isExistRes(pBtkName)) {
            pActor->mModelManager->startBtk(pBtkName);
            return true;
        }

        return false;
    }

    bool startBrkIfExist(const LiveActor* pActor, const char* pBrkName) {
        if (getResourceHolder(pActor)->mBrkResTable->isExistRes(pBrkName)) {
            pActor->mModelManager->startBrk(pBrkName);
            return true;
        }

        return false;
    }

    bool startBtpIfExist(const LiveActor* pActor, const char* pBtpName) {
        if (getResourceHolder(pActor)->mBtpResTable->isExistRes(pBtpName)) {
            pActor->mModelManager->startBtp(pBtpName);
            return true;
        }

        return false;
    }

    bool startBpkIfExist(const LiveActor* pActor, const char* pBpkName) {
        if (getResourceHolder(pActor)->mBpkResTable->isExistRes(pBpkName)) {
            pActor->mModelManager->startBpk(pBpkName);
            return true;
        }

        return false;
    }

    bool startBvaIfExist(const LiveActor* pActor, const char* pBvaName) {
        if (getResourceHolder(pActor)->mBvaResTable->isExistRes(pBvaName)) {
            pActor->mModelManager->startBva(pBvaName);
            return true;
        }

        return false;
    }

    bool isBtkPlaying(const LiveActor* pActor, const char* pBtkName) {
        return pActor->mModelManager->isBtkPlaying(pBtkName);
    }

    bool isBrkPlaying(const LiveActor* pActor, const char* pBrkName) {
        return pActor->mModelManager->isBrkPlaying(pBrkName);
    }

    bool isBtpPlaying(const LiveActor* pActor, const char* pBtpName) {
        return pActor->mModelManager->isBtpPlaying(pBtpName);
    }

    bool isBpkPlaying(const LiveActor* pActor, const char* pBpkName) {
        return pActor->mModelManager->isBpkPlaying(pBpkName);
    }

    bool isBvaPlaying(const LiveActor* pActor, const char* pBvaName) {
        return pActor->mModelManager->isBvaPlaying(pBvaName);
    }

    void stopBck(const LiveActor* pActor) {
        pActor->mModelManager->getBckCtrl()->setRate(0.0f);
    }

    void stopBtk(const LiveActor* pActor) {
        pActor->mModelManager->stopBtk();
    }

    void stopBrk(const LiveActor* pActor) {
        pActor->mModelManager->stopBrk();
    }

    void stopBtp(const LiveActor* pActor) {
        pActor->mModelManager->stopBtp();
    }

    void stopBva(const LiveActor* pActor) {
        pActor->mModelManager->stopBva();
    }

    void setBckRate(const LiveActor* pActor, f32 rate) {
        J3DFrameCtrl* pCtrl = pActor->mModelManager->getBckCtrl();
        f32 scale = 1.0f;
        pCtrl->setRate(rate * scale);
    }

    void setBtkRate(const LiveActor* pActor, f32 rate) {
        J3DFrameCtrl* pCtrl = pActor->mModelManager->getBtkCtrl();
        f32 scale = 1.0f;
        pCtrl->setRate(rate * scale);
    }

    void setBrkRate(const LiveActor* pActor, f32 rate) {
        J3DFrameCtrl* pCtrl = pActor->mModelManager->getBrkCtrl();
        f32 scale = 1.0f;
        pCtrl->setRate(rate * scale);
    }

    void setBtpRate(const LiveActor* pActor, f32 rate) {
        J3DFrameCtrl* pCtrl = pActor->mModelManager->getBtpCtrl();
        f32 scale = 1.0f;
        pCtrl->setRate(rate * scale);
    }

    void setBpkRate(const LiveActor* pActor, f32 rate) {
        J3DFrameCtrl* pCtrl = pActor->mModelManager->getBpkCtrl();
        f32 scale = 1.0f;
        pCtrl->setRate(rate * scale);
    }

    void setBvaRate(const LiveActor* pActor, f32 rate) {
        J3DFrameCtrl* pCtrl = pActor->mModelManager->getBvaCtrl();
        f32 scale = 1.0f;
        pCtrl->setRate(rate * scale);
    }

    void setBckFrame(const LiveActor* pActor, f32 frame) {
        pActor->mModelManager->getBckCtrl()->setFrame(frame);
        XanimePlayer* player = pActor->mModelManager->mXanimePlayer;
        player->mCachedFrame = player->mFrameCtrl->mFrame;
        if (pActor->mEffectKeeper) pActor->mEffectKeeper->resetSyncBckFrame();
        if (pActor->mSoundHolder) pActor->mSoundHolder->resetSyncBckFrame();
    }

    void setBtkFrame(const LiveActor* pActor, f32 frame) {
        pActor->mModelManager->getBtkCtrl()->setFrame(frame);
    }

    void setBrkFrame(const LiveActor* pActor, f32 frame) {
        pActor->mModelManager->getBrkCtrl()->setFrame(frame);
    }

    void setBtpFrame(const LiveActor* pActor, f32 frame) {
        pActor->mModelManager->getBtpCtrl()->setFrame(frame);
    }

    void setBpkFrame(const LiveActor* pActor, f32 frame) {
        pActor->mModelManager->getBpkCtrl()->setFrame(frame);
    }

    void setBvaFrame(const LiveActor* pActor, f32 frame) {
        pActor->mModelManager->getBvaCtrl()->setFrame(frame);
    }

    bool isBckPlaying(const LiveActor* pActor, const char* pBckName) {
        return MR::isBckPlaying(pActor->mModelManager->mXanimePlayer, pBckName);
    }

    bool isBckPlayingIfExist(const LiveActor* pActor, const char* pBckName) {
        if (isExistBck(pActor, pBckName) && MR::isBckPlaying(pActor->mModelManager->mXanimePlayer, pBckName)) {
            return true;
        }

        return false;
    }

    bool isBtkPlayingIfExist(const LiveActor* pActor, const char* pBtkName) {
        if (isExistBtk(pActor, pBtkName) && pActor->mModelManager->isBtkPlaying(pBtkName)) {
            return true;
        }

        return false;
    }

    bool isBrkPlayingIfExist(const LiveActor* pActor, const char* pBrkName) {
        if (isExistBrk(pActor, pBrkName) && pActor->mModelManager->isBrkPlaying(pBrkName)) {
            return true;
        }

        return false;
    }

    bool isBtpPlayingIfExist(const LiveActor* pActor, const char* pBtpName) {
        if (isExistBtp(pActor, pBtpName) && pActor->mModelManager->isBtpPlaying(pBtpName)) {
            return true;
        }

        return false;
    }

    bool isBpkPlayingIfExist(const LiveActor* pActor, const char* pBpkName) {
        if (isExistBpk(pActor, pBpkName) && pActor->mModelManager->isBpkPlaying(pBpkName)) {
            return true;
        }

        return false;
    }

    bool isBvaPlayingIfExist(const LiveActor* pActor, const char* pBvaName) {
        if (isExistBva(pActor, pBvaName) && pActor->mModelManager->isBvaPlaying(pBvaName)) {
            return true;
        }

        return false;
    }

    bool isPlayingAnim(const LiveActor* pActor, const char* pName) {
        if (isBckPlayingIfExist(pActor, pName)) {
            return true;
        }

        if (isBtkPlayingIfExist(pActor, pName)) {
            return true;
        }

        if (isBrkPlayingIfExist(pActor, pName)) {
            return true;
        }

        if (isBtpPlayingIfExist(pActor, pName)) {
            return true;
        }

        if (isBpkPlayingIfExist(pActor, pName)) {
            return true;
        }

        return isBvaPlayingIfExist(pActor, pName);
    }

    void changeTrackWeights(const LiveActor* pActor, const f32* pWeights) {
        XanimePlayer* pPlayer = pActor->mModelManager->mXanimePlayer;
        for (u32 i = 0; i < 4; i++) {
            pPlayer->changeTrackWeight(i, pWeights[i]);
        }
    }

    void setTrackWeights(const LiveActor* pActor, f32 w0, f32 w1, f32 w2) {
        f32 weights[4] = {};
        weights[0] = w0;
        weights[1] = w1;
        weights[2] = w2;
        changeTrackWeights(pActor, weights);
    }

    void copyBckBlendWeights(f32* pDst, u32 count, const LiveActor* pActor) {
        for (u32 i = 0; i < count; i++) {
            pDst[i] = pActor->mModelManager->mXanimePlayer->mWeights[i];
        }
    }

    bool setBckTrackWeight(const LiveActor* pActor, u32 index, f32 weight) {
        return pActor->mModelManager->mXanimePlayer->changeTrackWeight(index, weight);
    }

    J3DFrameCtrl* getBckCtrl(const LiveActor* pActor) {
        return pActor->mModelManager->getBckCtrl();
    }

    J3DFrameCtrl* getBtkCtrl(const LiveActor* pActor) {
        return pActor->mModelManager->getBtkCtrl();
    }

    J3DFrameCtrl* getBrkCtrl(const LiveActor* pActor) {
        return pActor->mModelManager->getBrkCtrl();
    }

    J3DFrameCtrl* getBtpCtrl(const LiveActor* pActor) {
        return pActor->mModelManager->getBtpCtrl();
    }

    J3DFrameCtrl* getBpkCtrl(const LiveActor* pActor) {
        return pActor->mModelManager->getBpkCtrl();
    }

    J3DFrameCtrl* getBvaCtrl(const LiveActor* pActor) {
        return pActor->mModelManager->getBvaCtrl();
    }

    void updateMaterial(LiveActor* pActor) {
        pActor->mModelManager->updateDL(true);
    }

};  // namespace MR
