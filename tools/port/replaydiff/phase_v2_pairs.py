#!/usr/bin/env python3
"""Review NEW phase-v2 proposed content pairs; legacy admission is not invoked."""
import argparse
import csv
import html
import json
from pathlib import Path
import re
from PIL import Image
import dolphin_visual_contract as shared
from native_draw_phase import PINS as NATIVE_PINS
from visual_pairs import bound_file, digest, geometry, metrics, private_path, COMPARISON_POLICY

CONTRACT='post-seam-request/next-observed-draw-v2'
COMMON=dict(ready={'trace','ready_origin','cpu_stop'},input={'input','actual_input','input_export'},
            draw={'draw_coverage','selected_draw_id'},publication={'image','png'},
            color={'image_geometry','color_transfer','color_validation'})
ROLE_GROUPS={
    'native':dict(copy={'copy','resolve','resolve_coverage_complete','resource_writes'},
                  selection={'selection','selection_history'},readback={'readback_encode','submit','mapped'}),
    'dolphin':dict(copy={'copy'},selection={'selection','selection_history','dump'},readback={'gpu'})}


class Rejected(ValueError):
    def __init__(self,category,message):
        super().__init__(message);self.category=category


class Files:
    def __init__(self):self.records=[]
    def file(self,record):
        path=bound_file(record);self.records.append(dict(record));return path
    def load(self,record):return json.loads(self.file(record).read_text())
    def nested(self,value):
        if isinstance(value,dict):
            if set(value)=={'path','sha256'}:self.file(value)
            else:
                for item in value.values():self.nested(item)
        elif isinstance(value,list):
            for item in value:self.nested(item)
    def recheck(self):
        for record in self.records:bound_file(record)


def require(value,message):shared.need(value,message)

def same(a,b,message):shared.equal(a,b,message)


def load_provider(descriptor,role,pair,synthetic,files):
    producer=files.load(descriptor['producer'])
    same(producer['role'],role,'producer role mismatch')
    same(producer['synthetic'],synthetic,'synthetic provenance mismatch')
    same(producer['status'],'fixture-only' if synthetic else 'captured','producer receipt is not complete')
    require(isinstance(producer['host'],str) and bool(producer['host']),'provider host unavailable')
    require(isinstance(producer.get('run_id'),str) and bool(producer['run_id']),'run identity missing')
    source=files.load(producer['source_manifest']);binary=files.file(producer['executable'])
    instrumentation=files.load(producer['instrumentation']);build=files.load(producer['build_receipt'])
    base=NATIVE_PINS['revision'] if role=='native' else shared.PINS['dolphin_revision']
    same(source['semantic_base_revision'],base,'provider semantic source base mismatch')
    require(re.fullmatch('[0-9a-f]{40}',str(source.get('source_revision'))) is not None,'actual source revision unavailable')
    require(isinstance(source['files'],list) and bool(source['files']),'source files are not independently pinned')
    for record in source['files']:files.file(record)
    source_hashes={record['sha256'] for record in source['files']}
    same(instrumentation['phase_contract'],CONTRACT,'instrumentation uses another phase contract')
    for hook in ('draw_entry','draw_exit','copy_tag','selection','readback','publication'):
        site=instrumentation['hooks'][hook]
        require(isinstance(site['site'],str) and bool(site['site']),'instrumentation site missing: '+hook)
        files.file(site['source_file'])
        require(site['source_file']['sha256'] in source_hashes,'hook source absent from source manifest')
    for key,value in dict(source_manifest_sha256=producer['source_manifest']['sha256'],
                          executable_sha256=digest(binary),instrumentation_manifest_sha256=producer['instrumentation']['sha256'],
                          status='fixture-only' if synthetic else 'captured').items():
        same(build.get(key),value,'build/source/binary binding mismatch: '+key)
    require(isinstance(producer['clock'],dict) and 'status' in producer['clock'],'clock facts/explicit unknown missing')
    evidence=dict(synthetic=synthetic,stage=pair['stage'],scenario=pair['scenario'],offset=pair['offset'],tail_frames=16,
                  sampling_definition=CONTRACT,source_revision=source['source_revision'],semantic_base_revision=base,executable_sha256=digest(binary),
                  instrumentation_manifest_sha256=producer['instrumentation']['sha256'],counters=producer['counters'])
    expected={**COMMON,**ROLE_GROUPS[role]}
    same(set(descriptor['artifacts']),set(expected),'independently pinned provider artifact set incomplete')
    for group,keys in expected.items():
        artifact=files.load(descriptor['artifacts'][group])
        same(set(artifact),keys,'provider artifact fields differ: '+group)
        files.nested(artifact)
        evidence.update(artifact)
    for name in ('loop','draw','update','vi','present','dump'):shared.integer(producer['counters'][name],'unknown provider counter: '+name)
    same(evidence['color_transfer'],'recorded-and-validated-srgb-code-values','unknown color transfer')
    calibration=files.load(evidence.pop('color_validation'))
    for key,value in dict(role=role,status='fixture-only' if synthetic else 'captured',
                          method='rgb-code-ramp-and-transfer',source_manifest_sha256=producer['source_manifest']['sha256'],
                          executable_sha256=digest(binary),image_geometry=evidence['image_geometry'],
                          color_transfer=evidence['color_transfer']).items():
        same(calibration.get(key),value,'independent color evidence mismatch: '+key)
    return evidence,producer,source


