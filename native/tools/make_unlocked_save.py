#!/usr/bin/env python3
"""Generate isolated SMG2 saves through a cooperating game's own save/loader.

No save-format parser or byte writer lives here. Runtime integration is required;
an old SMG1 executable/log cannot satisfy this protocol. See native/SAVES.md.
"""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import secrets
import shlex
import shutil
import subprocess
import sys
import tempfile

VARIANTS = ("worlds1-6", "postgame", "all-stars")
SAVE_RELATIVE = Path("NAND/title/00010000/53423445/data/GameData.bin")
MARKER = ".petari-make-unlocked-save"
INVENTORY = "unlocked-save-inventory.json"
REQUEST = "unlocked-save-request.json"
RECEIPT = "unlocked-save-receipt.json"
LEDGER = "unlocked-save-nand-ledger.json"
# Supported retail SB4E01 must independently enumerate all 242 identities.
# This is an acceptance guard, never a fabricated inventory or save counter.
SB4E01_STAR_TOTAL = 242
REQUIRED_CHECKS = {
    "worlds1-6": {"worlds-1-6-open"},
    "postgame": {"worlds-1-6-open", "postgame", "world-s-open"},
    "all-stars": {"worlds-1-6-open", "postgame", "world-s-open", "green-stars", "luigi", "master-luma"},
}
ROUND_TRIP_CHECKS = ("save_completed", "regular_loader_succeeded", "header_valid", "version_valid",
                     "checksum_valid", "readback_bytes_equal", "unrelated_slots_unchanged")


def sha256(path):
    h = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def require(ok, message):
    if not ok:
        raise ValueError(message)


def integer(value, low, high, name):
    require(type(value) is int and low <= value <= high, f"invalid {name}")
    return value


def digest(value, name):
    require(isinstance(value, str) and len(value) == 64 and all(c in "0123456789abcdef" for c in value),
            f"invalid {name} digest")
    return value


def text(value, name):
    require(isinstance(value, str) and bool(value.strip()) and "\0" not in value, f"missing {name}")
    return value


def star_key(row):
    require(isinstance(row, dict), "invalid star record")
    return (text(row.get("galaxy"), "galaxy"), integer(row.get("star_id"), 1, 32, "star_id"))


def read_json(path):
    path = Path(path)
    require(not path.is_symlink() and path.is_file() and path.stat().st_size <= 8 * 1024 * 1024,
            f"missing/oversized/nonregular JSON: {path}")
    def unique(pairs):
        out = {}
        for key, value in pairs:
            require(key not in out, f"duplicate JSON key: {key}")
            out[key] = value
        return out
    value = json.loads(path.read_text(), object_pairs_hook=unique)
    require(isinstance(value, dict), f"JSON object required: {path}")
    return value


def write_json(path, value):
    with Path(path).open("x") as stream:
        json.dump(value, stream, ensure_ascii=False, indent=2, allow_nan=False)
        stream.write("\n")


def validate_inventory(inv, run_id):
    require(type(inv.get("schema_version")) is int and inv.get("schema_version") == 1 and inv.get("game") == "SB4E01", "not SMG2 inventory v1")
    require(inv.get("run_id") == run_id, "inventory run_id mismatch")
    require(inv.get("complete") is True and inv.get("engine_validated") is True,
            "incomplete or non-engine-validated star inventory")
    require(inv.get("provider") == "ScenarioDataParser+GameDataAllGalaxyStorage", "wrong inventory provider")
    digest(inv.get("catalog_sha256"), "catalog")
    text(inv.get("provenance"), "inventory provenance")
    records = inv.get("stars")
    require(isinstance(records, list) and records, "empty star inventory")
    maximum = integer(inv.get("game_maximum"), 1, 4096, "game maximum")
    require(len(records) == maximum, "unique inventory count disagrees with game maximum")
    require(maximum == SB4E01_STAR_TOTAL, "SB4E01 complete inventory must resolve 242 actual identities")
    seen = set()
    for row in records:
        key = star_key(row)
        require(key not in seen, "duplicate star identity (not scenario rows)")
        seen.add(key)
        integer(row.get("world"), 1, 7, "world")
        require(type(row.get("green")) is bool and row.get("engine_validated") is True,
                "unvalidated star mapping/type")
        scenarios = row.get("scenario_numbers")
        require(isinstance(scenarios, list) and scenarios, "missing scenario associations")
        for number in scenarios:
            integer(number, 1, 0x7fffffff, "actual ScenarioNo")
        require(len(scenarios) == len(set(scenarios)), "duplicate scenario associations")
        text(row.get("provenance"), "star mapping provenance")
    integer(inv.get("slot"), 1, 0x7fffffff, "actual file slot")
    seed_owned = owned(inv.get("seed_owned"), seen)
    require(type(inv.get("seed_owned_count")) is int and inv["seed_owned_count"] == len(seed_owned),
            "inventory seed-owned count mismatch")
    return seen


