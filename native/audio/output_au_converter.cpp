#include "output_au_converter.hpp"
#include <cstring>

namespace PetariNative::AudioOutputAU::Detail {
Converter::~Converter() { if(converter) AudioConverterDispose(converter); }
bool Converter::open(unsigned guestRate, double deviceRate, unsigned planes, unsigned outputFrames) {
    if(converter || (guestRate!=32000 && guestRate!=48000) || !std::isfinite(deviceRate) || deviceRate<8000 ||
        deviceRate>192000 || (planes!=1 && planes!=2) || !outputFrames || outputFrames>16384) return false;
    AudioStreamBasicDescription in{};
    in.mSampleRate=guestRate; in.mFormatID=kAudioFormatLinearPCM;
    in.mFormatFlags=kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked | kAudioFormatFlagsNativeEndian;
    in.mBytesPerPacket=4; in.mFramesPerPacket=1; in.mBytesPerFrame=4; in.mChannelsPerFrame=2; in.mBitsPerChannel=16;
    AudioStreamBasicDescription out{};
    out.mSampleRate=deviceRate; out.mFormatID=kAudioFormatLinearPCM;
    out.mFormatFlags=kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved | kAudioFormatFlagsNativeEndian;
    out.mBytesPerPacket=4; out.mFramesPerPacket=1; out.mBytesPerFrame=4; out.mChannelsPerFrame=planes; out.mBitsPerChannel=32;
    if((lastError=AudioConverterNew(&in,&out,&converter))) return false;
    // Identity-rate PCM conversion has NO sample-rate converter. Apple returns
    // 'prop' for SRC-only properties there; do not configure an absent stage.
    if(deviceRate!=guestRate) {
        UInt32 quality=kAudioConverterQuality_High, complexity=kAudioConverterSampleRateConverterComplexity_Normal;
        UInt32 method=kConverterPrimeMethod_Normal;
        if((lastError=AudioConverterSetProperty(converter,kAudioConverterSampleRateConverterQuality,sizeof(quality),&quality)) ||
           (lastError=AudioConverterSetProperty(converter,kAudioConverterSampleRateConverterComplexity,sizeof(complexity),&complexity)) ||
           (lastError=AudioConverterSetProperty(converter,kAudioConverterPrimeMethod,sizeof(method),&method))) return false;
        UInt32 bytes=sizeof(prime);
        if((lastError=AudioConverterGetProperty(converter,kAudioConverterPrimeInfo,&bytes,&prime))) return false;
    }
    channels=planes; outputLimit=outputFrames;
    converted.reset(new float[std::size_t(outputFrames)*channels]{});
    return true;
}
bool Converter::push(const std::int16_t* frames, unsigned count) {
    if(!converter || !frames || pendingFrames || ending || !count || count>kInputFrames) return false;
    std::memcpy(input[pendingSlot].data(),frames,std::size_t(count)*4);
    pendingFrames=count; return true;
}
bool Converter::prewarm() {
    if(!converter || accepted || produced || pendingFrames) return false;
    Ring sink; if(!sink.configure(16384,0,channels)) return false;
    std::array<std::int16_t,kInputFrames*2> zero{};
    if(!push(zero.data(),kInputFrames) || !convert(sink)) return false;
    for(unsigned n=0;n<16;++n) {
        const auto before=produced;
        if(!convert(sink,!pendingFrames)) return false;
        if(produced==before && !pendingFrames) break;
    }
    if(pendingFrames || (lastError=AudioConverterReset(converter))) return false;
    accepted=produced=0; ending=false;
    return true;
}
OSStatus Converter::supply(AudioConverterRef, UInt32* packets, AudioBufferList* data,
                          AudioStreamPacketDescription**, void* ref) {
    auto& self=*static_cast<Converter*>(ref);
    if(!self.pendingFrames) {
        *packets=0; data->mBuffers[0].mDataByteSize=0;
        return self.ending ? noErr : kNeedInput;
    }
    std::swap(self.heldSlot,self.pendingSlot);
    *packets=self.pendingFrames;
    data->mNumberBuffers=1; data->mBuffers[0].mNumberChannels=2;
    data->mBuffers[0].mData=self.input[self.heldSlot].data();
    data->mBuffers[0].mDataByteSize=self.pendingFrames*4;
    self.accepted+=self.pendingFrames; self.pendingFrames=0;
    return noErr;
}
bool Converter::convert(Ring& ring, bool eof) {
    if(!converter || lastError || (eof && pendingFrames)) return false;
    ending=ending || eof;
    const auto limit=std::min<std::uint64_t>(ring.space(),outputLimit);
    if(!limit) return true;
    struct Buffers { UInt32 count; AudioBuffer b[2]; } buffers{};
    buffers.count=channels;
    for(unsigned c=0;c<channels;++c) buffers.b[c]={1,static_cast<UInt32>(limit*4),converted.get()+c*outputLimit};
    UInt32 count=static_cast<UInt32>(limit);
    const auto result=AudioConverterFillComplexBuffer(converter,supply,this,&count,
        reinterpret_cast<AudioBufferList*>(&buffers),nullptr);
    if(result!=noErr && result!=kNeedInput) { lastError=result; return false; }
    if(count>limit || !ring.write(converted.get(),channels==2 ? converted.get()+outputLimit : nullptr,count)) {
        lastError=kAudio_ParamError; return false;
    }
    produced+=count; return true;
}
}
