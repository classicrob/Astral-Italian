#include <revolution/aralt.h>
#include <JSystem/JKernel/JKRAram.hpp>
#include <JSystem/JKernel/JKRAramHeap.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRThread.hpp>

namespace {
    u32 sAramThreadStackSize = 0xC000;
    s32 sAramThreadMsgSize = 0x20;
};  // namespace

JKRAram::JKRAram(u32 audioSize, u32 graphSize, s32 priority) : JKRThread(::sAramThreadStackSize, ::sAramThreadMsgSize, priority) {
    u32 reserved = ARInit(mStackArray, 3);
    ARQInit();
    u32 total = ARGetSize();
    mAudioMemorySize = audioSize;
    if (graphSize == 0xFFFFFFFF) {
        mGraphMemorySize = total - audioSize - reserved;
        mAramMemorySize = 0;
    } else {
        mGraphMemorySize = graphSize;
        mAramMemorySize = total - (audioSize + graphSize) - reserved;
    }

    mAudioMemoryPtr = ARAlloc(mAudioMemorySize);
    mGraphMemoryPtr = ARAlloc(mGraphMemorySize);
    if (mAramMemorySize != 0) {
        mAramMemoryPtr = ARAlloc(mAramMemorySize);
    } else {
        mAramMemoryPtr = 0;
    }

    mAramHeap = new (JKRGetSystemHeap(), 0) JKRAramHeap(mGraphMemoryPtr, mGraphMemorySize);
}
