"""Fixed RouteOpen component adapter; never a production certificate or compiler."""
import hashlib
import json
from pathlib import Path
import crossed_dispatch_component as component
SPEC_SHA='4ded1ee73eda6681a6a79515d198358507e53f45c355952d3935d5d7e2df0d61'
def spec():
    raw=Path(__file__).with_name('crossed_routeopen_spec.json').read_bytes()
    component.require(hashlib.sha256(raw).hexdigest()==SPEC_SHA,'fixed family spec drift')
    return json.loads(raw)
def compare(before,after,envelope,pin,config_before,config_after):
    return component.compare_summaries(before,after,envelope,pin,spec(),config_before,config_after)
def original_slots(data):return component.original_slots(data,spec())
def qualify_files(before,after,envelope,pin,map_before,map_after):
    return component.qualify_files(before,after,envelope,pin,spec(),map_before,map_after)
def production_status():
    return {'status':'BLOCKED_PENDING_FRESH_ENVELOPE_OWNERSHIP_CONTROLS_AND_COMPILER_CONTAINMENT',
            'execution_enabled':False,'family':'RouteOpen','certificate':False}
