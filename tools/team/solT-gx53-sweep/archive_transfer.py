#!/usr/bin/env python3
"""One allocated opaque transfer. Never displays or interprets log contents."""
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import tarfile
import time

PLAN_SHA = "1c9f2a628bfa1ba56fcb4c1b3ef3260f484956b29f22e29648616fa015a97c61"
INVENTORY_SHA = "5282d41c23c9e5dc91470895728b26ffddf68f37441546d4b47bc5f3bba966b0"
FLOOR = 50 * 1024 ** 3
HOST = "roberthaisfield@MacBook-Pro-46.local"
SSH = ["ssh", "-o", "BatchMode=yes", "-o", "StrictHostKeyChecking=yes", "-o", "ConnectTimeout=10"]
os.umask(0o077)


def sha(path):
    with Path(path).open("rb") as handle:
        return stream_sha(handle)


def stream_sha(handle):
    h = hashlib.sha256()
    for block in iter(lambda: handle.read(1024 * 1024), b""):
        h.update(block)
    return h.hexdigest()


def write(path, value):
    with Path(path).open("x") as handle:
        json.dump(value, handle, indent=2)
        handle.write("\n")


def send(handle, value):
    handle.write(json.dumps(value) + "\n")
    handle.flush()


def read(handle):
    line = handle.readline()
    assert line, "control channel closed"
    return json.loads(line)


def metadata(plan_text, inventory_text):
    assert hashlib.sha256(plan_text.encode()).hexdigest() == PLAN_SHA
    assert hashlib.sha256(inventory_text.encode()).hexdigest() == INVENTORY_SHA
    plan, inv = json.loads(plan_text), json.loads(inventory_text)
    parent = Path(plan["source_parent_server"])
    entries = {}
    for batch in inv["batches"]:
        root = Path(batch["root"])
        assert root.parent == parent and root.name in (
            "gx53-batch1-20261021T061108Z", "gx53-batch2-20261021T063123Z")
        for row in batch["files"]:
            path = Path(row["path"])
            assert path.is_relative_to(root) and ".." not in path.parts
            member = str(path.relative_to(parent))
            assert member not in entries
            entries[member] = row
    assert len(entries) == 238 and sum(r["bytes"] for r in entries.values()) == 1234058773
    return plan, inv, entries


def lock():
    handle = (Path.home() / "MacioGalaxy2-team/slots/native.lock").open("a")
    fcntl.flock(handle, fcntl.LOCK_EX | fcntl.LOCK_NB)
    os.setpriority(os.PRIO_PROCESS, 0, 19)
    assert shutil.disk_usage(Path.home()).free >= FLOOR
    return handle


def verify_archive(path, entries):
    checked, logs = [], []
    with tarfile.open(path, "r:") as archive:
        members = archive.getmembers()
        assert len(members) == len(entries) and len({m.name for m in members}) == len(members)
        assert {m.name for m in members} == set(entries)
        for member in members:
            row = entries[member.name]
            assert member.isfile() and member.size == row["bytes"]
            assert not Path(member.name).is_absolute() and ".." not in Path(member.name).parts
            if Path(member.name).name == "app-UNREAD.log":
                logs.append(member.name)  # No member opening, decoding or parsing.
            else:
                with archive.extractfile(member) as handle:
                    assert stream_sha(handle) == row["sha256"]
                checked.append(member.name)
    return {"members": len(entries), "non_log_member_hashes_verified": len(checked),
            "opaque_log_members_size_verified": len(logs), "logs_analysis": "UNREAD/NOT_MEASURED"}


def receive():
    hello = read(sys.stdin)
    assert sha(__file__) == hello["tool_sha256"]
    plan, inv, entries = metadata(hello["plan_text"], hello["inventory_text"])
    with lock():
        dest = Path(plan["destination_directory_proposed"])
        assert not dest.exists()
        dest.mkdir(parents=True, mode=0o700)
        final = Path(plan["destination_archive_proposed"])
        assert final.parent == dest
        partial = final.with_suffix(final.suffix + ".partial")
        assert not partial.exists() and not final.exists()
        (dest / "INVENTORY.json").write_text(hello["inventory_text"])
        (dest / "PLAN.json").write_text(hello["plan_text"])
        send(sys.stdout, {"status": "M1_LOCKED_READY", "pid": os.getpid(),
            "partial": str(partial), "free_bytes": shutil.disk_usage(dest).free})
        command = read(sys.stdin)
        assert command["action"] == "VERIFY" and partial.is_file() and not partial.is_symlink()
        assert partial.stat().st_size == command["archive_bytes"] <= plan["archive_size_limit_bytes"]
        assert sha(partial) == command["archive_sha256"]
        receipt = verify_archive(partial, entries)
        assert shutil.disk_usage(dest).free >= FLOOR
        partial.rename(final)
        receipt.update(status="M1_ARCHIVE_VERIFIED", archive=str(final), archive_sha256=command["archive_sha256"],
            archive_bytes=command["archive_bytes"], inventory_sha256=INVENTORY_SHA,
            plan_sha256=PLAN_SHA, tool_sha256=sha(__file__), free_bytes=shutil.disk_usage(dest).free,
            original_server_paths_preserved=True, source_deletion_authorized=False)
        write(dest / "VERIFIED.json", receipt)
        receipt["verification_receipt_sha256"] = sha(dest / "VERIFIED.json")
        send(sys.stdout, receipt)