def native_chain(e,producer):
    request,origin=shared.ready_request(e)
    try:delivery=shared.idle_delivery(e,origin,'native-consumed-read-log')
    except (ValueError,KeyError,OSError,TypeError) as exc:raise Rejected('INPUT_DELIVERY',str(exc)) from exc
    draw=shared.review_draw_scope(e,request,origin,retail_sites=False)
    c=e['copy'];r=e['resolve'];s=e['selection'];encoded=e['readback_encode'];submit=e['submit'];mapped=e['mapped']
    for key,value in dict(draw_id=draw['id'],kind='display',buffer_mode=3,current_render=True,lineage_complete=True,
                          efb_writers=['current-draw','endGX'],clear_before_draw_proven=True).items():
        same(c.get(key),value,'native draw/copy scope mismatch: '+key)
    require(draw['exit_event']<c['endGX_event']<c['event']<e['draw_coverage']['next_seam_event'],'native copy outside witnessed draw/source interval')
    same(c['observed_source_seam'],draw['after_source_seam'],'native copy source witness mismatch')
    same(c['fifo_tag'],c['id'],'native copy tag missing')
    for record in (r,s,encoded,mapped):same(record['copy_id'],c['id'],'native copy identity mismatch')
    for key in ('resource_id','resource_generation','resource_revision'):
        require(r.get(key) is not None,'native resource identity/version missing')
        for record in (s,encoded):same(record.get(key),r[key],'native selected/readback resource differs: '+key)
    for key in ('resource_generation','resource_revision','renderer_packet_id'):shared.integer(r[key],'unknown native '+key)
    for record in (encoded,mapped):same(record['renderer_packet_id'],r['renderer_packet_id'],'native packet mismatch')
    same(encoded['command_buffer_id'],r['command_buffer_id'],'native readback encoder order unbound')
    before=shared.integer(r['operation_index'],'unknown resolve operation');after=shared.integer(encoded['operation_index'],'unknown readback operation')
    require(after>before,'native readback encoded before resolve')
    same(e['resolve_coverage_complete'],True,'native resource write coverage missing')
    require(isinstance(e['resource_writes'],list) and all(isinstance(w,dict) for w in e['resource_writes']),'native write ledger malformed')
    require(r in e['resource_writes'],'selected native resolve absent from write ledger')
    for write in e['resource_writes']:
        if write.get('resource_id')==r['resource_id']:
            same(write.get('resource_generation'),r['resource_generation'],'native resource reset/resize')
            same(write.get('command_buffer_id'),r['command_buffer_id'],'foreign command-buffer write ordering unbound')
            require(not(write.get('command_buffer_id')==r['command_buffer_id'] and before<write['operation_index']<after),'native resource overwritten before readback')
    history=e['selection_history'];same(history['complete'],True,'native selection history missing')
    order=[shared.integer(item['event'],'unknown native selection event') for item in history['events']]
    require(order==sorted(set(order)) and all(n>c['event'] for n in order),'native selection history order invalid')
    for item in history['events']:
        require(request['capture_source_frame']<=shared.integer(item['observed_source_seam'],'unknown native selection history source')<=e['cpu_stop']['source_frame'],'native selection history exceeds CPU stop')
    matches=[item for item in history['events'] if item.get('copy_id')==c['id']]
    require(bool(matches),'native target was not selected');same(s,matches[0],'later native selection substituted')
    same(s['policy'],producer['selection_policy'],'native selection policy changed')
    require(s['policy'] in ('latest','latched'),'unknown native selection policy')
    for key,value in dict(vi_configured=True,vi_black=False,vi_dimmed=False,free_aspect=False,
                          includes_chrome=False,includes_overlay=False,rml_replaces_game=False).items():same(s.get(key),value,'native selection unavailable: '+key)
    require(request['capture_source_frame']<=shared.integer(s['observed_source_seam'],'unknown native selection source')<=e['cpu_stop']['source_frame'],'native selection beyond CPU stop')
    same(s['image_geometry'],e['image_geometry'],'native geometry differs')
    require(s['color_encoding'] in ('srgb-code-values-preserved','explicit-linear-to-srgb'),'native color unknown')
    for key,value in dict(command_buffer_id=encoded['command_buffer_id'],success=True,validation_scope_result='no-error',device_lost=False).items():same(submit.get(key),value,'native submission unproven: '+key)
    for key,value in dict(submit_id=submit['id'],success=True,callback_completed=True).items():same(mapped.get(key),value,'native map completion unproven: '+key)
    png=e['png'];image=bound_file(e['image'])
    with Image.open(image) as pixels:
        same(pixels.format,'PNG','native artifact is not PNG');pixels.verify()
    with Image.open(image) as pixels:
        pixels.load();size=pixels.size
        require(pixels.convert('RGBA').getchannel('A').getextrema()==(255,255),'transparent native viewport')
    for record in (encoded,mapped,png):
        same(record['width'],size[0],'native width mismatch');same(record['height'],size[1],'native height mismatch')
    for key in ('write_success','flush_success','close_success','verified_temp_bytes','published_inode_verified','temp_link_removed'):same(png.get(key),True,'native checked PNG publication missing: '+key)
    same(png['readback_id'],mapped['id'],'native publication belongs to another readback')
    same(png['copy_id'],c['id'],'native publication belongs to another copy')
    same(png['publication_method'],'same-directory-no-replace-link','native publication method unknown')
    same(png['sha256'],digest(image),'native PNG hash mismatch')
    geometry(size,e['image_geometry'])
    return dict(request=request,ready_origin=origin,input_binding=delivery,draw=draw)


