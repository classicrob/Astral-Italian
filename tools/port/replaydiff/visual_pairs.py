#!/usr/bin/env python3
"""Rank private, provenance-bound rendered image pairs without auto-alignment."""
import argparse
import csv
import hashlib
import json
import math
from fractions import Fraction
import html
import re
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


COMPARISON_POLICY = {
    'name': 'common-grid-v1', 'max_long_side': 256,
    'resize_filter': 'LANCZOS', 'gaussian_radius': 0.75,
    'alignment': 'full viewport; declared display aspect; no crop/translation/temporal fitting',
    'rank_weights': {'robust_rgb_rmse': 0.45, 'one_minus_ssim': 0.45, 'worst_tile_rgb_mae': 0.10},
    'interpretation': 'review ranking only; no semantic equivalence threshold or PASS',
}


def geometry(size, declared=None):
    declared = declared or {'kind':'game-viewport', 'pixel_aspect_ratio':[1,1],
                            'display_aspect_ratio':[size[0],size[1]], 'color_space':'sRGB'}
    if declared.get('kind') != 'game-viewport' or declared.get('color_space') != 'sRGB':
        raise ValueError('bound opaque sRGB game viewport required')
    def ratio(key):
        values = declared.get(key)
        if not isinstance(values,list) or len(values)!=2 or any(type(v) is not int or v<=0 for v in values):
            raise ValueError('unknown/invalid image aspect: '+key)
        return Fraction(*values)
    aspect = Fraction(size[0],size[1])*ratio('pixel_aspect_ratio')
    if aspect != ratio('display_aspect_ratio'):
        raise ValueError('pixel extent/aspect does not match declared display aspect')
    return aspect


def normalized_images(first, second, first_geometry=None, second_geometry=None):
    from PIL import Image
    with Image.open(first) as left, Image.open(second) as right:
        if min(*left.size,*right.size)<8:
            raise ValueError('images are too small')
        for image in (left,right):
            if image.convert('RGBA').getchannel('A').getextrema() != (255,255):
                raise ValueError('transparent viewport is not supported')
        aspect=geometry(left.size,first_geometry)
        if aspect != geometry(right.size,second_geometry):
            raise ValueError('display aspect differs; no crop or registration is allowed')
        a,b=left.convert('RGB'),right.convert('RGB')
        if aspect>=1:
            width=min(256,a.width,b.width,int(a.height*aspect),int(b.height*aspect))
            height=round(width/aspect)
        else:
            height=min(256,a.height,b.height,int(a.width/aspect),int(b.width/aspect))
            width=round(height*aspect)
        if min(width,height)<8:raise ValueError('common grid is too small')
        size=(width,height)
        return a.resize(size,Image.Resampling.LANCZOS),b.resize(size,Image.Resampling.LANCZOS),{
            'native_pixels':list(a.size),'dolphin_pixels':list(b.size),'comparison_pixels':list(size),
            'display_aspect_ratio':[aspect.numerator,aspect.denominator],
            'pixel_geometry_only':True,'spatial_registration':False}


def image_metrics(a,b):
    from PIL import ImageChops, ImageStat
    stats=ImageStat.Stat(ImageChops.difference(a,b))
    mae=sum(stats.mean)/3/255
    rmse=math.sqrt(sum(v*v for v in stats.rms)/3)/255
    ga,gb=a.convert('L'),b.convert('L')
    ssim=[];tile_error=[]
    c1,c2=(0.01*255)**2,(0.03*255)**2
    for y in range(0,a.height,8):
        for x in range(0,a.width,8):
            box=(x,y,min(x+8,a.width),min(y+8,a.height))
            av,bv=list(ga.crop(box).get_flattened_data()),list(gb.crop(box).get_flattened_data())
            count=len(av);am,bm=sum(av)/count,sum(bv)/count
            va=sum((v-am)**2 for v in av)/count;vb=sum((v-bm)**2 for v in bv)/count
            cov=sum((aa-am)*(bb-bm) for aa,bb in zip(av,bv))/count
            ssim.append(((2*am*bm+c1)*(2*cov+c2))/((am*am+bm*bm+c1)*(va+vb+c2)))
            tile_error.append(sum(ImageStat.Stat(ImageChops.difference(a.crop(box),b.crop(box))).mean)/3/255)
    def dhash(image):
        thumb=list(image.resize((9,8)).get_flattened_data())
        return sum((thumb[y*9+x]>thumb[y*9+x+1])<<(y*8+x) for y in range(8) for x in range(8))
    return dict(normalized_rgb_mae=mae,normalized_rgb_rmse=rmse,
                luma_block_ssim_8x8=sum(ssim)/len(ssim),worst_tile_rgb_mae=max(tile_error),
                dhash64_hamming=(dhash(ga)^dhash(gb)).bit_count())


