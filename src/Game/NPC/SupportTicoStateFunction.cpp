#pragma auto_inline off


#include "Game/NPC/SupportTico.hpp"
#include "Game/Boss/DinoPackun.hpp"
#include "Game/Enemy/EnemyUtilAll.hpp"
#include "Game/Scene/GameScene.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

#include "Game/LiveActor/ActorPoseTQ.hpp"
#include "Game/MapObj/SpinCloudBlock.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/StarPointerTarget.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"





namespace SupportTicoStateFunction {
    bool isEnable() {
        if (!MR::isExistSceneObj(0x62)) {
            return false;
        }
        return MR::getSceneObj<SupportTico>(0x62)->mControlOn;
    }

    f32 getFollowScale() {
        return MR::max(200.0f, 100.0f + MR::getSpinCloudBlockSpinSpeed());
    }

    bool isSwing() {
        if (isEnable()) {
            return MR::isPadSwing(1);
        }
        return false;
    }

    bool isReleaseA() {
        if (isEnable()) {
            return !isPressA();
        }
        return false;
    }

    bool isPressA() {
        if (isEnable()) {
            return MR::testCorePadButtonA(1);
        }
        return false;
    }

    bool isTriggerA() {
        if (isEnable()) {
            return MR::testCorePadTriggerA(1);
        }
        return false;
    }

    void startSupportTicoCSSound() {
        if (!isEnable()) {
            return;
        }
        if (MR::testCorePadTrigger1(1)) {
            switch (MR::getRandom(static_cast< s32 >(0), static_cast< s32 >(3))) {
            case 0:
            case 1:
                MR::stopAndStartCSSound2P("CS_PERC_TICO2", 0x32, nullptr, 0, 1.0f);
                break;
            case 2:
            case 3:
                MR::stopAndStartCSSound2P("CS_PERC_TICO1", 0x32, nullptr, 0, 1.0f);
                break;
            }
        }
        if (MR::testCorePadTrigger2(1)) {
            switch (MR::getRandom(static_cast< s32 >(0), static_cast< s32 >(3))) {
            case 0:
            case 1:
                MR::stopAndStartCSSound2P("CS_PERC_COIN", 5, nullptr, 0, 1.0f);
                break;
            case 2:
            case 3:
                MR::stopAndStartCSSound2P("CS_PERC_1UP", 0x1F4, nullptr, 0, 1.0f);
                break;
            }
        }
        if (MR::testCorePadTriggerMinus(1)) {
            MR::stopAndStartCSSound2P("CS_PERC_AUD_BAD", 0x96, nullptr, 0, 1.0f);
        }
        if (MR::testCorePadTriggerPlus(1)) {
            MR::stopAndStartCSSound2P("CS_PERC_AUD_GOOD", 0x96, nullptr, 0, 1.0f);
        }
        if (MR::testCorePadTriggerUp(1)) {
            switch (MR::getRandom(static_cast< s32 >(0), static_cast< s32 >(3))) {
            case 0:
            case 1:
                MR::stopAndStartCSSound1P("CS_PERC_INST2", 0xC8, nullptr, 0, 1.0f);
                break;
            case 2:
            case 3:
                MR::stopAndStartCSSound1P("CS_PERC_INST1", 0xC8, nullptr, 0, 1.0f);
                break;
            }
        }
        if (MR::testCorePadTriggerDown(1)) {
            switch (MR::getRandom(static_cast< s32 >(0), static_cast< s32 >(3))) {
            case 0:
            case 1:
                MR::stopAndStartCSSound1P("CS_PERC_COIN", 5, nullptr, 0, 1.0f);
                break;
            case 2:
            case 3:
                MR::stopAndStartCSSound1P("CS_PERC_1UP", 0x1F4, nullptr, 0, 1.0f);
                break;
            }
        }
        if (MR::testCorePadTriggerLeft(1)) {
            MR::stopAndStartCSSound1P("CS_PERC_AUD_BAD", 0x96, nullptr, 0, 1.0f);
        }
        if (MR::testCorePadTriggerRight(1)) {
            MR::stopAndStartCSSound1P("CS_PERC_AUD_GOOD", 0x96, nullptr, 0, 1.0f);
        }
    }

    bool isHideDemo() {
        if (!MR::isSupportTicoShow()) {
            return true;
        }
        if (MR::isDemoActive()) {
            if (MR::isDemoActive("\203}\203\212\203I\225\317\220g") || MR::isDemoActive("\212G\226{\217u\212\324\210\332\223\256\203f\203\202") ||
                MR::isDemoActive("\212G\226{\203y\201[\203W\202\337\202\255\202\350")) {
                return false;
            }
            return true;
        }
        if (MR::isPowerStarGetDemoActive() || MR::isExecScenarioStarter() || MR::isStageStateScenarioOpeningCamera() || MR::isStageFileSelect()) {
            return true;
        }
        return false;
    }

    bool isHolding(const SupportTico* pHost) {
        return pHost->mHoldSensor != nullptr;
    }

    void hideHoldingObj(SupportTico* pHost) {
        if (pHost->mHoldSensor != nullptr) {
            MR::hideModel(MR::getSensorHost(pHost->mHoldSensor));
            MR::sendSimpleMsgToActor(0x89, MR::getSensorHost(pHost->mHoldSensor));
        }
        if (pHost->mReleasedSensor != nullptr) {
            MR::hideModel(MR::getSensorHost(pHost->mReleasedSensor));
            MR::sendSimpleMsgToActor(0x89, MR::getSensorHost(pHost->mReleasedSensor));
        }
    }

    void showHoldingObj(SupportTico* pHost) {
        if (pHost->mHoldSensor != nullptr) {
            MR::showModel(MR::getSensorHost(pHost->mHoldSensor));
            MR::sendSimpleMsgToActor(0x88, MR::getSensorHost(pHost->mHoldSensor));
        }
        if (pHost->mReleasedSensor != nullptr) {
            MR::showModel(MR::getSensorHost(pHost->mReleasedSensor));
            MR::sendSimpleMsgToActor(0x88, MR::getSensorHost(pHost->mReleasedSensor));
        }
    }

    void calcFollowOffset(TVec3f* pOut) {
        MR::calcPlayerFollowPassPosition(pOut, getFollowScale());
    }

    void calcFollowOffset(SupportTico* pHost) {
        MR::calcPlayerFollowPassPosition(&pHost->mPosition, getFollowScale());
    }

    // NONMATCHING: register allocation and instruction ordering.
    bool trySpinAttack(SupportTico* pHost) {
        u32 i;
        bool result;
        HitSensor* pSensor = pHost->getSensor("Body");
        f32 radius = MR::getSensorRadius(pSensor);
        result = false;
        u32 num = Collision::checkStrikeBallToMapWithThickness(*MR::getSensorPos(pSensor), radius, radius, nullptr, nullptr);
        for (i = 0; i < num; i++) {
            if (MR::sendArbitraryMsg(0x13, Collision::getStrikeInfoMap(i)->mParentTriangle.mSensor, pSensor)) {
                result = true;
            }
        }
        return result;
    }
};  // namespace SupportTicoStateFunction
