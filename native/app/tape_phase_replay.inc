// Schema2 uses the same raw Replay and strict state comparator as schema1,
// but opens each input interval at a game-defined PRE-READ boundary.
void Session::initializePhases(const std::filesystem::path& dir, std::uint64_t rows) {
    mMeta.segments.clear();
    std::uint64_t offset=0;
    std::vector<H::Replay::SegmentRange> ranges;
    for (std::size_t i=0;i<mMeta.inputPhases.size();++i) {
        const auto& phase=mMeta.inputPhases[i];
        const auto first=phase.marker.inputFrame, end=phase.endInputFrame;
        if (end>rows || first>=end) throw H::ReplayError("schema2 phase outside complete route prefix");
        SegmentMeta g; g.startRow=first-1; g.endRow=end-1; g.identityRow=first-1;
        g.stage=phase.marker.stage; g.scenario=phase.marker.scenario;
        g.seed=phase.marker.rng; g.hasSeed=phase.marker.rngObserved;
        mMeta.segments.push_back(g);
        mPlan.push_back({i,first-1,end-1,end-first,offset});
        auto& plan=mPlan.back();
        for (const auto& ready:mMeta.playStarts)
            if (ready.startRow>=first && ready.startRow<end && ready.stage==g.stage && ready.scenario==g.scenario) {
                if (ready.startRow==end-1) throw H::ReplayError("no complete loop after schema2 play-ready");
                plan.hasPlayReady=true; plan.playRow=ready.startRow; plan.playSeed=ready.seed; break;
            }
        SegmentReport r; r.index=static_cast<int>(i); r.stage=g.stage; r.scenario=g.scenario;
        r.sceneStartRow=first-1; r.identityRecorded=first-1;
        r.requiresPlayReady=plan.hasPlayReady; r.rngRecorded=plan.playSeed;
        r.startRow=plan.hasPlayReady ? plan.playRow : first-1;
        r.frames=end-1-r.startRow; mReport.push_back(r);
        ranges.push_back({first-1,end-1}); offset+=end-first;
    }
    mPhaseEvidence.resize(mPlan.size());
    std::ifstream kpd(dir/"route.kpd",std::ios::binary);
    if (!kpd) throw H::ReplayError("cannot open schema2 route.kpd");
    mReplay=std::make_unique<H::Replay>(kpd,true);
    if (mMeta.complete && mReplay->frames()!=mMeta.seams+1)
        throw H::ReplayError("schema2 complete KPD header does not cover exactly the recorded seams plus open loop");
    mReplay->requireNeutralControlsOutside(ranges,rows);
    mReplay->keepSegments(ranges);
    if (std::ifstream actions(dir/"route.actions"); actions) {
        std::string line;
        while (std::getline(actions,line)) {
            if (line.empty() || line[0]=='#') continue;
            unsigned long long frame; int mod,press,held;
            std::istringstream words(line);
            if (!(words>>frame>>mod>>press>>held) || mod<0 || mod>=16) throw H::ReplayError("bad schema2 mod action");
            bool kept=false;
            for (const auto& p:mPlan) if (frame>p.start && frame<=p.end) {
                mActions[{p.offset+frame-(p.start+1),mod}]={press!=0,held!=0}; kept=true;
            }
            if (!kept && frame<rows && (press || held)) throw H::ReplayError("schema2 mod action outside executable input phase");
        }
    }
}

void Session::phaseFailure(const std::string& why) {
    if (mError.empty()) mError=why;
    mDone=true; mPhaseSignal=Step::Kind::GaveUp;
}

