"""Private identity/ack gate for exact pinned proof commands, not a daemon.

Does not add signals to the parent lifecycle. Self-expires while unacknowledged;
after acknowledgement execs the ORIGINAL argv in the ORIGINAL cwd/environment.
"""
import hashlib
import json
import os
from pathlib import Path
import shutil
import signal
import stat
import sys
import time


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def publish(path, value):
    path = Path(path)
    if path.exists() or path.is_symlink():
        raise RuntimeError('launch record already exists')
    temporary = path.with_name(path.name+'.tmp')
    fd = os.open(temporary, os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW, 0o600)
    try:
        data = json.dumps(value, sort_keys=True).encode()
        if os.write(fd, data) != len(data):
            raise RuntimeError('incomplete launch-gate record')
    finally:
        os.close(fd)
    os.rename(temporary, path)


def check_request(request):
    if set(request) != {'nonce', 'argv', 'cwd', 'env', 'target', 'target_sha256', 'gate_sha256', 'python_sha256'}:
        raise RuntimeError('launch request fields')
    nonce = request['nonce']
    if type(nonce) is not str or len(nonce) != 32 or any(c not in '0123456789abcdef' for c in nonce):
        raise RuntimeError('launch nonce')
    argv = request['argv']
    if type(argv) is not list or not argv or not all(type(x) is str and x and '\0' not in x for x in argv):
        raise RuntimeError('launch argv')
    if digest(__file__) != request['gate_sha256'] or digest(sys.executable) != request['python_sha256']:
        raise RuntimeError('gate/interpreter drift')
    if dict(os.environ) != request['env'] or str(Path.cwd().resolve()) != request['cwd']:
        raise RuntimeError('launch cwd/environment drift')
    target = shutil.which(argv[0], path=request['env']['PATH'])
    if target is None or str(Path(target).resolve()) != request['target']:
        raise RuntimeError('launch target resolution drift')
    mode = Path(target).stat().st_mode
    if not stat.S_ISREG(mode) or mode & (stat.S_ISUID | stat.S_ISGID):
        raise RuntimeError('unsupported launch target mode')
    if digest(target) != request['target_sha256']:
        raise RuntimeError('launch target bytes drift')
    return request['target']


def main():
    if len(sys.argv) != 3:
        raise RuntimeError('request path and SHA required')
    request_path = Path(sys.argv[1])
    pin = sys.argv[2]
    signal.signal(signal.SIGALRM, signal.SIG_DFL)
    signal.alarm(10)  # Unverified gate always self-expires without parent signals.
    if request_path.is_symlink() or digest(request_path) != pin:
        raise RuntimeError('launch request pin/path')
    request = json.loads(request_path.read_text())
    target = check_request(request)
    identity = dict(pid=os.getpid(), pgid=os.getpgrp(), sid=os.getsid(0), uid=os.getuid())
    publish(request_path.parent/'identity.json', identity)
    publish(request_path.parent/'target.json', {
        'pid': os.getpid(), 'request_sha256': pin, 'target': target,
        'target_sha256': request['target_sha256'], 'cwd': request['cwd'],
    })
    until = time.monotonic()+8
    ack = request_path.parent/'ack.json'
    while not ack.exists():
        if time.monotonic() >= until:
            raise RuntimeError('launch acknowledgement deadline')
        time.sleep(0.01)
    if ack.is_symlink() or json.loads(ack.read_text()) != {'pid': os.getpid(), 'request_sha256': pin}:
        raise RuntimeError('launch acknowledgement mismatch')
    # Recheck immutable request/target/env immediately before exec, no argv edits.
    if digest(request_path) != pin:
        raise RuntimeError('launch request changed after acknowledgement')
    check_request(request)
    signal.alarm(0)
    os.execv(target, request['argv'])


if __name__ == '__main__':
    main()
