#include "replay.hpp"
#include "trace.hpp"
#include "stage_sync.hpp"
#include "fp_mode.hpp"
#include "platform.hpp"
#include "gx_ledger.hpp"
#include "startup_arrival.hpp"
#include "actor_trace.hpp"
#include <launch_stage.hpp>
#include <petari/headless.h>
#include <petari/app.hpp>
#include <petari/complete_save_paths.hpp>
#include <petari/boot.hpp>
#include <petari/host_allocation.hpp>
#include <petari/floating_point.hpp>
#include <petari/input.hpp>
#include <petari/kpad_replay.hpp>
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
    Driver(H::Replay& replay, H::Trace& trace, H::ActorTraceFiles* actors, std::filesystem::path tracePath, unsigned numerator, unsigned denominator, H::StageSyncOptions sync, bool startupRequested)
        : mReplay(replay),mTrace(trace),mActors(actors),mPath(std::move(tracePath)),mTiming(mPath.string()+".timing.csv"),mClock(numerator,denominator),mSync(std::move(sync)),mStartupRequested(startupRequested),mOwner(std::this_thread::get_id()) {
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
            const auto& input=d.mReplay.consume(channel,capacity);
            for (std::size_t i=0;i<input.samples.size();++i) buffer[i]=H::decodeSample(input.samples[i]);
            if (error) *error=input.error;
            return input.returned;
        } catch (const std::exception& e) { fail(e.what()); }
    }
    void frame() {
        PetariNative::HostAllocationScope host;
        if (std::this_thread::get_id()!=mOwner) throw H::ReplayError("frame seam ran on an unexpected thread");
        if (mAwaitingFrameWait) throw H::ReplayError("game loop advanced without the headless frame wait hook");
        if (mStartupRequested && L::startupStatus()==L::StartupStatus::Failed)
            throw H::ReplayError("startup request failed: "+L::startupError());
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
        mReplay.endFrame();
        const bool wasStarted=mSync.started();
        const auto frame=mSync.select(state,sourceFrame,ticks,viCount,[](std::uint32_t seed) {
            H::setGameRngSeed(seed);
            return H::observeGame();
        });
        if (!wasStarted && mSync.started()) {
            std::ofstream origin(mPath.string()+".sync.json");
            mSync.writeOrigin(origin);
            mReplay.startInputs();
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
        if (mReplay.finished()) {
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
                lock.unlock();
                for (unsigned channel=0;channel<4;++channel)
                    if (const auto* record=mReplay.connection(channel))
                        PetariNative::Input::replayConnection(channel,record->probeError,record->device);
                if (advance) H::advanceDevices(mClock.advance());
                P::VI::signalRetrace();
                lock.lock(); mRequest=false; mChanged.notify_all();
            }
        } catch (const std::exception& e) { fail(e.what()); }
    }
    H::Replay& mReplay; H::Trace& mTrace; H::ActorTraceFiles* mActors; std::filesystem::path mPath; std::ofstream mTiming; H::FrameClock mClock; H::StageSync mSync; std::uint64_t mTraceFrames=0,mSourceFrame=0;
    bool mStartupRequested=false; H::StartupArrival mStartupArrival;
    std::thread::id mOwner; std::mutex mMutex; std::condition_variable mChanged;
    bool mStop=false,mRequest=false,mAdvance=false,mSawFrame=false,mAwaitingFrameWait=false;
    std::thread mThread;
};
Driver* gDriver=nullptr;
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
        bool ftz=true,inputsFromSync=false,actorTrace=false,existingUnlockedUser=false;
        for (int i=1;i<argc;++i) {
            const std::string key=argv[i];
            if (key=="--inputs-from-sync") {
                if (inputsFromSync) throw H::ReplayError("duplicate --inputs-from-sync");
                inputsFromSync=true;continue;
            }
            if (key=="--no-ftz") {ftz=false;continue;}
            if (key=="--actor-trace") {
                if (actorTrace) throw H::ReplayError("duplicate --actor-trace");
                actorTrace=true; continue;
            }
            if (key=="--existing-unlocked-user") {existingUnlockedUser=true;continue;}
            if (key!="--request" && key!="--stage" && key!="--scenario" && key!="--inputs" && key!="--disc" && key!="--user" && key!="--trace" && key!="--epoch-ticks" && key!="--rate-numerator" && key!="--rate-denominator" && key!="--sync-stage" && key!="--sync-scenario" && key!="--sync-rng-seed") throw H::ReplayError("unknown argument: "+key);
            if (i+1==argc || !args.emplace(key,argv[++i]).second) throw H::ReplayError("missing or duplicate argument: "+key);
        }
        for (const char* key:{"--inputs","--disc","--user","--trace","--epoch-ticks"}) if (!args.count(key)) throw H::ReplayError(std::string("required argument: ")+key);
        const auto input=external(args.at("--inputs")),disc=external(args.at("--disc")),user=external(args.at("--user")),path=external(args.at("--trace"));
        for (const auto& output : {path, std::filesystem::path(path.string()+".complete"),std::filesystem::path(path.string()+".timing.csv"),std::filesystem::path(path.string()+".sync.json")})
            if (std::filesystem::exists(output) || std::filesystem::is_symlink(output)) throw H::ReplayError("refusing to overwrite trace artifacts");
        if (existingUnlockedUser && !PetariNative::CompleteSave::isIsolatedUser(user))
            throw H::ReplayError("--existing-unlocked-user requires the separate AstralItalian-unlocked directory");
        if (!existingUnlockedUser && std::filesystem::exists(user) && !std::filesystem::is_empty(user))
            throw H::ReplayError("--user must be a fresh isolated directory");
        std::filesystem::create_directories(user);
        std::ifstream source(input,std::ios::binary); if (!source) throw H::ReplayError("cannot open input tape");
        if (inputsFromSync && !args.count("--sync-stage")) throw H::ReplayError("--inputs-from-sync requires --sync-stage");
        H::Replay replay(source,inputsFromSync);
        const auto epoch=integer(args.at("--epoch-ticks"));
        if (epoch>static_cast<std::uint64_t>(INT64_MAX)) throw H::ReplayError("epoch outside SDK time range");
        auto n=args.count("--rate-numerator")?integer(args.at("--rate-numerator")):60000;
        auto d=args.count("--rate-denominator")?integer(args.at("--rate-denominator")):1001;
        if (!n || !d || n>1000000 || d>1000000) throw H::ReplayError("invalid frame rate");
        H::StageSyncOptions sync;
        if (args.count("--sync-stage")) {
            sync.stage=args.at("--sync-stage");
            if (sync.stage.empty()) throw H::ReplayError("--sync-stage cannot be empty");
        }
        if (args.count("--sync-scenario")) sync.scenario=H::syncInteger(args.at("--sync-scenario"));
        if (args.count("--sync-rng-seed")) sync.seed=H::syncInteger(args.at("--sync-rng-seed"));
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
        Driver driver(replay,trace,actors.get(),path,n,d,std::move(sync),startupRequested); gDriver=&driver;
        gSourceFrame.store(0,std::memory_order_relaxed);
        PetariNative::Input::setClock(PetariNative::Input::Clock::Manual);
        PetariNative::Input::setKpadReplayHooks({Driver::read,&driver});
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
