#!/usr/bin/env python3
"""Pinned-source/retail visual contract review only; never admit a live capture."""
import argparse
import csv
import hashlib
import json
from pathlib import Path
import re
import struct
from PIL import Image
from visual_pairs import bound_file, digest, geometry
from visual_readiness import ReadySchedule
from native_draw_phase import typed_draw_eligibility

PINS=json.loads(Path(__file__).with_name('dolphin_visual_source_pins.json').read_text())
TAIL_FRAMES=16


def need(value, reason):
    if not value:raise ValueError(reason)


def equal(actual, expected, reason):
    need(type(actual) is type(expected) and actual==expected,reason)


def integer(value, reason):
    need(type(value) is int and value>=0,reason)
    return value


def audit_sources(root,dol,symbols):
    root=Path(root)
    for name,expected in PINS['files'].items():
        equal(digest(root/name),expected,'Dolphin source drift: '+name)
    dol=Path(dol);symbols=Path(symbols)
    equal(digest(dol),PINS['retail']['dol_sha256'],'retail DOL hash mismatch')
    equal(digest(symbols),PINS['retail']['symbols_sha256'],'retail symbol binding mismatch')
    data=dol.read_bytes();sections=[]
    for i in range(18):
        offset,address,size=(struct.unpack_from('>I',data,k+4*i)[0] for k in (0,0x48,0x90))
        if size:sections.append((offset,address,size))
    for name,guard in PINS['retail']['guards'].items():
        address=int(guard['address'],16)
        section=next((s for s in sections if s[1]<=address and address+4<=s[1]+s[2]),None)
        need(section is not None,'unmapped retail site: '+name)
        offset,base,_=section
        equal(data[offset+address-base:offset+address-base+4].hex(),guard['word'],'retail site changed: '+name)
    return dict(dolphin_revision=PINS['dolphin_revision'],source_files=len(PINS['files']),
                retail=PINS['retail'],runtime_observed=False)


def ready_request(e):
    schedule=ReadySchedule(e['stage'],e['scenario'])
    request=None;last=-1
    with bound_file(e['trace']).open() as stream:
        for source,row in enumerate(csv.DictReader(stream)):
            found=schedule.observe(row,source);last=source
            if found and found['offset']==e['offset']:request=found
    need(not schedule.cancelled,'ready/epoch changed; no reanchor')
    need(request is not None and request['status']=='PENDING_PRESENTATION','actual ready/offset/player unavailable')
    equal(last,schedule.origin['source_frame']+600+TAIL_FRAMES,'exact observation-tail stop is absent/exceeded')
    stop=e['cpu_stop']
    for key,value in dict(status='stopped',phase='post-endFrame',source_frame=last,terminal_source_frame=last,
                          resume_after_stop_count=0,emulation_advanced_after_stop=False,input_frames=600+TAIL_FRAMES).items():
        equal(stop.get(key),value,'unproven CPU stop bound: '+key)
    integer(stop['event'],'unknown CPU stop event')
    return request,schedule.origin


