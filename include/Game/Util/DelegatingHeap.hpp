#pragma once

#include <JSystem/JKernel/JKRHeap.hpp>

// Heap view that forwards allocation operations to its parent (heap type 'DELE').
class DelegatingHeap : public JKRHeap {
public:
    DelegatingHeap(JKRHeap*);
    virtual ~DelegatingHeap();
    virtual u32 getHeapType();
    virtual bool check();
    virtual bool dump();
    virtual void do_destroy();
    virtual void* do_alloc(u32, int);
    virtual void do_free(void*);
    virtual void do_freeAll();
    virtual void do_freeTail();
    virtual void do_fillFreeArea();
    virtual s32 do_resize(void*, u32);
    virtual s32 do_getSize(void*);
    virtual s32 do_getFreeSize();
    virtual void* do_getMaxFreeBlock();
    virtual s32 do_getTotalFreeSize();

    /* 0x6C */ JKRHeap* mParentHeap;
};