def transfer(plan_path, remote_tool, remote_bounded):
    started = time.monotonic()
    plan_text = Path(plan_path).read_text()
    plan = json.loads(plan_text)
    inventory_text = Path(plan["inventory_server"]).read_text()
    plan, inv, entries = metadata(plan_text, inventory_text)
    with lock():
        base = Path(plan["source_archive_proposed"]).parent
        assert not base.exists()
        base.mkdir(parents=True, mode=0o700)
        archive_path = Path(plan["source_archive_proposed"])
        write(base / "STATE.json", {"phase": "SERVER_LOCKED", "pid": os.getpid(), "logs": "OPAQUE_UNREAD"})
        # Leave connection/outer stop grace inside the shared 300-second allocation.
        remote_command = ["python3", remote_bounded, "270", "python3", remote_tool, "receive"]
        peer = subprocess.Popen([*SSH, HOST, shlex.join(remote_command)], stdin=subprocess.PIPE,
            stdout=subprocess.PIPE, stderr=(base / "transport-metadata.stderr").open("xb"), text=True)
        try:
            send(peer.stdin, {"tool_sha256": sha(__file__), "plan_text": plan_text, "inventory_text": inventory_text})
            ready = read(peer.stdout)
            assert ready["status"] == "M1_LOCKED_READY" and ready["free_bytes"] >= FLOOR
            print(json.dumps({"status": "BOTH_LOCKS_HELD", "server_pid": os.getpid(), "m1_pid": ready["pid"]}), flush=True)
            before = {}
            for member, row in entries.items():
                path = Path(row["path"])
                assert path.is_file() and not path.is_symlink() and path.stat().st_size == row["bytes"]
                before[member] = (path.stat().st_size, path.stat().st_mtime_ns)
                if path.name != "app-UNREAD.log":
                    assert sha(path) == row["sha256"]
            with archive_path.open("xb") as output:
                with tarfile.open(fileobj=output, mode="w", format=tarfile.USTAR_FORMAT) as archive:
                    for member, row in sorted(entries.items()):
                        path = Path(row["path"])
                        info = archive.gettarinfo(str(path), arcname=member)
                        assert info.isfile()
                        info.uid = info.gid = 0
                        info.uname = info.gname = ""
                        info.mode, info.mtime = 0o600, 0
                        with path.open("rb") as handle:  # Opaque archive transport, no interpretation.
                            archive.addfile(info, handle)
            for member, row in entries.items():
                path = Path(row["path"])
                assert before[member] == (path.stat().st_size, path.stat().st_mtime_ns)
            size = archive_path.stat().st_size
            assert size <= plan["archive_size_limit_bytes"] and shutil.disk_usage(base).free >= FLOOR
            archive_sha = sha(archive_path)
            write(base / "SOURCE-ARCHIVE.json", {"archive": str(archive_path), "sha256": archive_sha,
                "bytes": size, "inventory_sha256": INVENTORY_SHA, "members": len(entries), "logs_analysis": "UNREAD/NOT_MEASURED"})
            rsync = subprocess.run(["rsync", "-a", "-e", shlex.join(SSH), str(archive_path), HOST + ":" + ready["partial"]],
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            assert rsync.returncode == 0, "opaque rsync failed; no retry"
            send(peer.stdin, {"action": "VERIFY", "archive_sha256": archive_sha, "archive_bytes": size})
            verified = read(peer.stdout)
            assert verified["status"] == "M1_ARCHIVE_VERIFIED"
            peer.stdin.close()
            assert peer.wait() == 0, "destination controller did not close normally"
            probe = subprocess.run([*SSH, HOST, shlex.join(["lockf", "-k", "-t", "0",
                str(Path.home() / "MacioGalaxy2-team/slots/native.lock"), "/usr/bin/true"])],
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            assert probe.returncode == 0, "M1 lock release not verified"
            recovery = {"status": "VERIFIED_OPAQUE_TRANSFER_ORIGINALS_PRESERVED", "destination": verified,
                "source_archive": str(archive_path), "source_archive_sha256": archive_sha,
                "inventory_sha256": INVENTORY_SHA, "plan_sha256": PLAN_SHA, "tool_sha256": sha(__file__),
                "elapsed_seconds": round(time.monotonic() - started, 3), "logs_analysis": "UNREAD/NOT_MEASURED",
                "m1_lock_reprobed_free": True, "server_free_bytes": shutil.disk_usage(base).free,
                "original_paths_must_remain_live": True, "deletion_requires_separate_allocation": True,
                "paths": [{"source": row["path"], "archive": verified["archive"], "member": member,
                    "bytes": row["bytes"], "sha256": row["sha256"]} for member, row in entries.items()]}
            write(base / "RECOVERY.json", recovery)
            print(json.dumps({"status": recovery["status"], "archive_sha256": archive_sha, "archive_bytes": size,
                "recovery": str(base / "RECOVERY.json"), "recovery_sha256": sha(base / "RECOVERY.json"),
                "elapsed_seconds": recovery["elapsed_seconds"], "destination": verified,
                "server_free_bytes": recovery["server_free_bytes"]}), flush=True)
        finally:
            if peer.stdin and not peer.stdin.closed:
                peer.stdin.close()  # EOF makes the destination return; bounded runners own deadlines.
            peer.wait()


if __name__ == "__main__":
    if sys.argv[1] == "receive":
        receive()
    else:
        assert sys.argv[1] == "transfer" and len(sys.argv) == 5
        transfer(*sys.argv[2:])
