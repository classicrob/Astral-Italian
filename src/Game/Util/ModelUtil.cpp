#include "Game/Util/ModelUtil.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/XanimeResource.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/Player/J3DModelX.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/System/ShapePacketUserData.hpp"
#include "Game/Util.hpp"
#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>
#include <cstdio>

namespace {
    inline bool isUseLightChanNo(J3DMaterial* pMaterial, int channel, int index) {
        u16 channelID = pMaterial->mColorBlock->getColorChan(channel)->mColorChanID;
        if (((channelID >> 1) & 0x1) == 0) {
            return false;
        }

        u8 useLightChan = (channelID >> 2 & 0xF | channelID >> 7 & 0xF0);
        return useLightChan & (1 << index);
    }
};  // namespace

namespace MR {






    void updateModelAnimPlayer(LiveActor* pActor) {
        XanimePlayer* pAnimePlayer = pActor->mModelManager->mXanimePlayer;
        pAnimePlayer->updateBeforeMovement();
        pAnimePlayer->updateAfterMovement();
    }

    void invalidateMtxCalc(J3DModelData* pModelData) {
        u16 jointNum = pModelData->getJointNum();
        for (u16 i = 0; i < jointNum; i++) {
            pModelData->getJointNodePointer(i)->setMtxCalc(nullptr);
        }
    }

    void invalidateJointCallback(J3DModelData* pModelData) {
        u16 jointNum = pModelData->getJointNum();
        for (u16 i = 0; i < jointNum; i++) {
            pModelData->getJointNodePointer(i)->setCallBack(nullptr);
        }
    }

    J3DModel* getJ3DModel(const LiveActor* pActor) {
        if (pActor->mModelManager == nullptr) {
            return nullptr;
        }

        return pActor->mModelManager->getJ3DModel();
    }

    void calcJ3DModel(LiveActor* pActor) {
        OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
        getJ3DModel(pActor)->calc();
        OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
    }

    J3DModelData* getJ3DModelData(const LiveActor* pActor) {
        if (pActor->mModelManager == nullptr) {
            return nullptr;
        }

        return pActor->mModelManager->getJ3DModelData();
    }

    s16 getBckFrameMax(const LiveActor* pActor, const char* pBckName) {
        return static_cast< J3DAnmBase* >(getResourceHolder(pActor)->mMotionResTable->getRes(pBckName))->mFrameMax;
    }

    s16 getBrkFrameMax(const LiveActor* pActor, const char* pBrkName) {
        return static_cast< J3DAnmBase* >(getResourceHolder(pActor)->mBrkResTable->getRes(pBrkName))->mFrameMax;
    }

    s16 getBvaFrameMax(const LiveActor* pActor, const char* pBvaName) {
        return static_cast< J3DAnmBase* >(getResourceHolder(pActor)->mBvaResTable->getRes(pBvaName))->mFrameMax;
    }

    bool isBckPlaying(XanimePlayer* pAnimePlayer, const char* pBckName) {
        bool running = pAnimePlayer->_24[pAnimePlayer->_54].checkState(1) ? false : true;
        return running && pAnimePlayer->isRun(pBckName) && pAnimePlayer->getRate() != 0.0f;
    }


    u16 getMaterialNo(J3DModelData* pModelData, const char* pMaterialName) {
        return pModelData->getMaterialName()->getIndex(pMaterialName);
    }

    u16 getMaterialNo(J3DModel* pModel, const char* pMaterialName) {
        JUTNameTab* materialName = pModel->getModelData()->getMaterialName();
        return materialName->getIndex(pMaterialName);
    }

    J3DMaterial* getMaterial(J3DModelData* pModelData, const char* pMaterialName) {
        return pModelData->getMaterialNodePointer(getMaterialNo(pModelData, pMaterialName));
    }

    J3DMaterial* getMaterial(J3DModelData* pModelData, int idx) {
        return pModelData->getMaterialNodePointer(idx);
    }

    J3DMaterial* getMaterial(J3DModel* pModel, int idx) {
        return pModel->mModelData->getMaterialNodePointer(idx);
    }

