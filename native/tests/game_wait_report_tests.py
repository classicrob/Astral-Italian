#!/usr/bin/env python3
import copy,importlib.util,json,re,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('report',ROOT/'native/tools/game_wait_report.py');r=importlib.util.module_from_spec(spec);spec.loader.exec_module(r)
spec2=importlib.util.spec_from_file_location('codegen',Path(__file__).with_name('game_wait_codegen_tests.py'));c=importlib.util.module_from_spec(spec2);spec2.loader.exec_module(c)
def event(typ,kind=3,**kw):
    row=dict(type=typ,sequence=2,kind=kind,name=r.NAMES[kind],begin_ns=3000000000,end_ns=3200000000,
             thread=5,object=0,value=0,frame=9,id=9,detail=2,cpu_ns=1000000,packet=0,ticket=0,generation=1,aux0=0,aux1=0)
    row.update(kw);return row
def fixture():
    return [dict(type='header',schema=1,clock='steady-ns',cells=65536,max_memory_bytes=33554432,max_output_bytes=r.LIMIT,
                 max_windows=16,cpu_scope='matched-retrace-to-end-frame',independent_audio_transport=True),
            event('summary'),dict(type='window',frame=9,start_ns=1000000000,end_ns=3450000000,triggers=1,copied=1,
                                  complete=True,prefix=True,suffix=True,missing=0,drops=0,clipped=0,attribution='eligible-boundaries-only'),
            dict(type='window-trigger',frame=9),event('event',kind=1,value=1),
            dict(type='footer',complete=True,output_failed=False,drops=0,invalid=0,windows=1,phases=10,slow_summaries=1,trigger_summaries=1)]
