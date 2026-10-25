#include "Game/Boss/DinoPackunBaby.hpp"
#include "Game/LiveActor/ActorParam.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace {
    void readStateString(const char** pResult, const JMapInfo* pCsv, const char* pStateName, const char* pFieldName) {
        MR::getCsvDataStrByElement(pResult, pCsv, "StateName", pStateName, pFieldName);
        if (*pResult != nullptr && **pResult == '\0') {
            *pResult = nullptr;
        }
    }
    const char* sWalkStateCsvName = "ActorStateWalk";
}

DinoPackunBabyStateParam::DinoPackunBabyStateParam()
    : mActionName(nullptr), mDuration(-1), mChaseMoveParam(nullptr), mEscapeMoveParam(nullptr), mIsBreakCrystal(false) {}

namespace NrvDinoPackunBabyState {
    NEW_NERVE(DinoPackunBabyStateNrvWait, DinoPackunBabyState, Wait);
}

DinoPackunBabyState::DinoPackunBabyState(DinoPackunBaby* pHost, const char* pName)
    : ActorStateBase<DinoPackunBaby>(pName, pHost), mStateName(pName), mParam() {}

// NONMATCHING: parameter-member address caching and register allocation differ.
void DinoPackunBabyState::init() {
    JMapInfo* pCsv = MR::tryCreateCsvParser(MR::getResourceHolder(mHost), "%s.bcsv", sWalkStateCsvName);
    if (pCsv != nullptr) {
        DinoPackunBabyStateParam& param = mParam;
        const char* pChaseParam = nullptr;
        const char* pEscapeParam = nullptr;
        const char* pBreakCrystal = nullptr;
        readStateString(&param.mActionName, pCsv, mStateName, "ActionName");
        const char* durationField = "StateStep";
        MR::getCsvDataS32ByElement(&param.mDuration, pCsv, "StateName", mStateName, durationField);
        readStateString(&pChaseParam, pCsv, mStateName, "ParamWalkToPlayer");
        readStateString(&pEscapeParam, pCsv, mStateName, "ParamEscapeFromPlayer");
        const char* breakField = "IsBreakCrystal";
        MR::getCsvDataStrByElement(&pBreakCrystal, pCsv, "StateName", mStateName, breakField);
        if (pBreakCrystal != nullptr && pBreakCrystal[0] == 'o') {
            param.mIsBreakCrystal = true;
        }
        if (pChaseParam != nullptr) {
            param.mChaseMoveParam = reinterpret_cast<const MR::ActorMoveParam*>(MR::getActorParamMove(mHost, pChaseParam));
        }
        if (pEscapeParam != nullptr) {
            param.mEscapeMoveParam = reinterpret_cast<const MR::ActorMoveParam*>(MR::getActorParamMove(mHost, pEscapeParam));
        }
    }
    initNerve(&NrvDinoPackunBabyState::DinoPackunBabyStateNrvWait::sInstance, 0);
    kill();
}

void DinoPackunBabyState::appear() {
    mIsDead = false;
    setNerve(&NrvDinoPackunBabyState::DinoPackunBabyStateNrvWait::sInstance);
}

void DinoPackunBabyState::exeWait() {
    if (MR::isFirstStep(this) && mParam.mActionName != nullptr) {
        MR::startAction(mHost, mParam.mActionName);
    }
    if (mParam.mChaseMoveParam != nullptr) {
        mHost->chasePlayer(mParam.mChaseMoveParam, mParam.mIsBreakCrystal);
    }
    if (mParam.mEscapeMoveParam != nullptr) {
        mHost->escapeFromPlayer(mParam.mEscapeMoveParam, mParam.mIsBreakCrystal);
    }
    if (mParam.mDuration > 0 && MR::isGreaterEqualStep(this, mParam.mDuration)) {
        kill();
    }
}

DinoPackunBabyState::~DinoPackunBabyState() {}