def inspect(descriptor,role,pair,synthetic,files):
    try:e,producer,source=load_provider(descriptor,role,pair,synthetic,files)
    except (ValueError,KeyError,OSError,TypeError) as exc:raise Rejected('PROVIDER_PROVENANCE',str(exc)) from exc
    try:
        request,origin=shared.ready_request(e)
        for key in ('phase','stage','scenario','source_frame','source_scene_epoch','player_valid'):
            same(e['ready_origin'].get(key),origin[key],'ready artifact shifted/inconsistent: '+key)
        for key in ('rng_seed','ticks','vi_count'):
            same(e['ready_origin'].get(key),origin.get(key),'ready RNG/clock evidence differs: '+key)
        # Classify input failure before image metrics or gameplay interpretation.
        try:delivery=shared.idle_delivery(e,origin,'native-consumed-read-log' if role=='native' else 'dolphin-actual-export')
        except (ValueError,KeyError,OSError,TypeError) as exc:raise Rejected('INPUT_DELIVERY',str(exc)) from exc
        if role=='dolphin':
            checked=shared.review(e)
            require(checked['status']=='PROPOSAL_CHAIN_CONSISTENT','; '.join(checked['reasons']))
            draw=e['draw_coverage']['draws'][0]
        else:draw=native_chain(e,producer)['draw']
        with bound_file(e['trace']).open() as stream:
            requested=next(row for row in csv.DictReader(stream) if row['frame']==str(request['capture_source_frame']))
        return e,dict(producer=producer,source=source,ready_origin=origin,request=request,requested_state=requested,
                      drawn_state=dict(entry_metadata=draw['snapshot'],exit_metadata=draw['exit_snapshot'],
                                       full_draw_state_observed=False,scope='draw-bracket metadata only'),
                      sampled_epoch=dict(value=origin['source_scene_epoch'],kind='sampled-identity',authoritative_lifecycle_id=None),
                      clock=producer['clock'],counters=producer['counters'],input_binding=delivery,evidence_refs=descriptor['artifacts'])
    except Rejected:raise
    except (ValueError,KeyError,OSError,TypeError,StopIteration) as exc:raise Rejected('PHASE_OR_IMAGE_EVIDENCE',str(exc)) from exc


