#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class GhostPacketHistory;
class InfernoMario;
class InfernoMarioManager : public LiveActor {
public:
    InfernoMarioManager(const char*);
    virtual ~InfernoMarioManager();
    virtual void init(const JMapInfoIter&);
    void exeManage();
    bool add(InfernoMario*);
    bool addFirst(InfernoMario*);
    void remove(InfernoMario*);
    GhostPacketHistory* mHistory;
    InfernoMario** mActors;
    s32 mCount;
    s32 mCapacity;
};
namespace MR {
    void createInfernoMarioManager();
    bool addInfernoMario(InfernoMario*);
    bool addInfernoMarioFirst(InfernoMario*);
    void removeInfernoMario(InfernoMario*);
}