def idle_delivery(e,origin,method='dolphin-actual-export'):
    from kpad_script import contract
    canonical=bound_file(e['input']);actual=bound_file(e['actual_input'])
    total=600+TAIL_FRAMES
    wanted=list(contract.rows_from_jsonl(canonical));observed=list(contract.rows_from_jsonl(actual))
    equal(len(wanted),total*4,'idle tape/tail read count mismatch')
    start=origin['source_frame']+1;end=start+total-1
    suffix=[r for r in observed if start<=r['frame']<=end]
    equal(len(suffix),len(wanted),'actual read suffix missing/extra reads')
    for index,row in enumerate(wanted):
        frame,channel=divmod(index,4);error=0 if channel==0 else -1
        expected=dict(frame=frame,read_ordinal=channel,channel=channel,requested_length=120,
                      returned_count=1 if channel==0 else 0,error=error,
                      connection=dict(probe_error=error,device_type=1 if channel==0 else 253),
                      samples=[contract.idle_sample(channel==0)])
        equal(row,expected,'noncanonical idle input/tail')
        equal(suffix[index],dict(expected,frame=start+frame),'actual KPAD delivery differs; not gameplay divergence')
    export=e['input_export']
    for key,value in dict(status='captured',sha256=digest(actual),bytes=actual.stat().st_size,read_count=len(observed)).items():
        equal(export.get(key),value,'actual closed input_export mismatch: '+key)
    original=bound_file(export['original_raw_export'])
    equal(digest(original),digest(actual),'actual export is not the separately pinned original raw export')
    closed=json.loads(bound_file(export['closed_receipt']).read_text())
    census_path=bound_file(export['read_census'])
    census=json.loads(census_path.read_text())
    for key,value in dict(status='closed',method=method,sha256=digest(original),bytes=original.stat().st_size,
                          read_count=len(observed),source_start=0,source_end=end,census_sha256=digest(census_path)).items():
        equal(closed.get(key),value,'original collector closure mismatch: '+key)
    equal(len(census),end+1,'pre-anchor/full read census coverage missing')
    grouped=[0]*(end+1)
    previous=(-1,-1)
    for row in observed:
        frame=integer(row['frame'],'invalid original source frame')
        need(frame<=end,'original input extends beyond CPU stop')
        ordinal=integer(row['read_ordinal'],'invalid original read ordinal')
        need((frame,ordinal)>previous,'original read history reordered/duplicated')
        equal(ordinal,grouped[frame],'missing original read ordinal')
        grouped[frame]+=1;previous=(frame,ordinal)
    for frame,item in enumerate(census):
        equal(item.get('source_frame'),frame,'read census has shifted/missing source seams')
        equal(item.get('reads'),grouped[frame],'original prefix/full read census differs')
    return dict(canonical_sha256=digest(canonical),actual_export_sha256=digest(actual),
                original_export_sha256=digest(original),closed_receipt_sha256=digest(bound_file(export['closed_receipt'])),
                source_start=start,source_end=end,frames=total,read_count=total*4,
                actual_equals_pinned_original=True,preanchor_coverage='verified against complete per-seam read census')



def review_draw_scope(e,request,origin,retail_sites=True):
    coverage=e['draw_coverage']
    equal(coverage['complete'],True,'draw ordering coverage incomplete')
    equal(coverage['request_source_frame'],request['capture_source_frame'],'request was shifted/rebased')
    event=integer(coverage['request_event'],'unknown request event')
    draws=coverage['draws']
    need(bool(draws),'no actual next draw observation')
    entries=[integer(d['entry_event'],'unknown draw entry') for d in draws]
    need(entries==sorted(set(entries)) and all(n>event for n in entries),'draw history is missing/reordered')
    draw=draws[0]
    equal(e['selected_draw_id'],draw['id'],'later convenient draw selected instead of first next draw')
    if retail_sites:
        for key,site in [('entry_pc','draw_call'),('exit_pc','draw_return'),('selected_scene_pc','selected_scene_return')]:
            equal(draw[key],PINS['retail']['guards'][site]['address'],'unbound retail '+key)
        equal(draw['guest_cpu_stopped'],True,'retail mutable state not stopped')
    else:
        equal(draw['entry_hook'],'after-beginRender/before-GameSystem::draw','unbound native draw entry')
        equal(draw['exit_hook'],'after-GameSystem::draw/before-endRender','unbound native draw exit')
    need(draw['entry_event']<draw['selected_scene_event']<draw['exit_event'],'draw/selected-scene ordering invalid')
    equal(coverage['next_source_frame'],request['capture_source_frame']+1,'next observed seam is not adjacent to request')
    need(draw['exit_event']<coverage['next_seam_event']<=e['cpu_stop']['event'],'draw is outside witnessed source-seam/stop interval')
    equal(draw['after_source_seam'],request['capture_source_frame'],'draw occurred after a different/later source seam')
    equal(draw['before_source_seam'],coverage['next_source_frame'],'draw lacks next-seam upper-bound witness')
    eligible=typed_draw_eligibility(draw['snapshot'],e['stage'],e['scenario'])
    need(eligible['typed_candidate'],'next draw is not actual ready Game/player/placed target: '+','.join(eligible['reasons']))
    equal(draw['snapshot']['scene_epoch'],origin['source_scene_epoch'],'draw crosses sampled scene epoch')
    for key in ('scene_valid','scene','stage','placed_scenario','init_state','current_scene_nonnull','executed_scene_is_current','canonical_player_valid','game_cpu_owned','scene_epoch','scene_epoch_kind'):
        equal(draw['exit_snapshot'].get(key),draw['snapshot'][key],'draw-exit identity/readiness differs: '+key)
    equal(draw['exit_snapshot'].get('observation_phase'),'after-GameSystem::draw','draw-exit phase is unbound')
    return draw


