#include "Game/System/ResourceCsvGroupHolder.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JKernel/JKRArchive.hpp>
namespace {
    const char* sActionSoundArchive = "/AudioRes/Info/ActionSound.arc";
    const char* sActionSoundCsv = "ActionSound.bcsv";
    const char* sEffectArchive = "/ParticleData/Effect.arc";
    const char* sAutoEffectCsv = "AutoEffectList.bcsv";
}
ResourceCsvGroupInfo::ResourceCsvGroupInfo():mName(NULL),mCount(0),mStartIndex(-1),_C(0),_10(-1){}
ResourceCsvGroupHolder::ResourceCsvGroupHolder():mCount(0),_4(0x500),mGroups(NULL),mActionSoundCsv(NULL),mAutoEffectCsv(NULL){
    mGroups=new ResourceCsvGroupInfo[_4];
    mActionSoundCsv=new JMapInfo();
    mAutoEffectCsv=new JMapInfo();
    reset();
}
// NONMATCHING: register allocation in the reset loop.
void ResourceCsvGroupHolder::reset(){
    for(s32 i=0;i<mCount;++i){ResourceCsvGroupInfo* group=&mGroups[i];group->mCount=0;group->mStartIndex=-1;group->_C=0;group->_10=-1;}
    mCount=0;
    initActionSoundGroups();
    initAutoEffectGroups();
}
const char* getEffectArchivePath(){
    // Distinct storage from the archive path pointer's literal, as in the target.
    static char path[]="/ParticleData/Effect.arc";
    return path;
}
// NONMATCHING: register allocation in the linear search.
const ResourceCsvGroupInfo* ResourceCsvGroupHolder::fn_804CC650(const char* name) const {
    for(s32 i=0;i<mCount;++i){const ResourceCsvGroupInfo* group=&mGroups[i];if(MR::isEqualStringCase(group->mName,name))return group;}
    return NULL;
}
// NONMATCHING: equivalent grouping branches and register allocation; savegpr24 rather than25.
void ResourceCsvGroupHolder::initActionSoundGroups(){
    mActionSoundCsv->attach(MR::receiveArchive(sActionSoundArchive)->getResource(sActionSoundCsv));
    s32 count=MR::getCsvDataElementNum(mActionSoundCsv);
    for(s32 i=0;i<count;++i){
        const char* name=NULL;
        MR::getCsvDataStr(&name,mActionSoundCsv,"ObjName",i);
        if(name[0]=='\0')continue;
        ResourceCsvGroupInfo* group=NULL;
        if(mCount>0){ResourceCsvGroupInfo* prev=&mGroups[mCount-1];if(MR::isEqualStringCase(prev->mName,name))group=prev;}
        if(group==NULL){group=&mGroups[mCount];group->mName=name;group->mCount=1;++mCount;}
        else ++group->mCount;
        if(group->mCount==1)group->mStartIndex=i;
    }
}
// NONMATCHING: equivalent grouping branch arrangement.
void ResourceCsvGroupHolder::initAutoEffectGroups(){
    mAutoEffectCsv->attach(MR::receiveArchive(sEffectArchive)->getResource(sAutoEffectCsv));
    s32 count=MR::getCsvDataElementNum(mAutoEffectCsv);
    for(s32 i=0;i<count;++i){
        const char* name=NULL;
        MR::getCsvDataStr(&name,mAutoEffectCsv,"GroupName",i);
        if(name[0]=='\0')continue;
        ResourceCsvGroupInfo* group=const_cast<ResourceCsvGroupInfo*>(fn_804CC650(name));
        if(group==NULL){group=&mGroups[mCount];group->mName=name;group->_C=1;++mCount;}
        else ++group->_C;
        if(group->_C==1)group->_10=i;
    }
}
