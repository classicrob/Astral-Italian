#pragma once

#include <revolution.h>
#include <stdint.h>

// Hash-code -> payload table. Payloads are pointer-width (uintptr_t; identical to u32 on the Wii), so callers may store
// either indices or object pointers; use addPtr/searchPtr for pointers instead of casting.
class HashSortTable {
public:
#ifdef GALAXY_NATIVE
    /// @brief Payload stored next to each hash. Some tables store object pointers, so the
    /// native payload is pointer-sized. Hash codes remain `u32` to match serialized data.
    typedef uintptr_t Value;
#else
    typedef u32 Value;
#endif

    HashSortTable(u32);

    bool add(const char*, uintptr_t, bool);
    bool add(u32, uintptr_t);
    bool addOrSkip(u32, uintptr_t);
    void sort();
    bool search(u32, uintptr_t*);
    bool search(const char*, uintptr_t*);
    bool search(const char*, const char*, uintptr_t*);
    void swap(const char*, const char*);

    template < typename T >
    bool addPtr(const char* pName, T* pPayload, bool isValidSkip) {
        return add(pName, reinterpret_cast< uintptr_t >(pPayload), isValidSkip);
    }

    template < typename T >
    bool addPtr(u32 hash, T* pPayload) {
        return add(hash, reinterpret_cast< uintptr_t >(pPayload));
    }

    // Like search(): *ppOut (must be non-null) becomes the found pointer, or nullptr when not found. Aliasing-safe (goes
    // through a uintptr_t local). Where the original stored into an initialised local, use `uintptr_t v = 0; search(name, &v)`
    // + reinterpret_cast<T*>(v) instead to keep the Wii code identical.
    template < typename T >
    bool searchPtr(const char* pName, T** ppOut) {
        uintptr_t payload;
        bool found = search(pName, &payload);
        *ppOut = reinterpret_cast< T* >(payload);
        return found;
    }

    template < typename T >
    bool searchPtr(u32 hash, T** ppOut) {
        uintptr_t payload;
        bool found = search(hash, &payload);
        *ppOut = reinterpret_cast< T* >(payload);
        return found;
    }

    /* 0x00 */ bool mHasBeenSorted;  // layout copied from Petari, not yet verified against SB4E01 (ctor is 0x7C)
    /* 0x04 */ u32* mHashCodes;
    /* 0x08 */ uintptr_t* mPayloads;
    /* 0x0C */ u16* _C;
    /* 0x10 */ u16* _10;
    /* 0x14 */ u32 mCurrentLength;
    /* 0x18 */ u32 mMaxLength;
};

namespace MR {
    u32 getHashCode(const char*);
    u32 getHashCodeLower(const char*);
};  // namespace MR