def metrics(first, second, first_geometry=None, second_geometry=None):
    from PIL import ImageFilter
    a,b,normalization=normalized_images(first,second,first_geometry,second_geometry)
    unfiltered=image_metrics(a,b)
    robust=image_metrics(a.filter(ImageFilter.GaussianBlur(.75)),b.filter(ImageFilter.GaussianBlur(.75)))
    distance=.45*robust['normalized_rgb_rmse']+.45*max(0,min(1,1-robust['luma_block_ssim_8x8']))+.10*robust['worst_tile_rgb_mae']
    return dict(width=a.width,height=a.height,**unfiltered,robust=robust,visual_distance=distance,
                normalization=normalization,policy=COMPARISON_POLICY,
                interpretation='heuristic review distance; no gameplay/readiness/render-equivalence PASS')


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


def inspect_provider(provider, stage, scenario, offset, role=None):
    image=bound_file(provider['image'])
    sync_path=bound_file(provider['sync']);origin=json.loads(sync_path.read_text())
    if origin.get('phase')!='post-endFrame' or origin.get('stage')!=stage or type(origin.get('scenario')) is not int or origin['scenario']!=scenario:
        raise ValueError('sync receipt stage/scenario/seam mismatch')
    if type(origin.get('source_scene_epoch')) is not int or origin['source_scene_epoch']<0:
        raise ValueError('unknown source scene epoch')
    if type(origin.get('source_frame')) is not int or origin['source_frame']<0:
        raise ValueError('unknown source origin')
    trace=bound_file(provider['trace']);selected={};epoch=None
    with trace.open() as stream:
        for expected,row in enumerate(csv.DictReader(stream)):
            if expected>offset:break
            if row.get('frame')!=str(expected):raise ValueError('missing/duplicate/noncontiguous trace frame')
            if any(row.get(k)!=v for k,v in dict(scene='Game',stage=stage,scenario=str(scenario),init_state='6',player_valid='1',scene_valid='1').items()):
                raise ValueError('ready/player context unavailable or changed inside capture interval')
            if not row.get('scene_epoch','').isdecimal():raise ValueError('unknown trace scene epoch')
            if epoch is None:epoch=row['scene_epoch']
            if row['scene_epoch']!=epoch:raise ValueError('capture crosses a scene epoch')
            if expected in (0,offset):selected[expected]=row
    if set(selected)!={0,offset}:raise ValueError('missing actual ready/capture row')
    source=provider.get('capture_source_frame')
    if type(source) is not int or source!=origin['source_frame']+offset:
        raise ValueError('capture source frame is not the declared post-ready offset')
    if provider.get('presentation_binding')!='frame-id-verified' or not provider.get('presentation_receipt'):
        raise ValueError('actual image presentation/frame binding unavailable')
    presentation=json.loads(bound_file(provider['presentation_receipt']).read_text())
    required=dict(schema_version=1,status='captured',stage=stage,scenario=scenario,
                  ready_source_frame=origin['source_frame'],capture_source_frame=source,
                  rendered_source_frame=source,image_sha256=provider['image']['sha256'])
    if any(type(presentation.get(k)) is not type(v) or presentation.get(k)!=v for k,v in required.items()) or presentation.get('render_fence_complete') is not True:
        raise ValueError('presentation receipt does not bind actual image/frame/fence')
    frame_id=presentation.get('rendered_frame_id')
    if type(frame_id) is not int or frame_id<0 or type(presentation.get('readback_frame_id')) is not int or presentation['readback_frame_id']!=frame_id:
        raise ValueError('renderer/readback frame IDs differ or are unknown')
    expected_method={'native':'native-renderer-readback','dolphin':'dolphin-xfb-present-readback'}
    if presentation.get('method') not in expected_method.values() or role and presentation['method']!=expected_method[role]:
        raise ValueError('presentation provider method/role mismatch')
    if not isinstance(provider.get('host'),str) or not re.fullmatch(r'[0-9a-f]{64}',str(provider.get('executable_sha256',''))) or not re.fullmatch(r'[0-9a-f]{40}',str(provider.get('source_revision',''))):
        raise ValueError('unknown provider host/binary/source identity')
    for field in ('host','executable_sha256','source_revision','renderer','image_geometry'):
        if not provider.get(field) or presentation.get(field)!=provider[field]:
            raise ValueError('missing/unbound provider provenance: '+field)
    from PIL import Image
    with Image.open(image) as pixels:geometry(pixels.size,provider['image_geometry'])
    actual_input=bound_file(provider['input'])
    delivery=json.loads(bound_file(provider['input_delivery_receipt']).read_text())
    method={'native':'native-consumed-read-log','dolphin':'dolphin-actual-export'}
    required=dict(schema_version=1,status='captured',input_sha256=digest(actual_input),frames=600,read_count=2400,
                  source_frame_start=origin['source_frame']+1,source_frame_end=origin['source_frame']+600)
    if any(type(delivery.get(k)) is not type(v) or delivery.get(k)!=v for k,v in required.items()) or delivery.get('method') not in method.values() or role and delivery.get('method')!=method[role]:
        raise ValueError('actual delivered input receipt missing or mismatched')
    if presentation.get('input_sha256')!=provider['input']['sha256']:
        raise ValueError('presentation provenance not bound to delivered input')
    return image,selected[offset]


