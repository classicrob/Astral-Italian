#!/usr/bin/env python3
"""Inventory only closed new sweep evidence. Logs are stat-only, never opened."""
import hashlib
import json
from pathlib import Path
import sys

home = Path.home()
parent = home / "MacioGalaxy2-gamedata/solT-gx53-direct"
names = ("gx53-batch1-20261021T061108Z", "gx53-batch2-20261021T063123Z")
batches = []


def digest(path):
    h = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


for name in names:
    root = parent / name
    closed = json.loads((root / "closed.json").read_text())
    assert closed["status"] == "BATCH_CLOSED" and closed["no_log_analysis"]
    paths = set(root.glob("*.json"))
    for row in closed["rows"]:
        case = Path(row["case"])
        assert case.parent == root and row["status"] == "BOUNDED_IDLE_OBSERVED"
        paths.update(case.glob("*.json"))
        paths.update(case / f for f in ("frames.csv", "route.csv", "app-UNREAD.log"))
        for kind in ("surface", "xfb"):
            captures = list((case / kind).glob("*.png"))
            assert len(captures) == 3
            paths.update(captures)
    files = []
    for path in sorted(paths):
        assert path.is_file() and not path.is_symlink()
        log = path.name == "app-UNREAD.log"
        files.append({"path": str(path), "relative_path": str(path.relative_to(root)),
            "bytes": path.stat().st_size,
            "sha256": "NOT_MEASURED_UNREAD_HOLD" if log else digest(path),
            "content_policy": "STAT_ONLY_NO_OPEN" if log else "HASH_ONLY_NO_VISUAL_OR_LOG_ANALYSIS"})
    batches.append({"root": str(root), "files": files,
        "selected_png_count": sum(x["path"].endswith(".png") for x in files),
        "total_listed_bytes": sum(x["bytes"] for x in files),
        "cache_retention": "Cache bytes already removed only after successful pair; command/result/cleanup JSON retain hashes, renderer key and row metadata.",
        "must_remain_live_until_verified_transfer_and_path_receipt": str(root)})

result = {"status": "INVENTORY_ONLY_NO_TRANSFER_OR_DELETE", "batches": batches,
    "required_live_server_paths": [
        str(home / "MacioGalaxy2-wt/solL-production53-r2"),
        str(home / "MacioGalaxy2-gamedata/solL/production53-gx-r2"),
        str(home / "MacioGalaxy2-team/scratch-port/solT/gx53-server"),
        str(home / "MacioGalaxy2-gamedata/SB4E01-extracted")],
    "recovery": "Preserve original absolute paths until separately allocated transfer is independently verified and a relocation receipt maps each path/hash. App/build/dependencies remain live; no pruning authorized.",
    "logs_unread": True, "old_worldmap_data_touched": False,
    "tool_sha256": digest(Path(__file__))}
out = Path(sys.argv[1])
with out.open("x") as handle:
    json.dump(result, handle, indent=2)
    handle.write("\n")
print(json.dumps({"inventory": str(out), "sha256": digest(out),
    "batches": [{"root": b["root"], "files": len(b["files"]),
        "selected_png_count": b["selected_png_count"], "total_bytes": b["total_listed_bytes"]} for b in batches]}))
