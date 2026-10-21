#!/usr/bin/env python3
"""Observe ready-relative Dolphin window images without inventing XFB frame IDs."""
import argparse
import json
from pathlib import Path
import sys

import dolphin_collect as collector
import dolphin_stage_entry
from visual_readiness import ReadySchedule
from visual_window import capture_window


def main():
    parser=argparse.ArgumentParser(description=__doc__,add_help=False)
    parser.add_argument('--visual-window-id',type=int)
    args,remaining=parser.parse_known_args()
    controls=argparse.ArgumentParser(add_help=False)
    controls.add_argument('--sync-stage',required=True)
    controls.add_argument('--sync-scenario',type=int,required=True)
    controls.add_argument('--output',required=True)
    controls.add_argument('--dolphin',required=True)
    known,_=controls.parse_known_args(remaining)
    output=collector.game_private_path(known.output)
    schedule=ReadySchedule(known.sync_stage,known.sync_scenario)
    captures=[]
    base=collector.Snapshot
    class Snapshot(base):
        def take(self,frame):
            row=super().take(frame)
            if row.get('observer_error'): return row
            request=schedule.observe(row,frame)
            if request:
                if request['status']=='BLOCKED':
                    captures.append(dict(status='BLOCKED',request=request))
                else:
                    if self.m.remote.reg(64)!=collector.SEAM or self.m.remote.reg(67)!=collector.CALLER:
                        raise collector.ObserverError('visual capture did not originate at the accepted source seam')
                    manifest=json.loads((output/'manifest.json').read_text())
                    capture=capture_window(manifest['dolphin_pid'],known.dolphin,manifest['dolphin_sha256'],
                                           output/f'visual-{request["offset"]}.png',request,args.visual_window_id)
                    capture.update(guest_stop_pc=f'{collector.SEAM:08x}',guest_stop_lr=f'{collector.CALLER:08x}',
                                   sampled_source_frame=frame,ready_origin=schedule.origin,
                                   method='owned window image while guest stopped; GPU/XFB age UNKNOWN')
                    captures.append(capture)
                (output/'visual-captures.json').write_text(json.dumps(dict(status='UNQUALIFIED',origin=schedule.origin,
                     cancellation=schedule.cancelled,captures=captures),indent=2)+'\n')
            return row
    collector.Snapshot=Snapshot
    sys.argv=[sys.argv[0],*remaining]
    try:
        return dolphin_stage_entry.main()
    finally:
        collector.Snapshot=base
        if output.is_dir():
            (output/'visual-captures.json').write_text(json.dumps(dict(schema_version=1,status='UNQUALIFIED',
                 origin=schedule.origin,cancellation=schedule.cancelled,captures=captures,
                 qualification='BLOCKED',rendered_frame_binding='UNKNOWN; raw window diagnostic only'),indent=2)+'\n')


if __name__=='__main__': sys.exit(main())