def html_report(report,output):
    rows=[]
    ranked=sorted(report['results'],key=lambda r:r.get('rank',10**9))
    for r in ranked:
        title=html.escape(f"{r.get('label','missing provider')} / {r['stage']} / scenario {r['scenario']} / ready+{r['offset']}")
        if r['measurement_status']=='NOTMEASURED':
            rows.append('<section><h2>'+title+'</h2><p>NOTMEASURED / BLOCKED: '+html.escape(r['error'])+'</p></section>');continue
        m=r['metrics'];images=''
        if r.get('retained_directory'):
            folder=Path(r['retained_directory']).name
            images=''.join('<a href="'+html.escape(folder+'/'+name+'.png',quote=True)+'"><img alt="'+name+'" src="'+html.escape(folder+'/'+name+'.png',quote=True)+'"></a>' for name in ('native-grid','dolphin-grid','difference'))
        pins=html.escape(json.dumps({'ready':r['ready_origins'],'producers':{role:{k:r['provenance'][role][k] for k in ('host','source_revision','executable_sha256','renderer','image_geometry','capture_source_frame')} for role in ('native','dolphin')},'input_sha256':r['input_sha256']},indent=2))
        rows.append('<section><h2>'+str(r['rank'])+'. '+title+'</h2><p>UNQUALIFIED; distance '+format(m['visual_distance'],'.6f')+'; RGB RMSE '+format(m['normalized_rgb_rmse'],'.6f')+'; SSIM8 '+format(m['luma_block_ssim_8x8'],'.6f')+'</p>'+images+'<details><summary>Bound provenance</summary><pre>'+pins+'</pre></details></section>')
    (output/'report.html').write_text('<!doctype html><meta charset="utf-8"><title>Visual pair review</title><style>body{font:16px system-ui;margin:2rem;background:#151820;color:#eef}section{padding:1rem;border-top:1px solid #667}img{max-width:31%;image-rendering:auto}a{color:#adf}</style><h1>Visual pair review — qualification BLOCKED</h1><p>Fixed common grid; no translation/crop/time fitting. Heuristics tested on synthetic fixtures; similarity is not a gameplay verdict.</p>'+''.join(rows)+('<p><strong>SYNTHETIC FIXTURES ONLY</strong></p>' if report['synthetic'] else '')+'<p>Full provenance and metric policy: <a href="report.json">report.json</a></p>')