def review(e):
    result=dict(schema_version=1,status='BLOCKED',measurement_status='NOTMEASURED',
                qualification='BLOCKED',qualified_frames=None,runtime_provider_accepted=False,
                synthetic=e.get('synthetic') is True,presentation_candidate=None,reasons=[])
    try:
        need(type(e['offset']) is int and e['offset'] in (60,300,600),'unknown requested offset')
        equal(e['sampling_definition'],'post-seam-request/next-observed-draw-v2','phase contract not declared')
        equal(e['tail_frames'],TAIL_FRAMES,'unreviewed observation tail')
        equal(e['source_revision'],PINS['dolphin_revision'],'unpinned Dolphin base source')
        for key in ('executable_sha256','instrumentation_manifest_sha256'):
            need(re.fullmatch('[0-9a-f]{64}',str(e.get(key))) is not None,'unknown producer: '+key)
        request,origin=ready_request(e)
        result.update(request=request,ready_origin=origin,input_binding=idle_delivery(e,origin))
        draw=review_draw_scope(e,request,origin)
        copy=e['copy']
        for key,value in dict(draw_id=draw['id'],kind='xfb',buffer_mode=3,current_render=True,lineage_complete=True).items():
            equal(copy.get(key),value,'unsupported/unbound copy '+key)
        need(draw['exit_event']<copy['endGX_event']<copy['event']<e['draw_coverage']['next_seam_event'],'copy does not follow draw/endGX within witnessed interval')
        equal(copy['observed_source_seam'],draw['after_source_seam'],'copy source-seam witness mismatch')
        equal(copy['efb_writers'],['current-draw','endGX'],'unknown/mixed EFB writer lineage')
        equal(copy['clear_before_draw_proven'],True,'retained EFB attribution unbound')
        selection=e['selection']
        history=e['selection_history']
        equal(history['complete'],True,'VI selection history incomplete')
        selected_events=history['events']
        order=[integer(s['event'],'unknown VI selection event') for s in selected_events]
        need(order==sorted(set(order)) and all(n>copy['event'] for n in order),'VI selection history order invalid')
        matching=[s for s in selected_events if copy['id'] in s.get('copy_ids',[])]
        need(bool(matching),'target copy was not selected')
        equal(selection,matching[0],'later selection substituted for first matching selection')
        for key,value in dict(reason='VideoInterface',duplicate=False,stitched=False,
                              copy_ids=[copy['id']],xfb_entry_id=copy['xfb_entry_id'],settings_unchanged=True).items():
            equal(selection.get(key),value,'VI selection not bound: '+key)
        need(selection['event']>copy['event'],'selection precedes target copy')
        need(request['capture_source_frame']<=integer(selection['originating_source_seam'],'unknown VI source seam')<=e['cpu_stop']['source_frame'],'VI selection originated beyond emulation bound')
        dump=e['dump']
        for key,value in dict(present_id=selection['present_id'],xfb_entry_id=selection['xfb_entry_id'],
                              copy_ids=[copy['id']],resolution_policy='XFBRawResolution',
                              includes_host_overlay=False).items():
            equal(dump.get(key),value,'dump provenance mismatch: '+key)
        need(dump['event']>selection['event'],'dump precedes selection')
        equal(dump['pending_overwrite_observed'],False,'pending frame dump metadata was overwritten')
        gpu=e['gpu']
        equal(gpu['staging_generation'],dump['staging_generation'],'readback staging resource version differs')
        equal(gpu['dump_id'],dump['id'],'readback belongs to another dump')
        for key in ('flush_returned','map_success','error_status_observed'):
            equal(gpu.get(key),True,'GPU completion missing: '+key)
        completion={'Metal':{'command-buffer-completed-no-error'},
                    'OGL':{'fence-signalled-no-error','blocking-map-complete-no-error'},
                    'Vulkan':{'fence-counter-complete-no-error'}}
        need(gpu.get('completion_status') in completion.get(gpu.get('backend'),set()),'Map/Flush alone is not successful GPU completion')
        need(gpu['event']>dump['event'],'GPU readback completion order invalid')
        image=bound_file(e['image'])
        with Image.open(image) as pixels:
            need(pixels.format=='PNG','image is not PNG');pixels.verify()
        with Image.open(image) as pixels:
            pixels.load();size=pixels.size
            need(pixels.convert('RGBA').getchannel('A').getextrema()==(255,255),'transparent viewport')
        geometry(size,e['image_geometry'])
        equal(e['color_transfer'],'recorded-and-validated-srgb-code-values','unknown color encoding')
        equal(dump['source_rect'],[0,0,*size],'source crop/geometry not bound')
        equal(dump['target_rect'],[0,0,*size],'dump scaling/geometry not qualified by this bounded proposal')
        png=e['png']
        for key,value in dict(dump_id=dump['id'],checked_write=True,checked_flush=True,checked_close=True,
                              atomic_publish=True,sha256=digest(image),width=size[0],height=size[1]).items():
            equal(png.get(key),value,'PNG publication unproven: '+key)
        need(png['event']>gpu['event'],'PNG publication precedes GPU completion')
        counters=e['counters']
        for name in ('loop','draw','update','vi','present','dump'):integer(counters[name],'unknown counter '+name)
        result.update(status='PROPOSAL_CHAIN_CONSISTENT',presentation_candidate=dict(status='proposal-only',
            phase='first observed draw after independent ready-offset request',draw_id=draw['id'],
            request_source_frame=request['capture_source_frame'],copy_id=copy['id'],present_id=selection['present_id'],
            dump_id=dump['id'],image_sha256=digest(image),counters=counters),
            reasons=['Source/fixture consistency only. Owner hooks, phase-v2 approval and GPU/color/publication runtime controls remain absent.'])
    except (KeyError,ValueError,OSError,TypeError) as exc:
        result['reasons']=[str(exc)]
    return result


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-root',required=True);parser.add_argument('--dol',required=True)
    parser.add_argument('--symbols',required=True);parser.add_argument('--evidence');parser.add_argument('--out',required=True)
    args=parser.parse_args()
    source=audit_sources(args.source_root,args.dol,args.symbols)
    result=review(json.loads(Path(args.evidence).read_text())) if args.evidence else dict(status='SOURCE_BOUND_ONLY',qualification='BLOCKED',qualified_frames=None,runtime_provider_accepted=False)
    result.update(source_binding=source,tool_sha256=digest(Path(__file__)))
    if args.evidence:result['evidence_sha256']=digest(Path(args.evidence))
    out=Path(args.out).resolve();out.relative_to(Path.home()/'MacioGalaxy2-gamedata')
    with out.open('x') as stream:json.dump(result,stream,indent=2);stream.write('\n')
    print(json.dumps({k:result[k] for k in ('status','qualification','runtime_provider_accepted')}))
