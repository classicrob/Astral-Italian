#include "Game/Demo/DemoParamCommonDataTable.hpp"
#include "Game/Util/StringUtil.hpp"

namespace {
    struct DemoParamCommon {
        const char* mDemoName;
        f32 mCameraShakeIntensity;
        f32 mCameraShakeSpeed;
        const char* mPadRumble;
    };

    const DemoParamCommon sDataTable[3] = {
        {"SeaBottomBigDoorA", 0.1f, 2.2f, "中"},
        {"SeaBottomBigDoorB", 0.1f, 2.2f, "中"},
        {"SeaBottomBigDoorC", 0.1f, 2.2f, "中"},
    };

    // Falls back to the first entry when the demo is not in the table.
    const DemoParamCommon* getParam(const char* pDemoName) {
        for (u32 i = 0; i < 3; i++) {
            if (MR::isEqualString(pDemoName, sDataTable[i].mDemoName)) {
                return &sDataTable[i];
            }
        }
        return &sDataTable[0];
    }
}  // namespace

namespace MR {
    namespace Demo {
        f32 getCameraShakeIntensity(const char* pDemoName) {
            return getParam(pDemoName)->mCameraShakeIntensity;
        }

        f32 getCameraShakeSpeed(const char* pDemoName) {
            return getParam(pDemoName)->mCameraShakeSpeed;
        }

        const char* getPadRumble(const char* pDemoName) {
            return getParam(pDemoName)->mPadRumble;
        }

        bool isExistCameraShaking(const char* pDemoName) {
            return 0.0f < getParam(pDemoName)->mCameraShakeIntensity;
        }

        bool isExistPadRumble(const char* pDemoName) {
            return getParam(pDemoName)->mPadRumble != nullptr;
        }

        const char* getStartEffect(const char*) {
            return "Open";
        }
    };  // namespace Demo
};  // namespace MR
