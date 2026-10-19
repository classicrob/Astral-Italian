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

    void List_Remove(List* list, void* object) {
        Link* link = GetLink_(list, object);
        if (link->prev == NULL) {
            list->head = link->next;
        } else {
            GetLink_(list, link->prev)->next = link->next;
        }
        if (link->next == NULL) {
            list->tail = link->prev;
        } else {
            GetLink_(list, link->next)->prev = link->prev;
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
