"""Compile the stable sample fixture, independent of the author's story/audio."""
import importlib.util, json, shutil, sys
from PIL import Image
from prepare_assets import transparent_character
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'generated/game-test'
def main():
    assets=OUT/'assets'; assets.mkdir(parents=True,exist_ok=True)
    shutil.copy2(ROOT/'tests/fixtures/story.json',assets/'story.json')
    shutil.copy2(ROOT/'assets/ui.json',assets/'ui.json')
    (assets/'audio.json').write_text(json.dumps({'music':{},'voice':{},'title_music':None,'default_music':None}),encoding='utf-8')
    story=json.loads((assets/'story.json').read_text(encoding='utf-8'))
    ui=json.loads((assets/'ui.json').read_text(encoding='utf-8'))
    refs=[s['file'] for s in story['scenes']]+[s['file'] for s in story['portraits']]+ui['backgrounds']+[ui['buttons']]
    for rel in set(refs):
        dest=assets/rel;dest.parent.mkdir(parents=True,exist_ok=True)
        shutil.copy2(ROOT/'assets'/rel,dest)
    (OUT/'generated').mkdir(exist_ok=True)
    # A shortened portrait tests vertical placement without going off-screen.
    source=assets/story['portraits'][0]['file']
    rgb,mask=transparent_character(source,'auto');rgb.putalpha(mask)
    rgb=rgb.crop((0,0,64,160))
    rgb.save(source)
    story['portraits'][0]['matte']='alpha'
    rgb.crop((0,0,64,120)).save(assets/'pose-test.png')
    story['portraits'].append(dict(story['portraits'][0],id='pose_test',file='pose-test.png',matte='alpha'))
    story['nodes'][16].update(character='pose_test',character_x=13,character_y=21,character_flip_x=True)
    # Add a conditional sample choice, a node effect, and a wait gate for runtime tests.
    story['nodes'][3]['choices'][0]['if']={'var':'bond','op':'>=','value':0}
    story['nodes'][4]['add']={'route':1}
    story['nodes'][17]['transition']={'background_in':'fade','background_out':'fade',
        'character_in':'slide_left','character_out':'slide_right','frames':4}
    (assets/'story.json').write_text(json.dumps(story,ensure_ascii=False,indent=2),encoding='utf-8')
    for name in ('prepare_story','prepare_assets','prepare_audio'):
        spec=importlib.util.spec_from_file_location(name,ROOT/'scripts'/f'{name}.py')
        mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
        mod.ROOT=OUT;mod.OUT=OUT/'generated'
        mod.main()
if __name__=='__main__':main()
