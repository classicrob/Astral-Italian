import copy
import unittest
from dispatch_commands import derive, check_commands, dependency_name

CTX={k:'/tools/'+k for k in ('wrapper','compilers','sjiswrap','assembler','dtk','python')}
def spec():
    return {'units':{'src/x.o':{'source':'src/x.cpp','options':{
        'add_to_all':True,'extab_padding':None,'cflags':['-pragma "cats off"'],
        'extra_cflags':[],'mw_version':'Wii/1.0','shift_jis':True,
        'asflags':['-mgekko'],'extra_asflags':[]}}}}

class Commands(unittest.TestCase):
    def test_cpp_exact_language_quotes_and_postprocess(self):
        rows=derive(spec(),['src/x.o'],CTX);t=rows[0]['tokens']
        self.assertIn('cats off',t);self.assertIn('-lang=c++',t)
        self.assertEqual(t[-5:],['&&','/tools/python','tools/transform_dep.py','build/SB4E01/src/x.d','build/SB4E01/src/x.d'])
        import shlex
        check_commands(shlex.join(t),rows)
        for new in [t[::-1],t+['-extra'],[x.replace('/tools/wrapper','/other/wrapper') for x in t]]:
            with self.assertRaises(ValueError):check_commands(shlex.join(new),rows)
    def test_c_and_assembly_pipeline(self):
        s=spec();s['units']['src/x.o']['source']='src/x.c'
        self.assertIn('-lang=c',derive(s,['src/x.o'],CTX)[0]['tokens'])
        s['units']['src/x.o']['source']='src/x.s'
        tokens=derive(s,['src/x.o'],CTX)[0]['tokens']
        self.assertEqual(tokens[-5:],['/tools/dtk','elf','fixup','build/SB4E01/src/x.o','build/SB4E01/src/x.o'])
    def test_inventory_order_unknown_rule_and_escape(self):
        s=spec()
        for order in ([],['src/x.o','src/x.o'],['other.o']):
            with self.assertRaises(ValueError):derive(s,order,CTX)
        s['units']['src/x.o']['options']['extab_padding']=[0]
        with self.assertRaises(ValueError):derive(s,['src/x.o'],CTX)
        s=spec();s['units']['src/x.o']['source']='src/../x.cpp'
        with self.assertRaises(ValueError):derive(s,['src/x.o'],CTX)
    def test_dependency_root_is_exact_not_execution_suffix(self):
        root='/owned/execution/before'
        self.assertEqual(dependency_name(root+'/include/a.hpp',root),'include/a.hpp')
        self.assertEqual(dependency_name('include/a.hpp',root),'include/a.hpp')
        for path in ('/foreign/execution/before/include/a.hpp','/owned/execution/after/include/a.hpp','include/../a.hpp','include//a.hpp'):
            with self.assertRaises(ValueError):dependency_name(path,root)

if __name__=='__main__':unittest.main()