    J3DMaterial* getMaterial(const LiveActor* pActor, int idx) {
        return getJ3DModelData(pActor)->getMaterialNodePointer(idx);
    }

    s32 getMaterialNum(J3DModel* pModel) {
        return pModel->mModelData->getMaterialNum();
    }

    const char* getMaterialName(const J3DModelData* pModelData, int idx) {
        return pModelData->getMaterialName()->getName(idx);
    }

    void updateModelDiffDL(LiveActor* pActor) {
        pActor->mModelManager->updateDL(true);
    }

    bool isEnvelope(J3DMaterial* pMaterial) {
        return pMaterial->getShape()->mHasPNMTXIdx;
    }

    void hideMaterial(J3DModel* pModel, const char* pMaterialName) {
        J3DShapePacket* pckt = pModel->getMatPacket(getMaterialNo(pModel, pMaterialName))->getShapePacket();
        pckt->mFlags |= 0x10;
    }

    void hideMaterial(const LiveActor* pActor, const char* pMaterialName) {
        hideMaterial(getJ3DModel(pActor), pMaterialName);
    }

    void showMaterial(J3DModel* pModel, const char* pMaterialName) {
        J3DShapePacket* pckt = pModel->getMatPacket(getMaterialNo(pModel, pMaterialName))->getShapePacket();
        pckt->mFlags &= ~0x10;
    }

    void showMaterial(const LiveActor* pActor, const char* pMaterialName) {
        showMaterial(getJ3DModel(pActor), pMaterialName);
    }


    bool isUseTex(J3DMaterial* pMaterial, u16 a2) {
        for (u32 idx = 0; idx < 8; idx++) {
            if (pMaterial->getTexNo(idx) != a2) {
                continue;
            }

            for (u32 stage = 0; stage < pMaterial->getTevStageNum(); stage++) {
                if (pMaterial->getTevBlock()->getTevOrder(stage)->mTexMap == idx) {
                    return true;
                }
            }
        }

        return false;
    }

    bool isNormalTexMtx(J3DMaterial* pMaterial) {
        for (u32 idx = 0; idx < 8; idx++) {
            J3DTexMtx* texMtx = pMaterial->mTexGenBlock->getTexMtx(idx);

            if (texMtx != nullptr && (texMtx->getTexMtxInfo().mInfo & 0x3F) != 0 && isUseTexMtx(pMaterial, idx)) {
                return false;
            }
        }

        return true;
    }




    bool isUseTexMtxEnvMap(J3DMaterial* pMaterial) {
        for (u32 idx = 0; idx < 8; idx++) {
            J3DTexMtx* texMtx = pMaterial->mTexGenBlock->getTexMtx(idx);
            if (texMtx == nullptr) {
                continue;
            }

            u32 temp = texMtx->getTexMtxInfo().mInfo & 0x3F;
            if (temp == 1 || temp == 6 || temp == 7) {
                if (!isUseTexMtx(pMaterial, idx)) {
                    return false;
                }

                return true;
            }
        }

        return false;
    }

    bool isUseTexMtxProjMap(J3DMaterial* pMaterial) {
        for (u32 idx = 0; idx < 8; idx++) {
            J3DTexMtx* texMtx = pMaterial->mTexGenBlock->getTexMtx(idx);
            if (texMtx == nullptr) {
                continue;
            }

            u32 temp = texMtx->getTexMtxInfo().mInfo & 0x3F;
            if (temp == 2 || temp == 3 || temp == 8 || temp == 9) {
                return true;
            }
        }

        return false;
    }



    ResTIMG* getResTIMG(const LiveActor* pActor, int idx) {
        return getResTIMG(getJ3DModelData(pActor), idx);
    }

    ResTIMG* getResTIMG(const J3DModelData* pModelData, int idx) {
        return pModelData->mMaterialTable.getTexture()->getResTIMG(idx);
    }

    ResTIMG* getTexture(ResourceHolder* pResourceHolder, const char* pTextureName) {
        return static_cast< ResTIMG* >(pResourceHolder->mFileInfoTable->getRes(pTextureName));
    }




