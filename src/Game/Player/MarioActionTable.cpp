#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MatrixControl.hpp"

namespace {
    const char* sActionModes[] = {"Normal", "Metal", "Fire", "Ice", "Bee", "Hopper", "Teresa", "Foo", "Bone", "Tornado", "Yoshi", "Horror", "Cloud", "Rock"};
}

namespace {
    MatrixValueTable sHeights[] = {
        {"クッパJｒ", 300.0f},
        {"ルイージ", 200.0f},
        {"ピーチ姫", 180.0f},
        {"ペンギンコーチ", 500.0f},
        {"ペンギン仙人", 500.0f},
        {"チコ", 100.0f},
        {"デブチコ", 200.0f},
        {"でかチコ", 500.0f},
        {"キノピオ", 100.0f},
        {"郵便屋さんキノピオ", 100.0f},
        {"銀行屋さんキノピオ", 100.0f},
        {"ペンギン", 160.0f},
        {"ロゼッタ", 240.0f},
        {"ウサギ", 100.0f},
        {"いたずらウサギ", 100.0f},
        {"シャッチー（会話用）", 220.0f},
        {"スーパードリーマー", 320.0f},
        {"", 0.0f},
    };
}

MarioActionTable::MarioActionTable(MarioActor* actor) {
    mActor = actor;
    mHeights = new MatrixValueGetter("キャラの身長", sHeights);
    initActions();
    initAutoBind();
    initEffects();
}


bool MarioActionTable::isActionOk(const char* name) const {
    return mActions->getValue(name, sActionModes[mActor->mPlayerMode]) != 0;
}

u8 MarioActionTable::selectAction(const char* name) const {
    return mActions->getValue(name, sActionModes[mActor->mPlayerMode]);
}

bool MarioActionTable::selectAutoBind(const char* name, u8* value) const {
    return mAutoBind->getValueOrNone(name, sActionModes[mActor->mPlayerMode], value);
}

bool MarioActionTable::testEffect(const char* name, const char* effect) const {
    u8 value;
    if (mEffects->getValueOrNone(name, effect, &value)) return value != 0;
    return false;
}

bool MarioActionTable::testEffectDisabled(const char* name, const char* effect) const {
    u8 value;
    if (mEffects->getValueOrNone(name, effect, &value)) return value == 0;
    return false;
}

f32 MarioActionTable::getFaceLookHeight(const char* name) const {
    f32 value;
    return mHeights->getValue(name, &value) ? value : 150.0f;
}
