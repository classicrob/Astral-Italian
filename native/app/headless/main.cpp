#include "replay.hpp"
#include "keyboard.hpp"
#include <petari/mods.hpp>
#include "trace.hpp"
#include "stage_sync.hpp"
#include "reload_policy.hpp"
#include "fp_mode.hpp"
#include "platform.hpp"
#include "gx_ledger.hpp"
#include "startup_arrival.hpp"
#include "actor_trace.hpp"
#include <launch_stage.hpp>
#include <host.hpp>
#include <petari/headless.h>
#include <petari/app.hpp>
#include <petari/complete_save_paths.hpp>
#include <petari/boot.hpp>
#include <petari/host_allocation.hpp>
#include <petari/floating_point.hpp>
#include <petari/input.hpp>
#include <petari/kpad_replay.hpp>
#include <petari/mods.hpp>
#include <map>
#include <sstream>
#include <petari/platform/virtual_time.hpp>
#include <petari/platform/vi.hpp>
#include <petari/platform/power.hpp>
#include <petari/platform/os_host.hpp>
#include <revolution/dvd.h>
#include <revolution/nand.h>
#include <revolution/vi.h>
#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <cstdio>
#include <clocale>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>
#include <limits>

extern "C" void petari_game_main(void);
namespace H=PetariNative::Headless;
namespace P=PetariNative::Platform;
namespace L=PetariNative::App::LaunchStage;
namespace {
[[noreturn]] void fail(const char* message) { std::fprintf(stderr,"headless: %s\n",message); std::fflush(nullptr); std::_Exit(2); }
std::atomic<std::uint64_t> gSourceFrame{UINT64_MAX};
class Driver {
public:
    Driver(H::Replay* replay, H::KeyboardScript* keyboard, H::Trace& trace, H::ActorTraceFiles* actors, std::filesystem::path tracePath, unsigned numerator, unsigned denominator, H::StageSyncOptions sync, bool startupRequested)
        : mReplay(replay),mKeyboard(keyboard),mTrace(trace),mActors(actors),mPath(std::move(tracePath)),mTiming(mPath.string()+".timing.csv"),mClock(numerator,denominator),mSync(std::move(sync)),mStartupRequested(startupRequested),mOwner(std::this_thread::get_id()) {
        mTiming << "frame,vi_count,ticks\n";
        if (!mTiming) throw H::ReplayError("cannot create timing sidecar");
        mThread=std::thread([this]{worker();});
    }
    ~Driver() { { std::lock_guard<std::mutex> lock(mMutex); mStop=true; } mChanged.notify_all(); mThread.join(); }
    static s32 read(s32 channel,KPADStatus* buffer,u32 capacity,s32* error,void* user) {
        try {
            auto& d=*static_cast<Driver*>(user);
            if (std::this_thread::get_id()!=d.mOwner || !buffer) throw H::ReplayError("unexpected KPAD reader thread or null buffer");
            PetariNative::HostAllocationScope host;
            const auto& input=d.mReplay->consume(channel,capacity);
            for (std::size_t i=0;i<input.samples.size();++i) buffer[i]=H::decodeSample(input.samples[i]);
            if (error) *error=input.error;
            return input.returned;
        } catch (const std::exception& e) { fail(e.what()); }
    }
    std::uint64_t tapeFrame() const { return mReplay.waitingForSync() ? UINT64_MAX : mReplay.frame(); }
    void frame() {
        if (std::this_thread::get_id()!=mOwner) throw H::ReplayError("frame seam ran on an unexpected thread");
        L::testReturnToShipFrame();
        PetariNative::HostAllocationScope host;
        if (mAwaitingFrameWait) throw H::ReplayError("game loop advanced without the headless frame wait hook");
        if (mStartupRequested && L::startupStatus()==L::StartupStatus::Failed)
            throw H::ReplayError("startup request failed: "+L::startupError());
        // The game's "ready for play" at this seam, read before this seam's update: the same ordering as the GX app's tape recorder.
        const bool playReady=PetariNative::App::Host::playerInPlay();
        PetariNative::App::Host::updatePlayerInPlay();
        auto state=H::observeGame();
        if (mStartupRequested && L::startupStatus()==L::StartupStatus::Applied) {
            const auto* selected=L::resolvedStartupRequest();
            const int placed=L::startupPlacedScenario();
            if (!selected || placed<=0) throw H::ReplayError("applied startup request has no resolved placed target");
            const bool alreadyObserved=mStartupArrival.observed();
            if (mStartupArrival.observe(state,selected->stage,placed) && !alreadyObserved)
                std::fprintf(stderr,"headless: startup target observed at frame %llu: %s selected %d placed %d\n",
                    static_cast<unsigned long long>(mSourceFrame),selected->stage.c_str(),selected->scenario,placed);
        }
        const auto sourceFrame=mSourceFrame++;
        const auto ticks=OSGetTime();
        if (ticks<0) throw H::ReplayError("negative SDK clock at frame seam");
        const auto viCount=VIGetRetraceCount();
        if (mReplay) mReplay->endFrame();
        const bool keyboardDone = mKeyboard && (!mStartupRequested || L::startupStatus()==L::StartupStatus::Applied) && mKeyboard->step(state);
        if (mKeyboard && mSourceFrame > 10000) throw H::ReplayError("keyboard script did not finish within 10000 loops");
        const bool wasStarted=mSync.started();
        mSync.setPlayReady(playReady);
        const auto frame=mSync.select(state,sourceFrame,ticks,viCount,[](std::uint32_t seed) {
            H::setGameRngSeed(seed);
            return H::observeGame();
        });
        const bool ready=mSync.enabled() ? mSync.started() :
            state.fields.count("scene") && state.fields.at("scene")=="Game" &&
            state.fields.count("init_state") && state.fields.at("init_state")=="6";
        mActivateAsync=mReloadPolicy.observe(ready,
            state.fields.count("init_state") && state.fields.at("init_state")=="0");
        if (!wasStarted && mSync.started()) {
            std::ofstream origin(mPath.string()+".sync.json");
            mSync.writeOrigin(origin);
            mReplay->startInputs();
        }
        if (frame) {
            mTiming << *frame << ',' << viCount << ',' << ticks << '\n';
            mTiming.flush();
            if (!mTiming) throw H::ReplayError("cannot write timing sidecar");
            mTrace.write(*frame,state);
            if (mActors) mActors->write(*frame,state,H::observeActors(state));
            ++mTraceFrames;
        }
        mSawFrame=true; mAwaitingFrameWait=true;
        if (keyboardDone || (mReplay && mReplay->finished())) {
            if (mStartupRequested && L::startupStatus()!=L::StartupStatus::Applied)
                throw H::ReplayError("startup request did not reach Applied before tape completion");
            if (mStartupRequested && !mStartupArrival.observed())
                throw H::ReplayError("requested stage/scenario was not observed ready before tape completion");
            if (mSync.enabled() && !mSync.started()) throw H::ReplayError("input tape ended before the stage-sync target was reached");
            mTrace.finish();
            if (mActors) mActors->finish(); // Both sidecars must publish before main completion.
            std::ofstream marker(mPath.string()+".complete");
            marker << mTraceFrames << '\n'; marker.flush();
            if (!marker) throw H::ReplayError("cannot write completion marker");
            std::fprintf(stderr,"headless: completed %llu game loops, %llu trace rows\n",static_cast<unsigned long long>(mSourceFrame),static_cast<unsigned long long>(mTraceFrames));
            const auto gx=H::gxStats();
            std::fprintf(stderr,"headless GX: %llu direct FIFO bytes, %llu discarded state calls, %llu display-list payload bytes\n",
                         static_cast<unsigned long long>(gx.fifoBytes),static_cast<unsigned long long>(gx.stateCalls),
                         static_cast<unsigned long long>(gx.displayListBytes));
            std::fflush(nullptr); std::_Exit(0);
        }
    }
    void wait(bool frame) {
        if (std::this_thread::get_id()!=mOwner) throw H::ReplayError("retrace wait ran on an unexpected thread");
        if (frame && !mAwaitingFrameWait) throw H::ReplayError("extra headless game-loop wait");
        if (frame) gSourceFrame.store(mSourceFrame,std::memory_order_relaxed);
        const int released=petari_os_try_begin_host_blocking();
        if (frame && !released) throw H::ReplayError("game-loop wait requires an interruptible OS thread");
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mAdvance=frame || !mSawFrame; mRequest=true; mChanged.notify_all();
            mChanged.wait(lock,[this]{return !mRequest;});
        }
        if (released) petari_os_end_host_blocking();
        if (frame) mAwaitingFrameWait=false;
    }
    static void viWait(void* user) { try { static_cast<Driver*>(user)->wait(false); } catch (const std::exception& e) { fail(e.what()); } }
private:
    void worker() {
        PetariNative::FloatingPointThreadPolicy::apply();
        try {
            std::unique_lock<std::mutex> lock(mMutex);
            while (true) {
                mChanged.wait(lock,[this]{return mRequest || mStop;});
                if (mStop) return;
                const bool advance=mAdvance;
                const bool activate=mActivateAsync;
                lock.unlock();
                if (activate && !P::OS::virtualAsyncActive()) {
                    P::OS::activateVirtualAsync();
                    std::fprintf(stderr,"headless: scoped ordered async reload policy activated after source frame %llu; DVD latency not emulated\n",
                        static_cast<unsigned long long>(mSourceFrame-1));
                }
                for (unsigned channel=0;mReplay && channel<4;++channel)
                    if (const auto* record=mReplay->connection(channel))
                        PetariNative::Input::replayConnection(channel,record->probeError,record->device);
                P::OS::waitVirtualWork();
                if (advance) H::advanceDevices(mClock.advance());
                P::VI::signalRetrace();
                P::OS::waitVirtualWork();
                lock.lock(); mRequest=false; mChanged.notify_all();
            }
        } catch (const std::exception& e) { fail(e.what()); }
    }
    H::Replay* mReplay; H::KeyboardScript* mKeyboard; H::Trace& mTrace; H::ActorTraceFiles* mActors; std::filesystem::path mPath; std::ofstream mTiming; H::FrameClock mClock; H::StageSync mSync; std::uint64_t mTraceFrames=0,mSourceFrame=0;
    bool mStartupRequested=false; H::StartupArrival mStartupArrival;
    std::thread::id mOwner; std::mutex mMutex; std::condition_variable mChanged;
    bool mStop=false,mRequest=false,mAdvance=false,mSawFrame=false,mAwaitingFrameWait=false;
    H::ReloadPolicy mReloadPolicy;
    bool mActivateAsync=false;
    std::thread mThread;
};
Driver* gDriver=nullptr;
// Mod buttons from <tape>.actions (frame mod press held), rebased to tape frames by tools/port/replay_tape.py.
struct ModActions {
    struct Entry { bool press=false,held=false; };
    std::map<std::pair<std::uint64_t,int>,Entry> entries;
    std::uint64_t consumedFrame=UINT64_MAX; unsigned consumedMask=0;
    static std::uint64_t now() { return gDriver ? gDriver->tapeFrame() : UINT64_MAX; }
    static bool press(int mod,void* user) {
        auto& self=*static_cast<ModActions*>(user); const auto f=now(); if (f==UINT64_MAX) return false;
        const auto it=self.entries.find({f,mod}); if (it==self.entries.end() || !it->second.press) return false;
        if (self.consumedFrame!=f) { self.consumedFrame=f; self.consumedMask=0; }
        if (self.consumedMask>>mod & 1U) return false;
        self.consumedMask|=1U<<mod; return true;
    }
    static bool held(int mod,void* user) {
        auto& self=*static_cast<ModActions*>(user); const auto f=now(); if (f==UINT64_MAX) return false;
        const auto it=self.entries.find({f,mod}); return it!=self.entries.end() && it->second.held;
    }
};
ModActions gModActions;
void loadModActions(const std::filesystem::path& path) {
    std::ifstream in(path); if (!in) throw H::ReplayError("cannot open --mod-actions");
    std::string line;
    while (std::getline(in,line)) {
        if (line.empty() || line[0]=='#') continue;
        std::istringstream words(line); unsigned long long frame; int mod,press,held;
        if (!(words>>frame>>mod>>press>>held) || mod<0 || mod>=16 || (press&~1) || (held&~1)) throw H::ReplayError("bad --mod-actions line: "+line);
        auto& entry=gModActions.entries[{frame,mod}]; entry.press=press; entry.held=held;
    }
    PetariNative::Input::setModReplayHooks({ModActions::press,ModActions::held,&gModActions});
}
std::filesystem::path external(const std::string& value) {
    const auto path=std::filesystem::weakly_canonical(value);
    const auto repo=std::filesystem::weakly_canonical(PETARI_HEADLESS_SOURCE_ROOT);
    const auto relative=path.lexically_relative(repo);
    if (path==repo || (!relative.empty() && *relative.begin()!="..")) throw H::ReplayError("game data, input and traces must be outside the checkout");
    return path;
}
std::uint64_t integer(const std::string& value) {
    if (value.empty() || value[0]=='-') throw H::ReplayError("expected unsigned integer");
    std::size_t used=0; const auto result=std::stoull(value,&used);
    if (used!=value.size()) throw H::ReplayError("invalid integer argument");
    return result;
}
}
extern "C" void petari_host_frame_seam() { try { if (!gDriver) fail("frame seam before harness initialization"); gDriver->frame(); } catch (const std::exception& e) { fail(e.what()); } }
extern "C" void petari_host_frame_mark(int) {}
extern "C" std::uint64_t petari_headless_source_frame() { return gSourceFrame.load(std::memory_order_relaxed); }
extern "C" void petari_headless_wait_frame() { try { gDriver->wait(true); } catch (const std::exception& e) { fail(e.what()); } }
extern "C" void petari_headless_wait_retrace() { Driver::viWait(gDriver); }
int main(int argc,char** argv) {
    try {
        std::setlocale(LC_NUMERIC,"C");
        std::map<std::string,std::string> args;
        bool ftz=true,syncPlay=false,inputsFromSync=false,actorTrace=false,existingUnlockedUser=false;
        for (int i=1;i<argc;++i) {
            const std::string key=argv[i];
            if (key=="--inputs-from-sync") {
                if (inputsFromSync) throw H::ReplayError("duplicate --inputs-from-sync");
                inputsFromSync=true;continue;
            }
            if (key=="--no-ftz") {ftz=false;continue;}
            if (key=="--sync-play") {
                if (syncPlay) throw H::ReplayError("duplicate --sync-play");
                syncPlay=true;continue;
            }
            if (key=="--actor-trace") {
                if (actorTrace) throw H::ReplayError("duplicate --actor-trace");
                actorTrace=true; continue;
            }
            if (key=="--existing-unlocked-user") {existingUnlockedUser=true;continue;}
            if (key!="--request" && key!="--stage" && key!="--scenario" && key!="--mod-actions" && key!="--inputs" && key!="--keyboard-script" && key!="--disc" && key!="--user" && key!="--trace" && key!="--epoch-ticks" && key!="--rate-numerator" && key!="--rate-denominator" && key!="--sync-stage" && key!="--sync-scenario" && key!="--sync-rng-seed") throw H::ReplayError("unknown argument: "+key);
            if (i+1==argc || !args.emplace(key,argv[++i]).second) throw H::ReplayError("missing or duplicate argument: "+key);
        }
        for (const char* key:{"--disc","--user","--trace","--epoch-ticks"}) if (!args.count(key)) throw H::ReplayError(std::string("required argument: ")+key);
        if (args.count("--inputs") == args.count("--keyboard-script")) throw H::ReplayError("choose --inputs or --keyboard-script");
        if (args.count("--keyboard-script") && (inputsFromSync || args.count("--sync-stage"))) throw H::ReplayError("keyboard scripts start after steady play; stage-sync is for KPAD replay");
        const auto disc=external(args.at("--disc")),user=external(args.at("--user")),path=external(args.at("--trace"));
        for (const auto& output : {path, std::filesystem::path(path.string()+".complete"),std::filesystem::path(path.string()+".timing.csv"),std::filesystem::path(path.string()+".sync.json")})
            if (std::filesystem::exists(output) || std::filesystem::is_symlink(output)) throw H::ReplayError("refusing to overwrite trace artifacts");
        if (existingUnlockedUser && !PetariNative::CompleteSave::isIsolatedUser(user))
            throw H::ReplayError("--existing-unlocked-user requires the separate AstralItalian-unlocked directory");
        if (!existingUnlockedUser && std::filesystem::exists(user) && !std::filesystem::is_empty(user))
            throw H::ReplayError("--user must be a fresh isolated directory");
        std::filesystem::create_directories(user);
        std::unique_ptr<H::Replay> replay;
        std::unique_ptr<H::KeyboardScript> keyboard;
        if (args.count("--inputs")) {
            {   // mods exactly as the GX app: <user>/mods.txt, then PETARI_MODS / PETARI_ODYSSEY_MOVEMENT (a fresh user: defaults)
                std::string modError;
                if (!PetariNative::Mods::load(user/"mods.txt",&modError)) throw H::ReplayError("mods: "+modError);
                for (int m=0;m<static_cast<int>(PetariNative::Mods::Mod::Count);++m)
                    if (PetariNative::Mods::enabled(static_cast<PetariNative::Mods::Mod>(m)))
                        std::fprintf(stderr,"PETARI MODS: %s on\n",PetariNative::Mods::name(static_cast<PetariNative::Mods::Mod>(m)));
            }
            if (args.count("--mod-actions")) loadModActions(external(args.at("--mod-actions")));
            std::ifstream source(external(args.at("--inputs")),std::ios::binary);
            if (!source) throw H::ReplayError("cannot open input tape");
            replay=std::make_unique<H::Replay>(source,inputsFromSync);
        } else {
            keyboard=std::make_unique<H::KeyboardScript>(external(args.at("--keyboard-script")).string());
            std::string error;
            if (!PetariNative::Mods::load(user / "mods.txt", &error)) throw H::ReplayError(error);
        }
        if (inputsFromSync && !args.count("--sync-stage")) throw H::ReplayError("--inputs-from-sync requires --sync-stage");
        const auto epoch=integer(args.at("--epoch-ticks"));
        if (epoch>static_cast<std::uint64_t>(INT64_MAX)) throw H::ReplayError("epoch outside SDK time range");
        auto n=args.count("--rate-numerator")?integer(args.at("--rate-numerator")):60000;
        auto d=args.count("--rate-denominator")?integer(args.at("--rate-denominator")):1001;
        if (!n || !d || n>1000000 || d>1000000) throw H::ReplayError("invalid frame rate");
        H::StageSyncOptions sync;
        sync.play=syncPlay;
        if (args.count("--sync-stage")) {
            sync.stage=args.at("--sync-stage");
            if (sync.stage.empty()) throw H::ReplayError("--sync-stage cannot be empty");
        }
        if (args.count("--sync-scenario")) sync.scenario=H::syncInteger(args.at("--sync-scenario"));
        if (args.count("--sync-rng-seed")) sync.seed=H::syncInteger(args.at("--sync-rng-seed"));
        if (syncPlay && !args.count("--sync-stage")) throw H::ReplayError("--sync-play requires --sync-stage");
        if (!args.count("--sync-stage") && args.count("--sync-scenario")) throw H::ReplayError("--sync-scenario requires --sync-stage");
        H::StageSync validateSync(sync);
        H::FloatingPointMode fp(ftz);
        PetariNative::FloatingPointThreadPolicy threadFp(ftz);
        if (args.count("--request") && args.count("--stage")) throw H::ReplayError("choose --request or --stage");
        if (args.count("--scenario") && !args.count("--stage")) throw H::ReplayError("--scenario requires --stage");
        const bool startupRequested=args.count("--request") || args.count("--stage");
        if (startupRequested) {
            std::string error;
            bool configured=false;
            if (args.count("--request")) configured=L::configureStartupRequest(args.at("--request"),&error);
            else {
                const auto scenario=args.count("--scenario")?H::syncInteger(args.at("--scenario")):1;
                if (!scenario || scenario>INT32_MAX) throw H::ReplayError("--scenario must be a positive selected ScenarioNo");
                configured=L::configureStartupRequest(args.at("--stage"),static_cast<int>(scenario),&error);
            }
            if (!configured || !L::configureCatalog(disc,&error)) throw H::ReplayError("startup request: "+error);
        }
        std::fprintf(stderr,"headless: rate %llu/%llu Hz, epoch %llu ticks, FPCR.FZ=%d\n",
                     static_cast<unsigned long long>(n),static_cast<unsigned long long>(d),
                     static_cast<unsigned long long>(epoch),H::FloatingPointMode::flushToZero());
        P::OS::enableVirtualTime(static_cast<std::int64_t>(epoch));
        std::ofstream output(path); if (!output) throw H::ReplayError("cannot create trace");
        H::Trace trace(output);
        std::unique_ptr<H::ActorTraceFiles> actors;
        if (actorTrace) actors=std::make_unique<H::ActorTraceFiles>(path);
        Driver driver(replay.get(),keyboard.get(),trace,actors.get(),path,n,d,std::move(sync),startupRequested); gDriver=&driver;
        gSourceFrame.store(0,std::memory_order_relaxed);
        PetariNative::Input::setClock(keyboard ? PetariNative::Input::Clock::Alarm : PetariNative::Input::Clock::Manual);
        if (replay) PetariNative::Input::setKpadReplayHooks({Driver::read,&driver});
        P::VI::setClock(P::VI::Clock::External); P::VI::setWaitDriver({Driver::viWait,&driver});
        H::configurePlatform(disc.c_str(),user.c_str());
        if (petari_kpad_replay_hook_version()!=1) throw H::ReplayError("incompatible KPAD source hook");
        OSInit(); DVDInit();
        if (NANDInit()!=NAND_RESULT_OK) throw H::ReplayError("isolated NAND initialization failed");
        P::Power::setExitHandler([](const P::Power::Exit&,void*){fail("game exited before the replay completed");},nullptr);
        petari_game_main();
        throw H::ReplayError("game main returned before the replay completed");
    } catch (const std::exception& error) { std::fprintf(stderr,"headless: %s\n",error.what()); return 2; }
}
