#pragma once

#include <revolution/types.h>

class JMapInfo;
class TalkMessageInfo;
class TalkNode;
struct MsbtInfo;
struct MsbfInfo;

struct MessageInfoBlock {
    /* 0x0 */ u32 mMagic;
    /* 0x4 */ u32 mBlockSize;
    /* 0x8 */ u16 mItemCount;
    /* 0xA */ u16 mItemSize;
    /* 0xC */ u32 _C;
};

struct MessageDataBlock {
    /* 0x0 */ u32 mMagic;
    /* 0x4 */ u32 mBlockSize;
};

struct MessageFlowBlock {
    /* 0x0 */ u32 mMagic;
    /* 0x4 */ u32 mBlockSize;
    /* 0x8 */ u16 mNodeCount;
    /* 0xA */ u16 _A;
    /* 0xC */ u32 _C;
};

struct MessageFLI1Block {
    /* 0x0 */ u32 mMagic;
    /* 0x4 */ u32 mBlockSize;
};

// SMG2: a MessageData wraps two LMS handles (message table + flowchart), see MarioFaceShipEventDataHolder.cpp
class MessageData {
public:
    MessageData(const char* pArchivePath, const char* pFileName);  // SMG2 fn_804C2860

    bool getMessageDirect(TalkMessageInfo*, const char*) const;     // SMG2 fn_804C2920
    bool getMessage(TalkMessageInfo*, u16, u16) const;
    bool getMessageByIndex(TalkMessageInfo*, s32) const;            // SMG2 fn_804C29E0
    s32 findEntryNodeIndex(const char*) const;                      // SMG2 fn_804C2A80
    void* getNodeBranchPtr(u32) const;                              // SMG2 fn_804C2AA0
    void* getNodeDataPtr(u32) const;                                // SMG2 fn_804C2AC0
    u16 getFlowNodeIndex(const TalkNode*) const;                                   // SMG2 fn_804C2AE0
    TalkNode* findNode(const char*) const;
    TalkNode* getNode(u32) const;
    TalkNode* getBranchNode(u32) const;
    bool isValidBranchNode(u32) const;
    u8* getMessageInfoTool(int) const;
    s32 findMessageIndex(const char*) const;

    /* 0x00 */ MsbtInfo* mMessageHandle;
    /* 0x04 */ MsbfInfo* mFlowHandle;
};

class MessageHolder {
public:
    MessageHolder();  // SMG2 fn_804C2B20

    void initSceneData();
    void destroySceneData();
    void initSystemData();
    void initGameData();

    /* 0x00 */ MessageData* mSystemMessageData;
    /* 0x04 */ MessageData** mGameMessageDatas;
    /* 0x08 */ MessageData** mSceneCommonMessageDatas;
    /* 0x0C */ MessageData** mZoneMessageDatas;
    /* 0x10 */ s32 mGameMessageNum;
    /* 0x14 */ s32 mSceneCommonMessageNum;
};

class MessageSystem {
public:
    class Node {};

    struct FlowNodeBranch {};

    struct FlowNodeEvent {
        /* 0x00 */ u8 mFlowType;
        /* 0x01 */ u8 mEventType;
        /* 0x02 */ u16 mBranchID;
        /* 0x04 */ u32 mArg;
    };

    static MessageData* getSceneMessageData(const char*, s32);
    static bool isExistGameMessage(const char*);
    static bool isExistSceneMessage(const char*);
    static bool isExistMessage(const char*, const MessageData*);
    static TalkNode* findNode(const char*, const MessageData*);
    static TalkNode* getNode(u32, const MessageData*);
    static bool getMessageDirect(TalkMessageInfo*, const char*, const MessageData*);
    static bool getMessage(TalkMessageInfo*, const TalkNode*, const MessageData*);
    static TalkNode* getBranchTrueNode(const TalkNode*, const MessageData*);
    static TalkNode* getBranchFalseNode(const TalkNode*, const MessageData*);
    static bool getSystemMessageDirect(TalkMessageInfo*, const char*);
    static bool getGameMessageDirect(TalkMessageInfo*, const char*);
    static bool getLayoutMessageDirect(TalkMessageInfo*, const char*);
    static MessageData* getSceneMessageData();
};
