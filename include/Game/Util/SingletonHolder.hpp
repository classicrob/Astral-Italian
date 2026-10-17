#pragma once

#include <revolution.h>

template < typename T >
class SingletonHolder {
public:
    static void init() {
        if (sInstance == nullptr) {
            sInstance = new T();
        }
    }

    static T* get() {
        return sInstance;
    }

    static void set(T* p) {
        sInstance = p;
    }

    static bool exists() {
        return sInstance != nullptr;
    }

    static T* sInstance;
};

// Petari-compatible. Define the storage per specialization in the owning .cpp:
// template <> Foo* AudSingletonHolder< Foo >::sInstance = nullptr;
template < typename T >
class AudSingletonHolder {
public:
    static void init() {
        if (sInstance == nullptr) {
            sInstance = new T();
        }
    }

    static T* get() {
        return sInstance;
    }

    static T* sInstance;
};
