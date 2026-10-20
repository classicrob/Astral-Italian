#!/usr/bin/env python3
"""Validate source-only remote Wii proofs before importing their certificate.

spec writes the local source/config/tool contract. verify requires a trusted host
and a receipt directory supplied by the operator, and checks the actual ELF
artifacts independently. No build or network access is performed.
"""
import argparse
import base64
import contextlib
import hashlib
import io
import json
from pathlib import Path, PurePosixPath
import runpy
import subprocess
import sys
import tarfile
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parent))
import objsame

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
INPUTS = ('src', 'include', 'libs', 'config')
GENERATORS = ('configure.py', 'tools/project.py', 'tools/ninja_syntax.py', 'tools/team/objsame.py')


def require(ok, message):
    if not ok:
        raise ValueError(message)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT).decode().strip()


def canonical_spec():
    require(not git('status', '--porcelain', '--untracked-files=all', '--', *INPUTS, *GENERATORS),
            'dirty Wii inputs or proof configuration')
    import tools.project as project
    captured = []
    old_generate, old_argv = project.generate_build, sys.argv
    try:
        project.generate_build = lambda cfg: captured.append(cfg)
        sys.argv = ['configure.py', '--no-progress']
        with contextlib.redirect_stdout(io.StringIO()):
            runpy.run_path(str(ROOT / 'configure.py'), run_name='__main__')
    finally:
        project.generate_build, sys.argv = old_generate, old_argv
    require(len(captured) == 1, 'could not capture official source configuration')
    cfg = captured[0]
    units = {}
    versions = set()
    for obj in cfg.objects().values():
        if not obj.src_path.is_file() or not obj.options['add_to_all']:
            continue
        rel = obj.src_obj_path.relative_to(cfg.out_path()).as_posix()
        units[rel] = {'source': obj.src_path.as_posix(),
                      'options': json.loads(json.dumps({k: v for k, v in obj.options.items() if k != 'objects'}, default=str))}
        versions.add(obj.options['mw_version'])
    require(len(units) > 0, 'empty source graph')
    toolpaths = [f'build/compilers/{v}/mwcceppc.exe' for v in sorted(versions)]
    toolpaths += ['build/tools/sjiswrap.exe', 'build/tools/wibo.real',
                  'build/binutils/powerpc-eabi-as', 'build/SB4E01/include/macros.inc']
    return {'schema': 1, 'base': git('rev-parse', 'main'),
            'before': {p: git('rev-parse', f'main:{p}') for p in INPUTS},
            'after': {p: git('rev-parse', f'HEAD:{p}') for p in INPUTS},
            'generators': {p: digest((ROOT / p).read_bytes()) for p in GENERATORS},
            'toolchain': {p: digest((ROOT / p).read_bytes()) for p in toolpaths},
            'units': units}


def confined(root, name):
    rel = PurePosixPath(name)
    require(not rel.is_absolute() and '..' not in rel.parts and str(rel) == name,
            'noncanonical artifact path')
    path = root / name
    require(not path.is_symlink() and path.resolve().is_relative_to(root), 'artifact path escapes proof root')
    require(path.is_file(), f'missing artifact: {name}')
    return path


