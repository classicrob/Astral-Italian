#include "Game/System/WorldMapFunction.hpp"
#include "Game/Screen/MapGalaxyLayouts.hpp"
#include "Game/System/WorldMapResultLocal.hpp"

WorldMapResultStateCometAppear::WorldMapResultStateCometAppear(WorldMapResultCommand* pHost)
    : ActorStateBase< WorldMapResultCommand >("State[コメット飛来]", pHost), mDone(false), mComet(nullptr), mNames(), mNum(0),
      mCurrent(0), mFrom(0.0f, 0.0f, 0.0f), mTo(0.0f, 0.0f, 100.0f) {
}

// NONMATCHING: regalloc; target retains redundant null check for embedded owned array.
WorldMapResultStateCometAppear::~WorldMapResultStateCometAppear() {
}

void WorldMapResultStateCometAppear::init() {
    mLayout = new MapGalaxyInfo(0x50);
    mLayout->initWithoutIter();
    mLayout->mPlaySound = 0;
    mLayout->kill();
    mComet = new MiniComet("デモ用コメット");
    mComet->initWithoutIter();
    mComet->makeActorDead();
    initNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearWait), 0);

    if (GameSequenceFunction::hasStageResultSequence()) {
        s32 count = 0;
        ScenarioDataIter iter = MR::makeBeginScenarioDataIter();

        while (!iter.isEnd()) {
            GalaxyStatusAccessor accessor = iter.makeAccessor();

            if (GameSequenceFunction::isNewlyOpenedGalaxyAtResult(accessor.getName())) {
                count++;
            }

            iter.goNext();
        }

        mNames.init(count);

        if (count > 0) {
            ScenarioDataIter iter2 = MR::makeBeginScenarioDataIter();

            while (!iter2.isEnd()) {
                GalaxyStatusAccessor accessor = iter2.makeAccessor();
                const char* pName = accessor.getName();

                if (GameSequenceFunction::isNewlyOpenedGalaxyAtResult(pName)) {
                    s32 idx = mNum;
                    mNum = idx + 1;
                    mNames[idx] = pName;
                    mHost->endGalaxyPointingAnim(pName, idx);
                }

                iter2.goNext();
            }
        }
    }

    mIsDead = true;
}

void WorldMapResultStateCometAppear::appear() {
    mComet->kill();
    mIsDead = false;

    if (mHost->isResultActive()) {
        WorldMapResultCommand* pHost = mHost;
        s32 world = WorldMapFunction::getGalaxyWorldNo(getCurrentComet());

        s32 id = pHost->getResultWorldId();

        if (id == world) {
            setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearZoom));
            return;
        }

        setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearClose));
        return;
    }

    setupComet();
    setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearWait));
}

void WorldMapResultStateCometAppear::kill() {
    mLayout->kill();
    mIsDead = true;
}

bool WorldMapResultStateCometAppear::hasComets() const {
    return mNum > 0;
}

const char* WorldMapResultStateCometAppear::getCurrentComet() const {
    return mNames[mCurrent];
}

void WorldMapResultStateCometAppear::setupComet() {
    WorldMapResultCommand* pHost = mHost;
    pHost->setupCometGalaxy(getCurrentComet());
    WorldMapResultCommand* pHost2 = mHost;
    pHost2->startResult(WorldMapFunction::getGalaxyWorldNo(getCurrentComet()));
    WorldMapResultCommand* pHost3 = mHost;
    pHost3->updateCometCamera(getCurrentComet(), &mFrom, &mTo, 1.0f);
}

void WorldMapResultStateCometAppear::exeClose() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeFade(-1);
    }

    if (MR::isWipeBlank()) {
        setupComet();
        setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearOpen));
    }
}

void WorldMapResultStateCometAppear::exeOpen() {
    if (!MR::isLessStep(this, getResultEndStep())) {
        if (MR::isStep(this, getResultEndStep())) {
            MR::openWipeFade(-1);
        }

        if (MR::isWipeOpen()) {
            setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearAppearComet));
        }
    }
}

