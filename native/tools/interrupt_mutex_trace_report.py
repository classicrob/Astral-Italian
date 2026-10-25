#!/usr/bin/env python3
"""Pure bounded-window mutex analysis. Input must retain full trace integrity.

An uncovered interval is UNKNOWN, never FREE. CV wake and mutex reacquisition
remain inseparable. This module executes no compiler, fixture or app.
"""

def summarize(events, start_ns, end_ns):
    assert start_ns < end_ns
    errors, held, notifications, mach = [], [], [], []
    local, sequence = {}, {}
    peak_requests = peak_cv_unknown = 0
    for e in events:
        t, ns, name = e['thread'], e['ns'], e['event']
        if e['sequence'] != sequence.get(t, 0) + 1:
            errors.append('producer sequence gap')
        sequence[t] = e['sequence']
        in_window = start_ns <= ns < end_ns
        if name == 'interrupt_mutex_protocol_error': errors.append('producer mutex protocol error')
        if name.startswith('interrupt_mutex_'):
            m = e.get('mutex', {})
            address = m.get('address')
            if address is None: errors.append('missing mutex observation'); continue
            key = t, address
            state = local.setdefault(key, {})
            if in_window:
                peak_requests = max(peak_requests, m['explicit_pending_requests'])
                peak_cv_unknown = max(peak_cv_unknown, m['cv_waits_reacquire_unknown'])
            if name in ('interrupt_mutex_acquired', 'interrupt_mutex_cv_reacquired_bound'):
                if 'since' in state: errors.append('acquisition while already observed held')
                state.update(since=ns, span=e['span'])
            if name in ('interrupt_mutex_release_begin', 'interrupt_mutex_release_pending', 'interrupt_mutex_cv_release_pending'):
                since = state.pop('since', None)
                if since is None or state.get('span') != e['span']:
                    errors.append('release without matching observed acquisition'); continue
                if e['held_ns'] != ns - since: errors.append('held duration mismatch')
                a, b = max(start_ns, since), min(end_ns, ns)
                if a < b: held.append(dict(mutex=address, thread=t, start_ns=a, end_ns=b,
                                         release_kind=name, duration_ns=b-a))
        elif name.startswith('interrupt_cv_notify_') and in_window:
            notifications.append(e)
        elif name == 'assigned_unresumed_mach_sample' and in_window:
            sample = e['mach']
            if sample['remote_cpu'] != -1: errors.append('remote CPU identity fabricated')
            if sample['sample_end_ns'] < sample['sample_begin_ns']: errors.append('Mach sample clock reversed')
            if sample['stable_assigned_unresumed'] and sample['generation_before'] != sample['generation_after']:
                errors.append('unstable sample labelled stable')
            if sample['stable_assigned_unresumed'] and (sample['publication_before'] & 1 or
                    sample['publication_before'] != sample['publication_after']):
                errors.append('in-progress publication labelled stable')
            mach.append(e)
    # Definite held *lower bounds* for different owners cannot overlap.
    by_mutex = {}
    for interval in held: by_mutex.setdefault(interval['mutex'], []).append(interval)
    coverage = []
    for address, intervals in by_mutex.items():
        intervals.sort(key=lambda i: i['start_ns'])
        for a, b in zip(intervals, intervals[1:]):
            if b['start_ns'] < a['end_ns']: errors.append('overlapping definite mutex ownership')
        total = sum(i['duration_ns'] for i in intervals)
        coverage.append(dict(mutex=address, observed_held_lower_bound_ns=total,
            not_covered_by_held_lower_bounds_ns=max(0, end_ns-start_ns-total),
            uncovered_classification='UNKNOWN, not FREE', intervals=intervals))
    cpu = []
    grouped = {}
    for e in mach:
        m=e['mach']
        if m['error'] == 0 and m['stable_assigned_unresumed']:
            grouped.setdefault((e['subject'],m['target_port'],m['generation_before']),[]).append(e)
    for key, values in grouped.items():
        values.sort(key=lambda e:e['mach']['sample_end_ns'])
        for a,b in zip(values,values[1:]):
            x,y=a['mach'],b['mach']
            delta=y['user_cpu_ns']+y['system_cpu_ns']-x['user_cpu_ns']-x['system_cpu_ns']
            if delta<0: errors.append('cumulative target CPU time reversed')
            cpu.append(dict(subject=key[0], target_port=key[1], generation=key[2],
                start_state=x['state'],end_state=y['state'],cpu_time_delta_ns=delta,
                wall_time_delta_ns=y['sample_end_ns']-x['sample_end_ns'],
                limit='TH_STATE_RUNNING includes runnable AND executing; remote CPU identity unknown'))
    return dict(errors=errors, coverage=coverage, notifications=notifications, mach_samples=mach,
        cpu_time_deltas=cpu, peak_explicit_pending_requests=peak_requests,
        peak_cv_waits_reacquire_unknown=peak_cv_unknown, pure_cv_wake_observed=False,
        acceptance=False, overhead_qualified=False)
