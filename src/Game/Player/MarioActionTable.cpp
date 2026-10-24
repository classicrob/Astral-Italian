#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MatrixControl.hpp"
#include "Game/Util/HashUtil.hpp"

namespace {
    const char* sActionModes[] = {"Normal", "Metal", "Fire", "Ice", "Bee", "Hopper", "Teresa", "Foo", "Bone", "Tornado", "Yoshi", "Horror", "Cloud", "Rock"};
}

namespace {
    const char* sActions[] = {
        "ジャンプ系バインド",
        "スピンアタック",
        "コイン引っ張り",
        "カメ持ち",
        "水解除",
        "ダメージ解除",
        "ノーダメージ解除",
        "アッパーパンチ",
        "スピン回復エフェクト",
    };
    const char* sAutoBindRows[] = {
        "ウォータープレッシャーの弾",
        "ポール",
        "ポール（モデル無し）",
        "ポール（木Ａ）",
        "ポール（木B）",
        "ポール（鉄骨）",
        "ポール（２方向）",
        "ポール(モデル無し鉄骨)",
        "グリーンスーパースピンドライバー",
        "スーパースピンドライバー",
        "スピンドライバ",
        "Gキャプチャー",
        "移動用砲台",
        "ウォーターロード",
        "土管",
        "噴水（大）",
        "テッポウ水テスト",
        "オオアワ[共有]",
        "スイングロープ",
        "空中ブランコ",
        "つる花",
        "グライバード",
        "パワースター",
        "グランドスター",
        "グリーンスター",
        "ゴロ水玉",
    };
    const char* sEffectRows[] = {
        "わたげ発生源",
        "わたげ",
        "杭スイッチ",
        "ダミー敵",
        "ザコカメムシ",
        "子連れカメムシ",
        "細野用マップパーツ75",
        "幽霊船たいまつ",
        "燭台",
        "燭台(テレサマンション)",
        "燭台(アイスボルケーノ)",
        "バブル",
        "カボクリ",
        "エイ",
        "ポール",
        "ポール（モデル無し）",
        "ポール（木Ａ）",
        "ポール（鉄骨）",
        "ポール（２方向）",
        "ポール(モデル無し鉄骨)",
        "オニマス(ピボット)",
        "オニマス",
        "移動用砲台",
        "スーパースピンドライバー",
        "スピンドライバ",
        "伸び植物",
        "空中ブランコ",
        "つる花",
        "つるスライダー",
        "土管",
        "タイマーピースブロック",
        "ゴム星",
        "電撃レール",
        "電撃レール点",
        "ビリキュー",
        "宝箱",
        "宝箱(空っぽ)",
        "宝箱ゴールド(空っぽ)",
        "宝箱(ライフＵＰキノコ)",
        "ひび割れ宝箱",
        "アイス床",
        "草",
        "花",
        "青い花",
        "トルネード小石",
        "サークルシェル",
        "サークルストロベリー",
        "タマコロチュートリアル",
        "コイン花",
        "隠れアイテム",
        "スターピーススター",
        "レバースイッチ",
        "クリスタルケージ[小]",
        "クリスタルケージ[中]",
        "クリスタルケージ[大]",
        "ファイアバー",
        "電撃ビリビリレール",
        "移動電撃ビリビリレール",
        "スターピース",
        "アイテムバブルピース",
        "スターピースディレクターピース",
        "フォロースターピース",
        "グループスターピース",
        "スターピースマザーピース",
        "トゲ植物",
        "トゲ植物(空中)",
        "陸ウニゾー",
        "ビリビリボール",
        "飾り足",
        "モグ",
        "蝶",
        "タコヘイ墨",
        "マグナムキラー",
        "雪だるま",
        "よわブロック",
        "ジャンプ台",
        "スピニングボックス",
        "杭スイッチビッグ",
        "ヒビ石",
        "メカクッパパーツローラーA",
        "サポートチコ",
        "リスタートオブジェ",
        "ゴーストアタックのゴースト",
        "チューブスライダー",
        "実",
        "光のフルーツ",
    };
    const char* sEffectColumns[] = {
        "スピンヒット音",
        "スピンヒット振動",
        "スピンキャッチ許可",
        "SHDホーミング許可",
        "地形移動無視",
        "ハチ出立",
        "乗換センサ",
        "青い炎エフェクト",
        "水柱エフェクト",
        "ダメージ中RUSH",
        "ナナメつぶし許可",
        "カメを捨てない",
        "ハチ回復",
        "ハチメータ消去",
        "テレサ時は無視",
        "リバインド制限",
        "HD中ラッシュ",
        "食い込み除外",
        "水陸移動特殊処理",
        "着地エフェクト",
        "影を消す",
        "ダメージ中跳ねる",
    };
    const char* sEffectKeys[] = {
        "Sound",
        "Rumble",
        "SpinCatch",
        "Homing",
        "InvalidMove",
        "ResetWallGravity",
        "JumpRush",
        "FireColor",
        "WaterInOutFx",
        "DamageRush",
        "TiltPress",
        "HandyRush",
        "RecoverFlyMeter",
        "HideFlyMeter",
        "TeresaThru",
        "RebindTimer",
        "HipDropRush",
        "PushOff",
        "WaterInOutRush",
        "LandEffect",
        "NoShadow",
        "DamagePop",
    };
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

void MarioActionTable::initActions() {
    mActions = new MarioNameMatrix("MorphConditionTable", 9, sActions, 14, sActionModes, nullptr);
    mActions->load("MarioConst.arc", "MorphConditionTable");
}

void MarioActionTable::initAutoBind() {
    mAutoBind = new MarioNameMatrix("RushInfluenceTable", 26, sAutoBindRows, 14, sActionModes, nullptr);
    mAutoBind->load("MarioConst.arc", "RushInfluenceTable");
}

void MarioActionTable::initEffects() {
    mEffects = new MarioNameMatrix("ObjectInfluenceTable", 86, sEffectRows, 22, sEffectColumns, sEffectKeys);
    mEffects->load("MarioConst.arc", "ObjectInfluenceTable");
}

namespace {
    struct MorphAction {
        const char* mName;
        const char* mVariants[2][14];
    };
    MorphAction sMorphNames[] = {
        { "DieBlackHole", {
            { "DieBlackHole", nullptr, nullptr, nullptr, "DieBlackHoleBee", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "DieBlackHoleCloud", nullptr },
            { "DieBlackHoleLuigi", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "DieBlackHoleCloudLuigi", nullptr }
        } }
    };
}

MarioActionHash::MarioActionHash() {
    _4 = 2;
    mTable = new HashSortTable(1);
    mTable->add(sMorphNames[0].mName, 0, false);
    mTable->sort();
}


// NONMATCHING: fallback mode and player-stride temporaries exchange r5/r6.
bool MarioActionHash::fn_803DB650(const char** result, const char* name, u16 player, u16 mode) {
    uintptr_t index;
    if (!mTable->search(name, &index))
        return false;

    s32 fallback = _4;
    *result = sMorphNames[index].mVariants[player][mode];
    switch (fallback) {
    case 1:
        if (!*result)
            *result = sMorphNames[index].mVariants[player][0];
        break;
    case 2:
        if (!*result)
            *result = sMorphNames[index].mVariants[0][0];
        break;
    }
    return true;
}
