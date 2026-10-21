#!/usr/bin/env python3
"""Rank private, provenance-bound rendered image pairs without auto-alignment."""
import argparse
import csv
import hashlib
import json
import math
from pathlib import Path
import shutil


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''): h.update(block)
    return h.hexdigest()


def private_path(value):
    path = Path(value).expanduser().resolve()
    if (Path.home() / 'MacioGalaxy2-gamedata') not in path.parents:
        raise ValueError('all manifests, images, receipts and outputs must stay private')
    return path


def bound_file(record):
    path = private_path(record['path'])
    if digest(path) != record['sha256']:
        raise ValueError(f'file hash mismatch: {path}')
    return path


def metrics(first, second):
    from PIL import Image, ImageChops, ImageStat
    with Image.open(first) as left, Image.open(second) as right:
        if left.size != right.size or min(left.size) < 8:
            raise ValueError('image dimensions differ or are too small; no resize/crop/registration is allowed')
        a, b = left.convert('RGB'), right.convert('RGB')
        difference = ImageChops.difference(a, b)
        stats = ImageStat.Stat(difference)
        mae = sum(stats.mean) / 3 / 255
        rmse = math.sqrt(sum(value * value for value in stats.rms) / 3) / 255
        gray_a, gray_b = a.convert('L'), b.convert('L')
        ssim = []
        c1, c2 = (0.01 * 255) ** 2, (0.03 * 255) ** 2
        for y in range(0, a.height, 8):
            for x in range(0, a.width, 8):
                box = (x, y, min(x + 8, a.width), min(y + 8, a.height))
                av, bv = list(gray_a.crop(box).getdata()), list(gray_b.crop(box).getdata())
                count = len(av); am, bm = sum(av) / count, sum(bv) / count
                va = sum((value - am) ** 2 for value in av) / count
                vb = sum((value - bm) ** 2 for value in bv) / count
                cov = sum((aa - am) * (bb - bm) for aa, bb in zip(av, bv)) / count
                ssim.append(((2 * am * bm + c1) * (2 * cov + c2)) / ((am * am + bm * bm + c1) * (va + vb + c2)))
        def dhash(image):
            thumb = list(image.resize((9, 8), Image.Resampling.LANCZOS).getdata())
            return sum((thumb[y * 9 + x] > thumb[y * 9 + x + 1]) << (y * 8 + x) for y in range(8) for x in range(8))
        hamming = bin(dhash(gray_a) ^ dhash(gray_b)).count('1')
        return dict(width=a.width, height=a.height, normalized_rgb_mae=mae, normalized_rgb_rmse=rmse,
                    luma_block_ssim_8x8=sum(ssim) / len(ssim), dhash64_hamming=hamming,
                    interpretation='heuristic visual distance; no gameplay, readiness or render-equivalence PASS')


def validate_idle(path):
    from kpad_script import contract
    rows = iter(contract.rows_from_jsonl(path))
    for frame in range(600):
        for channel in range(4):
            sample = contract.idle_sample(channel == 0)
            error = 0 if channel == 0 else -1
            expected = dict(frame=frame, read_ordinal=channel, channel=channel, requested_length=120,
                            returned_count=1 if channel == 0 else 0, error=error,
                            connection=dict(probe_error=error, device_type=1 if channel == 0 else 253), samples=[sample])
            if next(rows, None) != expected:
                raise ValueError('input is not the declared 600-frame canonical connected-idle tape')
    if next(rows, None) is not None:
        raise ValueError('idle tape has extra reads')


