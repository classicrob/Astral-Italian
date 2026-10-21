#!/usr/bin/env python3
"""Review a proposed native image event chain; never certify runtime capture."""
import argparse
import csv
import json
from pathlib import Path
from PIL import Image
from visual_pairs import bound_file, digest, geometry, private_path, validate_idle
from visual_readiness import ReadySchedule, presentation_receipt


class MissingEvidence(ValueError):
    pass


def need(condition, reason):
    if not condition:
        raise MissingEvidence(reason)


def same(actual, expected, reason):
    need(type(actual) is type(expected) and actual == expected, reason)


def nonnegative(value, name):
    need(type(value) is int and value >= 0, 'missing/invalid '+name)
    return value


def request_from_trace(evidence):
    offset=evidence['offset']
    need(type(offset) is int and offset in (60,300,600),'unsupported ready offset')
    schedule=ReadySchedule(evidence['stage'],evidence['scenario'])
    with bound_file(evidence['source_trace']).open() as f:
        for source,row in enumerate(csv.DictReader(f)):
            request=schedule.observe(row,source)
            if request and request['offset']==offset:
                need(request['status']=='PENDING_PRESENTATION','actual first ready/player is unavailable; no reanchor')
                return request,schedule.origin
    raise MissingEvidence('no continuous exact ready+offset observation; no inferred provider frame')


