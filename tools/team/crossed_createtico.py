"""Fixed CreateTico component adapter; never a production certificate or compiler."""
import hashlib
import json
from pathlib import Path
import crossed_dispatch_component as component
SPEC_SHA='2e9631f3d350113a97c9e6d3b69821183d1289276f9f8195b612cb10fe71ea7f'
def spec():
    raw=Path(__file__).with_name('crossed_createtico_spec.json').read_bytes()
    component.require(hashlib.sha256(raw).hexdigest()==SPEC_SHA,'fixed family spec drift')
    return json.loads(raw)
def compare(before,after,envelope,pin,config_before,config_after):
    return component.compare_summaries(before,after,envelope,pin,spec(),config_before,config_after)
def original_slots(data):return component.original_slots(data,spec())
def qualify_files(before,after,envelope,pin,map_before,map_after):
    return component.qualify_files(before,after,envelope,pin,spec(),map_before,map_after)
def production_status():
    return {'status':'BLOCKED_PENDING_FRESH_ENVELOPE_OWNERSHIP_CONTROLS_AND_COMPILER_CONTAINMENT',
            'execution_enabled':False,'family':'CreateTico','certificate':False}
