#include "Game/Map/NamePosHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/JMapLinkInfo.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"

NamePosHolder::NamePosHolder() : NameObj("位置テーブル保持"), mPosNum(), mInfos() {
    mPosNum = MR::getGeneralPosNum();
    mInfos = new NamePosInfo[mPosNum];

    for (s32 i = 0; i < mPosNum; i++) {
        NamePosInfo* curInf = &mInfos[i];
        curInf->mLinkInfo = nullptr;
        curInf->mLinkedObj = nullptr;
        MR::getGeneralPosData(&curInf->mName, &curInf->mPosition, &curInf->mRotation, &curInf->mLinkInfo, i);
    }
}

NamePosInfo::NamePosInfo() {
}

bool NamePosHolder::tryRegisterLinkObj(const NameObj* pObj, const JMapInfoIter& rIter, const char* pName) {
    JMapLinkInfo info = JMapLinkInfo(rIter, true);
    bool isRegistered = false;

    for (s32 i = 0; i < mPosNum; i++) {
        NamePosInfo* pInfo = &mInfos[i];
        if (MR::isEqualString(pName, pInfo->mName) && pInfo->mLinkInfo->isValid() && *pInfo->mLinkInfo == info) {
            pInfo->mLinkedObj = pObj;
            isRegistered = true;
        }
    }

    return isRegistered;
}

bool NamePosHolder::find(const char* pName, TVec3f* pPos, TVec3f* pRot) const {
    for (s32 idx = 0; idx < mPosNum; idx++) {
        NamePosInfo* pInfo = &mInfos[idx];

        if (!MR::isEqualString(pName, pInfo->mName)) {
            continue;
        }

        if (pInfo->mLinkInfo->isValid()) {
            return false;
        }

        pPos->set(pInfo->mPosition);

        if (pRot != nullptr) {
            pRot->set(pInfo->mRotation);
        }

        return true;
    }

    return false;
}

bool NamePosHolder::find(const NameObj* pObj, const char* pName, TVec3f* pPos, TVec3f* pRot) const {
    for (s32 idx = 0; idx < mPosNum; idx++) {
        NamePosInfo* pInfo = &mInfos[idx];

        if (!MR::isEqualString(pName, mInfos[idx].mName)) {
            continue;
        }

        if (!pInfo->mLinkInfo->isValid()) {
            return false;
        }
        if (pObj != pInfo->mLinkedObj) {
            continue;
        }

        pPos->set(pInfo->mPosition);

        if (pRot != nullptr) {
            pRot->set(pInfo->mRotation);
        }

        return true;
    }

    return false;
}

namespace MR {
    NamePosHolder* getNamePosHolder() {
        return MR::getSceneObj< NamePosHolder >(SceneObj_NamePosHolder);
    }
};  // namespace MR