def inspect_provider(provider, stage, scenario, offset):
    image = bound_file(provider['image'])
    sync_path = bound_file(provider['sync'])
    origin = json.loads(sync_path.read_text())
    if origin.get('phase') != 'post-endFrame' or origin.get('stage') != stage or origin.get('scenario') != scenario:
        raise ValueError('sync receipt stage/scenario/seam mismatch')
    if type(origin.get('source_frame')) is not int or origin['source_frame'] < 0:
        raise ValueError('unknown source origin')
    trace = bound_file(provider['trace'])
    selected = {}
    with trace.open() as stream:
        for row in csv.DictReader(stream):
            if row['frame'] in ('0', str(offset)): selected[int(row['frame'])] = row
    for frame in (0, offset):
        row = selected.get(frame)
        if row is None or row.get('scene') != 'Game' or row.get('stage') != stage or row.get('scenario') != str(scenario) or row.get('init_state') != '6' or row.get('player_valid') != '1':
            raise ValueError('missing actual ready/player trace row or capture crosses a scene boundary')
    if selected[0].get('scene_epoch') != selected[offset].get('scene_epoch'):
        raise ValueError('capture offset crosses a scene epoch')
    if provider['capture_source_frame'] != origin['source_frame'] + offset:
        raise ValueError('capture source frame is not the declared post-ready offset')
    if provider.get('presentation_binding') != 'frame-id-verified':
        raise ValueError('image presentation has not been bound to the sampled game frame')
    if not provider.get('presentation_receipt'):
        raise ValueError('actual renderer presentation receipt required')
    presentation = json.loads(bound_file(provider['presentation_receipt']).read_text())
    required = dict(schema_version=1, status='captured', stage=stage, scenario=scenario,
                    ready_source_frame=origin['source_frame'],
                    capture_source_frame=provider['capture_source_frame'],
                    rendered_source_frame=provider['capture_source_frame'],
                    image_sha256=provider['image']['sha256'], render_fence_complete=True)
    if any(presentation.get(key) != value for key, value in required.items()):
        raise ValueError('presentation receipt does not bind the actual image/frame/fence')
    frame_id = presentation.get('rendered_frame_id')
    if type(frame_id) is not int or frame_id < 0 or presentation.get('readback_frame_id') != frame_id:
        raise ValueError('renderer and image-readback frame IDs differ or are unknown')
    if presentation.get('method') not in ('native-renderer-readback', 'dolphin-xfb-present-readback'):
        raise ValueError('unknown presentation provider method')
    for field in ('host', 'executable_sha256', 'source_revision', 'renderer'):
        if not provider.get(field): raise ValueError(f'missing provider provenance: {field}')
    return image, selected[offset]


def compare_manifest(manifest, output, keep=10):
    data = json.loads(manifest.read_text())
    if data.get('schema_version') != 1 or not data.get('pairs'):
        raise ValueError('manifest must declare schema 1 and nonempty pairs')
    if output.exists(): raise ValueError('use a fresh private output directory')
    output.mkdir(parents=True)
    results = []
    for index, pair in enumerate(data['pairs']):
        entry = dict(index=index, stage=pair['stage'], scenario=pair['scenario'], offset=pair['offset'], status='BLOCKED')
        try:
            if not isinstance(pair['stage'], str) or not pair['stage'] or not all(c.isascii() and (c.isalnum() or c == '_') for c in pair['stage']):
                raise ValueError('noncanonical stage name')
            if type(pair['scenario']) is not int or pair['scenario'] <= 0: raise ValueError('invalid scenario')
            if pair['offset'] not in (60, 300, 600): raise ValueError('unsupported capture offset')
            left, native = inspect_provider(pair['native'], pair['stage'], pair['scenario'], pair['offset'])
            right, dolphin = inspect_provider(pair['dolphin'], pair['stage'], pair['scenario'], pair['offset'])
            for provider in (pair['native'], pair['dolphin']): bound_file(provider['input'])
            if pair['native']['input']['sha256'] != pair['dolphin']['input']['sha256'] or pair.get('input_kind') != 'idle':
                raise ValueError('matching delivered idle-input receipt is missing')
            validate_idle(bound_file(pair['native']['input']))
            entry.update(status='UNQUALIFIED_VISUAL_DIAGNOSTIC', metrics=metrics(left, right),
                         input_sha256=pair['native']['input']['sha256'],
                         native_state=native, dolphin_state=dolphin,
                         provenance=dict(native=pair['native'], dolphin=pair['dolphin']))
        except (ValueError, KeyError, OSError) as exc:
            entry['error'] = str(exc)
        results.append(entry)
    ranked = sorted((row for row in results if 'metrics' in row), key=lambda row: row['metrics']['luma_block_ssim_8x8'])
    retained = []
    for rank, row in enumerate(ranked[:keep]):
        folder = output / f'{rank + 1:02d}-{row["stage"]}-{row["offset"]}'
        folder.mkdir()
        original = data['pairs'][row['index']]
        for provider in ('native', 'dolphin'):
            source = bound_file(original[provider]['image'])
            shutil.copyfile(source, folder / (provider + source.suffix))
        (folder / 'pair.json').write_text(json.dumps(row, indent=2) + '\n')
        retained.append(str(folder))
    report = dict(schema_version=1, status='UNQUALIFIED_VISUAL_DIAGNOSTIC' if ranked else 'BLOCKED', manifest_sha256=digest(manifest),
                  metric_policy='same-size full image; no crop, resize, temporal shift or image registration; rank by luma block SSIM8',
                  results=results, retained=retained, qualification='BLOCKED', measured_qualified_frames=None)
    (output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', required=True)
    parser.add_argument('--out', required=True)
    parser.add_argument('--keep', type=int, default=10)
    args = parser.parse_args()
    if not 1 <= args.keep <= 100: parser.error('--keep must be 1..100')
    compare_manifest(private_path(args.manifest), private_path(args.out), args.keep)


if __name__ == '__main__': main()
