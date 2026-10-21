#!/usr/bin/env python3
"""Bounded isolated fixture; acquire the existing server POSIX lock without waiting."""
import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import signal
import subprocess
import time
from PIL import Image


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()
    home = Path.home()
    output = Path(args.output).resolve()
    output.relative_to(home / 'MacioGalaxy2-gamedata')
    output.mkdir(parents=True, exist_ok=False)
    source = Path(__file__).resolve().parent
    lock = (home / 'MacioGalaxy2-team/slots/native.lock').open('a')
    try:
        fcntl.lockf(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except BlockingIOError:
        print('LOCK_BUSY: no compiler or fixture started')
        return 2
    start = time.monotonic()
    receipt = dict(status='incomplete',runtime_visual_qualification='BLOCKED',pid=os.getpid(),
                   lock=str(lock.name),source_hashes={p.name: sha(p) for p in source.iterdir() if p.is_file()},
                   output=str(output),nice=os.getpriority(os.PRIO_PROCESS, 0),commands=[])
    state = home / 'MacioGalaxy2-team/scratch-port/solD/STATE.md'
    state.write_text(f'HOST server | SOURCE checked PNG isolated fixture | BINARY private fixture only | PHASE lock acquired | OUTPUT {output} | PID {os.getpid()} | RESUME bounded <=180s compile j1 + native synthetic fixture; no app/GPU\n')
    child = None
    def stop(signum, _frame):
        raise RuntimeError(f'interrupted by signal {signum}')
    for sig in (signal.SIGTERM, signal.SIGINT):
        signal.signal(sig, stop)
    def run(argv, log):
        nonlocal child
        left = 180 - (time.monotonic() - start)
        if left <= 0:
            raise TimeoutError('180-second fixture bound')
        with (output / log).open('wb') as stream:
            child = subprocess.Popen(argv, stdout=stream, stderr=subprocess.STDOUT, start_new_session=True)
            receipt['commands'].append(dict(argv=argv,pid=child.pid))
            state.write_text(f'HOST server | SOURCE checked PNG isolated fixture | PHASE {log} | OUTPUT {output} | PID driver={os.getpid()} child={child.pid} | RESUME bounded compile/fixture, owned process group only\n')
            result = child.wait(timeout=left)
            child = None
            if result:
                raise RuntimeError(f'{log} failed: exit {result}')
    try:
        compiler = Path('/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++')
        receipt['compiler_sha256'] = sha(compiler)
        receipt['compiler_version'] = subprocess.check_output([str(compiler),'--version'],text=True,timeout=5)
        sdk = Path(subprocess.check_output(['/usr/bin/xcrun','--show-sdk-path'],text=True,timeout=5).strip()).resolve()
        receipt['sdk'] = str(sdk)
        receipt['sdk_settings_sha256'] = sha(sdk/'SDKSettings.json')
        exe = output / 'fixture'
        run([str(compiler),'-isysroot',str(sdk),'-std=c++17','-O1','-gline-tables-only','-Wall','-Wextra','-Werror','-Wno-deprecated-declarations','-fsanitize=address,undefined',str(source/'checked_png.cpp'),str(source/'fixture.cpp'),'-o',str(exe)], 'compile.log')
        receipt['executable_sha256'] = sha(exe)
        run([str(exe),str(output/'cases')], 'fixture.log')
        results = []
        for line in (output/'fixture.log').read_text().splitlines():
            if not line.startswith('RECEIPT '):
                continue
            _, name, digest, size = line.split()
            path = output/'cases'/name
            assert sha(path) == digest and path.stat().st_size == int(size)
            with Image.open(path) as checked:
                checked.verify()
            with Image.open(path) as image:
                image.load()
                assert image.format == 'PNG' and image.mode == 'RGB'
                expected = (1, 1) if name == 'one.png' else (640, 48)
                assert image.size == expected
                assert image.tobytes() == (bytes([1,2,3]) if name == 'one.png' else (output/'cases/expected.rgb').read_bytes())
            results.append(dict(path=name,sha256=digest,bytes=int(size),dimensions=list(expected)))
        assert len(results) == 7
        assert 'PASS 29 native controls' in (output/'fixture.log').read_text()
        assert not list((output/'cases').glob('*.partial.*'))
        size = sum(p.stat().st_size for p in output.rglob('*') if p.is_file())
        assert size <= 256 * 1024 * 1024
        receipt.update(status='PASS',native_controls=29,independent_png_decode_and_hash_controls=7,images=results,output_bytes_before_cleanup=size)
        # Keep only compact reproducible receipts/logs, not disposable binaries/images.
        import shutil
        shutil.rmtree(output/'cases')
        for path in output.glob('fixture*'):
            if path.is_file() and path.name != 'fixture.log':
                path.unlink()
        for path in output.glob('*.dSYM'):
            shutil.rmtree(path)
    except BaseException as exc:
        receipt['error'] = str(exc)
        raise
    finally:
        if child is not None:
            try:
                os.killpg(child.pid, signal.SIGTERM)
                child.wait(timeout=3)
            except subprocess.TimeoutExpired:
                os.killpg(child.pid, signal.SIGKILL)
                child.wait()
            except ProcessLookupError:
                child.wait()
        receipt.update(elapsed_seconds=time.monotonic()-start,children_reaped=True)
        fcntl.lockf(lock,fcntl.LOCK_UN)
        lock.close()
        receipt['lock_released']=True
        (output/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
        state.write_text(f'HOST server | SOURCE checked PNG fixture | PHASE {receipt["status"]}; children reaped, actual lock released | OUTPUT {output}/receipt.json | PID none | RESUME source review/delivery only; GPU/frame/color joins BLOCKED\n')
        print(json.dumps(receipt))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
