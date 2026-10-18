#pragma once

#include "Game/NPC/TalkMessageCtrl.hpp"
#include "Game/NPC/TalkMessageInfo.hpp"

class MessageData;

class TalkMessageHistory {
public:
    void entry(u16);
    bool search(u16) const;

    /* 0x00 */ u16 mHistory[0x10];
    /* 0x20 */ s32 mCount;
};

class TalkNode {
public:
    /* 0x0 */ u16 mNodeType;
    /* 0x2 */ u16 mGroupID;
    /* 0x4 */ u16 mEventType;
    /* 0x6 */ u16 mIndex;
    union {
        /* 0x8 */ u32 mUnknown;
        struct {
            /* 0x8 */ u16 mNextIdx;
            /* 0xA */ u16 mNextGroup;
        };
    };
};

class RecursiveHelper {
public:
    inline bool hasNode(const TalkNode*) const;
    TalkNode* mStack[64];
    s32 mIndex;
};

class MessageNode {
public:
    /* 0x0 */ const char* mMessage;
};

class TalkNodeCtrl {
public:
    TalkNodeCtrl();

    bool createFlowNodeDirect(TalkMessageCtrl*, const JMapInfoIter&, const char*, ActorCameraInfo**);
    void resetFlowNode();
    void resetTempFlowNode();
    void recordTempFlowNode();
    void forwardFlowNode();
    bool isExistNextNode() const;
    bool isNextNodeMessage() const;
    bool isCurrentNodeEvent() const;
    bool isForwardNode() const;
    TalkNode* getNextNode() const;
    TalkNode* getNextNodeBranch() const;
    TalkNode* getCurrentNodeBranch() const;
    TalkNode* getCurrentNodeMessage() const;
    TalkNode* getCurrentNodeEvent() const;
    TalkNode* getNextNodeEvent() const;
    void updateMessage();
    void readMessage();
    const wchar_t* getSubMessage() const;
    void forwardCurrentBranchNode(bool);
    bool createFlowNode(TalkMessageCtrl*, const JMapInfoIter&, const char*, ActorCameraInfo**);
    void initNodeRecursive(TalkMessageCtrl*, const JMapInfoIter&, ActorCameraInfo*, RecursiveHelper*, s32*, s32*);

    /* 0x00 */ MessageData* _0;
    /* 0x04 */ char* mMessageName;
    /* 0x08 */ s32 mCurrentNodeIdx;
    /* 0x0C */ TalkMessageInfo mMessageInfo;
    /* 0x18 */ TalkMessageHistory mHistory;
    /* 0x3C */ TalkNode* _38;
    /* 0x40 */ TalkNode* mCurrentNode;
    /* 0x44 */ TalkNode* mFlowNode;
    /* 0x48 */ s16 mNodeData;
};
