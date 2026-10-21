"""Synthetic pure models ONLY. No actual platform value/environment is read."""
import hashlib
import unittest
import dispatch_platform_context_proposal as proposal

class PlatformContextProposalModels(unittest.TestCase):
    def model(self):
        base={key:'SYNTHETIC_BASE_'+key for key in proposal.BASE_KEYS}
        env={**base,proposal.KEY:'SYNTHETIC_VALUE_NOT_OBSERVED'}
        record={'schema':1,'env':env,'env_sha256':hashlib.sha256(proposal.canonical(env)).hexdigest(),
                'named_value':proposal.observe_named_only({proposal.KEY:env[proposal.KEY]})}
        raw=proposal.canonical(record)
        return base,env,record,raw,hashlib.sha256(raw).hexdigest()
    def test_operator_exact_mapping_and_wrong_pin(self):
        base,env,record,raw,pin=self.model()
        self.assertEqual(proposal.require_operator_mapping(raw,pin,base),env)
        with self.assertRaises(ValueError):proposal.require_operator_mapping(raw,'0'*64,base)
    def test_missing_tenth_changed_and_case_reject_at_every_checkpoint(self):
        _,env,_,_,_=self.model()
        for checkpoint in ('parent','child','postack','postexec'):
            proposal.require_exact_observed(env,env)
            for wrong in ({k:v for k,v in env.items() if k!=proposal.KEY},
                          {**env,'UNDECLARED_TENTH':'v'},
                          {**env,proposal.KEY:'DIFFERENT_SYNTHETIC'},
                          {**env,proposal.KEY.lower():'v'}):
                with self.subTest(checkpoint=checkpoint),self.assertRaises(ValueError):
                    proposal.require_exact_observed(wrong,env)
    def test_operator_malformed_mapping_or_value_pin_rejects(self):
        base,env,record,_,_=self.model()
        for malformed in ({**record,'env':{k:v for k,v in env.items() if k!=proposal.KEY}},
                          {**record,'env':{**env,'OTHER':'v'}},
                          {**record,'env_sha256':'0'*64},
                          {**record,'named_value':{'key':proposal.KEY,'present':False}}):
            raw=proposal.canonical(malformed)
            with self.assertRaises(ValueError):
                proposal.require_operator_mapping(raw,hashlib.sha256(raw).hexdigest(),base)
    def test_named_only_missing_empty_bound_escape(self):
        self.assertEqual(proposal.observe_named_only({}),{'key':proposal.KEY,'present':False})
        self.assertTrue(proposal.observe_named_only({proposal.KEY:''})['present'])
        self.assertEqual(proposal.observe_named_only({proposal.KEY:'\n\u2603'})['value_hex'],'0ae29883')
        self.assertEqual(proposal.observe_named_only({proposal.KEY:'x'*512})['bytes'],512)
        for mapping in ({'OTHER':'secret'}, {proposal.KEY:'x'*513},{proposal.KEY:'bad\0value'},{proposal.KEY:None},{proposal.KEY:'\udcff'}):
            with self.assertRaises((ValueError,UnicodeError)):proposal.observe_named_only(mapping)

if __name__=='__main__':unittest.main()
