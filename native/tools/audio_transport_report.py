#!/usr/bin/env python3
"""Read-only schema-2 flight-history intake. Never qualifies overhead or audio.

Nested spans overlap; do NOT add them. AQ interarrival and outside-callback
intervals are observations, not proof of kernel/AQ scheduling causality.
"""
import argparse
from collections import defaultdict
import json
from pathlib import Path
import re

def fields(s):
    return dict(re.findall(r'([a-zA-Z_0-9]+)=([^\s]+)',s))
def signed32(n):
    return n-(1<<32) if n&(1<<31) else n

def analyze(text):
    config=None; pending=None; events={}; errors=[]; dumps=0; count=0; previous_seq=0
    def require(ok,why):
        if not ok: errors.append(why)
    for line in text.splitlines():
        if line.startswith('[audio-workgroup-config]'):
            config=fields(line)
            require(config.get('schema')=='2' and config.get('boundary_clock')=='steady_clock_ns','missing schema-2 steady clock')
        elif line.startswith('[audio-workgroup-history] begin'):
            require(pending is None,'nested dump')
            pending=fields(line);count=0;previous_seq=0
            require(config is not None,'missing clock config')
            for key in ('drops','unstable','clipped','retention_unknown','rejected','rejected_omitted'):
                require(pending.get(key)=='0','history '+key)
            try:
                require(int(pending['trigger_mach'])<=int(pending['end_mach'])<=int(pending['trigger_mach'])+int(config['max_post_ticks']), 'post window exceeds bound')
            except (KeyError,ValueError,TypeError): errors.append('invalid post window')
        elif line.startswith('[audio-workgroup-history] event'):
            require(pending is not None,'event outside dump');count+=1
            try:
                f=fields(line)
                e={k:int(f[k],16 if k in ('thread','object') else 10) for k in ('seq','mach','kind','thread','value','steady_ns','ticket','object')}
                require(e['seq']>previous_seq,'dump sequence not increasing');previous_seq=e['seq']
                require(e['steady_ns']>0 and 1<=e['kind']<=42,'invalid event timestamp/kind')
                if pending and config:
                    require(max(0,int(pending['trigger_mach'])-int(config['pre_ticks']))<=e['mach']<=int(pending['end_mach']),'event outside window')
                require(e['seq'] not in events or events[e['seq']]==e,'overlap payload differs')
                events[e['seq']]=e
            except (KeyError,ValueError): errors.append('invalid event')
        elif line.startswith('[audio-workgroup-history] reject'):
            errors.append('rejected history cell')
        elif line.startswith('[audio-workgroup-history] end'):
            try:
                require(pending is not None and count==int(pending['rows'])==int(fields(line)['rows']) and count<=24576,'invalid dump count')
            except (KeyError,ValueError,TypeError): errors.append('invalid dump footer')
            pending=None;dumps+=1
    require(pending is None,'incomplete dump')
    require(config is not None,'missing schema config')
    groups=defaultdict(list)
    for e in sorted(events.values(),key=lambda e:e['seq']):
        if e['ticket']: groups[e['ticket']].append(e)
    callbacks=[]
    for ticket,rows in sorted(groups.items()):
        by_kind=defaultdict(list)
        for e in rows: by_kind[e['kind']].append(e)
        entry=by_kind[17];exit=by_kind[18]
        require(len(entry)<=1 and len(exit)<=1,'duplicate callback entry/exit')
        require(len({e['thread'] for e in rows})==1,'callback ticket changes thread')
        def spans(begin,end):
            starts=defaultdict(list);result=[]
            for e in rows:
                if e['kind']==begin: starts[e['object']].append(e)
                if e['kind']==end and starts[e['object']]:
                    a=starts[e['object']].pop(0)
                    require(e['steady_ns']>=a['steady_ns'],'negative boundary span')
                    result.append({'object':hex(e['object']),'begin_seq':a['seq'],'end_seq':e['seq'],
                                   'ms':(e['steady_ns']-a['steady_ns'])/1e6})
            return result
        metadata={}
        for kind,name in ((36,'qos_self_requested'),(37,'qos_error_relative'),(38,'sched_policy_priority'),
                          (39,'mach_pth_curpri_priority'),(40,'cpu_error_number'),(41,'sched_mach_errors'),(42,'mach_policy_state')):
            if by_kind[kind]:
                v=by_kind[kind][-1]['value'];metadata[name]=[signed32(v>>32),signed32(v&0xffffffff)]
            else: metadata[name]=None
        callback={'ticket':ticket,'thread':hex(rows[0]['thread']),'complete_entry_exit':bool(entry and exit),
                  'device':hex(entry[0]['object']) if entry else None,
                  'entry_ns':entry[0]['steady_ns'] if entry else None,'exit_ns':exit[0]['steady_ns'] if exit else None,
                  'ring_samples':[{'steady_ns':e['steady_ns'],'level':e['value']>>32,'requested':e['value']&0xffffffff}
                                  for e in by_kind[6]],
                  'shortfall_frames':sum(e['value'] for e in by_kind[7]),
                  'metadata':metadata,'spans':{name:spans(a,b) for name,a,b in (
                      ('callback',17,18),('device_lock_wait',19,20),('device_lock_hold_upper_bound',20,21),
                      ('get',22,29),('stream_get_lock_wait',23,24),('fill',25,26),('conversion',27,28),
                      ('put',30,34),('put_lock_wait',31,32),('put_lock_hold_upper_bound',32,33))}}
        callbacks.append(callback)
    previous_by_device={}
    for c in callbacks:
        c['aq_interarrival_ms']=None;c['outside_callback_ms']=None
        if c['entry_ns'] is not None:
            key=(c['thread'],c['device'])
            previous=previous_by_device.get(key)
            if previous and previous['entry_ns'] is not None:
                c['aq_interarrival_ms']=(c['entry_ns']-previous['entry_ns'])/1e6
                if previous['exit_ns'] is not None:
                    c['outside_callback_ms']=(c['entry_ns']-previous['exit_ns'])/1e6
            previous_by_device[key]=c
    return {'trace_valid':not errors,'acceptance_claimed':False,'errors':sorted(set(errors)), 'dumps':dumps,
            'unique_rows':len(events),'callbacks':callbacks,
            'limits':'Bounded trigger windows; null means unobserved, not zero. Nested spans overlap. '
                     'An AQ entry gap includes prior callback work and run-loop/AQ/device availability; no scheduling attribution by itself.'}
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('log',type=Path);args=p.parse_args()
    print(json.dumps(analyze(args.log.read_text(errors='replace')),indent=2))
