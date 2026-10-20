#include "JSystem/JParticle/JPAResourceManager.hpp"
#include "JSystem/JParticle/JPADynamicsBlock.hpp"
#include "JSystem/JParticle/JPAResource.hpp"
#include <cstring>

#include "JSystem/JParticle/JPAResourceLoader.hpp"

JPAResourceManager::JPAResourceManager(void const* pData, JKRHeap* pHeap) {
    mpResArr = NULL;
    mpTexArr = NULL;
    mResMax = 0;
    mResNum = 0;
    mTexMax = 0;
    mTexNum = 0;
    mpHeap = pHeap;
    JPAResourceLoader loader((u8 const*)pData, this);
}

const ResTIMG* JPAResourceManager::swapTexture(ResTIMG const* img, char const* swapName) {
    const ResTIMG* ret = NULL;

    for (s32 i = 0; i < mTexNum; i++) {
        if (strcmp(swapName, mpTexArr[i]->getName()) == 0) {
            JUTTexture* tex = mpTexArr[i]->getJUTTexture();
            ret = tex->getTexInfo();
            tex->storeTIMG(img, (u8)0);
            break;
        }
    }

    return ret;
}

void JPAResourceManager::registRes(JPAResource* res) {
    mpResArr[mResNum] = res;
    mResNum++;
}

void JPAResourceManager::registTex(JPATexture* tex) {
    mpTexArr[mTexNum] = tex;
    mTexNum++;
}
