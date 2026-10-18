"""Generate short tones in an isolated fixture tree and compile a playback test ROM."""
import importlib.util
import json
import math
import os
from pathlib import Path
import struct
import subprocess
import wave
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'generated/audio-test'
def main():
    (OUT/'assets').mkdir(parents=True,exist_ok=True)
    for name,freq in [('music',220),('voice',440)]:
        with wave.open(str(OUT/'assets'/f'{name}.wav'),'wb') as w:
            w.setparams((1,2,22050,0,'NONE','not compressed'))
            w.writeframes(b''.join(struct.pack('<h',round(6000*math.sin(2*math.pi*freq*i/22050))) for i in range(11025)))
    config={'music':{'test':'music.wav'},'voice':{'line':'voice.wav'},'title_music':'test','default_music':'test'}
    (OUT/'assets/audio.json').write_text(json.dumps(config))
    (OUT/'assets/story.json').write_text(json.dumps({'nodes':[{'voice':'line'}]}))
    spec=importlib.util.spec_from_file_location('prepare_audio',ROOT/'scripts/prepare_audio.py')
    mod=importlib.util.module_from_spec(spec); spec.loader.exec_module(mod)
    mod.ROOT=OUT; mod.main(output=OUT)
    kit=Path(os.environ.get('DEVKITPRO','C:/devkitPro'))
    subprocess.run([str(kit/'devkitARM/bin/arm-none-eabi-gcc.exe'),'-std=gnu11','-O2','-Wall','-Wextra','-mcpu=arm7tdmi','-mthumb','-mthumb-interwork','-g',
        '-I'+str(OUT),'-I'+str(ROOT/'include'),'-I'+str(kit/'libgba/include'),str(ROOT/'scripts/test_audio.c'),str(OUT/'vn_audio.c'),
        '-L'+str(kit/'libgba/lib'),'-specs=gba.specs','-lmm','-lgba','-o',str(OUT/'test-audio.elf')],check=True)
    subprocess.run([str(kit/'devkitARM/bin/arm-none-eabi-objcopy.exe'),'-O','binary',str(OUT/'test-audio.elf'),str(OUT/'test-audio.gba')],check=True)
    subprocess.run([str(kit/'tools/bin/gbafix.exe'),str(OUT/'test-audio.gba'),'-tAUDIOTEST'],check=True)
if __name__=='__main__': main()
