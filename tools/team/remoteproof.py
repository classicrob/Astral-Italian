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
import os
from pathlib import Path, PurePosixPath
import runpy
import subprocess
import sys
import tarfile
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parent))
import objsame
import mario52

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
INPUTS = ('src', 'include', 'libs', 'config')
GENERATORS = ('configure.py', 'tools/project.py', 'tools/ninja_syntax.py', 'tools/team/objsame.py')


APPROVED_NATIVE_52 = {
    "a75c18f6a3b942975ba827151c53b91c4b4cff2a": [
        "CMakeLists.txt",
        "native/cmake/LegacySources.cmake",
        "native/input/CMakeLists.txt",
        "native/resource/rfl/CMakeLists.txt",
        "native/tests/legacy_source_build_tests.py",
        "native/tests/source_encoding_tests.py",
        "native/tools/encode_legacy_sources.py"
    ],
    "21933226cdebc7decdc0652af28d1b8bb6465037": [
        "docs/port/headless-virtual-io.md",
        "native/app/headless/main.cpp",
        "native/app/headless/platform.cpp",
        "native/app/headless/reload_policy.hpp",
        "native/platform/CMakeLists.txt",
        "native/platform/dvd/dvd_host.cpp",
        "native/platform/gx_sync/gx_sync.cpp",
        "native/platform/include/petari/platform/virtual_time.hpp",
        "native/platform/nand/nand_async.cpp",
        "native/platform/os/os_thread.cpp",
        "native/tests/headless_reload_repeat.py",
        "native/tests/virtual_async_tests.cpp"
    ],
    "05623e76ad94dea46415c38fdf6e6adaac8c85d2": [
        "native/app/launch_stage.cpp",
        "native/home_menu/include/petari/launch_stage.hpp",
        "native/home_menu/launch_stage.cpp",
        "native/tests/app_launch_stage_tests.cpp",
        "native/tests/level_select_menu_tests.cpp"
    ]
}

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


def base_equivalence(proven, expected, equivalent_base=None, approved_native_52=False):
    if proven == expected:
        return None
    require(equivalent_base and proven.get('base') == equivalent_base,
            'stale or mismatched source/config/tool contract')
    compared = dict(proven)
    compared['base'] = expected['base']
    require(compared == expected, 'equivalent base cannot change other contract fields')
    result = subprocess.run(['git', 'merge-base', '--is-ancestor', equivalent_base, expected['base']],
                            cwd=ROOT, capture_output=True)
    require(result.returncode == 0, 'proof base is not an ancestor of landing base')
    changed = git('diff', '--name-only', equivalent_base, expected['base']).splitlines()
    if approved_native_52:
        require(equivalent_base == mario52.BASE, 'native exception is only approved for the 5.2 base')
        commits = git('rev-list', equivalent_base + '..' + expected['base']).splitlines()
        require(set(commits) == set(APPROVED_NATIVE_52) and len(commits) == 3,
                'unapproved native base commits')
        for commit in commits:
            paths = git('diff-tree', '--no-commit-id', '--name-only', '-r', commit).splitlines()
            require(paths == APPROVED_NATIVE_52[commit], 'approved native commit path mismatch')
    else:
        require(changed and all(p.endswith('.md') for p in changed),
                'base advancement is not documentation-only')
    inputs = {}
    for path in (*INPUTS, 'tools', 'configure.py'):
        before = git('rev-parse', f'{equivalent_base}:{path}')
        current = git('rev-parse', f'{expected["base"]}:{path}')
        require(before == current, f'base compile input changed: {path}')
        if path in INPUTS:
            require(before == expected['before'][path], f'proof before tree mismatch: {path}')
        inputs[path] = before
    return {'proof_base': equivalent_base, 'landing_base': expected['base'],
            'changed_paths': changed, 'approved_native_52': approved_native_52, 'identical_inputs': inputs}


