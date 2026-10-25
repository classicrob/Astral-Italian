#!/usr/bin/env python3
"""Pure schema-2 attribution validation. No app launch or native fixture.

Pass the full, drop-free event list to audit. Window selection belongs AFTER
validation; a sliced trace cannot certify missing handoffs. Times overlap and
are never summed into a synthetic CPU/GPU total.
"""

def audit(events):
    errors, assignments, waits, hosts = [], [], [], []
    holder, since = 0, 0
    stacks, monitor = {}, {}
    for event in sorted(events, key=lambda e: (e['ns'], e['thread'], e['sequence'])):
        name, ns = event['event'], event['ns']
        if event.get('span') and (name.startswith('monitor_') or name in
                ('forced_preemption_waiter', 'forced_preemption_attempt_result', 'host_wait_begin', 'host_wait_return')):
            monitor.setdefault(event['span'], []).append(event)
        if 'host' in event:
            host = event['host']
            if host['sampled_ns'] < ns:
                errors.append('host sample precedes boundary')
            if host['target'] != host['observer'] and host['cpu'] != -1:
                errors.append('remote CPU number fabricated from emitter')
            hosts.append(event)
        if name in ('baton_grant', 'baton_release', 'baton_refusal'):
            if event['holder'] != holder:
                errors.append('baton holder chain mismatch')
            expected = ns - since if holder else 0
            if event['held_ns'] != expected:
                errors.append('baton assignment duration mismatch')
            if not event.get('reason'):
                errors.append('baton transition/refusal missing reason')
            if name != 'baton_refusal':
                target = event['subject'] if name == 'baton_grant' else 0
                if event['waiter'] != target:
                    errors.append('grant recipient/waiter mismatch')
                assignments.append(dict(holder=holder, waiter=target, start_ns=since if holder else None,
                                        end_ns=ns, held_ns=event['held_ns'], reason=event['reason'],
                                        host=event.get('host'), wall_time_not_cpu_time=True))
                holder, since = target, ns if target else 0
        elif name == 'host_wait_begin':
            stacks.setdefault(event['thread'], []).append(event)
        elif name == 'host_wait_return':
            stack = stacks.setdefault(event['thread'], [])
            if not stack:
                errors.append('wait return without begin')
                continue
            begin = stack.pop()
            if event['primitive'] != begin['primitive'] or event['deadline_ns'] != begin['deadline_ns']:
                errors.append('wait primitive/deadline mismatch')
            if event['wait_ns'] != ns - begin['ns']:
                errors.append('wait duration mismatch')
            deadline = begin['deadline_ns']
            late = max(0, ns - deadline) if deadline else 0
            if event['late_ns'] != late:
                errors.append('wait deadline lateness mismatch')
            waits.append(dict(thread=event['thread'], primitive=event['primitive'], start_ns=begin['ns'],
                              end_ns=ns, deadline_ns=deadline or None, late_ns=late,
                              late_includes_mutex_reacquire_and_predicate=True))
    # Background threads can legitimately remain parked when trace shutdown
    # stops accepting records. Expose right-censoring rather than invent a wake.
    attempts = []
    for span, values in monitor.items():
        if not any(x['event'] == 'monitor_wait_plan' for x in values): continue
        returned = [x for x in values if x['event'] == 'host_wait_return']
        woke = [x for x in values if x['event'] == 'monitor_woke']
        if woke and (len(returned) != 1 or len(woke) != 1 or woke[0]['ns'] != returned[0]['ns']):
            errors.append('monitor wake does not match wait return')
        attempts.append(dict(span=span, events=values, wait_return=returned[0] if len(returned) == 1 else None,
                             timed_wait_lateness_is_not_end_to_end_handoff=True))
    return dict(errors=errors, assignments=assignments, waits=waits, host_samples=hosts, monitor_attempts=attempts,
                waits_open_at_shutdown=[e for stack in stacks.values() for e in stack],
                holder_open_at_shutdown=holder,
                backend_observation='queue/GPU vs dispatch inseparable at the pinned backend',
                acceptance=False)
