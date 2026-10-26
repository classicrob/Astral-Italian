#!/usr/bin/env python3
"""Validate bounded wait history. Eligible boundaries do NOT prove a root cause."""
import argparse,json
from pathlib import Path
NAMES='unknown phase near-miss trigger invalid-phase csv interrupt-mutex interrupt-owner cpu-wait baton-grant cpu-resumed sleep ready host-work token fifo draw-done event-pump handle-events compose end-frame begin-frame display-update drawable present submit frame-slot staging render-queue render-sync snapshot capture worker run-state vm writer queue-done map-callback encoder-finish window-state thread-bind'.split()
LIMIT=64*1024*1024

def need(ok,why):
    if not ok:raise ValueError(why)
def integer(v):return type(v) is int and 0<=v<=2**64-1
def analyze(rows,bytes_read):
    need(0<bytes_read<=LIMIT,'output cap/empty file')
    need(rows and rows[0].get('type')=='header' and rows[-1].get('type')=='footer','missing header/footer')
    h,f=rows[0],rows[-1]
    need(h.get('schema')==1 and h.get('clock')=='steady-ns','unknown schema/clock')
    need(h.get('cells')==65536 and h.get('max_memory_bytes')==32*1024*1024 and h.get('max_output_bytes')==LIMIT and h.get('max_windows')==16,'hard cap metadata')
    need(h.get('cpu_scope')=='matched-retrace-to-end-frame' and h.get('independent_audio_transport') is True,'phase/transport binding')
    need(f.get('complete') is True and f.get('output_failed') is False and f.get('drops')==0 and f.get('invalid')==0,'incomplete transport/footer')
    need(all(integer(f.get(k)) for k in ('windows','phases','slow_summaries','trigger_summaries')),'malformed footer counts')
    windows=[]; summaries=[]; current=None; refusals=[]
    def close():
        if current is not None:
            need(len(current['events'])==current['header']['copied'],'window event count mismatch')
            need(len(current['triggers'])==current['header']['triggers'],'window trigger count mismatch')
    for row in rows[1:-1]:
        typ=row.get('type')
        if typ in ('event','summary','binding'):
            required=('sequence','kind','begin_ns','end_ns','thread','object','value','frame','id','detail','cpu_ns','packet','ticket','generation','aux0','aux1')
            need(all(integer(row.get(k)) for k in required),'malformed event fields')
            k=row['kind'];need(0<k<len(NAMES) and row.get('name')==NAMES[k],'unknown event kind/name')
            need(row['begin_ns']<=row['end_ns'] and row['generation']>0,'invalid timestamp/generation')
        if typ=='summary':
            need(row['kind'] in (2,3) and row['end_ns']-row['begin_ns']>50000000,'bad near-miss threshold')
            wall=row['end_ns']-row['begin_ns'];cpu=row['cpu_ns']
            trigger=wall>100000000 and cpu*10<wall
            need((row['kind']==3)==trigger,'trigger ratio/wall mismatch')
            need(all(r['frame']!=row['frame'] for r in summaries),'duplicate phase summary')
            summaries.append(row)
        elif typ=='window':
            close();need(len(windows)<16,'window cap')
            need(all(integer(row.get(k)) for k in ('frame','start_ns','end_ns','copied','triggers','missing','drops','clipped')),'malformed window counts')
            need(0<row['triggers']<=16 and 0<row['copied']<=65536,'bounded window sizes')
            need(row.get('complete') is True and row.get('prefix') is True and row.get('suffix') is True and
                 row.get('missing')==0 and row.get('drops')==0 and row.get('clipped')==0 and row.get('attribution')=='eligible-boundaries-only','REFUSED incomplete window')
            need(integer(row.get('start_ns')) and integer(row.get('end_ns')) and row['start_ns']<row['end_ns'],'window range')
            current={'header':row,'events':[],'triggers':[]};windows.append(current)
        elif typ=='event':
            need(current is not None,'orphan window event')
            need(not current['events'] or row['sequence']>current['events'][-1]['sequence'],'duplicate/nonincreasing window sequence')
            need(row['end_ns']>=current['header']['start_ns'] and row['begin_ns']<=current['header']['end_ns'],'event outside window')
            need(row['kind']!=4,'invalid phase in window')
            current['events'].append(row)
        elif typ=='window-trigger':
            need(current is not None and integer(row.get('frame')),'orphan window trigger')
            need(row['frame'] not in current['triggers'],'duplicate coalesced trigger');current['triggers'].append(row['frame'])
        elif typ=='binding':need(row['kind']==40,'unknown thread binding')
        elif typ=='refused':refusals.append(row)
        elif typ=='writer':need(integer(row.get('bytes_total')) and row['bytes_total']<=LIMIT,'writer byte cap')
        else:raise ValueError('unknown record '+str(typ))
    close();need(not refusals,'explicit refusal')
    need(f.get('windows')==len(windows) and f.get('slow_summaries')==len(summaries),'summary/window accounting')
    triggers=[r for r in summaries if r['kind']==3]
    need(f.get('trigger_summaries')==len(triggers),'trigger summary accounting')
    admitted=[t for w in windows for t in w['triggers']]
    need(sorted(admitted)==sorted(r['frame'] for r in triggers),'trigger window coverage')
    for window in windows:
        for frame in window['triggers']:
            summary=next(row for row in triggers if row['frame']==frame)
            phases=[row for row in window['events'] if row['kind']==1 and row['frame']==frame and row['value']==1]
            need(len(phases)==1 and all(phases[0][key]==summary[key] for key in ('begin_ns','end_ns','cpu_ns')),
                 'missing/ambiguous/mismatched native phase endpoints')
    need(f['phases']>=len(summaries),'more slow summaries than phases')
    return {'protocol_passed':True,'status':'ELIGIBLE_BOUNDARIES' if windows else 'COMPLETE_NO_TRIGGER',
            'root_cause':'UNASSIGNED: inspect named spans and dependencies; CPU scarcity alone is not a cause',
            'windows':len(windows),'slow_phases':len(summaries),'triggers':len(triggers),
            'phase_count':f.get('phases'),'filtered_short_spans':f.get('filtered_short_spans'),
            'vm_scope':'process-wide only; run-state samples are point observations',
            'summaries':summaries}
def main():
    p=argparse.ArgumentParser();p.add_argument('history',type=Path);p.add_argument('--out',type=Path);a=p.parse_args()
    need(a.history.stat().st_size<=LIMIT,'file exceeds hard output cap')
    with a.history.open() as stream:rows=[json.loads(line) for line in stream]
    result=analyze(rows,a.history.stat().st_size)
    text=json.dumps(result,indent=2)+'\n'
    if a.out:
        with a.out.open('x') as out:out.write(text)
    else:print(text,end='')
if __name__=='__main__':main()
