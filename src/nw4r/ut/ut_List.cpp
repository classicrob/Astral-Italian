#include "nw4r/ut/list.h"

namespace nw4r {
namespace ut {
    namespace {
        inline Link* GetLink_(const List* list, void* object) {
            return reinterpret_cast<Link*>(static_cast<u8*>(object) + list->offset);
        }
        inline const Link* GetLink_(const List* list, const void* object) {
            return reinterpret_cast<const Link*>(static_cast<const u8*>(object) + list->offset);
        }
    }

    void List_Init(List* list, u16 offset) {
        list->head = NULL;
        list->tail = NULL;
        list->count = 0;
        list->offset = offset;
    }

    void List_Append(List* list, void* object) {
        if (list->head == NULL) {
            Link* link = GetLink_(list, object);
            link->prev = link->next = NULL;
            list->head = object;
            list->tail = object;
            ++list->count;
        } else {
            Link* link = GetLink_(list, object);
            link->prev = list->tail;
            link->next = NULL;
            GetLink_(list, list->tail)->next = object;
            list->tail = object;
            ++list->count;
        }
    }

    // Ported from zeldaret/ss@96f2dd5497c4ceb595de76fd897fbb1542b27323:src/nw4r/ut/ut_list.cpp
    void List_Remove(List* list, void* object) {
        Link* link = ((Link*)((char*)object + list->offset));
        if (link->prev == NULL) {
            list->head = link->next;
        } else {
            ((Link*)((char*)link->prev + list->offset))->next = link->next;
        }
        if (link->next == NULL) {
            list->tail = link->prev;
        } else {
            ((Link*)((char*)link->next + list->offset))->prev = link->prev;
        }
        link->prev = NULL;
        link->next = NULL;
        --list->count;
    }

    void* List_GetNext(const List* list, const void* object) {
        if (object == NULL) return list->head;
        return GetLink_(list, object)->next;
    }
}
}
