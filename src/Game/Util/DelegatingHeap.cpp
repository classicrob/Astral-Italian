#include "Game/Util/DelegatingHeap.hpp"
#include <cstdlib>
#include <algorithm>

namespace {
    s32 cMinimumAlignment = 4;
}

DelegatingHeap::DelegatingHeap(JKRHeap* pParent) : JKRHeap(nullptr, 0, pParent, false), mParentHeap(pParent) {
    mStart = pParent->mEnd;
    mEnd = pParent->mEnd;
    mSize = pParent->mSize;
}

DelegatingHeap::~DelegatingHeap() {}

u32 DelegatingHeap::getHeapType() { return 'DELE'; }
bool DelegatingHeap::check() { return mParentHeap->check(); }
bool DelegatingHeap::dump() { return mParentHeap->dump(); }

void* DelegatingHeap::do_alloc(u32 size, int alignment) {
    s32 magnitude = abs(alignment);
    mStart = static_cast<u8*>(mParentHeap->alloc(size, -std::max(cMinimumAlignment, magnitude)));
    return mStart;
}

void DelegatingHeap::do_destroy() { mParentHeap->destroy(); }
void DelegatingHeap::do_free(void* pMemory) { mParentHeap->free(pMemory); }
void DelegatingHeap::do_freeAll() { do_freeTail(); }
void DelegatingHeap::do_freeTail() {
    mParentHeap->freeTail();
    mStart = mEnd;
}
void DelegatingHeap::do_fillFreeArea() { mParentHeap->fillFreeArea(); }
s32 DelegatingHeap::do_resize(void* pMemory, u32 size) { return mParentHeap->resize(pMemory, size); }
s32 DelegatingHeap::do_getSize(void* pMemory) { return mParentHeap->getSize(pMemory); }
s32 DelegatingHeap::do_getFreeSize() { return mParentHeap->getFreeSize(); }
void* DelegatingHeap::do_getMaxFreeBlock() { return mParentHeap->getMaxFreeBlock(); }
s32 DelegatingHeap::do_getTotalFreeSize() { return mParentHeap->getTotalFreeSize(); }
