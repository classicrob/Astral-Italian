#!/usr/bin/env python3
"""Fair FIFO landing lock shared by every lane (and land.sh).

  landlock.py acquire WHO      blocks until it is WHO's turn, then holds ~/MacioGalaxy2-team/land.lock;
                               prints the pid to use as LAND_LOCK_HELD and exits 0 (the lock stays held)
  landlock.py release          releases land.lock if held by this caller's pid (or a dead pid)
  landlock.py status           shows the holder and the queue

Each waiter writes a ticket (land.queue/<monotonic-ns>-<pid>); the lock is taken only by the OLDEST live ticket,
so nobody can be starved by faster pollers. Tickets of dead processes are discarded. The lock directory format
(land.lock/pid, land.lock/who) is unchanged, so older scripts that mkdir the lock still interoperate.
`acquire` holds the lock in the name of the CALLER's process (its parent pid) so it survives this script's exit.
"""
import os, sys, time

T = os.path.expanduser("~/MacioGalaxy2-team")
LOCK, Q = f"{T}/land.lock", f"{T}/land.queue"


def alive(pid):
    try:
        os.kill(int(pid), 0)
        return True
    except (OSError, ValueError):
        return False


def holder():
    try:
        return open(f"{LOCK}/pid").read().strip()
    except OSError:
        return None


def clear_stale():
    h = holder()
    if os.path.isdir(LOCK) and h and not alive(h):
        os.system(f"rm -rf '{LOCK}'")


def live_tickets():
    os.makedirs(Q, exist_ok=True)
    out = []
    for t in sorted(os.listdir(Q)):
        pid = t.split("-")[-1]
        if alive(pid):
            out.append(t)
        else:
            try:
                os.unlink(f"{Q}/{t}")
            except OSError:
                pass
    return out


cmd = sys.argv[1] if len(sys.argv) > 1 else "status"
owner = str(os.getppid())  # the calling shell/python process holds the lock
if cmd == "acquire":
    who = sys.argv[2] if len(sys.argv) > 2 else owner
    os.makedirs(Q, exist_ok=True)
    ticket = f"{time.monotonic_ns():020d}-{owner}"
    open(f"{Q}/{ticket}", "w").write(who)
    told = False
    while True:
        clear_stale()
        ts = live_tickets()
        if ts and ts[0] == ticket:
            try:
                os.mkdir(LOCK)
                open(f"{LOCK}/pid", "w").write(owner)
                open(f"{LOCK}/who", "w").write(who)
                os.unlink(f"{Q}/{ticket}")
                print(owner)
                break
            except FileExistsError:
                pass
        if not told:
            print(f"queued for landing lock (position {ts.index(ticket) + 1 if ticket in ts else '?'}; holder: "
                  f"{open(f'{LOCK}/who').read().strip() if os.path.isdir(LOCK) else 'none'})", file=sys.stderr)
            told = True
        time.sleep(3)
elif cmd == "release":
    h = holder()
    if h in (owner, None) or not alive(h):
        os.system(f"rm -rf '{LOCK}'")
elif cmd == "status":
    print("holder:", (open(f"{LOCK}/who").read().strip() + f" (pid {holder()})") if os.path.isdir(LOCK) else "none")
    for i, t in enumerate(live_tickets()):
        print(f"  {i + 1}. {open(f'{Q}/{t}').read().strip()} (pid {t.split('-')[-1]})")
