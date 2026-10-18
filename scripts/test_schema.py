"""Authoring validation, transparency and audio-reference regression tests."""
import copy, json, tempfile, unittest, wave
from pathlib import Path
from PIL import Image
from story_schema import validate, transition_settings, portrait_settings
from audio_schema import validate_audio
from prepare_assets import transparent_character
ROOT=Path(__file__).resolve().parents[1]
class SchemaTests(unittest.TestCase):
    def setUp(self):
        self.s=json.loads((ROOT/'tests/fixtures/story.json').read_text(encoding='utf-8'))
    def check(self): return validate(self.s,ROOT/'assets')
    def bad(self):
        with self.assertRaises((ValueError,KeyError)): self.check()
    def test_sample_and_portraits_without_legacy(self):
        self.s.pop('character',None);self.check()
        self.s['portraits'].append(dict(self.s['portraits'][0],id='mio2'))
        self.s['nodes'][1]['character']='mio2';self.check()
    def test_duplicate_node(self):
        self.s['nodes'][1]['id']=self.s['nodes'][0]['id'];self.bad()
    def test_transition_inheritance(self):
        self.s['transition']={'background_in':'fade','character_in':'slide_right','frames':20}
        t=transition_settings(self.s,{'transition':{'character_in':'slide_left'}})
        self.assertEqual((t['background_in'],t['character_in'],t['frames']),('fade','slide_left',20))
        self.assertEqual(transition_settings(self.s,{'transition':False})['background_in'],'none')
        self.check()
    def test_bad_transition(self):
        for t in [True, None, [], {'frames':0}, {'frames':121}, {'frames':True},
                  {'background_in':'slide_right'}, {'character_out':'fade'}, {'typo':1}]:
            with self.subTest(t=t):
                self.s['nodes'][0]['transition']=t;self.bad()
    def test_unknown_target(self):
        self.s['nodes'][0]['next']='missing';self.bad()
    def test_unknown_variable(self):
        self.s['nodes'][0]['add']={'missing':1};self.bad()
    def test_unknown_portrait(self):
        self.s['nodes'][0]['character']='missing';self.bad()
    def test_portrait_pose_defaults_and_overrides(self):
        base={'x':158,'y':0,'flip_x':True}
        self.assertEqual(portrait_settings(base,{'character_x':20,'character_flip_x':False}),
                         {'x':20,'y':0,'flip_x':False})
        self.assertEqual(portrait_settings(base,{}),base)
        self.s['nodes'][1].update(character_x=18,character_y=0,character_flip_x=True)
        self.check()
    def test_bad_portrait_pose(self):
        for value in [-1,240,True,1.5,'20']:
            self.s['nodes'][1]['character_x']=value;self.bad()
        self.s['nodes'][1].pop('character_x')
        for value in [-1,160,True]:
            self.s['nodes'][1]['character_y']=value;self.bad()
        self.s['nodes'][1].pop('character_y')
        self.s['nodes'][1]['character_flip_x']=1;self.bad()
        self.s['nodes'][1].pop('character_flip_x')
        self.s['nodes'][0]['character_x']=0;self.bad()
    def test_bad_portrait_default(self):
        self.s['portraits'][0]['flip_x']='yes';self.bad()
    def test_conditional_choice_requires_fallback(self):
        for c in self.s['nodes'][3]['choices']: c['if']={'var':'bond','value':1}
        self.bad()
    def test_branch_requires_else(self):
        self.s['nodes'][5].pop('else');self.bad()
    def test_loop_is_valid(self):
        self.s['nodes'][5]['else']='boat_talk';self.check()
    def test_true_end_cannot_be_normal(self):
        self.s['nodes'][0]['next']=self.s['true_start'];self.bad()
    def test_asset_cannot_escape(self):
        self.s['portraits'][0]['file']='../README.md';self.bad()
    def test_bool_not_integer(self):
        self.s['nodes'][0]['add']={'bond':True};self.bad()
    def test_alpha_threshold(self):
        with tempfile.TemporaryDirectory() as tmp:
            file=Path(tmp)/'alpha.png'; im=Image.new('RGBA',(4,1))
            im.putdata([(255,255,255,0),(1,2,3,127),(255,255,255,128),(4,5,6,255)]);im.save(file)
            _,mask=transparent_character(file,'auto')
            self.assertEqual(list(mask.tobytes()),[0,0,255,255])
    def test_white_matte_preserves_enclosed_white(self):
        with tempfile.TemporaryDirectory() as tmp:
            file=Path(tmp)/'matte.bmp';im=Image.new('RGB',(6,5),'white')
            for x,y in [(x,y) for x in range(1,5) for y in range(1,4) if x in (1,4) or y in (1,3)]:im.putpixel((x,y),(20,40,60))
            im.save(file);_,mask=transparent_character(file,'white')
            self.assertEqual(mask.getpixel((0,0)),0);self.assertEqual(mask.getpixel((2,2)),255)
    def test_audio_unknown_reference(self):
        with self.assertRaises(ValueError):validate_audio({'music':{},'voice':{}},{'nodes':[{'id':'a','voice':'missing'}]},ROOT/'assets')
    def test_audio_outside_assets(self):
        with self.assertRaises(ValueError):validate_audio({'music':{'x':'../x.wav'},'voice':{}},{'nodes':[]},ROOT/'assets')
    def test_audio_valid_and_empty(self):
        with tempfile.TemporaryDirectory() as tmp:
            assets=Path(tmp)
            with wave.open(str(assets/'line.wav'),'wb') as f:
                f.setparams((1,2,22050,0,'NONE','not compressed'));f.writeframes(bytes(16))
            validate_audio({'music':{},'voice':{'line':'line.wav'}},{'nodes':[{'voice':'line'}]},assets)
            validate_audio({'music':{},'voice':{}},{'nodes':[{}]},assets)
if __name__=='__main__':unittest.main()