def verify(receipt, root, expected, host, equivalent_base=None, approved_native_52=False, mario_52=False):
    require(receipt['schema'] == 1 and receipt['status'] == 'PASS', 'incomplete remote proof')
    require(receipt['host'] == host, 'unexpected proof host')
    equivalence = base_equivalence(receipt['spec'], expected, equivalent_base, approved_native_52)
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
    require(mario_52 == ('sanctioned_delta' in receipt), 'sanctioned evidence requires explicit Mario52 approval')
    if mario_52:
        mario52.verify_evidence(receipt, root, confined)
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
    wanted_cert = (receipt['spec']['base'] + ' ' + ' '.join(expected['after'][p] for p in INPUTS[:3]) + '\n').encode()
    require(cert == wanted_cert, 'certificate does not bind the original proof')
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
            before, after = dest / 'before' / rel, dest / 'after' / rel
            if mario_52 and rel == mario52.OBJECT:
                mario52.verify_objects(before, after)
                mario52.measure(before, after, ROOT, json.loads(confined(
                    root, receipt['sanctioned_delta']['matching']['path']).read_text()))
            else:
                require(not objsame.compare(before, after), f'Wii semantic difference: {rel}')
        original, perturbed, restored = [dest / 'control' / f'{p}.o' for p in ('original', 'perturbed', 'restored')]
        require(files['control/original.o'] == files[f'after/{ctl["object"]}'], 'control baseline mismatch')
        require(original.read_bytes() == restored.read_bytes(), 'control object not restored byte-identically')
        require(bool(objsame.compare(original, perturbed)), 'negative control failed to change semantics')
    if equivalence:
        return (expected['base'] + ' ' + ' '.join(expected['after'][p] for p in INPUTS[:3]) + '\n').encode()
    return cert


def main():
    os.chdir(ROOT)
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='command', required=True)
    spec = sub.add_parser('spec')
    spec.add_argument('--output', type=Path, required=True)
    check = sub.add_parser('verify')
    check.add_argument('--receipt', type=Path, required=True)
    check.add_argument('--proof-root', type=Path, required=True)
    check.add_argument('--host', required=True)
    check.add_argument('--equivalent-base', help='Explicit original proof base; only a verified docs-only advance is allowed')
    check.add_argument('--approved-native-52', action='store_true', help='Exact three reviewed 5.2 native prerequisite commits only')
    check.add_argument('--mario-52', action='store_true', help='Preapproved pinned Mario22D0 envelope and private-reference metrics')
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
    receipt = json.loads(path.read_text())
    equivalence = base_equivalence(receipt['spec'], contract, args.equivalent_base, args.approved_native_52)
    cert = verify(receipt, root, contract, args.host, args.equivalent_base, args.approved_native_52, args.mario_52)
    require(contract == canonical_spec(), 'inputs changed during remote verification')
    target = ROOT / 'build/hookproof-land.cert'
    target.parent.mkdir(exist_ok=True)
    pending = target.with_suffix('.cert.pending')
    pending.write_bytes(cert)
    pending.replace(target)
    (ROOT / 'build/remote-proof-contract.json').write_text(json.dumps(contract, sort_keys=True) + '\n')
    original_cert = confined(root, receipt['certificate']['path'])
    imported = {'receipt_path': str(path), 'receipt_sha256': digest(path.read_bytes()),
                'original_certificate_path': str(original_cert),
                'original_certificate_sha256': digest(original_cert.read_bytes()),
                'imported_certificate_sha256': digest(cert), 'base_equivalence': equivalence,
                'mario52_sanction': mario52.PLAN_SHA if args.mario_52 else None,
                'contract': contract}
    (ROOT / 'build/remote-proof-import.json').write_text(json.dumps(imported, sort_keys=True) + '\n')
    print(f'REMOTE PROOF OK: {len(contract["units"])} objects; control changed/restored; {digest(path.read_bytes())}')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, KeyError, OSError, tarfile.TarError) as exc:
        sys.exit(f'remote proof rejected: {exc}')