Step Session::beforeRead(const TapePhase::State& s) {
    Step step;
    if (mMeta.schema!=2 || !wholeSession() || mDone) return step;
    if (s.inputFrame!=mSeams) { phaseFailure("schema2 pre-read and route clocks disagree"); return {Step::Kind::GaveUp}; }
    if (mActualOrdinal!=0) { phaseFailure("schema2 marker is not before ordinal-zero KPAD read"); return {Step::Kind::GaveUp}; }
    if (mPendingMarkerSeed) { phaseFailure("schema2 marker RNG acknowledgement missing"); return {Step::Kind::GaveUp}; }
    if (mPhaseExitPending) {
        const auto& old=mMeta.inputPhases[mExitPhase];
        auto& proof=mPhaseEvidence[mExitPhase];
        proof.exitChecked=true;
        if (s.kind!="none" && s.key()==old.marker.key() && !s.firstNormal) {
            phaseFailure("schema2 interactive phase outlived its declared read interval"); return {Step::Kind::GaveUp};
        }
        proof.exitPassed=true; mPhaseExitPending=false;
        if (mSegIndex==mPlan.size()) {
            mDone=true; mPhaseSignal=Step::Kind::Finished; return {Step::Kind::Finished};
        }
    }
    if (mSegIndex>=mPlan.size()) return step;
    const auto& phase=mMeta.inputPhases[mSegIndex];
    auto& evidence=mPhaseEvidence[mSegIndex];
    if (!mActive) {
        if (s.kind=="none") return step;
        evidence.markerChecked=true; evidence.actualMarker=s; evidence.rngNatural=s.rng;
        const auto& want=phase.marker;
        if (s.key()!=want.key() || s.firstNormal!=want.firstNormal || s.valid!=want.valid ||
            s.grounded!=want.grounded || s.dead!=want.dead || s.position!=want.position ||
            s.mapWait!=want.mapWait || s.galaxyMap!=want.galaxyMap || s.rngObserved!=want.rngObserved) {
            phaseFailure("schema2 pre-read marker state differs"); return {Step::Kind::GaveUp};
        }
        auto& report=mReport[mSegIndex];
        const auto offset=static_cast<std::int64_t>(s.inputFrame)-static_cast<std::int64_t>(want.inputFrame);
        std::int64_t previousOffset=0;
        if (mSegIndex) {
            const auto& prev=mReport[mSegIndex-1];
            previousOffset=prev.requiresPlayReady ? static_cast<std::int64_t>(prev.replayPlayStart)-static_cast<std::int64_t>(mPlan[mSegIndex-1].playRow)
                                                  : static_cast<std::int64_t>(prev.replaySceneStart)-static_cast<std::int64_t>(prev.sceneStartRow);
        }
        if (std::abs(offset-previousOffset)>kIdentityOffsetLimit) {
            phaseFailure("schema2 scene start offset exceeds 30 loops"); return {Step::Kind::GaveUp};
        }
        report.identityObserved=true; report.identityReplay=s.inputFrame-1;
        report.identityExpected=static_cast<std::int64_t>(want.inputFrame-1)+previousOffset;
        report.identityOffset=offset-previousOffset;
        report.started=true; report.replaySceneStart=s.inputFrame-1;
        mSegRow=1; mWaitSeams=0; mActive=true; mStrict=!mPlan[mSegIndex].hasPlayReady;
        mExpectedPointer=false; mPointerReadMade=false;
        mReplay->startInputs();
        evidence.markerPassed=true;
        if (mStrict) ++report.rowsCompared; // the exact pre-read marker; NOT a fabricated prior route row
        step.kind=Step::Kind::Start; step.restoreSeed=want.rngObserved; step.seed=want.rng;
        mPendingMarkerSeed=want.rngObserved;
        if (!want.rngObserved) evidence.rngVerified=true;
    } else if (s.kind=="none" || s.key()!=phase.marker.key() ||
               (s.firstNormal && s.inputFrame!=evidence.actualMarker.inputFrame)) {
        phaseFailure("schema2 interactive phase ended before its declared read interval"); return {Step::Kind::GaveUp};
    }
    if (!mReplay->waitingForSync()) {
        const auto canonical=mPlan[mSegIndex].start+1+mReplay->frame()-mPlan[mSegIndex].offset;
        bool expectedMap=false,expectedGalaxy=false;
        for (const auto& point:phase.checkpoints) {
            if (point.state.inputFrame!=canonical) continue;
            const auto& want=point.state;
            const bool pendingRestore=mPendingMarkerSeed && canonical==phase.marker.inputFrame;
            if (s.valid!=want.valid || s.grounded!=want.grounded || s.dead!=want.dead || s.position!=want.position ||
                s.rngObserved!=want.rngObserved || (!pendingRestore && want.rngObserved && s.rng!=want.rng) ||
                (point.kind=="map-wait" && !s.mapWait) || (point.kind=="galaxy-map" && !s.galaxyMap)) {
                phaseFailure("schema2 map checkpoint state/RNG differs"); return {Step::Kind::GaveUp};
            }
            expectedMap|=point.kind=="map-wait"; expectedGalaxy|=point.kind=="galaxy-map";
            ++evidence.checkpointsChecked;
        }
        if ((s.mapWait && !evidence.lastMapWait && !expectedMap) ||
            (s.galaxyMap && !evidence.lastGalaxyMap && !expectedGalaxy)) {
            phaseFailure("schema2 map checkpoint occurs at a different relative input loop"); return {Step::Kind::GaveUp};
        }
        evidence.lastMapWait=s.mapWait; evidence.lastGalaxyMap=s.galaxyMap;
    }
    return step;
}

