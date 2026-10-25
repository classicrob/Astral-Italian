#!/usr/bin/env python3
"""Join audio counter windows to trace time, without guessing a game frame.

Caller first validates full trace binding/footer/drops and frame mapping. This
module consumes audio boundaries only, never launches or accepts an app.
"""

def frame_candidates(begin, end, intervals):
    return [frame for a,b,frame in intervals if begin <= b and end >= a]

def summarize(events, trace_start_ns, intervals=()):
    events = list(events)
    errors, rings = [], {}
    callbacks = callback_summary(events, errors)
    for e in events:
        if e['event'] not in ('audio_device_ring_shortfall','audio_underrun_report_exchange'):continue
        a=e['audio'];kind=a['kind']
        if a['begin_ns']>a['end_ns'] or a['end_ns']!=e['ns']:errors.append('audio clock bracket mismatch')
        if a['missing_frames']!=e['value']:errors.append('audio count mismatch')
        group=rings.setdefault(e['subject'],dict(reads=[],reports=[]))
        if kind=='ring-read':
            if e['event']!='audio_device_ring_shortfall' or not (0<a['missing_frames']==a['requested_frames']-a['copied_frames']):
                errors.append('invalid ring-read shortfall')
            group['reads'].append(e)
        elif kind=='report-counter-exchange':
            if e['event']!='audio_underrun_report_exchange':errors.append('report event kind mismatch')
            group['reports'].append(e)
        else:errors.append('unknown audio observation')
    result=[]
    for ring,g in rings.items():
        reads=g['reads'];reports=sorted(g['reports'],key=lambda e:e['audio']['begin_ns'])
        previous_begin=previous_end=trace_start_ns;windows=[]
        for report in reports:
            a=report['audio'];begin,end=a['begin_ns'],a['end_ns']
            if begin<previous_end:errors.append('overlapping serialized reporter exchanges')
            definite=[e for e in reads if e['audio']['begin_ns']>previous_end and e['audio']['end_ns']<begin]
            possible=[e for e in reads if e['audio']['end_ns']>=previous_begin and e['audio']['begin_ns']<=end]
            lower=sum(e['audio']['missing_frames'] for e in definite)
            upper=sum(e['audio']['missing_frames'] for e in possible)
            count=a['missing_frames']
            if not lower<=count<=upper:errors.append('report count outside read-bracket bounds')
            windows.append(dict(report_source=a['report_source'],reported_missing_frames=count,
                possible_interval_begin_ns=previous_begin,possible_interval_end_ns=end,
                definite_read_missing_frames=lower,possible_read_missing_frames=upper,
                possible_report_frames=frame_candidates(previous_begin,end,intervals),
                read_events=[dict(begin_ns=e['audio']['begin_ns'],end_ns=e['audio']['end_ns'],
                    missing_frames=e['audio']['missing_frames'],emitter_thread=e['thread'],
                    possible_frames=frame_candidates(e['audio']['begin_ns'],e['audio']['end_ns'],intervals)) for e in possible],
                exact_single_underrun_frame_claimed=False))
            previous_begin,previous_end=begin,end
        read_total=sum(e['audio']['missing_frames'] for e in reads)
        reported=sum(e['audio']['missing_frames'] for e in reports)
        if reported>read_total:errors.append('reported total exceeds traced read shortfalls')
        result.append(dict(ring=ring,read_shortfall_frames=read_total,reported_frames=reported,
            pending_or_boundary_ambiguous_frames=read_total-reported,reports=windows))
    return dict(errors=errors,rings=result,callbacks=callbacks,acceptance=False,hardware_output_time_known=False,
        limit='report aggregates use exchange brackets; ring shortfall is before SDL enqueue, not hardware playback time')


def callback_summary(events, errors):
    """Callback gaps are observed entry/exit bounds, not hardware deadlines.

    A host sample belongs to its emitter at sampled_ns, after the boundary.
    It is not continuous scheduling coverage or effective RT-policy proof.
    """
    entries, exits = {}, {}
    for e in events:
        if e['event'] not in ('audio_device_callback_entry','audio_device_callback_exit'):continue
        a=e['audio'];span=a['callback_span']
        entering=e['event']=='audio_device_callback_entry'
        expected='device-callback-entry' if entering else 'device-callback-exit'
        if a['kind']!=expected or not span:errors.append('invalid callback kind/span')
        if a['begin_ns']>a['end_ns'] or a['end_ns']!=e['ns'] or a['ring_level_sample_ns']<e['ns'] or (entering and a['begin_ns']!=e['ns']):
            errors.append('callback clock bracket mismatch')
        if a['protocol_error'] or a['active_callbacks_before']!=(0 if entering else 1):
            errors.append('overlapping or unbalanced device callbacks')
        target=entries if entering else exits
        key=(e['subject'],span)
        if key in target:errors.append('duplicate callback span boundary')
        target[key]=e
    result=[]
    previous_by_ring={}
    for key,e in sorted(entries.items(),key=lambda item:item[1]['ns']):
        ring,span=key;a=e['audio'];end=exits.get(key);prior=previous_by_ring.get(ring)
        gap=idle=None
        if prior:
            previous,previous_end=prior
            if a['previous_entry_ns']!=previous['ns']:errors.append('callback entry-chain mismatch')
            if e['ns']<previous['ns']:errors.append('reversed callback entries')
            else:gap=e['ns']-previous['ns']
            if previous_end and previous_end['ns']<=e['ns'] and a['previous_exit_ns']==previous_end['ns']:
                idle=e['ns']-previous_end['ns']
            else:errors.append('callback exit-chain mismatch')
        duration=None
        if end:
            if end['thread']!=e['thread'] or end['audio']['begin_ns']!=e['ns'] or end['ns']<e['ns']:
                errors.append('callback exit identity/clock mismatch')
            else:duration=end['ns']-e['ns']
        # Tail may be right-censored if tracing stops with callback in flight.
        result.append(dict(ring=ring,span=span,thread=e['thread'],entry_ns=e['ns'],
            exit_ns=None if end is None else end['ns'],duration_ns=duration,
            entry_gap_ns=gap,since_previous_exit_ns=idle,
            entry_level_frames=a['ring_level_frames'],
            exit_level_frames=None if end is None else end['audio']['ring_level_frames'],
            entry_host=e.get('host'),exit_host=None if end is None else end.get('host'),
            realtime_api_status_entry=a['realtime_api_status'],
            realtime_api_status_exit=None if end is None else end['audio']['realtime_api_status'],
            effective_realtime_policy_known=False,right_censored=end is None))
        previous_by_ring[ring]=(e,end)
    for key in exits:
        if key not in entries:errors.append('callback exit without entry')
    for e in events:
        if e['event']!='audio_device_ring_shortfall':continue
        span=e['audio'].get('callback_span',0)
        if not span:continue # The standalone ring fixture need not be a device callback.
        key=(e['subject'],span);start=entries.get(key);end=exits.get(key)
        if start is None or e['audio']['begin_ns']<start['ns'] or (end and e['ns']>end['ns']):
            errors.append('shortfall outside linked device callback')
    return result