def compare_manifest(manifest,output,operator_pins,operator_pins_sha256):
    manifest=private_path(manifest);output=private_path(output)
    require(not output.exists(),'fresh output directory required')
    data=json.loads(manifest.read_text());manifest_hash=digest(manifest)
    require(re.fullmatch('[0-9a-f]{64}',str(operator_pins_sha256)) is not None,'independent operator pin digest required')
    operator_record=dict(path=str(operator_pins),sha256=operator_pins_sha256)
    trusted=json.loads(bound_file(operator_record).read_text())
    same(set(trusted),{'schema_version','scope','contract','manifest_sha256'},'operator integrity pin fields differ')
    same(trusted['schema_version'],1,'operator pin version mismatch')
    same(trusted['scope'],'operator-integrity-pin','operator pin is not a qualification waiver')
    same(trusted['contract'],CONTRACT,'operator pin contract mismatch')
    same(trusted['manifest_sha256'],manifest_hash,'manifest differs from independent operator pin')
    same(data.get('schema_version'),2,'phase-v2 requires its own manifest version')
    same(data.get('contract'),CONTRACT,'legacy/unknown contract cannot enter phase-v2')
    same(data.get('phase_contract_status'),'proposed','no runtime-approval waiver supported')
    require(type(data.get('synthetic')) is bool,'explicit synthetic identity required')
    require(isinstance(data.get('pairs'),list) and bool(data['pairs']),'nonempty content pairs required')
    rows=[];ledgers=[]
    for index,pair in enumerate(data['pairs']):
        row=dict(index=index,status='BLOCKED',measurement_status='NOTMEASURED',qualification='BLOCKED',qualified_frames=None,measured_qualified_frames=None)
        files=Files()
        try:
            require(isinstance(pair,dict),'pair must be an object')
            row.update(stage=pair.get('stage'),scenario=pair.get('scenario'),offset=pair.get('offset'))
            require(isinstance(pair['stage'],str) and re.fullmatch('[A-Za-z0-9_]+',pair['stage']),'invalid stage')
            require(type(pair['scenario']) is int and pair['scenario']>0,'invalid scenario')
            require(type(pair['offset']) is int and pair['offset'] in (60,300,600),'invalid offset')
            left,native=inspect(pair['native'],'native',pair,data['synthetic'],files)
            right,dolphin=inspect(pair['dolphin'],'dolphin',pair,data['synthetic'],files)
            if left['input']['sha256']!=right['input']['sha256']:raise Rejected('INPUT_DELIVERY','provider canonical idle tapes differ')
            measured=metrics(bound_file(left['image']),bound_file(right['image']),left['image_geometry'],right['image_geometry'])
            files.recheck()
            row.update(status='PROPOSED_CONTENT_PAIR',measurement_status='SYNTHETIC_METRIC_ONLY' if data['synthetic'] else 'MEASURED_UNQUALIFIED',
                       metrics=measured,providers=dict(native=native,dolphin=dolphin),
                       source_origins_equal_required=False,draw_counts_equal_required=False,
                       input_delivery='verified original identities/census and canonical suffix; no gameplay equality inferred')
        except (ValueError,KeyError,OSError,TypeError) as exc:
            row.update(error=str(exc),failure_category=getattr(exc,'category','MANIFEST_OR_PROVENANCE'))
        rows.append(row);ledgers.append((row,files))
    for row,files in ledgers:
        try:files.recheck()
        except (ValueError,KeyError,OSError,TypeError) as exc:
            row.pop('metrics',None);row.pop('providers',None)
            row.update(status='BLOCKED',measurement_status='NOTMEASURED',error=str(exc),failure_category='PROVENANCE_DRIFT')
    groups={(r['stage'],r['scenario']) for r in rows if isinstance(r.get('stage'),str) and type(r.get('scenario')) is int}
    for stage,scenario in sorted(groups):
        for offset in (60,300,600):
            if not any(r.get('stage')==stage and r.get('scenario')==scenario and r.get('offset')==offset for r in rows):
                rows.append(dict(index=None,stage=stage,scenario=scenario,offset=offset,status='BLOCKED',measurement_status='NOTMEASURED',qualification='BLOCKED',qualified_frames=None,measured_qualified_frames=None,failure_category='MISSING_PROVIDER',error='requested content slot absent; no draw inferred'))
    same(digest(manifest),manifest_hash,'manifest changed during review')
    bound_file(operator_record)
    ranked=sorted((r for r in rows if 'metrics' in r),key=lambda r:-r['metrics']['visual_distance'])
    for rank,row in enumerate(ranked,1):row['rank']=rank
    report=dict(adapter_sha256=digest(Path(__file__)),legacy_metric_tool_sha256=digest(Path(__file__).with_name('visual_pairs.py')),shared_validator_sha256=digest(Path(shared.__file__)),schema_version=2,contract=CONTRACT,phase_contract_status='proposed',synthetic=data['synthetic'],
                status='PROPOSED_CONTENT_REVIEW',qualification='BLOCKED',qualified_frames=None,measured_qualified_frames=None,runtime_provider_accepted=False,
                manifest_sha256=manifest_hash,operator_pin_sha256=operator_pins_sha256,operator_pin_scope='integrity only; not producer authentication or runtime approval',metric_policy=COMPARISON_POLICY,results=rows)
    output.mkdir(parents=True)
    (output/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    body=''.join('<li>'+html.escape(f"{r.get('stage')} ready+{r.get('offset')}: {r['measurement_status']} / {r.get('error',('proposed content distance '+format(r['metrics']['visual_distance'],'.6f')) if 'metrics' in r else 'not measured')}")+'</li>' for r in rows)
    (output/'report.html').write_text('<!doctype html><meta charset="utf-8"><title>Proposed phase-v2 content review</title><h1>Phase-v2 proposal — qualification BLOCKED</h1><p>Requested state and draw metadata are distinct; no equal scanout/time/lifecycle or gameplay claim.</p><ul>'+body+'</ul><a href="report.json">Evidence and metrics</a>')
    return report


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest',required=True);parser.add_argument('--out',required=True)
    parser.add_argument('--operator-pins',required=True);parser.add_argument('--operator-pins-sha256',required=True)
    args=parser.parse_args();report=compare_manifest(args.manifest,args.out,args.operator_pins,args.operator_pins_sha256)
    print(json.dumps(dict(status=report['status'],qualification=report['qualification'],qualified_frames=None,measured_qualified_frames=None)))
