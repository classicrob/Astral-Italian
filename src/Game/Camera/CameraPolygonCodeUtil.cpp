#include "Game/Camera/CameraPolygonCodeUtil.hpp"
#include "Game/Camera/CameraDirector.hpp"

namespace MR {
    void initCameraCodeCollection(const char* name, s32 zoneId) {
        getCameraDirector()->initCameraCodeCollection(name, zoneId);
    }

    void registerCameraCode(u32 code) {
        getCameraDirector()->registerCameraCode(code);
    }

    void termCameraCodeCollection() {
        getCameraDirector()->termCameraCodeCollection();
    }
}