def make_plan(inv, variant, run_id):
    require(variant in VARIANTS, "unsupported SMG2 variant")
    validate_inventory(inv, run_id)
    require(isinstance(inv.get("profiles"), dict), "missing runtime profiles")
    profile = inv["profiles"].get(variant)
    require(isinstance(profile, dict) and profile.get("supported") is True,
            f"runtime profile unsupported/unresolved: {variant}")
    checks = profile.get("checks")
    require(isinstance(checks, list), "missing profile checks")
    names = set()
    for check in checks:
        require(isinstance(check, dict), "invalid profile check")
        name = text(check.get("id"), "check id")
        require(name not in names, "duplicate profile check")
        names.add(name)
        require(check.get("applicability") in ("required", "not_applicable"), "unresolved applicability")
        text(check.get("provenance"), "check/getter provenance")
        if check["applicability"] == "not_applicable":
            # These are requested only if actually applicable to SMG2. Worlds/endings
            # cannot be waived by marking the entire progression profile N/A.
            require(name in ("luigi", "master-luma"), "required progression check cannot be waived")
            text(check.get("reason"), "not-applicable reason")
    require(REQUIRED_CHECKS[variant] <= names, "profile omits required progression checks")
    stars = [row for row in inv["stars"] if variant == "all-stars" or
             (not row["green"] and (variant == "postgame" or row["world"] <= 6))]
    require(stars, "variant selects no validated stars")
    selected = set(map(star_key, stars))
    require(set(owned(inv["seed_owned"], set(map(star_key, inv["stars"])))) <= selected,
            "seed has out-of-profile stars; use a lower-progress isolated seed")
    return {"schema_version": 1, "game": "SB4E01", "run_id": run_id, "variant": variant,
            "catalog_sha256": inv["catalog_sha256"], "slot": inv["slot"],
            "stars": [{"galaxy": key[0], "star_id": key[1]} for key in sorted(map(star_key, stars))],
            "checks": checks}


def owned(records, valid):
    require(isinstance(records, list), "missing owned-star readback")
    result = {}
    for record in records:
        key = star_key(record)
        require(key in valid and key not in result and type(record.get("bronze")) is bool,
                "invalid/duplicate owned-star identity or bronze state")
        result[key] = record["bronze"]
    return result


def verify_receipt(receipt, request, inventory, work):
    require(type(receipt.get("schema_version")) is int and receipt.get("schema_version") == 1 and receipt.get("game") == "SB4E01", "wrong receipt schema/title")
    for key in ("run_id", "variant", "catalog_sha256", "app_sha256", "seed_save_sha256", "slot"):
        require(receipt.get(key) == request[key], f"receipt {key} mismatch")
    require(receipt.get("request_sha256") == sha256(work / REQUEST), "receipt request hash mismatch")
    require(receipt.get("inventory_sha256") == request["inventory_sha256"], "receipt inventory mismatch")
    require(receipt.get("provider") == "GameDataHolder+SaveDataHandleSequence", "wrong save/readback provider")
    for check in ROUND_TRIP_CHECKS:
        require(receipt.get(check) is True, f"game verification failed: {check}")
    for check in ("game_data_corrupted", "config_data_corrupted"):
        require(receipt.get(check) is False, f"game loader corruption flag: {check}")
    integer(receipt.get("slot"), 1, 0x7fffffff, "receipt file slot")
    valid = validate_inventory(inventory, request["run_id"])
    expected = owned(receipt.get("before_owned"), valid)
    require(type(receipt.get("before_owned_count")) is int and receipt["before_owned_count"] == len(expected),
            "before-owned count mismatch")
    require(expected == owned(inventory["seed_owned"], valid), "seed snapshot mismatch before mutation")
    selected = {star_key(row) for row in request["stars"]}
    require(set(expected) <= selected, "seed has out-of-profile stars; use a lower-progress isolated seed")
    expected = {key: False for key in selected}
    require(owned(receipt.get("after_owned"), valid) == expected, "game readback star inventory/bronze state mismatch")
    require(type(receipt.get("owned_count")) is int and receipt["owned_count"] == len(expected),
            "game readback count mismatch")
    if request["variant"] == "all-stars":
        require(receipt["owned_count"] == SB4E01_STAR_TOTAL, "all-stars readback must contain 242 actual identities")
    checks = receipt.get("checks")
    require(isinstance(checks, list), "missing semantic readback checks")
    actual = {}
    for check in checks:
        require(isinstance(check, dict), "invalid readback check")
        name = text(check.get("id"), "readback check id")
        require(name not in actual, "duplicate readback check")
        text(check.get("provenance"), "actual readback getter provenance")
        actual[name] = check
    require(set(actual) == {c["id"] for c in request["checks"]}, "readback check identities mismatch")
    for check in request["checks"]:
        want = "pass" if check["applicability"] == "required" else "not_applicable"
        require(actual[check["id"]].get("status") == want, f"semantic readback failed: {check['id']}")
    digest(receipt.get("save_sha256"), "saved file")
    require(receipt["save_sha256"] == sha256(work / SAVE_RELATIVE), "saved-file/readback hash mismatch")


