#include <JSystem/JAudio2/JAUInitializer.hpp>
#include <JSystem/JAudio2/JASAramStream.hpp>
#include <JSystem/JAudio2/JASAudioThread.hpp>
#include <JSystem/JAudio2/JASDriverIF.hpp>
#include <JSystem/JAudio2/JASDvdThread.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include <JSystem/JAudio2/JASTrack.hpp>
#include <JSystem/JAudio2/JASWaveArcLoader.hpp>
#include <JSystem/JKernel/JKRAram.hpp>
#include <JSystem/JKernel/JKRThread.hpp>
#include <revolution/sc.h>

void JAU_JASInitializer::initJASystem(JKRSolidHeap* pHeap) {
    if (JASAudioThread::getThreadPointer() == nullptr && JASDvd::getThreadPointer() == nullptr) {
        JASKernel::setupRootHeap(pHeap, heapSize_);
        if (audioMemory_ == 0) {
            audioMemory_ = JKRAram::getManager()->getAudioMemory();
        }

        if (audioMemSize_ == 0) {
            audioMemSize_ = JKRAram::getManager()->getAudioMemSize();
        }

        JASKernel::setupAramHeap(audioMemory_, audioMemSize_);
        JASTrack::newMemPool(field_0x1c);
        if (field_0x20 > 0) {
            JASTrack::TChannelMgr::newMemPool(field_0x20);
        }

        JASDvd::createThread(dvdThreadPriority_, 0x100, 0x8000);
        JASAudioThread::create(audioThreadPriority_);
        JKRThreadSwitch* pThreadSwitch = JKRThreadSwitch::getManager();
        if (pThreadSwitch != nullptr) {
            if (dvdThreadId_ >= 0) {
                pThreadSwitch->enter(JASDvd::getThreadPointer(), dvdThreadId_);
            }

            if (audioThreadId_ >= 0) {
                pThreadSwitch->enter(JASAudioThread::getThreadPointer(), audioThreadId_);
            }
        }

        JASDriver::setDSPLevel(dspLevel_);
        JASAramStream::initSystem(aramBlockSize_, aramChannelNum_);
    }

    if (waveArcDir_ != nullptr) {
        JASWaveArcLoader::setCurrentDir(waveArcDir_);
    }

    switch (SCGetSoundMode()) {
    case 0:
        JASDriver::setOutputMode(0);
        break;
    case 1:
        JASDriver::setOutputMode(1);
        break;
    case 2:
        JASDriver::setOutputMode(2);
        break;
    }
}
