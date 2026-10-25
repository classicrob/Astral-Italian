import unittest
from receipts54m import excluded_by_functional_scope

class FunctionalScope(unittest.TestCase):
 def test_only_explicit_port_exclusions(self):
  excluded={'v2-cold-stall','fifo-default-cadence-latency','wii-proof','wii-old-baseline-lineage'}
  functional={'macos-15-compat','seed-pack','gx-fresh-warm','normal-route-grandmap','normal-boot-ui','full-star-flipswap','intro-skip','flipswap-cappy','v2-image-pair','v2-warm-pairs','input-latency','audio-thread-priority'}
  self.assertTrue(all(excluded_by_functional_scope(g,True) for g in excluded))
  self.assertTrue(all(not excluded_by_functional_scope(g,True) for g in functional))
 def test_original_scope_excludes_nothing(self):
  self.assertTrue(all(not excluded_by_functional_scope(g,False) for g in ('v2-cold-stall','fifo-default-cadence-latency','wii-proof','wii-old-baseline-lineage','input-latency')))

if __name__=='__main__':unittest.main()