def reject_personal(path):
    normal = (Path.home() / "Library/Application Support/Petari").resolve()
    require(path != normal and normal not in path.parents, "personal user directory is forbidden")


def require_server_execution():
    require(sys.platform == "darwin", "real inventory/save generation requires the approved macOS server")
    try:
        host = subprocess.run(["/usr/sbin/scutil", "--get", "LocalHostName"], capture_output=True,
                              text=True, check=True, timeout=5).stdout.strip()
    except (OSError, subprocess.SubprocessError) as error:
        raise ValueError("cannot identify approved save server") from error
    marker = Path.home() / "MacioGalaxy2-gamedata/README.txt"
    require(host == "MacBook-Pro-52" and marker.is_file() and not marker.is_symlink(),
            "real inventory/save generation is server-only: MacBook-Pro-52 with gamedata README marker")


def require_private_data_path(path):
    path = Path(path).resolve()
    # Catalogs, NAND, receipts and logs must never enter an upload/archive area
    # or a checkout (including ignored build directories and git worktrees).
    forbidden = {"maciogalaxy2-team", ".bb-machines", ".bb", "attachments", "build", "builds"}
    require(not (set(part.lower() for part in path.parts) & forbidden),
            f"game-data path is in a build/team/upload area: {path}")
    for parent in (path, *path.parents):
        require(not (parent / ".git").exists(), f"game-data path is inside a repository/worktree: {path}")
    reject_personal(path)


def tree_hashes(root):
    require(root.is_dir() and not root.is_symlink(), f"missing/nonregular NAND tree: {root}")
    result = {}
    for parent, dirs, files in os.walk(root, followlinks=False):
        for name in dirs + files:
            path = Path(parent) / name
            require(not path.is_symlink(), f"symlink in isolated NAND: {path}")
            require(path.is_dir() or path.is_file(), f"nonregular NAND entry: {path}")
        for name in files:
            path = Path(parent) / name
            result[str(path.relative_to(root))] = sha256(path)
    return result


def run_game(app, disc, work, action, timeout):
    require_server_execution()
    require_private_data_path(work)
    require_private_data_path(disc)
    command = [str(app), "--disc", str(disc), "--user", str(work), "--make-unlocked-save", action]
    log = work / f"{action}.log"
    with log.open("x") as stream:
        stream.write("$ " + shlex.join(command) + "\n")
        stream.flush()
        try:
            status = subprocess.run(command, stdout=stream, stderr=subprocess.STDOUT, timeout=timeout).returncode
        except subprocess.TimeoutExpired:
            raise ValueError(f"game timed out; diagnostic work retained: {work}") from None
    require(status == 0, f"game failed ({status}); diagnostic work retained: {work}")