    void copyJointAnimation(J3DModel* pCopyTo, J3DModel* pCopyFrom) {
        u16 jointNum = pCopyTo->getModelData()->getJointNum();
        for (u16 i = 0; i < jointNum; i++) {
            pCopyTo->setAnmMtx(i, pCopyFrom->getAnmMtx(i));
        }

        u16 wEvlpMtxNum = pCopyTo->getModelData()->getWEvlpMtxNum();
        for (u16 i = 0; i < wEvlpMtxNum; i++) {
            pCopyTo->setWeightAnmMtx(i, pCopyFrom->getWeightAnmMtx(i));
        }
    }

    void copyJointAnimation(LiveActor* pCopyTo, const LiveActor* pCopyFrom) {
        return copyJointAnimation(getJ3DModel(pCopyTo), getJ3DModel(pCopyFrom));
    }

    void syncJointAnimation(LiveActor* pActor1, const LiveActor* pActor2) {
        getJ3DModel(pActor1)->mMtxBuffer = getJ3DModel(pActor2)->getMtxBuffer();
    }

    void syncMaterialAnimation(J3DModel* pModel1, J3DModel* pModel2) {
        u16 materialNum = pModel1->getModelData()->getMaterialNum();
        u16 i;
        u16 j;
        u16 idx;
        for (i = 0; i < materialNum; i++) {
            J3DShapePacket* shapePacketModel2 = pModel2->getShapePacket(i);
            idx = shapePacketModel2->getShape()->getMaterial()->getIndex();
            for (j = 0; j < materialNum; j++) {
                J3DShapePacket* shapePacketModel1 = pModel1->getShapePacket(j);
                if (shapePacketModel1->getShape()->getMaterial()->getIndex() == idx) {
                    shapePacketModel1->setDisplayListObj(shapePacketModel2->getDisplayListObj());
                    break;
                }
            }
        }
    }

    void syncMaterialAnimation(LiveActor* pActor1, const LiveActor* pActor2) {
        return syncMaterialAnimation(getJ3DModel(pActor1), getJ3DModel(pActor2));
    }


    void getLightNum(J3DMaterial* pMaterial, s32* pChan1, s32* pChan2, s32* pChan3, s32* pChan4) {
        for (int i = 0; i < 8; i++) {
            if (::isUseLightChanNo(pMaterial, 0, i)) {
                *pChan1 = *pChan1 + 1;
            }

            if (::isUseLightChanNo(pMaterial, 1, i)) {
                *pChan2 = *pChan2 + 1;
            }
        }

        for (int i = 0; i < 8; i++) {
            if (::isUseLightChanNo(pMaterial, 2, i)) {
                *pChan3 = *pChan3 + 1;
            }

            if (::isUseLightChanNo(pMaterial, 3, i)) {
                *pChan4 = *pChan4 + 1;
            }
        }
    }

    s32 getLightNum(J3DMaterial* pMaterial) {
        s32 chan1 = 0;
        s32 chan2 = 0;
        s32 chan3 = 0;
        s32 chan4 = 0;
        getLightNum(pMaterial, &chan1, &chan2, &chan3, &chan4);
        return chan1 + chan2 + chan3 + chan4;
    }


    s32 getLightNumMax(J3DModelData* pModelData) {
        int maxNum = 0;
        for (int i = 0; i < pModelData->mMaterialTable.mMaterialNum; i++) {
            int currentNum = getLightNum(pModelData->mMaterialTable.mMaterialNodePointer[static_cast< u16 >(i)]);

            if (maxNum < currentNum) {
                maxNum = currentNum;
            }
        }

        return maxNum;
    }



    void calcModelBoundingRadius(f32* pOut, const LiveActor* pActor) {
        MR::getJointNum(pActor);
        TBox3f boundingBox;
        calcModelBoundingBox(&boundingBox, pActor);
        TVec3f i = boundingBox.i;
        TVec3f f = boundingBox.f;

        TVec3f max;
        max.set(MR::max(MR::abs(i.x), MR::abs(f.x)), MR::max(MR::abs(i.y), MR::abs(f.y)), MR::max(MR::abs(i.z), MR::abs(f.z)));

        *pOut = max.length();
    }

