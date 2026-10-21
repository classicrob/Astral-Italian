import copy
import json
import unittest
import dispatch_monitors as m

ROOT='/owned/proof';RESERVE=16777216

def fixture():
    contract={'terminal_metadata':{'reserved_bytes':RESERVE,'maximum_allocated_bytes':RESERVE,'scope':'monitor-copy-and-terminal-receipt-transfer-only'},'required_live_budget':{'minimum_free_bytes':50*1024**3,'maximum_owned_growth_bytes':4*1024**3,'sample_interval_seconds':0.25},'scan_limits':{'entries':200000,'seconds':2.0},'work_seconds':2400,'packaging_seconds':600,'cleanup_grace_seconds':4}
    data={};payloads={'execution':{'actual_result_sha256':'a'*64},'packaging':{'primary_encoded_sha256':'b'*64,'secondary_encoded_sha256':'c'*64}}
    for stage,start in [('execution',10.0),('packaging',20.0)]:
        cleanup={'all_owned_groups_absent':True,'remaining_pgids':[],'failures':[],'execution_error':None}
        budget={'root':ROOT,'failure':None,'minimum_free_bytes':50*1024**3+RESERVE,'maximum_owned_growth_bytes':4*1024**3-RESERVE,'sampling_seconds':0.25,'samples':1,'initial_allocated_bytes':123,'peak_observed_growth_bytes':456,'lowest_observed_free_bytes':60*1024**3,'max_scan_seconds':0.01,'cleanup':{'status':'CONFIRMED','all_owned_groups_absence_confirmed':True,'remaining_groups':[],'error':None,'base_report_read_error':None,'base_report':cleanup}}
        data[stage]={'budget':budget,'cleanup':cleanup,'completion':{'schema':1,'status':'COMPLETE','stage':stage,'root':ROOT,'contract_sha256':'d'*64,'started_monotonic':start,'ended_monotonic':start+1,'elapsed_seconds':1.0,'work_limit_seconds':2400 if stage=='execution' else 600,'cleanup_grace_seconds':4,'budget_sha256':'','cleanup_sha256':'','payload':payloads[stage]}}
    return contract,data,payloads

def seal(data):
    artifacts={};records={}
    for stage,files in data.items():
        files=copy.deepcopy(files)
        raw={n:m.canonical(files[n]) for n in ('budget','cleanup')}
        files['completion']['budget_sha256']=m.digest(raw['budget']);files['completion']['cleanup_sha256']=m.digest(raw['cleanup'])
        raw['completion']=m.canonical(files['completion']);artifacts[stage]=raw
        records[stage]={n:{'path':stage+'/'+n+'.json','sha256':m.digest(v)} for n,v in raw.items()}
    return records,artifacts

class Monitors(unittest.TestCase):
    def verify(self,contract,data,payloads):
        r,a=seal(data)
        return m.check(r,a,contract,'d'*64,{'before':ROOT+'/execution/before','after':ROOT+'/execution/after'},payloads)
    def test_successful_final_joins_only(self):
        c,d,p=fixture();self.assertEqual(self.verify(c,d,p)['status'],'FINAL_MONITOR_JOINS_MATCH')
    def test_missing_stage_and_stale_artifact_reject(self):
        c,d,p=fixture();r,a=seal(d);a['execution']['budget']+=b' '
        with self.assertRaises(ValueError):m.check(r,a,c,'d'*64,{'before':ROOT+'/execution/before','after':ROOT+'/execution/after'},p)
        del d['packaging']
        with self.assertRaises(ValueError):self.verify(c,d,p)
    def test_rehashed_false_success_still_rejects(self):
        mutations=[('budget','failure','disk crossed'),('budget','root','/foreign'),('budget','initial_allocated_bytes',0),('budget','peak_observed_growth_bytes',5*1024**3),('budget','lowest_observed_free_bytes',1),('budget','max_scan_seconds',3),('budget','samples',0),('cleanup','all_owned_groups_absent',False),('cleanup','remaining_pgids',[42]),('cleanup','failures',['EPERM']),('cleanup','execution_error','failed'),('completion','status','RUNNING'),('completion','contract_sha256','f'*64),('completion','ended_monotonic',float('nan')),('completion','elapsed_seconds',6000),('completion','payload',{})]
        for part,key,value in mutations:
            c,d,p=fixture();d['packaging'][part][key]=value
            with self.subTest(part=part,key=key),self.assertRaises(ValueError):self.verify(c,d,p)
    def test_no_reserve_or_overlapping_stages_reject(self):
        c,d,p=fixture();c['terminal_metadata']['reserved_bytes']=0
        with self.assertRaises(ValueError):self.verify(c,d,p)
        c,d,p=fixture();d['packaging']['completion'].update(started_monotonic=9.0,ended_monotonic=10.0)
        with self.assertRaises(ValueError):self.verify(c,d,p)



