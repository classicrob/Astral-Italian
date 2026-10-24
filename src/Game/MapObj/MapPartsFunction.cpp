#include "Game/Util.hpp"
#include "Game/MapObj/MapPartsFunction.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Spine.hpp"

MapPartsFunction::MapPartsFunction(LiveActor* pHost, const char* pName)
    : NameObj(pName != nullptr ? pName : "マップパーツ機能"), mSpine(), mHost(pHost), mIsActive(true), mParamScale(1.0f), mSensorName("Body") {
}

void MapPartsFunction::init(const JMapInfoIter& rIter) {
    MR::useStageSwitchReadParam(mHost, rIter);
    MR::getJMapInfoParamScale(rIter, &mParamScale);
}

bool MapPartsFunction::sendMsgToHost(u32 msg) {
    HitSensor* pReceiver;
    LiveActor* host;
    if (mSensorName == nullptr) {
        return false;
    }

    if (MR::getSensorNum(mHost) <= 0) {
        return false;
    }

    host = mHost;
    pReceiver = host->getSensor(mSensorName);
    return host->receiveMessage(msg, host->getSensor(mSensorName), pReceiver);
}

void MapPartsFunction::movement() {
    if (mIsActive) {
        mSpine->update();
        control();
    }
}

f32 MapPartsFunction::getSpeed() const {
    if (MR::isValidSwitchParam(mHost) && MR::isOnSwitchParam(mHost)) {
        return mParamScale;
    }

    return 1.0f;
}

void MapPartsFunction::clearSensorName() {
    mSensorName = nullptr;
}

void MapPartsFunction::initNerve(const Nerve* pNerve) {
    mSpine = new Spine(this, pNerve);
}

void MapPartsFunction::setNerve(const Nerve* pNerve) {
    mSpine->setNerve(pNerve);
}

s32 MapPartsFunction::getStep() const {
    return mSpine->mStep;
}

bool MapPartsFunction::isStep(s32 step) const {
    return step == mSpine->mStep;
}

bool MapPartsFunction::isNerve(const Nerve* pNerve) const {
    return mSpine->getCurrentNerve() == pNerve;
}

bool MapPartsFunction::isFirstStep() const {
    return mSpine->mStep == 0;
}
