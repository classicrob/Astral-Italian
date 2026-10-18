#include "Game/Player/RaceDataReader.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Player/GhostPacketData.hpp"
#include "Game/Player/GhostPacketReader.hpp"
#include "Game/Player/GhostPlayer.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/MtxUtil.hpp"

RaceDataReader::RaceDataReader(LiveActor* host, const char* name, const XanimeResourceTable* table)
    : mHost(host), mPacketReader(nullptr), mXanimePlayer(nullptr), mGhostXanimePlayer(nullptr), mHasAnimation(false) {
    if (table != nullptr) {
        mHasAnimation = true;
        init(table);
        MR::makeMtxTR(mBaseMtx, mHost->mPosition, mHost->mRotation);
    }
    mPacketReader = MR::createGhostPacketReader(name);
}

s32 RaceDataReader::receivePacket() {
    GhostPacketData packet;
    if (!mPacketReader->read(&packet))
        return 1;
    if (packet.testPosition())
        mHost->mPosition.set(packet.mPosition);
    if (packet.testRotationX())
        mHost->mRotation.x = packet.mRotation.x;
    if (packet.testRotationY())
        mHost->mRotation.y = packet.mRotation.y;
    if (packet.testRotationZ())
        mHost->mRotation.z = packet.mRotation.z;
    if (packet.testScale())
        mHost->mScale.set(packet.mScale);
    if (packet.testVelocity())
        mHost->mVelocity.set(packet.mVelocity);
    if (packet.testActionName())
        startActionName(packet.mActionName);
    if (packet.testActionHash())
        startActionHash(packet.mActionHash);
    if (packet.testBckFrame())
        setBckFrame(packet.mBckFrame);
    if (packet.testBckRate())
        setBckRate(packet.mBckRate);
    for (u32 i = 0; i < 4; i++) {
        if (packet.testTrackWeight(i))
            setTrackWeight(i, packet.mTrackWeights[i]);
    }
    if (packet.testTRMtx())
        setMtxTR();
    return 0;
}

void RaceDataReader::init(const XanimeResourceTable* table) {
    mXanimePlayer = MR::fn_8037FF80(mHost, const_cast<XanimeResourceTable*>(table));
    mGhostXanimePlayer = MR::fn_80380030(mHost, mXanimePlayer, const_cast<XanimeResourceTable*>(table));
}

void RaceDataReader::startActionName(const char* name) {
    if (!mHasAnimation)
        MR::startAction(mHost, name);
    else
        MR::fn_803800C0(mXanimePlayer, name);
}

void RaceDataReader::startActionHash(u32 hash) {
    if (mHasAnimation)
        MR::fn_80380130(mXanimePlayer, mGhostXanimePlayer, mHost, hash, nullptr);
}

void RaceDataReader::setBckFrame(f32 frame) {
    if (mHasAnimation)
        mXanimePlayer->_20->mFrame = frame;
    else
        MR::setBckFrame(mHost, frame);
}

void RaceDataReader::setTrackWeight(u32 index, f32 weight) {
    if (mHasAnimation)
        mXanimePlayer->changeTrackWeight(index, weight);
    else
        MR::setBckTrackWeight(mHost, index, weight);
}

void RaceDataReader::setBckRate(f32 rate) {
    if (mHasAnimation)
        mXanimePlayer->_20->mRate = rate;
    else
        MR::setBckRate(mHost, rate);
}

void RaceDataReader::setMtxTR() {
    if (mHasAnimation)
        MR::makeMtxTR(mBaseMtx, mHost->mPosition, mHost->mRotation);
}
