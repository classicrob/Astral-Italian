#include "JSystem/JKernel/JKRDisposer.hpp"
#include "JSystem/JKernel/JKRHeap.hpp"

JKRDisposer::JKRDisposer() : mLink(this) {
    mHeap = JKRHeap::findFromRoot(this);

    if (mHeap) {
        mHeap->appendDisposer(this);
    }
}

JKRDisposer::~JKRDisposer() {
    if (mHeap) {
        mHeap->removeDisposer(this);
    }
}