def review(evidence):
    result=dict(schema_version=1,status='BLOCKED',measurement_status='NOTMEASURED',
                qualification='BLOCKED',measured_qualified_frames=None,runtime_provider_accepted=False,
                synthetic=evidence.get('synthetic') is True,reasons=[],presentation_candidate=None)
    try:
        request,origin=request_from_trace(evidence)
        target=request['capture_source_frame'];epoch=request['source_scene_epoch']
        result.update(request=request,ready_origin=origin)
        path=bound_file(evidence['image']);input_path=bound_file(evidence['input'])
        validate_idle(input_path)
        producer=evidence['producer']
        same(producer['input_sha256'],digest(input_path),'producer input hash mismatch')
        delivery=evidence['input_delivery']
        for key,value in dict(status='captured',method='native-consumed-read-log',input_sha256=digest(input_path),frames=600,read_count=2400,
                              source_frame_start=origin['source_frame']+1,source_frame_end=origin['source_frame']+600).items():
            same(delivery.get(key),value,'missing actual consumed-input binding: '+key)
        draw=evidence['draw'];copy=evidence['copy'];resolve=evidence['resolve'];selection=evidence['selection']
        encoded=evidence['readback_encode'];submit=evidence['submit'];mapped=evidence['mapped'];png=evidence['png']
        same(draw['phase'],'observed-draw-state','draw association is only a queued/copy counter')
        for key,value in dict(source_frame=target,scene_epoch=epoch,stage=evidence['stage'],scenario=evidence['scenario']).items():
            same(draw.get(key),value,'missing/different draw-state binding: '+key)
        same(copy['draw_id'],draw['id'],'copy is not bound to this draw')
        same(copy['kind'],'display','texture/unknown copy cannot substitute for display copy')
        copy_id=nonnegative(copy['id'],'copy serial')
        same(copy['fifo_tag'],copy_id,'copy identity was not carried with the FIFO operation')
        # Keep issue/selection source frames separate; no compensating fixed lag.
        same(copy['issued_source_frame'],target,'copy issued at another source frame; phase mapping needs independent review')
        same(selection['source_frame'],target,'selected at another source frame; no queued-counter relabel')
        same(resolve['copy_id'],copy_id,'resolved another copy serial')
        same(selection['copy_id'],copy_id,'selected another copy serial')
        same(encoded['copy_id'],copy_id,'readback encoded another copy serial')
        for key in ('resource_id','resource_generation','resource_revision'):
            need(resolve.get(key) is not None,'unknown GPU resource version: '+key)
            same(selection.get(key),resolve[key],'selected resource version differs: '+key)
            same(encoded.get(key),resolve[key],'readback resource version differs: '+key)
        nonnegative(resolve['resource_generation'],'resource generation')
        nonnegative(resolve['resource_revision'],'resource revision')
        same(selection['vi_configured'],True,'VI mode unavailable')
        same(selection['vi_black'],False,'raw XFB does not represent VI black')
        same(selection['vi_dimmed'],False,'raw XFB does not include VI dimming')
        need(selection['policy'] in ('latest','latched'),'unknown selection policy')
        same(selection['free_aspect'],False,'editor/free aspect is outside this bounded provider proposal')
        same(selection['includes_chrome'],False,'window chrome is not a game viewport')
        same(selection['includes_overlay'],False,'overlay surface is not the raw XFB viewport')
        same(selection['rml_replaces_game'],False,'RmlUi replaces the game in the selected frame')
        packet=nonnegative(resolve['renderer_packet_id'],'renderer packet id')
        same(encoded['renderer_packet_id'],packet,'readback packet is not the resolved packet')
        same(encoded['command_buffer_id'],resolve['command_buffer_id'],'separate command buffer without reviewed GPU ordering proof')
        before=nonnegative(resolve['operation_index'],'resolve operation')
        after=nonnegative(encoded['operation_index'],'readback encode operation')
        need(after>before,'readback encoded before the tagged resolve')
        same(evidence['resolve_coverage_complete'],True,'resource overwrite coverage incomplete')
        writes=evidence['resource_writes']
        need(any(w==resolve for w in writes),'selected resolve is absent from complete write ledger')
        for write in writes:
            if write.get('resource_id')==resolve['resource_id']:
                if write.get('resource_generation')!=resolve['resource_generation']:
                    raise MissingEvidence('resource resized/reset in bounded capture interval')
                if write.get('command_buffer_id')==resolve['command_buffer_id'] and before<write['operation_index']<after:
                    raise MissingEvidence('same GPU texture overwritten between resolve and readback')
        same(submit['command_buffer_id'],encoded['command_buffer_id'],'tagged command buffer was not submitted')
        same(submit['success'],True,'GPU queue submission failed/unknown')
        same(submit['validation_scope_result'],'no-error','GPU validation result is unknown/failed')
        same(submit['device_lost'],False,'device loss is unknown/present')
        same(mapped['submit_id'],submit['id'],'mapped result is not from this submission')
        same(mapped['success'],True,'map callback did not complete successfully')
        same(mapped['callback_completed'],True,'map callback completion unavailable')
        same(mapped['copy_id'],copy_id,'map completion identity differs')
        same(mapped['renderer_packet_id'],packet,'map completion packet differs')
        need(selection['color_encoding'] in ('srgb-code-values-preserved','explicit-linear-to-srgb'),
             'texture format alone does not prove PNG color encoding')
        same(selection['image_geometry'],producer['image_geometry'],'viewport/color geometry provenance differs')
        with Image.open(path) as image:
            same(image.format,'PNG','artifact is not a PNG')
            size=image.size;image.verify();geometry(size,producer['image_geometry'])
        for record,name in [(encoded,'encoded'),(mapped,'mapped'),(png,'PNG')]:
            same(record['width'],size[0],name+' width differs')
            same(record['height'],size[1],name+' height differs')
        for field in ('write_success','close_success','atomic_rename_success'):
            same(png[field],True,'PNG completion not verified: '+field)
        same(png['sha256'],digest(path),'closed PNG bytes/hash differ')
        candidate=presentation_receipt(request,
            dict(frame_id=packet,source_frame=draw['source_frame'],scene_epoch=epoch),
            dict(frame_id=packet,fence_complete=True,success=True,image_sha256=digest(path)),
            path,'native-renderer-readback',producer)
        candidate.update(status='proposal-only',copy_id=copy_id,resource_generation=resolve['resource_generation'],
                         resource_revision=resolve['resource_revision'],command_buffer_id=encoded['command_buffer_id'],
                         queue_submit_id=submit['id'],copy_issued_source_frame=copy['issued_source_frame'],
                         selection_source_frame=selection['source_frame'],selection_policy=selection['policy'],
                         physical_display_time=None)
        result.update(status='SOURCE_CHAIN_CONSISTENT',presentation_candidate=candidate,
                      reasons=['Consistency only: producer hooks, draw/frame phase and color/GPU controls have no runtime acceptance. Candidate is deliberately not status=captured.'])
    except (KeyError,ValueError,OSError,TypeError) as exc:
        result['reasons']=[str(exc)]
    return result


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--evidence',required=True)
    parser.add_argument('--out',required=True)
    args=parser.parse_args()
    source=private_path(args.evidence);output=private_path(args.out)
    data=json.loads(source.read_text());result=review(data)
    result['evidence_sha256']=digest(source)
    result['tool_sha256']=digest(Path(__file__))
    with output.open('x') as f:json.dump(result,f,indent=2);f.write('\n')
    print(json.dumps({k:result[k] for k in ('status','measurement_status','qualification')}))
