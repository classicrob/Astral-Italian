#include "output_au_render.hpp"
#include <cstring>

namespace PetariNative::AudioOutputAU::Detail {
namespace {
bool validBuffers(AudioBufferList* buffers, unsigned channels, UInt32 frames) noexcept {
    if (!buffers || buffers->mNumberBuffers != channels) return false;
    for (unsigned c=0; c<channels; ++c) {
        const auto& b=buffers->mBuffers[c];
        if (b.mNumberChannels != 1 || (b.mData && b.mDataByteSize < std::uint64_t(frames)*sizeof(float))) return false;
    }
    return true;
}
// Error paths may only zero caller-provided, bounded storage. Never substitute
// generation scratch on a concurrent/reentrant invocation.
void silence(AudioBufferList* buffers, UInt32 frames, AudioUnitRenderActionFlags* flags) noexcept {
    if (buffers && buffers->mNumberBuffers <= 2 && frames <= 16384) {
        for (unsigned c=0;c<buffers->mNumberBuffers;++c) {
            auto& b=buffers->mBuffers[c];
            const auto count=std::min<std::uint64_t>(b.mDataByteSize/sizeof(float),std::uint64_t(frames)*b.mNumberChannels);
            if (b.mData) { auto* out=static_cast<float*>(b.mData); for(std::size_t n=0;n<count;++n) out[n]=0.f; }
        }
    }
    if (flags) *flags |= kAudioUnitRenderAction_OutputIsSilence;
}
void observe(RenderData& d, unsigned kind, std::uint64_t value, std::uint64_t now) noexcept {
    d.publish({now,d.callbackTicket,d.generation,value,kind});
}
}

__attribute__((noinline)) OSStatus render(void* ref, AudioUnitRenderActionFlags* flags,
    const AudioTimeStamp* time, UInt32 bus, UInt32 frames, AudioBufferList* buffers) noexcept {
    auto& owner=*static_cast<RenderOwner*>(ref);
    owner.readers.fetch_add(1,std::memory_order_seq_cst);
    auto* d=owner.published.load(std::memory_order_seq_cst);
    if (!d || !frames) {
        silence(buffers,frames,flags);
        owner.readers.fetch_sub(1,std::memory_order_seq_cst); return noErr;
    }
    if (owner.consumer.test_and_set(std::memory_order_acquire)) {
        owner.faults.fetch_or(Reentrant,std::memory_order_relaxed);
        silence(buffers,frames,flags);
        owner.readers.fetch_sub(1,std::memory_order_seq_cst); return kAudioUnitErr_CannotDoInCurrentContext;
    }
    OSStatus result=noErr;
    if (frames>d->maxFrames || frames>16384 || bus!=0 || !validBuffers(buffers,d->ring.channels(),frames)) {
        d->faults.fetch_or(frames>d->maxFrames ? OverLimit : BadBuffers,std::memory_order_relaxed);
        silence(buffers,frames,flags); result=kAudioUnitErr_TooManyFramesToProcess;
    } else {
        // No trace clock, TLS, publisher or identity work before this cached flag.
        const bool on=d->observing;
        const auto entry=on ? d->clock() : 0;
        ++d->callbackTicket;
        if (on) {
            observe(*d,43,frames,entry); // AUEntry
            observe(*d,45,d->ring.level(),entry); // AURingBefore
            if (d->lastExit && entry>=d->lastExit) {
                const auto gap=entry-d->lastExit;
                if (gap>d->maxGapTicks.load(std::memory_order_relaxed)) d->maxGapTicks.store(gap,std::memory_order_relaxed);
                if (gap>d->gapTriggerTicks) observe(*d,51,gap,entry); // trigger
            }
        }
        if (time && (time->mFlags & kAudioTimeStampSampleTimeValid) && std::isfinite(time->mSampleTime)) {
            if (d->timelineStarted && std::fabs(time->mSampleTime-d->nextSample)>0.25) {
                d->timelineFaults.fetch_add(1,std::memory_order_relaxed);
                d->faults.fetch_or(BadTimeline,std::memory_order_relaxed);
                if (on) observe(*d,51,BadTimeline,entry);
            }
            d->timelineStarted=true; d->nextSample=time->mSampleTime+frames;
            std::uint64_t bits=0; std::memcpy(&bits,&time->mSampleTime,sizeof(bits));
            d->sampleBits.store(bits,std::memory_order_relaxed);
            if (on) observe(*d,48,bits,entry);
        } else {
            d->invalidTimestamps.fetch_add(1,std::memory_order_relaxed);
            d->faults.fetch_or(MissingTimestamp,std::memory_order_relaxed);
        }
        if (time && (time->mFlags & kAudioTimeStampHostTimeValid)) {
            d->hostTime.store(time->mHostTime,std::memory_order_relaxed);
            if (on) observe(*d,49,time->mHostTime,entry);
        }
        if(flags) *flags &= ~kAudioUnitRenderAction_OutputIsSilence;
        for (unsigned c=0;c<d->ring.channels();++c) {
            auto& b=buffers->mBuffers[c];
            if (!b.mData) b.mData=d->scratch.get()+c*d->maxFrames;
            b.mDataByteSize=frames*sizeof(float);
        }
        const auto level=d->ring.level();
        if (level>d->ring.capacity()) d->faults.fetch_or(RingCorrupt,std::memory_order_relaxed);
        const auto got=d->ring.read(static_cast<float*>(buffers->mBuffers[0].mData),
            d->ring.channels()==2 ? static_cast<float*>(buffers->mBuffers[1].mData) : nullptr,frames,d->mute);
        d->requested.fetch_add(frames,std::memory_order_relaxed);
        d->copied.fetch_add(got,std::memory_order_relaxed);
        d->shortfalls.fetch_add(frames-got,std::memory_order_relaxed);
        d->callbacks.fetch_add(1,std::memory_order_release);
        if (d->mute || !got) { if(flags) *flags|=kAudioUnitRenderAction_OutputIsSilence; }
        if (on) {
            const auto exit=d->clock();
            observe(*d,46,d->ring.level(),exit); // AURingAfter
            observe(*d,53,d->ring.consumed(),exit); // cumulative, not reset by a history window
            if (got<frames) observe(*d,47,frames-got,exit); // AUShortfall
            observe(*d,44,got,exit); // AUExit
            d->lastExit=exit;
        }
    }
    owner.consumer.clear(std::memory_order_release);
    owner.readers.fetch_sub(1,std::memory_order_seq_cst);
    return result;
}
}
