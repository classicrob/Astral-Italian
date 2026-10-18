#pragma once

#include <revolution/types.h>

namespace nw4r {
namespace ut {
    struct Link {
        void* prev;
        void* next;
    };

    struct List {
        void* head;
        void* tail;
        u16 count;
        u16 offset;
    };

    void List_Init(List*, u16);
    void List_Append(List*, void*);
    void List_Remove(List*, void*);
    void* List_GetNext(const List*, const void*);
}
}
