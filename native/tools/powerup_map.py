#!/usr/bin/env python3
"""Which stages/scenarios place the objects that load each power-up's player resource.

The game loads a power-up's models only when a placed object asks for it (NameObjFactoryNative.cpp: MorphItemNeoBee ->
enablePlayerResourceMorph, SpinCloudItem -> enablePlayerResourceCloud, CometHopperArea ...). Applying a mode in a
stage without its resource is not something a player can do, so the headless power-up sweep only runs a mode where the
scenario places a matching object (any zone of the stage whose layer mask is active in that scenario).

  python3 native/tools/powerup_map.py --disc <extracted disc> [--json out.json]
"""
import argparse
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import stage_sweep as S  # noqa: E402
from collect_pipeline_seed_inputs import field_hash  # noqa: E402

# placed object name -> modes the headless harness can apply (--player-mode)
OBJECT_MODES = {
    "MorphItemNeoBee": "bee", "MorphItemNeoFire": "fire", "MorphItemNeoFoo": "foo", "MorphItemNeoHopper": "hopper",
    "MorphItemNeoIce": "ice", "MorphItemNeoTeresa": "teresa", "MorphItemRock": "rock", "BenefitItemInvincible": "invincible",
    "SpinCloudItem": "cloud", "CometHopperArea": "hopper", "CometRainbowArea": "invincible",
    "YoshiEgg": "yoshi", "Yoshi": "yoshi",
}
NAME = field_hash("name")


def zone_objects(files, zone):
    """{layer letter or 'common': set(object names)} for a zone's Map.arc."""
    arc = files / "StageData" / zone / f"{zone}Map.arc"
    out = {}
    if not arc.is_file():
        return out
    for path, data in S.archive_tree(arc.read_bytes()):
        parts = path.split("/")
        if "placement" not in parts or not parts[-1].endswith("objinfo"):
            continue
        layer = parts[parts.index("placement") + 1]
        for row in S.bcsv_rows(data):
            n = row.get(NAME)
            if isinstance(n, str):
                out.setdefault(layer, set()).add(n)
    return out


def scenario_rows(files, stage):
    scenario = files / "StageData" / stage / f"{stage}Scenario.arc"
    rows, zones = [], []
    for path, data in S.archive_tree(scenario.read_bytes()):
        if path.endswith("/scenariodata.bcsv"):
            rows = list(S.bcsv_rows(data))
        elif path.endswith("/zonelist.bcsv"):
            zones = [r for r in S.bcsv_rows(data)]
    return rows, zones


def build(files):
    result = {}  # mode -> {stage: [scenarios]}
    for stage in S.enumerate_stages(files):
        name = stage["stage"]
        rows, zonelist = scenario_rows(files, name)
        zone_names = [name]
        for z in zonelist:
            for v in z.values():
                if isinstance(v, str) and v:
                    zone_names.append(v)
        objs = {z: zone_objects(files, z) for z in dict.fromkeys(zone_names)}
        for row in rows:
            sc = row.get("ScenarioNo")
            for zone, layers in objs.items():
                mask = row.get(field_hash(zone))
                if not mask:  # a zone whose layer mask is 0 in this scenario is not loaded at all (common layer included)
                    continue
                for layer, names in layers.items():
                    if layer != "common" and not (mask & (1 << (ord(layer[-1]) - ord("a")))):
                        continue
                    for n in names:
                        mode = OBJECT_MODES.get(n)
                        if mode:
                            lst = result.setdefault(mode, {}).setdefault(name, [])
                            if sc not in lst:
                                lst.append(sc)
    return {m: {s: sorted(v) for s, v in sorted(d.items())} for m, d in sorted(result.items())}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--disc", type=Path, default=Path.home() / "MacioGalaxy2-gamedata/SB4E01-extracted")
    ap.add_argument("--json", type=Path)
    a = ap.parse_args()
    result = build(a.disc / "files")
    for mode, stages in result.items():
        print(f"{mode}: {len(stages)} stages, {sum(len(v) for v in stages.values())} scenarios: " + ", ".join(f"{s}{v}" for s, v in list(stages.items())[:6]) + (" ..." if len(stages) > 6 else ""))
    if a.json:
        a.json.write_text(json.dumps(result, indent=1))


if __name__ == "__main__":
    main()
