#include "Game/MapObj/DummyDisplayModel.hpp"
#include "Game/MapObj/SuperSpinDriver.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/MapObj/CoinRotater.hpp"
#include "Game/MapObj/PowerStar.hpp"
#include "Game/NameObj/NameObjArchiveListCollector.hpp"
#include "Game/Util.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JMath/JMath.hpp>

#include "Game/Util/StringUtil.hpp"

namespace {
    const DummyDisplayModelInfo cDummyDisplayModelInfoTable[15] = {
        {"Coin", nullptr, {0.0f, 70.0f, 0.0f}, 16, nullptr, false},
        {"Kinopio", nullptr, {0.0f, 50.0f, 0.0f}, 17, "Freeze", true},
        {"SpinDriver", nullptr, {0.0f, 0.0f, 0.0f}, 12, nullptr, false},
        {"SuperSpinDriver", nullptr, {0.0f, 0.0f, 0.0f}, 12, "Freeze", false},
        {"StarPieceDummy", nullptr, {-30.0f, 100.0f, -30.0f}, 14, "Freeze", false},
        {"Tico", "Dummy", {0.0f, 50.0f, 0.0f}, 17, nullptr, true},
        {"KeySwitch", nullptr, {0.0f, 0.0f, 0.0f}, 11, "InRotation", false},
        {"PowerStar", "Demo", {0.0f, 0.0f, 0.0f}, -1, nullptr, false},
        {"KinokoOneUp", "Dummy", {0.0f, 40.0f, 0.0f}, 14, nullptr, false},
        {"Kuribo", "Dummy", {0.0f, 80.0f, 0.0f}, 19, nullptr, false},
        {"BlueChip", nullptr, {0.0f, 0.0f, 0.0f}, 12, nullptr, false},
        {"YellowChip", nullptr, {0.0f, 0.0f, 0.0f}, 12, nullptr, false},
        {"StrayTico", nullptr, {0.0f, 50.0f, 0.0f}, 17, nullptr, false},
        {"GrandStar", nullptr, {0.0f, 0.0f, 0.0f}, 14, nullptr, false},
        {"KinokoLifeUp", "Dummy", {0.0f, 40.0f, 0.0f}, 14, nullptr, false},
    };

    const DummyDisplayModelInfo cDummyDisplayModelInfoCrystalPowerStar = {"PowerStar", "Dummy", {0.0f, 0.0f, 0.0f}, -1, nullptr, false};

    s32 getItemType(const JMapInfoIter& rIter) {
        s32 type = 0;

        if (MR::isValidInfo(rIter)) {
            MR::getJMapInfoArg6NoInit(rIter, &type);
        }

        return type;
    }

    const DummyDisplayModelInfo* getInfo(int idx) {
        if (idx < 0) {
            idx = 0;
        }

        return &cDummyDisplayModelInfoTable[idx];
    }

};  // namespace

namespace MR {
DummyDisplayModel* tryCreateDummyModel(LiveActor* pHost, const JMapInfoIter& rIter, s32 a3, int itemIdx) {
    s32 modelId = MR::getDummyDisplayModelId(rIter, a3);

    if (modelId == -1) {
        return nullptr;
    }

    if (modelId == 0 && MR::isGalaxyDarkCometAppearInCurrentStage()) {
        return nullptr;
    }

    const DummyDisplayModelInfo* pInfo = &::cDummyDisplayModelInfoTable[modelId];

    if (modelId == 7 && itemIdx == 34) {
        pInfo = &::cDummyDisplayModelInfoCrystalPowerStar;
    }

    DummyDisplayModel* mdl = new DummyDisplayModel(pHost, pInfo, itemIdx < 0 ? pInfo->_14 : itemIdx, modelId, getItemType(rIter));
    mdl->initWithoutIter();
    return mdl;
}
};  // namespace MR

DummyDisplayModel::DummyDisplayModel(LiveActor* pHost, const DummyDisplayModelInfo* pInfo, int a3, s32 itemType, s32 a5)
    : PartsModel(pHost, pInfo->mName, pInfo->mSubName == nullptr ? pInfo->mName : nullptr, nullptr, a3, 0) {
    mModelInfo = pInfo;
    mItemType = itemType;
    _A4 = a5;
    _A8 = 0;
    _AC = a3 == 34;
    if (pInfo->mSubName != nullptr) {
        MR::initActor(this, pInfo->mName, pInfo->mSubName, false);
    }
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;

    switch (mItemType) {
    case ITEM_TYPE_COIN: {
        if (!_AC) {
            MR::createCoinRotater();
        }
    }
    }
}

void DummyDisplayModel::init(const JMapInfoIter& rIter) {
    PartsModel::init(rIter);

    if (mModelInfo->mAnim != nullptr) {
        MR::startAction(this, mModelInfo->mAnim);
    }

    if (mModelInfo->mHasColorChange) {
        MR::startBrk(this, "ColorChange");
        MR::setBrkFrameAndStop(this, _A4);
    }

    if (mItemType != ITEM_TYPE_GRAND_STAR && mItemType != 9 && mItemType != 8 && mItemType != 14 && MR::getLightNumMax(this) > 0) {
        MR::initLightCtrl(this);
    }

    switch (mItemType) {
    case ITEM_TYPE_STARPIECE: {
        if (!_AC) {
            MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
        }

        break;
    }

    case ITEM_TYPE_POWER_STAR: {
        s32 color = PowerStar::setupColor(this, mHost, -1);
        PowerStar::emitColorEffect(this, "Light", color, false);

        if (!_AC) {
            PowerStar::initShadowPowerStar(this, false);
        }

        break;
    }

    case ITEM_TYPE_GRAND_STAR:
        PowerStar::setupColor(this, mHost, -1);
        MR::emitEffect(this, "Light");
        break;

    case ITEM_TYPE_SUPER_SPIN_DRIVER:
        MR::startSuperSpinDriverYellowColorAnim(this);
        break;

    case ITEM_TYPE_KINOPIO:
        _A8 = new LodCtrl(this, rIter);
        _A8->createLodModel(34, -1, -1);
        _A8->syncMaterialAnimation();
        _A8->syncJointAnimation();
        _A8->initLightCtrl();
        _A8->setDistanceToMiddleAndLow(800.0f, 1500.0f);
        break;
    }

    MR::registerDemoSimpleCastAll(this);
}