Step Session::phaseSeam(const SeamState& s) {
    ++mSeams; mActualOrdinal=0;
    if (mReadMap.is_open()) mReadMap.flush();
    if (mPhaseSignal!=Step::Kind::None) {
        const auto signal=mPhaseSignal; mPhaseSignal=Step::Kind::None; return {signal};
    }
    if (mDone) return {};
    if (mPendingMarkerSeed || mPendingSeed) {
        phaseFailure("schema2 seam before RNG acknowledgement"); mPhaseSignal=Step::Kind::None; return {Step::Kind::GaveUp};
    }
    try { mReplay->endFrame(); } catch (const std::exception& e) { phaseFailure(e.what()); }
    mLastObserved=s;
    if (!mError.empty()) { mPhaseSignal=Step::Kind::None; return {Step::Kind::GaveUp}; }
    if (!mActive) {
        ++mNeutralLoops;
        if (++mWaitSeams>kWaitSeamLimit) { phaseFailure("schema2 game-defined phase marker not reached"); return {Step::Kind::GaveUp}; }
        return {};
    }
    const auto& plan=mPlan[mSegIndex];
    if (!mStrict) {
        if (s.playReady) return startPlay(s);
        ++mSegRow;
        if (++mWaitSeams>kWaitSeamLimit || s.stage!=mReport[mSegIndex].stage || s.scenario!=mReport[mSegIndex].scenario) {
            phaseFailure("schema2 arrival never reached recorded play-ready"); return {Step::Kind::GaveUp};
        }
        if (mReplay->frame()>=plan.offset+plan.playRow-plan.start) mReplay->pauseInputs();
        return {};
    }
    compare(s,mSegRow); ++mSegRow;
    if (!mError.empty()) { phaseFailure(mError); mPhaseSignal=Step::Kind::None; return {Step::Kind::GaveUp}; }
    if (mReplay->segmentEnded() || mReplay->finished()) {
        auto& e=mPhaseEvidence[mSegIndex];
        if (e.checkpointsChecked!=mMeta.inputPhases[mSegIndex].checkpoints.size()) {
            phaseFailure("schema2 required map checkpoint missing"); return {Step::Kind::GaveUp};
        }
        mReport[mSegIndex].completed=true; mActive=false;
        mExitPhase=mSegIndex;
        mPhaseExitPending=!mMeta.inputPhases[mSegIndex].open;
        if (++mSegIndex==mPlan.size() && !mPhaseExitPending) {
            mDone=true; return {Step::Kind::Finished};
        }
        mWaitSeams=0;
    }
    return {};
}

void Session::recordReadMap(const H::Read& input, bool recorded) {
    if (!mReadMap.is_open()) return;
    const auto canonical=recorded ? mPlan[mSegIndex].start+1+mReplay->frame()-mPlan[mSegIndex].offset : 0;
    mReadMap << "{\"actual_frame\":" << mSeams << ",\"actual_ordinal\":" << mActualOrdinal
             << ",\"source\":" << jsonString(recorded ? "recorded" : "neutral")
             << ",\"canonical_frame\":" << (recorded ? std::to_string(canonical) : "null")
             << ",\"canonical_ordinal\":" << (recorded ? std::to_string(input.ordinal) : "null")
             << ",\"phase\":" << (recorded ? std::to_string(mSegIndex) : "null")
             << ",\"channel\":" << input.channel << ",\"capacity\":" << input.capacity
             << ",\"returned\":" << input.returned << ",\"error\":" << input.error
             << ",\"samples\":" << input.samples.size() << "}\n";
}
