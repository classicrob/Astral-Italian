import csv
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
from PIL import Image, ImageDraw, ImageFilter
import visual_pairs as visual


def synthetic_images(root):
    root.mkdir(parents=True,exist_ok=True)
    base=Image.new('RGB',(320,180),(45,92,160));d=ImageDraw.Draw(base)
    d.rectangle((0,130,319,179),fill=(48,124,61))
    d.polygon([(20,130),(85,35),(140,130)],fill=(180,170,110))
    d.rectangle((210,64,248,128),fill=(230,40,25));d.rectangle((218,75,240,115),fill=(250,220,35))
    for x in range(0,320,20):d.line((x,152,x+10,152),fill=(120,170,80),width=2)
    cases={'identical':base.copy(),'resolution_nearest':base.resize((640,360),Image.Resampling.NEAREST),
           'resolution_bilinear':base.resize((960,540),Image.Resampling.BILINEAR),
           'filter':base.filter(ImageFilter.GaussianBlur(.5))}
    missing=base.copy();ImageDraw.Draw(missing).rectangle((210,64,248,128),fill=(45,92,160));cases['missing_object']=missing
    color=base.copy();ImageDraw.Draw(color).rectangle((210,64,248,128),fill=(20,180,220));cases['color']=color
    translated=Image.new('RGB',base.size,(45,92,160));translated.paste(base,(7,0));cases['translation']=translated
    paths={}
    for name,image in cases.items():
        paths[name]=root/(name+'.png');image.save(paths[name])
    return paths


def bind(path):return {'path':str(path),'sha256':visual.digest(path)}


def synthetic_manifest(root,images,offset=60):
    from kpad_script import contract
    tape=root/'idle.jsonl'
    if not tape.exists():
        with tape.open('w') as f:
            for frame in range(600):
                for channel in range(4):
                    error=0 if channel==0 else -1
                    f.write(json.dumps(dict(frame=frame,read_ordinal=channel,channel=channel,requested_length=120,
                        returned_count=1 if channel==0 else 0,error=error,
                        connection=dict(probe_error=error,device_type=1 if channel==0 else 253),
                        samples=[contract.idle_sample(channel==0)]))+'\n')
    pair={'stage':'Synthetic','scenario':1,'offset':offset,'input_kind':'idle'}
    for role,source,image in [('native',100,images[0]),('dolphin',200,images[1])]:
        directory=root/role;directory.mkdir(exist_ok=True)
        sync=directory/'sync.json';sync.write_text(json.dumps(dict(phase='post-endFrame',stage='Synthetic',scenario=1,source_frame=source,source_scene_epoch=0)))
        trace=directory/'trace.csv'
        with trace.open('w') as f:
            fields=['frame','scene','stage','scenario','init_state','scene_valid','player_valid','scene_epoch']
            w=csv.DictWriter(f,fieldnames=fields);w.writeheader()
            for frame in range(601):w.writerow(dict(frame=frame,scene='Game',stage='Synthetic',scenario=1,init_state=6,scene_valid=1,player_valid=1,scene_epoch=0))
        g={'kind':'game-viewport','pixel_aspect_ratio':[1,1],'display_aspect_ratio':[16,9],'color_space':'sRGB'}
        provenance=dict(host='synthetic-host',executable_sha256='a'*64,source_revision='b'*40,renderer='synthetic-fixture',image_geometry=g)
        presentation=directory/'presentation.json'
        presentation.write_text(json.dumps(dict(schema_version=1,status='captured',stage='Synthetic',scenario=1,
            ready_source_frame=source,capture_source_frame=source+offset,rendered_source_frame=source+offset,
            image_sha256=visual.digest(image),render_fence_complete=True,rendered_frame_id=700+offset,readback_frame_id=700+offset,
            method='native-renderer-readback' if role=='native' else 'dolphin-xfb-present-readback',input_sha256=visual.digest(tape),**provenance)))
        delivery=directory/'delivery.json';delivery.write_text(json.dumps(dict(schema_version=1,status='captured',input_sha256=visual.digest(tape),frames=600,read_count=2400,
            source_frame_start=source+1,source_frame_end=source+600,method='native-consumed-read-log' if role=='native' else 'dolphin-actual-export')))
        pair[role]=dict(image=bind(image),sync=bind(sync),trace=bind(trace),input=bind(tape),capture_source_frame=source+offset,
                        presentation_binding='frame-id-verified',presentation_receipt=bind(presentation),input_delivery_receipt=bind(delivery),**provenance)
    return {'schema_version':1,'synthetic':True,'pairs':[pair]}


class ResolutionTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name)
        self.images=synthetic_images(self.root/'images')

    def test_filter_resolution_tolerance_does_not_hide_missing_color_or_translation(self):
        results={k:visual.metrics(self.images['identical'],v) for k,v in self.images.items()}
        self.assertEqual(results['identical']['visual_distance'],0)
        benign=max(results[k]['visual_distance'] for k in ('resolution_nearest','resolution_bilinear','filter'))
        self.assertLess(benign,.01)
        for key in ('missing_object','color','translation'):
            self.assertGreater(results[key]['visual_distance'],benign*3,key)
        self.assertGreater(results['missing_object']['robust']['worst_tile_rgb_mae'],.1)
        self.assertFalse(results['translation']['normalization']['spatial_registration'])

    def test_color_changes_with_matching_luminance_are_still_ranked(self):
        a,b=self.root/'a.png',self.root/'b.png'
        Image.new('RGB',(32,32),(255,0,0)).save(a)
        Image.new('RGB',(32,32),(0,130,0)).save(b)
        r=visual.metrics(a,b)
        self.assertGreater(r['luma_block_ssim_8x8'],.99)
        self.assertGreater(r['visual_distance'],.2)

    def test_aspect_mismatch_is_not_fitted(self):
        other=self.root/'other.png';Image.new('RGB',(320,200)).save(other)
        with self.assertRaisesRegex(ValueError,'aspect'):visual.metrics(self.images['identical'],other)

    def test_declared_pixel_aspect_is_applied_without_cropping(self):
        other=self.root/'other.png'
        with Image.open(self.images['identical']) as image:image.resize((320,240)).save(other)
        g={'kind':'game-viewport','pixel_aspect_ratio':[4,3],'display_aspect_ratio':[16,9],'color_space':'sRGB'}
        r=visual.metrics(self.images['identical'],other,None,g)
        self.assertLess(r['visual_distance'],.02)
        self.assertEqual(r['normalization']['dolphin_pixels'],[320,240])

    def test_transparency_and_unknown_colorspace_rejected(self):
        p=self.root/'alpha.png';Image.new('RGBA',(320,180),(1,2,3,0)).save(p)
        with self.assertRaisesRegex(ValueError,'transparent'):visual.metrics(self.images['identical'],p)
        with self.assertRaisesRegex(ValueError,'sRGB'):visual.metrics(self.images['identical'],self.images['identical'],{'kind':'game-viewport'})

    def run_manifest(self,data):
        path=self.root/'pairs.json';path.write_text(json.dumps(data));out=self.root/'report'
        with patch('visual_pairs.private_path',side_effect=Path):report=visual.compare_manifest(path,out,keep=2)
        return report,out

    def fixture(self,offset=60):return synthetic_manifest(self.root,(self.images['identical'],self.images['resolution_nearest']),offset)

    def test_complete_provenance_each_offset_and_ranked_html(self):
        data=self.fixture(60)
        for offset in (300,600):
            directory=self.root/str(offset);directory.mkdir()
            data['pairs']+=synthetic_manifest(directory,(self.images['identical'],self.images['missing_object']),offset)['pairs']
        data['pairs'].append({'stage':'Missing','scenario':1,'offset':60,'input_kind':'idle'})
        result,out=self.run_manifest(data)
        self.assertEqual(result['measured_pairs'],3)
        self.assertEqual(result['notmeasured_pairs'],3)
        self.assertEqual(result['qualification'],'BLOCKED');self.assertIsNone(result['measured_qualified_frames'])
        self.assertEqual(result['results'][0]['rank'],3)
        self.assertEqual(len(result['retained']),2)
        self.assertIn('NOTMEASURED',(out/'report.html').read_text())
        self.assertTrue((Path(result['retained'][0])/'difference.png').exists())

    def test_missing_or_unbound_providers_never_get_metrics(self):
        for missing in ('presentation_receipt','input_delivery_receipt','image_geometry','dolphin'):
            with self.subTest(missing=missing),tempfile.TemporaryDirectory() as tmp:
                root=Path(tmp);data=synthetic_manifest(root,(self.images['identical'],self.images['identical']))
                if missing=='dolphin':del data['pairs'][0]['dolphin']
                else:del data['pairs'][0]['native'][missing]
                path=root/'pairs.json';path.write_text(json.dumps(data))
                with patch('visual_pairs.private_path',side_effect=Path):r=visual.compare_manifest(path,root/'report')
                self.assertEqual(r['status'],'BLOCKED');self.assertEqual(r['results'][0]['measurement_status'],'NOTMEASURED')
                self.assertNotIn('metrics',r['results'][0])

    def test_intermediate_reload_even_with_matching_endpoints_blocks(self):
        data=self.fixture();provider=data['pairs'][0]['native'];trace=Path(provider['trace']['path'])
        text=trace.read_text().replace('30,Game,Synthetic,1,6,1,1,0','30,Game,Synthetic,1,0,1,0,0')
        trace.write_text(text);provider['trace']=bind(trace)
        r,_=self.run_manifest(data);self.assertEqual(r['measured_pairs'],0)
        self.assertIn('inside capture interval',r['results'][0]['error'])

    def test_unsupplied_offsets_are_explicit_notmeasured(self):
        result,_=self.run_manifest(self.fixture())
        missing=[r for r in result['results'] if r['measurement_status']=='NOTMEASURED']
        self.assertEqual([r['offset'] for r in missing],[300,600])
        self.assertTrue(all('metrics' not in r for r in missing))

    def test_changed_provider_identity_and_geometry_are_unbound(self):
        data=self.fixture();data['pairs'][0]['native']['executable_sha256']='c'*64
        result,_=self.run_manifest(data)
        self.assertEqual(result['measured_pairs'],0)
        self.assertIn('unbound provider',result['results'][0]['error'])

    def test_missing_trace_frame_blocks_instead_of_interpolating(self):
        data=self.fixture();provider=data['pairs'][0]['native'];trace=Path(provider['trace']['path'])
        trace.write_text(''.join(line for line in trace.read_text().splitlines(True) if not line.startswith('30,')))
        provider['trace']=bind(trace)
        r,_=self.run_manifest(data);self.assertEqual(r['measured_pairs'],0)
        self.assertIn('noncontiguous',r['results'][0]['error'])

    def test_stale_frame_or_input_provenance_blocks(self):
        data=self.fixture();provider=data['pairs'][0]['native'];p=Path(provider['presentation_receipt']['path'])
        receipt=json.loads(p.read_text());receipt['rendered_source_frame']-=1;p.write_text(json.dumps(receipt));provider['presentation_receipt']=bind(p)
        r,_=self.run_manifest(data);self.assertEqual(r['measured_pairs'],0)

    def test_receipt_builder_binds_geometry_and_identity_without_filling_unknowns(self):
        from visual_readiness import presentation_receipt
        private=self.root/'MacioGalaxy2-gamedata';private.mkdir();image=private/'synthetic.png'
        Image.new('RGB',(32,18)).save(image)
        request=dict(status='PENDING_PRESENTATION',stage='Synthetic',scenario=1,ready_source_frame=100,capture_source_frame=160,source_scene_epoch=0)
        copy=dict(frame_id=2,source_frame=160,scene_epoch=0)
        readback=dict(frame_id=2,fence_complete=True,success=True,image_sha256=visual.digest(image))
        metadata=dict(host='synthetic',executable_sha256='a'*64,source_revision='b'*40,renderer='synthetic',input_sha256='c'*64,
                      image_geometry=dict(kind='game-viewport',pixel_aspect_ratio=[1,1],display_aspect_ratio=[16,9],color_space='sRGB'))
        with patch('visual_readiness.Path.home',return_value=self.root.resolve()):
            result=presentation_receipt(request,copy,readback,image,'native-renderer-readback',metadata)
            self.assertEqual(result['image_geometry'],metadata['image_geometry'])
            self.assertEqual(result['input_sha256'],'c'*64)
            with self.assertRaises(ValueError):presentation_receipt(request,copy,readback,image,'native-renderer-readback',{})

    def test_delivery_receipt_must_bind_actual_input(self):
        data=self.fixture();provider=data['pairs'][0]['dolphin'];p=Path(provider['input_delivery_receipt']['path'])
        receipt=json.loads(p.read_text());receipt['input_sha256']='0'*64;p.write_text(json.dumps(receipt));provider['input_delivery_receipt']=bind(p)
        r,_=self.run_manifest(data);self.assertEqual(r['measured_pairs'],0)
        self.assertIn('delivered input',r['results'][0]['error'])


if __name__=='__main__':unittest.main()
