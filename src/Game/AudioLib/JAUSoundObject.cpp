#include "JSystem/JAudio2/JAUSoundObject.hpp"
#include "JSystem/JAudio2/JAISeMgr.hpp"
#include "JSystem/JAudio2/JAISoundHandles.hpp"
#include "JSystem/JAudio2/JAISoundInfo.hpp"
#include "JSystem/JAudio2/JAISoundStarter.hpp"
#include "JSystem/JAudio2/JASGlobal.hpp"
#include "JSystem/JKernel/JKRHeap.hpp"

#include "JSystem/JAudio2/JAUSoundObject.hpp"
#include "JSystem/JAudio2/JAISeMgr.hpp"
#include "JSystem/JAudio2/JAISoundHandles.hpp"
#include "JSystem/JAudio2/JAISoundInfo.hpp"
#include "JSystem/JAudio2/JAISoundStarter.hpp"
#include "JSystem/JAudio2/JASGlobal.hpp"
#include "JSystem/JKernel/JKRHeap.hpp"











JAISoundHandle* JAUSoundObject::startSound(JAISoundID soundID) {
    if (!mIsAllocated) {
        return nullptr;
    }

    JAISoundStarter* starter = JASGlobalInstance< JAISoundStarter >::getInstance();

    JAISoundHandle* handle = getHandleSoundID(soundID);
    if (handle == nullptr) {
        handle = getFreeHandleNotReserved();
    }

    if (handle == nullptr) {
        handle = getLowPrioSound(soundID);
    }

    if (handle != nullptr) {
        starter->startSound(soundID, handle, mPos);
    }

    return handle;
}



JAISoundHandle* JAUSoundObject::startLevelSound(JAISoundID soundID) {
    if (!mIsAllocated) {
        return nullptr;
    }

    JAISeMgr* seMgr = JASGlobalInstance< JAISeMgr >::getInstance();

    JAISoundHandle* handle = getHandleSoundID(soundID);
    if (handle != nullptr) {
        handle->getSound()->updateLifeTime(1);
    } else {
        handle = getFreeHandleNotReserved();

        if (handle == nullptr) {
            handle = getLowPrioSound(soundID);
        }

        if (handle != nullptr) {
            seMgr->startSound(soundID, handle, mPos);

            if (handle->getSound() != nullptr) {
                handle->getSound()->setLifeTime(1, false);
            }
        }
    }

    return handle;
}