def compare_manifest(manifest, output, keep=10):
    manifest_bytes=manifest.read_bytes()
    manifest_sha256=hashlib.sha256(manifest_bytes).hexdigest()
    data=json.loads(manifest_bytes)
    if data.get('schema_version')!=1 or not isinstance(data.get('pairs'),list) or not data['pairs']:raise ValueError('manifest must declare schema1 and nonempty pairs')
    if not 1<=keep<=100:raise ValueError('keep must be 1..100')
    if output.exists():raise ValueError('use a fresh private output directory')
    output.mkdir(parents=True);results=[]
    for index,pair in enumerate(data['pairs']):
        if not isinstance(pair,dict):
            results.append(dict(index=index,stage=None,scenario=None,offset=None,status='BLOCKED',measurement_status='NOTMEASURED',error='pair is not an object'))
            continue
        entry=dict(index=index,label=pair.get('label',str(index)),stage=pair.get('stage'),scenario=pair.get('scenario'),offset=pair.get('offset'),
                   status='BLOCKED',measurement_status='NOTMEASURED')
        try:
            if not isinstance(pair.get('stage'),str) or not pair['stage'] or not all(c.isascii() and (c.isalnum() or c=='_') for c in pair['stage']):raise ValueError('noncanonical stage name')
            if type(pair.get('scenario')) is not int or pair['scenario']<=0:raise ValueError('invalid scenario')
            if type(pair.get('offset')) is not int or pair['offset'] not in (60,300,600):raise ValueError('unsupported capture offset')
            left,native=inspect_provider(pair['native'],pair['stage'],pair['scenario'],pair['offset'],'native')
            right,dolphin=inspect_provider(pair['dolphin'],pair['stage'],pair['scenario'],pair['offset'],'dolphin')
            if pair['native']['input']['sha256']!=pair['dolphin']['input']['sha256'] or pair.get('input_kind')!='idle':raise ValueError('matching delivered idle inputs missing')
            validate_idle(bound_file(pair['native']['input']))
            measured=metrics(left,right,pair['native']['image_geometry'],pair['dolphin']['image_geometry'])
            for provider in (pair['native'],pair['dolphin']):
                for field in ('image','sync','trace','input','presentation_receipt','input_delivery_receipt'):
                    bound_file(provider[field])
            entry.update(status='UNQUALIFIED_VISUAL_DIAGNOSTIC',measurement_status='MEASURED_UNQUALIFIED',
                         metrics=measured,
                         input_sha256=pair['native']['input']['sha256'],native_state=native,dolphin_state=dolphin,
                         provenance=dict(native=pair['native'],dolphin=pair['dolphin']),
                         ready_origins={role:json.loads(bound_file(pair[role]['sync']).read_text()) for role in ('native','dolphin')})
        except (ValueError,KeyError,OSError,TypeError) as exc:entry['error']=str(exc)
        results.append(entry)
    groups={(r['stage'],r['scenario']) for r in results if isinstance(r['stage'],str) and type(r['scenario']) is int}
    for stage,scenario in sorted(groups):
        for offset in (60,300,600):
            if not any(r['stage']==stage and r['scenario']==scenario and r['offset']==offset for r in results):
                results.append(dict(index=None,stage=stage,scenario=scenario,offset=offset,status='BLOCKED',
                                    measurement_status='NOTMEASURED',error='requested ready-offset pair absent; no provider frame inferred'))
    ranked=sorted((r for r in results if 'metrics' in r),key=lambda r:(-r['metrics']['visual_distance'],r['index']))
    retained=[]
    for rank,row in enumerate(ranked):
        row['rank']=rank+1
        if rank>=keep:continue
        folder=output/f'{rank+1:02d}-{row["stage"]}-{row["offset"]}';folder.mkdir()
        original=data['pairs'][row['index']]
        for provider in ('native','dolphin'):
            source=bound_file(original[provider]['image']);target=folder/(provider+source.suffix)
            shutil.copyfile(source,target)
            if digest(target)!=original[provider]['image']['sha256']:raise ValueError('retained image hash changed')
        a,b,_=normalized_images(bound_file(original['native']['image']),bound_file(original['dolphin']['image']),original['native']['image_geometry'],original['dolphin']['image_geometry'])
        from PIL import ImageChops
        a.save(folder/'native-grid.png');b.save(folder/'dolphin-grid.png');ImageChops.difference(a,b).save(folder/'difference.png')
        row['retained_directory']=str(folder)
        (folder/'pair.json').write_text(json.dumps(row,indent=2)+'\n');retained.append(str(folder))
    if digest(manifest)!=manifest_sha256:raise ValueError('manifest changed during comparison')
    report=dict(schema_version=1,metric_schema_version=2,status='UNQUALIFIED_VISUAL_DIAGNOSTIC' if ranked else 'BLOCKED',
                manifest_sha256=manifest_sha256,synthetic=data.get('synthetic') is True,metric_policy=COMPARISON_POLICY,results=results,retained=retained,
                measured_pairs=len(ranked),notmeasured_pairs=len(results)-len(ranked),qualification='BLOCKED',measured_qualified_frames=None)
    (output/'report.json').write_text(json.dumps(report,indent=2)+'\n');html_report(report,output)
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
