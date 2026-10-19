#include "Game/NPC/NPCParameter.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/NPCActor.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/types.h>

template MR::Vector< MR::FixedArray< NPCParameterBase*, 48 > >::~Vector();
template MR::Vector< MR::FixedArray< NPCParameterBase*, 48 > >::Vector();
template void MR::Vector< MR::FixedArray< NPCParameterBase*, 48 > >::push_back(NPCParameterBase* const&);

NPCParameterEdit::NPCParameterEdit(const char* pName, const char** pPpChar, bool a3) : NPCParameterBase(pName), mValue(pPpChar), mIsResourceName(a3) {
}

void NPCParameterEdit::read(JMapInfo* pInfo, s32 a2) {
    pInfo->getValue(a2, mName, mValue);
}

NPCParameterBool::NPCParameterBool(const char* pName, bool* pBool) : NPCParameterBase(pName), mValue(pBool) {
}

void NPCParameterBool::read(JMapInfo* pInfo, s32 a2) {
    pInfo->getValue(a2, mName, mValue);
}

NPCParameterV3f::NPCParameterV3f(const char* pName, TVec3f* pVec, f32 f1, f32 f2) : NPCParameterBase(pName), mValue(pVec), mMinValue(f1), mMaxValue(f2) {
}

void NPCParameterV3f::read(JMapInfo* pInfo, s32 a2) {
    char str[255];
    MR::copyString(str, mName, sizeof(str));
    *(str + strlen(mName) + 1) = '\0';
    *(str + strlen(mName)) = 'X';
    pInfo->getValue(a2, str, &mValue->x);
    *(str + strlen(mName)) = 'Y';
    pInfo->getValue(a2, str, &mValue->y);
    *(str + strlen(mName)) = 'Z';
    pInfo->getValue(a2, str, &mValue->z);
}

NPCParameterJoint::NPCParameterJoint(const char* pName, const char* pChar, const char** ppChar)
    : NPCParameterBase(pName), mValue(ppChar), mModelName(pChar), mJointIndex(-1) {
}

NPCParameterReader::NPCParameterReader(const char* pName) : NPCParameterBase(pName), MR::Vector< MR::FixedArray< NPCParameterBase*, 48 > >() {
}

void NPCParameterReader::read(JMapInfo* pInfo, s32 a2) {
    if (a2 <= -1 || a2 >= pInfo->getNumEntries()) {
        return;
    }

    for (NPCParameterBase** pIter = begin(); pIter != end(); pIter++) {
        (*pIter)->read(pInfo, a2);
    }
}

NPCCapsParameterReader::NPCCapsParameterReader(const char* pName) : NPCParameterReader(pName), mActorCaps("") {
    mActorCaps.setDefault();
    mActorCaps.mObjectName = pName;
    mActorCaps.mMessageId = pName;
    init();
}

void NPCItemParameterReader::copy(const NPCActorItem* pItem) {
    mItem.mActor = pItem->mActor;
    mItem.mGoods0 = pItem->mGoods0;
    mItem.mGoodsJoint0 = pItem->mGoodsJoint0;
    mItem.mGoods1 = pItem->mGoods1;
    mItem.mGoodsJoint1 = pItem->mGoodsJoint1;
}

template <>
NPCParameterRange< s32 >::NPCParameterRange(const char* pName, s32* pLong, s32 a3, s32 a4) : NPCParameterBase(pName), mValue(pLong), mMinValue(a3), mMaxValue(a4) {
}

template <>
void NPCParameterRange< s32 >::read(JMapInfo* pInfo, s32 a2) {
    pInfo->getValue(a2, mName, mValue);
}

template <>
NPCParameterRange< f32 >::NPCParameterRange(const char* pName, f32* pLong, f32 a3, f32 a4) : NPCParameterBase(pName), mValue(pLong), mMinValue(a3), mMaxValue(a4) {
}

