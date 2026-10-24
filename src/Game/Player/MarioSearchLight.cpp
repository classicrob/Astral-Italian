#include "Game/Player/MarioSearchLight.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Player/J3DModelX.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include <JSystem/JKernel/JKRHeap.hpp>
#include <revolution/mtx.h>

namespace {
    const char* sSensorNames[] = {"eye1", "eye2", "eye3", "eye4"};
}

MarioSearchLight::MarioSearchLight(LiveActor* actor)
    : PartsModel(actor, "サーチライト", "SearchLightCone", nullptr, -1, false),
      _A0(0), _A4(nullptr), _A8(false), _AC(0.0f, 0.0f, 1.0f), _B8(false), _BC(0.0f), _C0(5000.0f), _C4(0) {
}

// NONMATCHING: loop-counter and actor register allocation.
void MarioSearchLight::init(const JMapInfoIter& iter) {
    PartsModel::init(iter);
    initHitSensor(4);
    for (u32 i = 0; i < 4; i++) {
        MR::addHitSensorCallback(this, sSensorNames[i], 0x7A, 16, 300.0f);
    }
    makeActorDead();
    J3DModelX* model = static_cast<J3DModelX*>(MR::getJ3DModel(this));
    model->_DD = 16;
    u32 matrixCount = model->getModelData()->getDrawMtxNum();
    for (u32 i = 0; i < model->_DD; i++) {
        model->mExtraMtxBuffer[i] = new (0x20) Mtx[matrixCount];
    }
}

void MarioSearchLight::initAfterPlacement() {
    _B8 = true;
}

// NONMATCHING: equivalent distance-check branch folding.
void MarioSearchLight::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (MR::isSensorPlayer(receiver)) {
        return;
    }
    if (_C4 < 30) {
        return;
    }
    TVec3f front;
    MR::calcFrontVec(&front, this);
    TVec3f perpendicular;
    TVec3f delta = *MR::getSensorPos(receiver) - *MR::getSensorPos(sender);
    JMAVECScaleAdd(&front, &delta, &perpendicular, -front.dot(delta));
    if (_A8 && MR::abs(_AC.dot(front)) < 0.17364799976348877f) {
        JMAVECScaleAdd(&_AC, &perpendicular, &perpendicular, -_AC.dot(perpendicular));
    }
    if (perpendicular.squared() > 22500.0f) {
        return;
    }
    TVec3f toReceiver = *MR::getSensorPos(receiver) - mPosition;
    f32 distance = front.dot(toReceiver);
    if (distance < 0.0f || distance > _C0) {
        return;
    } else {
        if (MR::sendArbitraryMsg(0xC, receiver, sender)) {
            return;
        }
        if (MR::isExistInAttributeGroupSearchTurtle(MR::getSensorHost(receiver))) {
            fn_803F1E80(receiver);
        }
    }
}

void MarioSearchLight::fn_803F1E80(HitSensor* sensor) {
    if (sensor != _A4) {
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
    }
    _A4 = sensor;
    _A0 = 30;
}

// NONMATCHING: two constructor-argument setup instructions are reversed.
void MarioSearchLight::updateHitSensor(HitSensor* sensor) {
    f32 spacing = 1250.0f;
    for (s32 i = 0; static_cast<u32>(i) < 4; i++) {
        if (getSensor(sSensorNames[i]) == sensor) {
            TVec3f front;
            MR::calcFrontVec(&front, this);
            TVec3f position = mPosition + front * (_BC + spacing * i);
            sensor->mPosition.x = position.x;
            sensor->mPosition.y = position.y;
            sensor->mPosition.z = position.z;
            return;
        }
    }
}

void MarioSearchLight::calcAndSetBaseMtx() {
    Mtx baseMtx;
    PSMTXCopy(getBaseMtx(), baseMtx);
    PartsModel::calcAndSetBaseMtx();
    MR::blendMtxRotateSlerp(baseMtx, getBaseMtx(), 0.1f, baseMtx);
    TVec3f trans;
    MR::extractMtxTrans(getBaseMtx(), &trans);
    MR::setMtxTrans(baseMtx, trans.x, trans.y, trans.z);
    MR::setBaseTRMtx(this, baseMtx);
}

void MarioSearchLight::appear() {
    LiveActor::appear();
    _C4 = 0;
}

// NONMATCHING: floating-point register allocation.
void MarioSearchLight::control() {
    f32 offset = _BC;
    offset += 100.0f;
    _BC = offset;
    if (offset >= 1250.0f) {
        _BC = 0.0f;
    }
    fn_803F21E0();
    fn_803F2160();
    fn_803F2280();
    if (_C4 < 30) {
        _C4++;
    }
}

void MarioSearchLight::fn_803F2160() {
    if (_A4 != nullptr && !MR::isSensorValid(_A4)) {
        _A4 = nullptr;
    }
    if (_A0 != 0) {
        _A0--;
    }
    if (_A0 == 0) {
        _A4 = nullptr;
    }
}

void MarioSearchLight::fn_803F21E0() {
    TVec3f front;
    MR::calcFrontVec(&front, this);
    _C0 = 5000.0f;
    if (_B8) {
        Triangle triangle;
        TVec3f hitPosition;
        if (MR::getFirstPolyOnLineToMap(&hitPosition, &triangle, mPosition, front * 5000.0f)) {
            _C0 = (hitPosition - mPosition).length();
        }
    }
}

void MarioSearchLight::fn_803F2280() {
    f32 scale = _C0 / 5000.0f;
    mScale.z = scale;
    f32 width = 0.3f + 0.7f * scale;
    mScale.x = width;
    mScale.y = width;
}

MarioSearchLight::~MarioSearchLight() {
}
