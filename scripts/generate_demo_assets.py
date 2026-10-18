"""Recreate the repository's original geometric demo artwork with Pillow.

No downloaded images, fonts, music or commercial game assets are used.
Run from any directory: python scripts/generate_demo_assets.py
"""
import json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT=Path(__file__).resolve().parents[1]
ASSETS=ROOT/'assets'


def background(index):
    night=index>=7;sunset=3<=index<=6
    top=(17,29,55) if night else (137,94,119) if sunset else (89,157,181)
    bottom=(45,73,104) if night else (249,182,121) if sunset else (190,225,224)
    im=Image.new('RGB',(240,160));d=ImageDraw.Draw(im)
    for y in range(160):
        t=min(y/100,1);d.line((0,y,239,y),fill=tuple(round(a+(b-a)*t) for a,b in zip(top,bottom)))
    d.ellipse((169,18,193,42),fill=(220,235,226) if night else (255,230,157))
    d.polygon([(0,89),(24,79),(55,87),(84,71),(117,90),(147,85),(178,92),(240,79),(240,110),(0,110)],fill=(45,84,104))
    d.rectangle((0,97,239,159),fill=(20,49,76) if night else (45,101,124))
    for y in range(103,160,7):
        for x in range((y*7)%31,240,37):d.line((x,y,min(x+18,239),y),fill=(61,110,133))
    if index in (0,1,6,7,8):
        d.polygon([(0,126),(145,111),(156,118),(0,146)],fill=(62,65,76))
        for x,y in [(17,125),(67,119),(117,114)]:
            d.rectangle((x,y-22,x+3,y+6),fill=(44,48,62));d.rectangle((x-2,y-24,x+5,y-20),fill=(255,213,133) if night else (200,204,187))
    if index in (2,3,4,5):
        d.polygon([(22,113),(97,113),(84,127),(36,127)],fill=(238,229,211))
        d.line((56,79,56,113),fill=(64,66,79),width=2);d.polygon([(59,82),(59,108),(85,108)],fill=(242,211,152))
    return im


def actor():
    im=Image.new('RGBA',(64,160));d=ImageDraw.Draw(im)
    d.ellipse((14,153,52,159),fill=(0,0,0,50))
    d.rounded_rectangle((18,113,28,155),radius=4,fill='#314b68')
    d.rounded_rectangle((37,113,47,155),radius=4,fill='#314b68')
    d.rounded_rectangle((9,62,55,120),radius=12,fill='#e5e4d5',outline='#314b68',width=2)
    d.rounded_rectangle((1,70,12,108),radius=4,fill='#64c4bf')
    d.rounded_rectangle((52,70,63,108),radius=4,fill='#64c4bf')
    d.line((32,11,32,27),fill='#314b68',width=3);d.ellipse((28,5,36,13),fill='#efb85c')
    d.rounded_rectangle((8,26,56,65),radius=10,fill='#64c4bf',outline='#314b68',width=2)
    d.rounded_rectangle((14,35,50,54),radius=6,fill='#213c58')
    for x in (23,41):d.ellipse((x-2,41,x+2,45),fill='#f4e7b9')
    d.rectangle((23,76,41,91),fill='#efb85c');d.line((25,99,39,99),fill='#314b68',width=2)
    return im


def main():
    fixture=ROOT/'tests/fixtures/story.json'
    story=json.loads(fixture.read_text(encoding='utf-8'))
    for i,scene in enumerate(story['scenes']):
        scene['file']=f'images/bg/demo_{i:02}.png'
        path=ASSETS/scene['file'];path.parent.mkdir(parents=True,exist_ok=True);background(i).save(path)
    file='images/fg/guide.png'
    for p in story['portraits']:p.update(file=file,matte='alpha')
    story['character'].update(file=file,matte='alpha')
    path=ASSETS/file;path.parent.mkdir(parents=True,exist_ok=True);actor().save(path)
    story.update(title='潮声里的约定',subtitle='STORY STUDIO DEMO')
    for path in (fixture,ASSETS/'story.json'):
        path.write_text(json.dumps(story,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    ui=ASSETS/'images/ui';ui.mkdir(parents=True,exist_ok=True)
    for i,name in enumerate(('title','title_unlocked')):
        im=background(8 if i else 0);d=ImageDraw.Draw(im)
        d.rectangle((0,0,135,159),fill='#20374f')
        d.text((12,13),'GBA STORY',font=ImageFont.load_default(),fill='#f6e7bf')
        d.text((12,29),'STUDIO DEMO',font=ImageFont.load_default(),fill='#7ed0c6')
        guide=actor();im.paste(guide,(164,0),guide);im.save(ui/(name+'.png'))
    sheet=Image.new('RGB',(144,91),'white');d=ImageDraw.Draw(sheet)
    for row,label in enumerate(('TRUE END','CONTINUE','START','LOAD','SYSTEM','EXTRA','EXIT')):
        for col in range(2):
            x=col*72;y=row*13
            d.rounded_rectangle((x,y,x+71,y+12),radius=3,fill='#64c4bf' if col else '#334d66')
            d.text((x+5,y),label,font=ImageFont.load_default(),fill='#17394b' if col else '#f4e7c2')
    sheet.save(ui/'buttons.png')
    config={'backgrounds':['images/ui/title.png','images/ui/title_unlocked.png'],
            'buttons':'images/ui/buttons.png','button_width':72,'button_height':13,
            'columns':[0,72],'rows':[i*13 for i in range(7)],'menu_x':12,'menu_y':60,'menu_spacing':13}
    (ASSETS/'ui.json').write_text(json.dumps(config,indent=2)+'\n',encoding='utf-8')
    (ASSETS/'audio.json').write_text(json.dumps({'music':{},'voice':{},'title_music':None,'default_music':None},indent=2)+'\n',encoding='utf-8')
    print('Generated 9 backgrounds, 1 guide, 2 title images and 1 button sheet.')


if __name__=='__main__':main()
