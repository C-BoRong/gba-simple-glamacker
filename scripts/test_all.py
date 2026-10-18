"""Build isolated test ROMs and read their SRAM result markers in mGBA."""
import argparse, json, os, shutil, subprocess, sys, time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def command(args,env=None): subprocess.run([str(x) for x in args],cwd=ROOT,env=env,check=True)
def emulate(emulator,rom,expected,timeout=60):
    sav=rom.with_suffix('.sav'); sav.unlink(missing_ok=True)
    env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
    if os.name=='nt':
        # Do not import the user's mGBA profile (pause, BIOS, save and sync settings).
        system=os.environ.get('SystemRoot','C:/Windows')
        env={'SystemRoot':system,'PATH':str(Path(system)/'System32'),'SDL_VIDEODRIVER':'dummy','SDL_AUDIODRIVER':'dummy'}
    flags=subprocess.CREATE_NO_WINDOW if os.name=='nt' else 0
    process=subprocess.Popen([str(emulator),'-C','savegamePath='+rom.parent.as_posix(),rom.as_posix()],cwd=ROOT,env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,creationflags=flags)
    data=b''; deadline=time.monotonic()+timeout
    try:
        while time.monotonic()<deadline:
            if sav.exists():
                try: data=sav.read_bytes()
                except PermissionError: pass
                if len(data)>=90 and (data[64:64+len(expected)]==expected or data[64:65]==b'F'): break
            if process.poll() is not None: raise RuntimeError(f'mGBA exited early: {process.returncode}')
            time.sleep(.25)
    finally:
        if process.poll() is None: process.terminate()
        try: process.wait(timeout=5)
        except subprocess.TimeoutExpired: process.kill(); process.wait()
    if sav.exists(): data=sav.read_bytes()
    if len(data)<90 or data[64:64+len(expected)]!=expected:
        line=int.from_bytes(data[88:90],'little') if len(data)>=90 else -1
        raise RuntimeError(f'{rom.name} failed/timed out; source line={line}, marker={data[64:80]!r}')
    result={'rom':str(rom.relative_to(ROOT)),'checks':int.from_bytes(data[80:82],'little'),'result':'PASS'}
    if expected==b'VN_TEST_PASS':
        result['animation_draw_cycles']=int.from_bytes(data[92:96],'little')
        result['gba_frame_cycles']=280896
    return result
def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--skip-emulator',action='store_true')
    parser.add_argument('--emulator',default=os.environ.get('GBA_EMULATOR') or shutil.which('mgba-sdl.exe') or shutil.which('mgba-sdl') or '')
    args=parser.parse_args()
    command([sys.executable,ROOT/'scripts/test_schema.py'])
    kit=Path(os.environ.get('DEVKITPRO','C:/devkitPro'))
    env=dict(os.environ,DEVKITPRO=str(kit))
    env['PATH']=str(kit/'msys2/usr/bin')+os.pathsep+env.get('PATH','')
    command([sys.executable,ROOT/'scripts/test_game.py'],env)
    command([kit/'msys2/usr/bin/bash.exe','--noprofile','--norc','./scripts/test_game.sh'],env)
    command([sys.executable,ROOT/'scripts/test_audio.py'],env)
    results=[]
    if not args.skip_emulator:
        emulator=Path(args.emulator)
        if not emulator.is_file(): raise SystemExit('mGBA SDL missing. Set GBA_EMULATOR or use --skip-emulator for build-only validation.')
        for rom,marker in [(ROOT/'generated/test-game.gba',b'VN_TEST_PASS'),(ROOT/'generated/audio-test/test-audio.gba',b'AUDIO_PASS')]:
            result=emulate(emulator,rom,marker);results.append(result)
            print(f'{rom.name}: PASS ({result["checks"]} checks)',flush=True)
    (ROOT/'generated/test-results.json').write_text(json.dumps({'host_tests':'PASS','emulator_tests':results,'emulator_skipped':args.skip_emulator},indent=2)+'\n',encoding='utf-8')
if __name__=='__main__':main()