void WorldMapResultStateCometAppear::exeZoom() {
    if (MR::isFirstStep(this)) {
        mFrom.set(mHost->getCameraTarget());
        mTo.set(mHost->getCameraEye());
    }

    WorldMapResultCommand* pHost = mHost;
    f32 rate = MR::calcNerveRate(this, 20);

    if (pHost->updateCometCamera(getCurrentComet(), &mFrom, &mTo, rate)) {
        if (mDone) {
            setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearMove));
            return;
        }

        setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearAppearComet));
    }
}

void WorldMapResultStateCometAppear::exeAppearComet() {
    if (MR::isFirstStep(this)) {
        WorldMapResultCommand* pHost = mHost;
        pHost->appearCometAtGalaxy(getCurrentComet(), mComet);
    }

    if (MR::isStep(this, 19)) {
        MR::startStageBGM("STM_SMG2_JG_COMETSE", false);
    }

    if (MR::isStep(this, 50)) {
        mDone = true;
        setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearMove));
    }
}

void WorldMapResultStateCometAppear::exeMove() {
    if (MR::isFirstStep(this)) {
        WorldMapResultCommand* pHost = mHost;
        pHost->clearGalaxyComet(getCurrentComet());
    }

    if (MR::isGreaterEqualStep(this, 90)) {
        mLayout->showGalaxy(getCurrentComet(), 4);
    }

    if (MR::isGreaterEqualStep(this, 150)) {
        u32 next = mCurrent + 1;

        if (next < mNum) {
            mCurrent = next;
            WorldMapResultCommand* pHost = mHost;
            s32 world = WorldMapFunction::getGalaxyWorldNo(getCurrentComet());
            s32 id = pHost->getResultWorldId();

            if (id == world) {
                setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearZoom));
                return;
            }

            setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearClose));
            return;
        }

        setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearMessage));
    }
}

void WorldMapResultStateCometAppear::exeMessage() {
    if (MR::isStep(this, 12)) {
        MR::appearInformationMessageDown("System_ResultWorldMapCometAppear", true);
        MR::startSubBGM("BGM_SMG2_JG_COMET", false);
    }

    mLayout->showGalaxy(getCurrentComet(), 4);

    if (!MR::isLessEqualStep(this, 12) && MR::isDeadInformationMessage()) {
        setNerve(GET_NERVE(WorldMapResultStateCometAppear, WorldMapResultStateCometAppearEnd));
    }
}

void WorldMapResultStateCometAppear::exeEnd() {
    mLayout->showGalaxy(getCurrentComet(), 30);

    if (MR::isStep(this, 12)) {
        mIsDead = true;
    }
}

namespace NrvWorldMapResultStateCometAppear {
    WMR_NERVE_DEF(WorldMapResultStateCometAppearWait, WorldMapResultStateCometAppear, Wait);
    WMR_NERVE_DEF(WorldMapResultStateCometAppearClose, WorldMapResultStateCometAppear, Close);
    WMR_NERVE_DEF(WorldMapResultStateCometAppearOpen, WorldMapResultStateCometAppear, Open);
    WMR_NERVE_DEF(WorldMapResultStateCometAppearAppearComet, WorldMapResultStateCometAppear, AppearComet);
    WMR_NERVE_DEF(WorldMapResultStateCometAppearMove, WorldMapResultStateCometAppear, Move);
    WMR_NERVE_DEF(WorldMapResultStateCometAppearZoom, WorldMapResultStateCometAppear, Zoom);
    WMR_NERVE_DEF(WorldMapResultStateCometAppearMessage, WorldMapResultStateCometAppear, Message);
    WMR_NERVE_DEF(WorldMapResultStateCometAppearEnd, WorldMapResultStateCometAppear, End);
};  // namespace NrvWorldMapResultStateCometAppear