def generate(variant, seed, output, app, disc, timeout=300):
    require_server_execution()
    require(variant in VARIANTS, "unsupported SMG2 variant")
    require(math.isfinite(timeout) and timeout > 0, "timeout must be positive and finite")
    seed, app, disc = Path(seed), Path(app), Path(disc)
    output = Path(output).absolute()
    require(not output.exists() and not output.is_symlink(), f"output already exists: {output}")
    require(not seed.is_symlink(), "seed symlink is forbidden")
    seed, app, disc = seed.resolve(strict=True), app.resolve(strict=True), disc.resolve(strict=True)
    output = output.parent.resolve() / output.name
    for path in (seed, output, disc):
        require_private_data_path(path)
    require(seed not in output.parents and output not in seed.parents and seed != output,
            "seed/output trees must be disjoint")
    require(app.is_file() and os.access(app, os.X_OK), "built executable required")
    require(disc.is_dir(), "extracted SMG2 disc directory required")
    before = tree_hashes(seed / "NAND")
    relative = str(SAVE_RELATIVE.relative_to("NAND"))
    require(relative in before, "seed has no GameData.bin")
    output.parent.mkdir(parents=True, exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix=f".{output.name}.work-", dir=output.parent))
    work.chmod(0o700)
    shutil.copytree(seed / "NAND", work / "NAND", symlinks=False)
    require(tree_hashes(work / "NAND") == before and tree_hashes(seed / "NAND") == before, "seed changed while copying")
    run_id = secrets.token_hex(24)
    write_json(work / MARKER, {"schema_version": 1, "game": "SB4E01", "run_id": run_id})
    app_hash = sha256(app)
    run_game(app, disc, work, "inventory", timeout)
    after_inventory = tree_hashes(work / "NAND")
    require(after_inventory == before, "inventory action modified NAND")
    inventory = read_json(work / INVENTORY)
    request = make_plan(inventory, variant, run_id)
    request.update(inventory_sha256=sha256(work / INVENTORY), app_sha256=app_hash,
                   seed_save_sha256=before[relative])
    write_json(work / REQUEST, request)
    request_hash = sha256(work / REQUEST)
    # A previous receipt must never be reused as the generation proof.
    require(not (work / RECEIPT).exists() and not (work / RECEIPT).is_symlink(), "stale generation receipt")
    run_game(app, disc, work, variant, timeout)
    require(sha256(app) == app_hash, "executable changed during generation")
    require(sha256(work / REQUEST) == request_hash, "request changed during generation")
    require(sha256(work / INVENTORY) == request["inventory_sha256"], "inventory changed during generation")
    receipt = read_json(work / RECEIPT)
    verify_receipt(receipt, request, inventory, work)
    after = tree_hashes(work / "NAND")
    require({k: v for k, v in after.items() if k != relative} == {k: v for k, v in before.items() if k != relative},
            "game changed NAND outside GameData.bin")
    require(tree_hashes(seed / "NAND") == before, "seed changed during generation")
    ledger = {"schema_version": 1, "game": "SB4E01", "run_id": run_id,
              "before": before, "after_inventory": after_inventory, "after_game_save": after,
              "changed": sorted(k for k in set(before) | set(after) if before.get(k) != after.get(k)),
              "seed_unchanged": True}
    write_json(work / LEDGER, ledger)
    manifest = {"schema_version": 1, "game": "SB4E01", "variant": variant, "run_id": run_id,
                "seed": str(seed), "seed_save_sha256": before[relative], "app_sha256": app_hash,
                "save_sha256": after[relative], "inventory_sha256": request["inventory_sha256"],
                "request_sha256": sha256(work / REQUEST), "receipt_sha256": sha256(work / RECEIPT), "nand_ledger_sha256": sha256(work / LEDGER),
                "selected_stars": request["stars"], "selected_count": len(request["stars"]),
                "game_owned_count": receipt["owned_count"], "verification": "game-save-and-regular-loader-readback"}
    # mkdir is exclusive, including a concurrent output creator. Manifest is last:
    # an interrupted copy never advertises itself as a published verified save.
    output.mkdir(mode=0o700)
    shutil.copytree(work / "NAND", output / "NAND")
    for name in (INVENTORY, REQUEST, RECEIPT, LEDGER, "inventory.log", f"{variant}.log"):
        shutil.copy2(work / name, output / name)
    require(tree_hashes(output / "NAND") == after, "published copy mismatch (no manifest written)")
    write_json(output / "unlocked-save.json", manifest)
    shutil.rmtree(work)
    return manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--variant", required=True, choices=VARIANTS)
    parser.add_argument("--seed", required=True, type=Path, help="explicit PRIVATE SERVER SMG2 user directory; read only")
    parser.add_argument("--output", required=True, type=Path, help="new PRIVATE SERVER user directory outside repo/build/team/upload trees")
    parser.add_argument("--app", required=True, type=Path)
    parser.add_argument("--disc", required=True, type=Path)
    parser.add_argument("--timeout", type=float, default=300)
    parser.add_argument("--run", action="store_true", help="explicitly launch the cooperating game (port run clearance required)")
    args = parser.parse_args()
    if not args.run:
        parser.error("--run is required; no runtime save verification has been performed")
    try:
        result = generate(args.variant, args.seed, args.output, args.app, args.disc, args.timeout)
    except (OSError, ValueError, TypeError, KeyError) as error:
        print(f"make_unlocked_save: {error}", file=sys.stderr)
        return 1
    print(f"Published {args.output}: {result['selected_count']} selected identities, {result['game_owned_count']} read back")
    return 0


if __name__ == "__main__":
    sys.exit(main())