void DummyDisplayModel::makeActorAppeared() {
    PartsModel::makeActorAppeared();

    if (_A8 != nullptr) {
        _A8->appear();
    }
}

void DummyDisplayModel::makeActorDead() {
    if (_A8 != nullptr) {
        _A8->kill();
    }

    PartsModel::makeActorDead();
}

void DummyDisplayModel::control() {
    switch (mItemType) {
    case ITEM_TYPE_POWER_STAR:
    case ITEM_TYPE_GRAND_STAR:
        PowerStar::requestPointLight(this, mHost, -1);

        if (!_AC) {
            mRotation.y = MR::repeat(mRotation.y + PowerStar::getPowerStarWaitRotateSpeed(), 0.0f, 360.0f);
        }

        break;
    case ITEM_TYPE_KINOPIO:
        _A8->update();
        break;
    }
}

void DummyDisplayModel::calcAndSetBaseMtx() {
    PartsModel::calcAndSetBaseMtx();
    TPos3f* m = (TPos3f*)getBaseMtx();

    TVec3f offset;
    TVec3f v19 = mModelInfo->_8;
    offset.negate(v19);
    PSMTXMultVec((MtxPtr)m, offset, &mPosition);
    m->setTrans(mPosition);

    switch (mItemType) {
    case ITEM_TYPE_COIN:

        if (!_AC) {
            MR::multMtx(*m, MR::getCoinRotateYMatrix(), *m);
        }

        break;
    case ITEM_TYPE_POWER_STAR:
    case ITEM_TYPE_GRAND_STAR:

        if (!_AC) {
            TPos3f rot;
            rot.makeRotate(TVec3f(0.0f, 1.0f, 0.0f), MR::toRadian(mRotation.y));
            MR::multMtx(*m, rot, *m);
        }

        break;
    }
}

namespace MR {
    DummyDisplayModel* createDummyDisplayModel(LiveActor* pHost, const JMapInfoIter& rIter, s32 a3, const TVec3f& a4, const TVec3f& a5) {
        DummyDisplayModel* mdl = MR::tryCreateDummyModel(pHost, rIter, a3, -1);

        if (mdl == nullptr) {
            return nullptr;
        }

        mdl->initFixedPosition(a4, a5, nullptr);
        return mdl;
    }

    DummyDisplayModel* createDummyDisplayModel(LiveActor* pHost, const JMapInfoIter& rIter, const char* pName, s32 a4, const TVec3f& rTrans, const TVec3f& rRot) {
        DummyDisplayModel* mdl = MR::tryCreateDummyModel(pHost, rIter, a4, -1);

        if (mdl == nullptr) {
            return nullptr;
        }

        mdl->initFixedPosition(rTrans, rRot, pName);
        return mdl;
    }

    DummyDisplayModel* createDummyDisplayModel(LiveActor* pHost, s32 itemType, const char* pJointName, const TVec3f& rTrans, const TVec3f& rRot) {
        DummyDisplayModel* mdl = MR::tryCreateDummyModel(pHost, JMapInfoIter(), itemType, -1);
        mdl->initFixedPosition(rTrans, rRot, pJointName);
        return mdl;
    }

    DummyDisplayModel* createDummyDisplayModelCrystalItem(LiveActor* pHost, const JMapInfoIter& rIter, const TVec3f& a4, const TVec3f& a5) {
        DummyDisplayModel* mdl = MR::tryCreateDummyModel(pHost, rIter, -1, 0x22);

        if (mdl == nullptr) {
            return nullptr;
        }

        mdl->initFixedPosition(a4, a5, nullptr);
        return mdl;
    }

    DummyDisplayModel* createDummyDisplayModelCrystalItem(LiveActor* pHost, s32 a2, const TVec3f& a3, const TVec3f& a4) {
        DummyDisplayModel* mdl = MR::tryCreateDummyModel(pHost, JMapInfoIter(), a2, 0x22);
        mdl->initFixedPosition(a3, a4, nullptr);
        return mdl;
    }

    s32 getDummyDisplayModelId(const JMapInfoIter& rIter, s32 a2) {
        s32 arg = -1;

        if (MR::isValidInfo(rIter)) {
            MR::getJMapInfoArg7NoInit(rIter, &arg);
        }

        if (arg == -1 && a2 != -1) {
            arg = a2;
        }

        return arg;
    }

    s32 getDummyDisplayModelId(const LiveActor* pActor) {
        return static_cast< const DummyDisplayModel* >(pActor)->mItemType;
    }

    void makeArchiveListDummyDisplayModel(NameObjArchiveListCollector* pCollector, const JMapInfoIter& rIter) {
        s32 id = MR::getDummyDisplayModelId(rIter, -1);

        if (id >= 0) {
            pCollector->addArchive(::cDummyDisplayModelInfoTable[id].mName);
        }
    }
};  // namespace MR

DummyDisplayModel::~DummyDisplayModel() {
}
