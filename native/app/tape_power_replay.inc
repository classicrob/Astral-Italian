// No host-clock input alignment: events use the existing scene/play-ready
// offset. The real power callback is issued after this seam's exact comparison.
std::int64_t Session::controlOffset(std::size_t phase) const {
    const auto& plan=mPlan.at(phase);const auto& report=mReport.at(phase);
    const auto& event=mMeta.sessionEvents.at(mNextControl<mMeta.sessionEvents.size() ? mNextControl : mMeta.sessionEvents.size()-1);
    if (plan.hasPlayReady && report.playReadyChecked && event.afterRow>=plan.playRow)
        return static_cast<std::int64_t>(report.replayPlayStart)-static_cast<std::int64_t>(plan.playRow);
    return static_cast<std::int64_t>(mPhaseEvidence.at(phase).actualMarker.inputFrame)-
           static_cast<std::int64_t>(mMeta.inputPhases.at(phase).marker.inputFrame);
}
Step Session::controlSeam(const SeamState&) {
    Step step;
    if (mPowerPending) { phaseFailure("power callback missing before next update");return {Step::Kind::GaveUp}; }
    if (mNextControl>=mMeta.sessionEvents.size()) return step;
    const auto& event=mMeta.sessionEvents[mNextControl];
    if (!mPhaseEvidence[event.phase].markerPassed) return step;
    const auto wanted=static_cast<std::int64_t>(event.afterRow)+controlOffset(event.phase);
    const auto actual=static_cast<std::int64_t>(mSeams)-1;
    if (actual<wanted) return step;
    if (actual>wanted) { phaseFailure("missed exact session-control boundary");return {Step::Kind::GaveUp}; }
    if (event.kind=="neutral-tail-begin") {
        mNeutralTailActive=true;mControlActualRows.push_back(mSeams-1);++mNextControl;
        step.neutralTailBegin=true;
    } else {
        mPowerPending=true;mPowerActualRow=mSeams-1;step.powerRequest=true;
    }
    return step;
}
bool Session::beforePowerRequest() {
    if (!mMeta.powerProtocol) return false;
    if (!mError.empty()) return false; // cleanup cannot replace the original failure
    if (!mPowerPending || mPowerCalling || mPowerDelivered || mNextControl>=mMeta.sessionEvents.size() ||
        mMeta.sessionEvents[mNextControl].kind!="power-request" || mPowerActualRow+1!=mSeams) {
        phaseFailure("unexpected, early, late or duplicate power request");return false;
    }
    mPowerCalling=true;return true;
}
bool Session::acknowledgePower(bool actualPowerFlag) {
    if (!mMeta.powerProtocol) return true;
    if (!mPowerPending || !mPowerCalling || mPowerDelivered || !actualPowerFlag) {
        phaseFailure("real power callback acknowledgement missing or ambiguous");return false;
    }
    mPowerPending=false;mPowerCalling=false;mPowerDelivered=true;
    mControlActualRows.push_back(mPowerActualRow);++mNextControl;return true;
}
void Session::rejectEarlyPower() {
    mEarlyPower=true;phaseFailure("recorded power event reached before wall-time observation gate");
}
void Session::compareShutdown(const SeamState& s,std::uint64_t row) {
    if (row>=mMeta.seams || row>=mRoute.size()) { phaseFailure("extra shutdown row beyond recorded prefix");return; }
    const auto& expected=mRoute[row];
    if (s.stage!=expected.stage || s.scenario!=expected.scenario || s.valid!=expected.valid ||
        s.grounded!=expected.grounded || s.dead!=expected.dead ||
        std::memcmp(&s.x,&expected.x,sizeof(float)) || std::memcmp(&s.y,&expected.y,sizeof(float)) ||
        std::memcmp(&s.z,&expected.z,sizeof(float))) {
        phaseFailure("shutdown state/position bits differ at route row "+std::to_string(row));return;
    }
    ++mShutdownRows;
}
void Session::normalExit() {
    if (!mMeta.powerProtocol || !mError.empty()) return;
    const auto offset=controlOffset(mMeta.sessionEvents.back().phase);
    const auto expected=static_cast<std::int64_t>(mMeta.seams)+offset;
    if (!mDone || mPowerPending || !mPowerDelivered || mNextControl!=mMeta.sessionEvents.size() ||
        static_cast<std::int64_t>(mSeams)!=expected ||
        mShutdownRows!=mMeta.seams-mMeta.sessionEvents.back().beforeInputFrame) {
        phaseFailure("normal exit before complete strict power/shutdown replay");return;
    }
    mNormalExitVerified=true;
}

void Session::rejectUnexpectedPower() { phaseFailure("power request outside the recorded game-thread seam"); }
void Session::validateNeutralTail(const H::Read& input) {
    if (!mMeta.powerProtocol || !mNeutralTailActive || mPowerDelivered) return;
    if (mMeta.neutralTailPolicy == TapeNeutral::Policy::NativeControls) {
        mNativeNeutralTail.validate(input);
        return;
    }
    const auto& neutral=mReplay->neutral(input.channel,input.capacity);
    if (input.returned!=neutral.returned || input.error!=neutral.error || input.probeError!=neutral.probeError ||
        input.device!=neutral.device || input.samples!=neutral.samples)
        throw H::ReplayError("non-neutral raw read inside declared neutral observation tail");
}
