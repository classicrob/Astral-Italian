// Standalone native regression: compile with the game's native include context.
// This uses the canonical pool/storage type; it does not boot the game.
#ifndef GALAXY_NATIVE
#error This regression requires GALAXY_NATIVE.
#endif
#include "JSystem/JAudio2/JASTaskThread.hpp"
#include <cstddef>
#include <cstdio>

int main() {
    using Pool = JASTaskThread::ThreadMemPool;
    using Chunk = Pool::MemoryChunk;
    constexpr unsigned capacity = 1024;
    Chunk chunk(nullptr);
    const auto* bytes = reinterpret_cast<const u8*>(&chunk);
    const std::size_t offset = offsetof(Chunk, mBuffer);
    unsigned admittedHeaderBytes = 0;
    unsigned rejectedPayloadBytes = 0;
    unsigned rejectedAllocations = 0;
    unsigned freeCounterFailures = 0;
    unsigned allocationCases = 0;

    for (std::size_t i = 0; i < offset; ++i) {
        admittedHeaderBytes += chunk.checkArea(bytes + i);
    }
    for (unsigned i = 0; i < capacity; ++i) {
        rejectedPayloadBytes += !chunk.checkArea(chunk.mBuffer + i);
    }
    const bool endRejected = !chunk.checkArea(chunk.mBuffer + capacity);

    // Pool::free searches separately allocated chunks. Pointer ordering across
    // those allocations must be an address comparison, not an array relation.
    Chunk* older = new Chunk(nullptr);
    Chunk* newer = new Chunk(older);
    void* first = older->alloc(capacity - 1);
    void* tail = older->alloc(1);
    void* newest = newer->alloc(1);
    const bool foreignRejected = !older->checkArea(newest) && !newer->checkArea(first);
    const bool nullRejected = !older->checkArea(nullptr) && !newer->checkArea(nullptr);
    const bool linkedEndsRejected = !older->checkArea(older->mBuffer + capacity) &&
                                    !newer->checkArea(newer->mBuffer + capacity);
    void* allocations[] = {tail, first, newest};
    Chunk* owners[] = {older, older, newer};
    unsigned linkedLookupFailures = 0;
    for (unsigned i = 0; i < 3; ++i) {
        Chunk* found = newer;
        while (found != nullptr && !found->checkArea(allocations[i])) {
            found = found->getNextChunk();
        }
        linkedLookupFailures += found != owners[i];
        if (found != nullptr) {
            found->free(allocations[i]);
        }
    }
    // Keep ownership in the fixture; only the real chunk search/free methods
    // above run here, not the pool's allocator, mutex or empty-chunk deletion.
    const bool linkedCountersBalanced = older->isEmpty() && newer->isEmpty();
    delete newer;
    delete older;

    // Every positive allocation allowed by the pool's capacity check, including
    // starts in the final four bytes, must be recognized by the free predicate.
    for (unsigned prefix = 0; prefix < capacity; ++prefix) {
        for (unsigned size = 1; size <= capacity - prefix; ++size) {
            Chunk storage(nullptr);
            void* previous = prefix ? storage.alloc(prefix) : nullptr;
            void* allocated = storage.alloc(size);
            rejectedAllocations += !storage.checkArea(allocated);
            storage.free(allocated);
            if (previous) {
                rejectedAllocations += !storage.checkArea(previous);
                storage.free(previous);
            }
            freeCounterFailures += !storage.isEmpty();
            ++allocationCases;
        }
    }

    // Existing edge case: alloc(0) on a full chunk returns the one-past pointer.
    // Both old and corrected half-open checks reject it. This fix does not
    // change that allocation policy or claim that zero-size result is freeable.
    Chunk full(nullptr);
    full.alloc(capacity);
    void* zeroAtEnd = full.alloc(0);
    const bool zeroFullRejected = zeroAtEnd == full.mBuffer + capacity && !full.checkArea(zeroAtEnd);

    std::printf("{\"pointer_size\":%zu,\"buffer_offset\":%zu,\"chunk_size\":%zu,"
                "\"header_bytes_admitted\":%u,\"payload_bytes_rejected\":%u,"
                "\"allocation_cases\":%u,\"allocation_starts_rejected\":%u,"
                "\"free_counter_failures\":%u,\"end_rejected\":%s,"
                "\"foreign_rejected\":%s,\"null_rejected\":%s,\"linked_ends_rejected\":%s,"
                "\"linked_lookup_failures\":%u,\"linked_counters_balanced\":%s,"
                "\"existing_zero_full_rejected\":%s}\n",
                sizeof(void*), offset, sizeof(Chunk), admittedHeaderBytes, rejectedPayloadBytes,
                allocationCases, rejectedAllocations, freeCounterFailures, endRejected ? "true" : "false",
                foreignRejected ? "true" : "false", nullRejected ? "true" : "false",
                linkedEndsRejected ? "true" : "false", linkedLookupFailures,
                linkedCountersBalanced ? "true" : "false",
                zeroFullRejected ? "true" : "false");
    return admittedHeaderBytes || rejectedPayloadBytes || rejectedAllocations || freeCounterFailures ||
                   !endRejected || !foreignRejected || !nullRejected || !linkedEndsRejected ||
                   linkedLookupFailures || !linkedCountersBalanced || !zeroFullRejected ? 1 : 0;
}