    J3DModel* newJ3DModel(const ResourceHolder* pResourceHolder, const char* pChar, J3DMdlFlag mdlFlag) {
        J3DModelData* modelData = static_cast< J3DModelData* >(pResourceHolder->mModelResTable->getRes(pChar));
        J3DModel* newModel;
        invalidateMtxCalc(modelData);
        invalidateJointCallback(modelData);

        for (u16 i = 0; i < modelData->getShapeNum(); i++) {
            modelData->getShapeNodePointer(i)->mFlags &= ~0x1;
        }

        if (MR::isEqualString(pChar, "Mario") || MR::isEqualString(pChar, "Luigi") || MR::isEqualString(pChar, "MarioHandL") ||
            MR::isEqualString(pChar, "MarioHandR") || MR::isEqualString(pChar, "MarioFace") || MR::isEqualString(pChar, "LuigiFace") ||
            MR::isEqualString(pChar, "MarioShadow") || MR::isEqualString(pChar, "LuigiShadow") || MR::isEqualString(pChar, "IceMario") ||
            MR::isEqualString(pChar, "IceLuigi") || MR::isEqualString(pChar, "IceMarioHandL") || MR::isEqualString(pChar, "IceMarioHandR") ||
            MR::isEqualString(pChar, "InvincibleMarioHandL") || MR::isEqualString(pChar, "InvincibleMarioHandR") ||
            MR::isEqualString(pChar, "InvincibleMario") || MR::isEqualString(pChar, "InvincibleLuigi") || MR::isEqualString(pChar, "BeeMario") ||
            MR::isEqualString(pChar, "BeeLuigi") || MR::isEqualString(pChar, "HopperMario") || MR::isEqualString(pChar, "HopperLuigi") ||
            MR::isEqualString(pChar, "BoneMario") || MR::isEqualString(pChar, "BoneLuigi") || MR::isEqualString(pChar, "SearchLightCone") ||
            strstr(pChar, "FlexibleSphere") || strstr(pChar, "FlexibleSandPlanetParts") || MR::isEqualString(pChar, "ScaleDownRelayPlanet") ||
            MR::isEqualString(pChar, "GhostMario") || MR::isEqualString(pChar, "GhostLuigi") || MR::isEqualString(pChar, "Koura")) {
            OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
            newModel = static_cast< J3DModel* >(new J3DModelX(modelData, mdlFlag, 1));
            OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
        } else {
            OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
            newModel = new J3DModel(modelData, mdlFlag, 1);
            OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
        }

        initJ3DShapePacketUserData(newModel);
        Mtx mtx;
        PSMTXIdentity(mtx);
        newModel->unlock();
        OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
        PSMTXCopy(mtx, j3dSys.mViewMtx);
        newModel->calc();
        newModel->calcMaterial();
        newModel->viewCalc();
        newModel->viewCalc();
        OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
        if (modelData->getModelDataType() == 0) {
            ProhibitSchedulerAndInterrupts prohibit(false);
            OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
            newModel->makeDL();
            OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
        }

        newModel->lock();
        return newModel;
    }

    bool isExistCollisionResource(const LiveActor* pActor, const char* pName) {
        char buff[0x80];
        snprintf(buff, sizeof(buff), "%s.kcl", pName);
        return getResourceHolder(pActor)->mFileInfoTable->findFileInfo(buff) != nullptr;
    }

    bool isExistModel(const char* pName) {
        char buff[0x100];
        snprintf(buff, sizeof(buff), "/ObjectData/%s.arc", pName);
        return isFileExist(buff, false);
    }

    bool isExistSubModel(const char* pModelName, const char* pSubModelName) {
        char buff[0x100];
        snprintf(buff, sizeof(buff), "%s%s", pModelName, pSubModelName);
        return isExistModel(buff);
    }

    J3DModelData* getJ3DModelData(const char* pName) {
        char buff[0x100];
        snprintf(buff, sizeof(buff), "%s.arc", pName);
        return static_cast< J3DModelData* >(createAndAddResourceHolder(buff)->mModelResTable->getRes(pName));
    }

};  // namespace MR
