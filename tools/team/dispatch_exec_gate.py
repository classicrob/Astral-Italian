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


# Only the reviewed chosen context is disclosed. Unrelated env names/values
# are never exported, even when they cause strict equality to fail.
CONTEXT_KEYS = ('HOME', 'PATH', 'LANG', 'LC_ALL', 'TMPDIR',
                'PYTHONCOERCECLOCALE', 'PYTHONUTF8', 'PYTHONNOUSERSITE')
CONTEXT_RECORD_CAP = 65536
CWD_TEXT_CAP = 4096


def context_observation(request, actual_cwd, actual_env):
    requested_env = request['env'] if type(request['env']) is dict else {}
    def value_pin(mapping, key):
        if key not in mapping:
            return {'present': False}
        value = mapping[key]
        if type(value) is not str:
            return {'present': True, 'string': False}
        data = value.encode('utf-8', errors='surrogatepass')
        return {'present': True, 'string': True, 'bytes': len(data),
                'sha256': hashlib.sha256(data).hexdigest()}
    def cwd_pin(value):
        if type(value) is not str:
            return {'string': False}
        data = value.encode('utf-8', errors='surrogatepass')
        return {'string': True, 'text': value[:CWD_TEXT_CAP],
                'text_truncated': len(value) > CWD_TEXT_CAP,
                'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
    fields = {key: {'equal': actual_env.get(key) == requested_env.get(key)
                   and (key in actual_env) == (key in requested_env),
                   'actual': value_pin(actual_env, key),
                   'requested': value_pin(requested_env, key)}
              for key in CONTEXT_KEYS}
    # Counts only outside the allowlist: no names, values or value hashes.
    outside = (set(actual_env) | set(requested_env)) - set(CONTEXT_KEYS)
    return {'schema': 1, 'type': 'PRE_IDENTITY_CONTEXT_COMPARISON',
            'cwd_equal': actual_cwd == request['cwd'],
            'env_equal': actual_env == request['env'],
            'cwd': {'actual': cwd_pin(actual_cwd), 'requested': cwd_pin(request['cwd'])},
            'environment': fields,
            'outside_allowlist': {'actual_count': len(set(actual_env)-set(CONTEXT_KEYS)),
                                  'requested_count': len(set(requested_env)-set(CONTEXT_KEYS)),
                                  'different_count': sum(actual_env.get(k) != requested_env.get(k)
                                      or (k in actual_env) != (k in requested_env) for k in outside)},
            'limits': {'cwd_text_characters_per_side': CWD_TEXT_CAP,
                       'record_bytes': CONTEXT_RECORD_CAP,
                       'unrelated_environment_names_and_values': 'OMITTED'}}


def check_request(request, observation_path=None):
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
    actual_env = dict(os.environ)
    actual_cwd = str(Path.cwd().resolve())
    if observation_path is not None:
        observation = context_observation(request, actual_cwd, actual_env)
        if len(json.dumps(observation, sort_keys=True).encode()) > CONTEXT_RECORD_CAP:
            raise RuntimeError('context observation exceeds bound')
        publish(observation_path, observation)
    env_equal = actual_env == request['env']
    cwd_equal = actual_cwd == request['cwd']
    if not env_equal or not cwd_equal:
        reason = 'cwd+environment' if not env_equal and not cwd_equal else ('environment' if not env_equal else 'cwd')
        raise RuntimeError('launch '+reason+' drift')
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
    target = check_request(request, request_path.parent/'context-before-identity.json')
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