template <>
void NPCParameterRange< f32 >::read(JMapInfo* pInfo, s32 a2) {
    pInfo->getValue(a2, mName, mValue);
}

void NPCParameterJoint::read(JMapInfo* pInfo, s32 a2) {
    pInfo->getValue(a2, mName, mValue);

    if (strcmp(*mValue, "") == 0) {
        *mValue = "";
    }
}

void NPCCapsParameterReader::init() {
    push_back(new NPCParameterBool("mModel", &mActorCaps.mModel));
    push_back(new NPCParameterBool("mMakeActor", &mActorCaps.mMakeActor));
    push_back(new NPCParameterBool("mHostIO", &mActorCaps.mHostIO));
    push_back(new NPCParameterBool("mMessage", &mActorCaps.mMessage));
    push_back(new NPCParameterV3f("mMessageOffset", &mActorCaps.mMessageOffset, 0.0f, 1000.0f));
    push_back(new NPCParameterBool("mInterpole", &mActorCaps.mInterpole));
    push_back(new NPCParameterBool("mConnectTo", &mActorCaps.mConnectTo));
    push_back(new NPCParameterBool("mLightCtrl", &mActorCaps.mLightCtrl));
    push_back(new NPCParameterBool("mEffect", &mActorCaps.mEffect));
    push_back(new NPCParameterBool("mSound", &mActorCaps.mSound));
    push_back(new NPCParameterRange< s32 >("mSoundSize", &mActorCaps.mSoundSize, 0, 16));
    push_back(new NPCParameterBool("mAttribute", &mActorCaps.mAttribute));
    push_back(new NPCParameterBool("mPosition", &mActorCaps.mPosition));
    push_back(new NPCParameterBool("mLodCtrl", &mActorCaps.mLodCtrl));
    push_back(new NPCParameterBool("mNerve", &mActorCaps.mNerve));
    push_back(new NPCParameterBool("mBinder", &mActorCaps.mBinder));
    push_back(new NPCParameterRange< f32 >("mBinderSize", &mActorCaps.mBinderSize, 0.0f, 1000.0f));
    push_back(new NPCParameterBool("mSensor", &mActorCaps.mSensor));
    push_back(new NPCParameterJoint("mSensorJoint", mActorCaps.mObjectName, &mActorCaps.mSensorJoint));
    push_back(new NPCParameterRange< f32 >("mSensorSize", &mActorCaps.mSensorSize, 0.0f, 1000.0f));
    push_back(new NPCParameterRange< s32 >("mSensorMax", &mActorCaps.mSensorMax, 0, 16));
    push_back(new NPCParameterBool("mShadow", &mActorCaps.mShadow));
    push_back(new NPCParameterRange< f32 >("mShadowSize", &mActorCaps.mShadowSize, 0.0f, 1000.0f));
    push_back(new NPCParameterBool("mRailKeeper", &mActorCaps.mRailRider));
    push_back(new NPCParameterBool("mSwitchDead", &mActorCaps.mSwitchDead));
    push_back(new NPCParameterBool("mSwitchAppear", &mActorCaps.mSwitchAppear));
    push_back(new NPCParameterBool("mPointer", &mActorCaps.mPointer));
    push_back(new NPCParameterRange< f32 >("mPointerSize", &mActorCaps.mPointerSize, 0.0f, 1000.0f));
}

NPCItemParameterReader::NPCItemParameterReader(const char* pName) : NPCParameterReader(pName), mItem("") {
    push_back(new NPCParameterEdit("mGoods0", &mItem.mGoods0, true));
    push_back(new NPCParameterEdit("mGoods1", &mItem.mGoods1, true));
    push_back(new NPCParameterJoint("mGoodsJoint0", pName, &mItem.mGoodsJoint0));
    push_back(new NPCParameterJoint("mGoodsJoint1", pName, &mItem.mGoodsJoint1));
}

void NPCParameterBase::read(JMapInfo*, s32) {
}
