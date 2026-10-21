#!/usr/bin/env python3
"""Inspect detached draw-observer journals; cannot emit a phase-v2 provider."""
import argparse
import hashlib
import json
from pathlib import Path
from visual_pairs import private_path

CAPACITY=4096

def need(value,message):
    if not value:raise ValueError(message)

def integer(value,message):
    need(type(value) is int and value>=0,message);return value

def key(value):
    need(isinstance(value,dict) and value.get('key_version')==1,'unknown scene key')
    for name in ('scene_hex','stage_hex'):
        raw=value[name];need(isinstance(raw,str) and len(raw)%2==0 and len(raw)<=126,'unbounded scene key')
        need(all(c in '0123456789abcdef' for c in raw),'invalid scene bytes')
        need(b'\0' not in bytes.fromhex(raw),'embedded scene terminator')
    for name in ('placed_scenario','selected_scenario'):need(type(value[name]) is int,'scenario is not signed integer')
    return value

def inspect(data):
    need(data.get('schema_version')==1 and data.get('status')=='detached-observations-only','not a detached journal')
    need(data.get('qualification')=='BLOCKED' and data.get('qualified_frames') is None and data.get('runtime_provider_accepted') is False,'journal may not claim qualification')
    need(data.get('fault')==0,'observer fault; retain as incomplete evidence')
    events=data['events'];need(isinstance(events,list) and 0<len(events)<=CAPACITY,'empty/overflow journal')
    need(data['capacity']==CAPACITY and data['event_count']==len(events),'journal count/capacity mismatch')
    need(len(events)%4==0,'journal ends in an unfinished loop; not a complete bounded interval')
    initialized_not_selected=selected_unready=candidates=0;previous_source=None
    for index,event in enumerate(events):
        phase=index%4;loop=index//4+1
        need(event['event_serial']==index+1 and event['phase']==phase,'missing/reordered event')
        need(event['loop_serial']==loop and event['draw_serial']==loop-(phase==0),'loop/draw serial mismatch')
        source=event['source'];key(source['key']);integer(source['frame'],'unknown source frame');integer(source['sampled_epoch'],'unknown sampled epoch')
        need(source['valid']==1 and source['authority']=='caller-sampled-identity-not-lifecycle','unknown source authority')
        if phase==0:
            if previous_source is not None:need(source==previous_source,'missing preceding source boundary')
            loop_source=source
        elif phase<3:need(source==loop_source,'source changes inside draw bracket')
        else:need(source['frame']==loop_source['frame']+1,'missing/rebased source seam')
        previous_source=source
        if phase==3:
            need(event['source_key_matches']==0,'post-seam zero snapshot is not a draw');continue
        s=event['snapshot'];key(s['key'])
        for name in ('controller_present','current_scene_nonnull','executed_scene_nonnull','executed_scene_is_current','initialized_game','canonical_player_valid','framework_present','xfb_present','render_flag','invalid'):
            need(type(s[name]) is int and s[name] in (0,1),'noncanonical snapshot boolean: '+name)
        need(s['invalid']==0,'invalid typed snapshot')
        matches=int(s['key']==source['key']);need(event['source_key_matches']==matches,'source-key match claim differs')
        if phase==1:
            initialized_not_selected+=int(s['initialized_game']==1 and s['executed_scene_is_current']==0)
            selected_unready+=int(s['executed_scene_is_current']==1 and s['initialized_game']==0)
            candidate=(s['key']['scene_hex']==b'Game'.hex() and s['key']['stage_hex']==b'RedBlueExGalaxy'.hex() and s['key']['placed_scenario']==1 and s['init_state']==6 and all(s[k]==1 for k in ('controller_present','current_scene_nonnull','executed_scene_nonnull','executed_scene_is_current','initialized_game','canonical_player_valid','framework_present','xfb_present')) and s['buffer_mode']==3 and matches==1)
            candidates+=int(candidate)
    return dict(status='DETACHED_JOURNAL_CONSISTENT',qualification='BLOCKED',qualified_frames=None,runtime_provider_accepted=False,
                loops=len(events)//4,initialized_not_selected=initialized_not_selected,selected_unready=selected_unready,
                pilot_metadata_candidates=candidates,copy_gpu_image_binding='ABSENT',source_epoch_authority='caller-sampled-identity-not-lifecycle',
                phase_v2_provider_emitted=False)

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--journal',required=True);parser.add_argument('--sha256',required=True)
    args=parser.parse_args();path=private_path(args.journal);raw=path.read_bytes()
    need(hashlib.sha256(raw).hexdigest()==args.sha256,'journal differs from independent expected hash')
    report=inspect(json.loads(raw));report['journal_sha256']=args.sha256;print(json.dumps(report,indent=2))