class Tests(unittest.TestCase):
    def test_qualified_protocol_is_boundaries_not_cause(self):
        out=r.analyze(fixture(),1000);self.assertTrue(out['protocol_passed']);self.assertIn('UNASSIGNED',out['root_cause'])
    def test_missing_dropped_clipped_and_suffix_refused(self):
        for key,val in [('missing',1),('drops',1),('clipped',1),('prefix',False),('suffix',False),('complete',False)]:
            rows=fixture();rows[2][key]=val
            with self.assertRaises(ValueError):r.analyze(rows,1000)
    def test_caps_footer_and_unknown_kind(self):
        for change in ('footer','bytes','kind','record'):
            rows=fixture();size=1000
            if change=='footer':rows.pop()
            if change=='bytes':size=r.LIMIT+1
            if change=='kind':rows[4]['kind']=100
            if change=='record':rows.insert(-1,dict(type='invented'))
            with self.assertRaises(ValueError):r.analyze(rows,size)
    def test_strict_ratio_wall_and_summary_count(self):
        for value in (20000000,20000001):
            rows=fixture();rows[1]['cpu_ns']=value
            with self.assertRaises(ValueError):r.analyze(rows,1000)
        rows=fixture();rows[1]['end_ns']=3100000000
        with self.assertRaises(ValueError):r.analyze(rows,1000)
        rows=fixture();rows[-1]['slow_summaries']=0
        with self.assertRaises(ValueError):r.analyze(rows,1000)
    def test_unmatched_duplicate_and_silently_dropped_rows(self):
        for change in ('trigger','event','sequence','summary'):
            rows=fixture()
            if change=='trigger':rows[3]['frame']=10
            if change=='event':rows.pop(4)
            if change=='sequence':rows.insert(5,copy.deepcopy(rows[4]));rows[2]['copied']=2
            if change=='summary':rows.insert(2,copy.deepcopy(rows[1]))
            with self.assertRaises(ValueError):r.analyze(rows,1000)
    def test_explicit_refusal_and_invalid_phase(self):
        rows=fixture();rows.insert(-1,dict(type='refused',reason='cap'))
        with self.assertRaises(ValueError):r.analyze(rows,1000)
        rows=fixture();rows[4]=event('event',kind=4)
        with self.assertRaises(ValueError):r.analyze(rows,1000)
    def test_near_miss_never_requests_window(self):
        rows=fixture();rows=[rows[0],event('summary',kind=2,end_ns=3080000000,cpu_ns=1000),rows[-1]]
        rows[-1].update(windows=0,trigger_summaries=0)
        self.assertEqual(r.analyze(rows,500)['windows'],0)
    def test_schema_dictionary_matches_cpp(self):
        header=(ROOT/'native/include/petari/game_wait_history.hpp').read_text()
        names=re.search(r'kindNames\[\].*?\{(.*?)\};',header,re.S).group(1)
        self.assertEqual(re.findall(r'"([\w-]+)"',names),r.NAMES)
    def test_runtime_has_no_audio_transport_and_no_suspension_or_priority_change(self):
        source=(ROOT/'native/app/game_wait_history.cpp').read_text()
        for forbidden in ('AudioWorkgroup','thread_suspend(','pthread_set_qos_class','pthread_setschedparam','task_threads('):
            self.assertNotIn(forbidden,source)
        self.assertIn('static_assert(sizeof(State)<32ull*1024*1024',source)
        self.assertIn('std::fopen(path,"wx")',source)
    def test_release_checker_accepts_guarded_original_and_rejects_instrumented_fast_path(self):
        ir="""define i32 @gw_probe_guarded(ptr %p) {
  %flag = load atomic i8, ptr @enabled monotonic, align 1
  %off = icmp eq i8 %flag, 0
  br i1 %off, label %fast, label %slow
fast:
  %result = tail call i32 @gw_work(ptr %p)
  ret i32 %result
slow:
  call void @observer()
  ret i32 0
}
define void @gw_probe_metadata(ptr %p) {
  %flag = load atomic i8, ptr @enabled monotonic, align 1
  %on = icmp ne i8 %flag, 0
  br i1 %on, label %work, label %done
work:
  %v = call i32 @gw_work(ptr %p)
  br label %done
done:
  ret void
}
"""
        self.assertTrue(c.verify(ir))
        with self.assertRaises(ValueError):c.verify(ir.replace('  %result = tail call','  call void @observer()\n  %result = tail call'))

    def test_release_actual_apple_clang21_shared_return(self):
        ir=(ROOT/'native/tests/fixtures/game_wait_apple_clang21.ll').read_text()
        self.assertTrue(c.verify(ir))
        # The disabled successor is %5, whose original-call result %6 is
        # selected in shared return block %26 via the [%6, %5] PHI operand.
        negatives=[
            ('  %6 = tail call', '  call void @observer()\n  %6 = tail call'),
            ('  %6 = tail call', '  %bad = load ptr, ptr @hooks\n  %6 = tail call'),
            ('[ %6, %5 ]', '[ 0, %5 ]'),
            ('[ %6, %5 ]', '[ %11, %5 ]'),
            ('  ret i32 %27', '  ret i32 %11'),
            ('  %6 = tail call', '  %extra = call i32 @gw_work(ptr %0)\n  %6 = tail call'),
            ('  br label %26\n\n7:', '  br label %5\n\n7:'),
            ('  br i1 %4, label %7, label %5', '  br i1 %4, label %5, label %7'),
            ('16:                                               ; preds = %12, %9, %4, %1\n  ret void',
             '16:                                               ; preds = %12, %9, %4, %1\n  call i32 @gw_work(ptr %0)\n  ret void'),
            ('  %3 = load atomic i8', '  call void @observer()\n  %3 = load atomic i8'),
        ]
        for old,new in negatives:
            with self.subTest(mutation=new):
                self.assertEqual(ir.count(old),1)
                with self.assertRaises(ValueError):c.verify(ir.replace(old,new))

    def test_release_checker_refuses_work_before_guard(self):
        with self.assertRaises(ValueError):c.verify('define i32 @gw_probe_guarded(ptr %p) {\n call i32 @gw_work(ptr %p)\n ret i32 0\n}\n')
if __name__=='__main__':unittest.main()
