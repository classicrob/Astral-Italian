#pragma once

#include <revolution/types.h>

class JKRHeap;

namespace MR {
    struct StationedFileInfo;
};  // namespace MR

class StationedArchiveLoader {
public:
    class Condition {
    public:
        virtual ~Condition();
#ifdef GALAXY_NATIVE
        // No base body in the DOL: every condition overrides it.
        virtual bool isExecute(const MR::StationedFileInfo*) const = 0;
#else
        virtual bool isExecute(const MR::StationedFileInfo*) const;
#endif
        virtual JKRHeap* getProperHeap(const MR::StationedFileInfo*) const;
    };

    static JKRHeap* getProperHeap(const MR::StationedFileInfo*);
    static void loadResourcesFromTable(const StationedArchiveLoader::Condition&);
    static void createAndAddResourcesFromTable(const StationedArchiveLoader::Condition&);
    static bool createAndAddResourcesFromTable(const StationedArchiveLoader::Condition&, s32, s32);
    static void loadScenarioData(JKRHeap*);
};