def terminal_fixture():
    c,d,p=fixture();records,artifacts=seal(d)
    receipt={'monitors':records,'archive':{'encoded_sha256':'1'*64}}
    secondary={'encoded_sha256':'2'*64}
    rb=m.canonical(receipt)
    files={rec['path']:artifacts[stage][name] for stage,rows in records.items() for name,rec in rows.items()}
    files['receipt.json']=rb
    files['transfer.json']=m.canonical({'receipt_sha256':m.digest(rb),'primary_archive':receipt['archive'],'secondary_archive':secondary})
    publication={'schema':1,'type':'DISPATCH_TERMINAL_PUBLICATION_V1','status':'COMPLETE','root':ROOT,'contract_sha256':m.digest(m.canonical(c)),'receipt_sha256':m.digest(rb),'transfer_sha256':m.digest(files['transfer.json']),'maximum_allocated_bytes':16777216,'publication_reserve_bytes':65536,'original_baseline_bytes':123,'allocated_before_metadata':1000,'allocated_after_metadata':1000+8*4096,'free_before_publication':60*1024**3,'files':{name:{'sha256':m.digest(raw),'bytes':len(raw),'allocated_bytes':4096} for name,raw in files.items()}}
    return c,receipt,rb,secondary,files,publication

class Terminal(unittest.TestCase):
    def verify(self,parts,mutate_hash=False):
        c,r,rb,s,files,t=parts;raw=m.canonical(t)
        return m.terminal_check(raw,'f'*64 if mutate_hash else m.digest(raw),rb,r,c,ROOT,123,files,s)
    def test_detached_success_does_not_self_hash(self):
        self.assertEqual(self.verify(terminal_fixture())['status'],'DETACHED_TERMINAL_PUBLICATION_JOINS_MATCH')
    def test_operator_pin_and_rehashed_bad_publication_reject(self):
        with self.assertRaises(ValueError):self.verify(terminal_fixture(),True)
        for key,value in [('root','/foreign'),('status','RUNNING'),('receipt_sha256','f'*64),('transfer_sha256','e'*64),('publication_reserve_bytes',0),('maximum_allocated_bytes',999999999),('original_baseline_bytes',0),('allocated_before_metadata',1),('allocated_after_metadata',9999999999),('free_before_publication',50*1024**3)]:
            parts=terminal_fixture();parts[-1][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):self.verify(parts)
    def test_extra_missing_and_changed_metadata_reject(self):
        parts=terminal_fixture();del parts[-1]['files']['transfer.json']
        with self.assertRaises(ValueError):self.verify(parts)
        parts=terminal_fixture();parts[-1]['files']['unexpected']={'sha256':'0'*64,'bytes':0,'allocated_bytes':0}
        with self.assertRaises(ValueError):self.verify(parts)
        parts=terminal_fixture();parts[-2]['receipt.json']+=b' '
        with self.assertRaises(ValueError):self.verify(parts)
    def test_allocation_and_record_mismatch_reject(self):
        for key,value in [('bytes',0),('allocated_bytes',-1),('allocated_bytes',16777216),('sha256','f'*64)]:
            parts=terminal_fixture();parts[-1]['files']['receipt.json'][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):self.verify(parts)

if __name__=='__main__':unittest.main()