def verify(receipt, root, expected, host):
    require(receipt['schema'] == 1 and receipt['status'] == 'PASS', 'incomplete remote proof')
    require(receipt['host'] == host, 'unexpected proof host')
    require(receipt['spec'] == expected, 'stale or mismatched source/config/tool contract')
    require(receipt['restored_trees'] == expected['after'], 'tested sources not restored')
    require(receipt['builds'] == {'before': 0, 'after': 0, 'control': 0, 'restore': 0},
            'a clean build or control failed')
    for phase in ('before', 'after'):
        require(receipt['clean'][phase] is True, 'build was not clean')
    provenance = receipt['provenance']
    require(set(provenance) == {'before_log', 'after_log', 'control_log', 'recipe', 'unit_manifest', 'spec_comparison'},
            'missing proof provenance')
    for record in provenance.values():
        data = confined(root, record['path']).read_bytes()
        require(data and digest(data) == record['sha256'], 'provenance artifact hash mismatch')
    files = receipt['files']
    require(isinstance(files, dict) and files, 'empty artifact manifest')
    archive = receipt['archive']
    encoded = confined(root, archive['path']).read_bytes()
    require(digest(encoded) == archive['encoded_sha256'], 'encoded archive hash mismatch')
    require(len(encoded) <= 1024 * 1024 * 1024, 'archive exceeds import size limit')
    raw = base64.b64decode(b''.join(encoded.split()), validate=True)
    require(digest(raw) == archive['sha256'], 'archive hash mismatch')
    cert_info = receipt['certificate']
    cert = confined(root, cert_info['path']).read_bytes()
    require(digest(cert) == cert_info['sha256'], 'certificate hash mismatch')
    wanted_cert = (expected['base'] + ' ' + ' '.join(expected['after'][p] for p in INPUTS[:3]) + '\n').encode()
    require(cert == wanted_cert, 'certificate does not bind this landing')
    ctl = receipt['control']
    require(ctl['object'] in expected['units'], 'control is not a configured source object')
    source = expected['units'][ctl['object']]['source']
    require(ctl['source'] == source, 'control source/object mismatch')
    candidate_source = subprocess.check_output(['git', 'show', f'HEAD:{source}'], cwd=ROOT)
    require(digest(candidate_source) == ctl['source_sha256'], 'control source mismatch')
    require(ctl['source_sha256'] == ctl['restored_source_sha256'], 'control source not restored')
    changed = set(git('diff', '--name-only', expected['base'], 'HEAD', '--', *INPUTS).splitlines())
    require(source in changed, 'control must be a changed source unit')
    expected_files = {f'{phase}/{p}' for phase in ('before', 'after') for p in expected['units']}
    expected_files |= {'control/original.o', 'control/perturbed.o', 'control/restored.o'}
    require(set(files) == expected_files, 'incomplete or unexpected object manifest')
    with tempfile.TemporaryDirectory(prefix='remote-wii-proof-') as tmp:
        dest = Path(tmp)
        with tarfile.open(fileobj=io.BytesIO(raw), mode='r:gz') as tar:
            seen = set()
            total_size = 0
            for member in tar:
                require(member.isfile() and member.name in expected_files and member.name not in seen,
                        'unsafe, duplicate, or unexpected archive member')
                require(0 < member.size <= 64 * 1024 * 1024, 'invalid object size')
                total_size += member.size
                require(total_size <= 2 * 1024 * 1024 * 1024, 'expanded archive exceeds limit')
                seen.add(member.name)
                data = tar.extractfile(member).read()
                require(digest(data) == files[member.name], f'object hash mismatch: {member.name}')
                target = dest / member.name
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(data)
            require(seen == expected_files, 'missing archive objects')
        for rel in expected['units']:
            require(not objsame.compare(dest / 'before' / rel, dest / 'after' / rel),
                    f'Wii semantic difference: {rel}')
        original, perturbed, restored = [dest / 'control' / f'{p}.o' for p in ('original', 'perturbed', 'restored')]
        require(files['control/original.o'] == files[f'after/{ctl["object"]}'], 'control baseline mismatch')
        require(original.read_bytes() == restored.read_bytes(), 'control object not restored byte-identically')
        require(bool(objsame.compare(original, perturbed)), 'negative control failed to change semantics')
    return cert


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='command', required=True)
    spec = sub.add_parser('spec')
    spec.add_argument('--output', type=Path, required=True)
    check = sub.add_parser('verify')
    check.add_argument('--receipt', type=Path, required=True)
    check.add_argument('--proof-root', type=Path, required=True)
    check.add_argument('--host', required=True)
    current = sub.add_parser('check')
    current.add_argument('--contract', type=Path, required=True)
    args = parser.parse_args()
    contract = canonical_spec()
    if args.command == 'spec':
        args.output.write_text(json.dumps(contract, indent=2, sort_keys=True) + '\n')
        return
    if args.command == 'check':
        require(contract == json.loads(args.contract.read_text()), 'proof contract changed after verification')
        return
    root = args.proof_root.resolve(strict=True)
    path = args.receipt.resolve(strict=True)
    require(path.is_relative_to(root), 'receipt outside trusted proof directory')
    cert = verify(json.loads(path.read_text()), root, contract, args.host)
    require(contract == canonical_spec(), 'inputs changed during remote verification')
    target = ROOT / 'build/hookproof-land.cert'
    target.parent.mkdir(exist_ok=True)
    pending = target.with_suffix('.cert.pending')
    pending.write_bytes(cert)
    pending.replace(target)
    (ROOT / 'build/remote-proof-contract.json').write_text(json.dumps(contract, sort_keys=True) + '\n')
    print(f'REMOTE PROOF OK: {len(contract["units"])} objects; control changed/restored; {digest(path.read_bytes())}')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, KeyError, OSError, tarfile.TarError) as exc:
        sys.exit(f'remote proof rejected: {exc}')
