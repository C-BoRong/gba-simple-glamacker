/* Separate ROM with generated synthetic clips; never touches the game save. */
#include <gba.h>
#include "../source/audio.c"
const char audio_save_signature[] __attribute__((used)) = "SRAM_V113";
static unsigned checks;
static void check(bool ok, unsigned line) {
    volatile uint8_t *s = (volatile uint8_t *)0x0E000000;
    if (ok) { ++checks; return; }
    s[64]='F'; s[88]=line; s[89]=line>>8;
    while (true) VBlankIntrWait();
}
#define CHECK(x) check((x),__LINE__)
static void frames(unsigned n) {
    for (unsigned i=0; i<n; ++i) { audio_frame(); VBlankIntrWait(); }
}
int main(void) {
    irqInit(); irqEnable(IRQ_VBLANK);
    audio_init();
    SaveData s = {.music_enabled=true,.voice_enabled=true,.music_volume=6,.voice_volume=10};
    audio_settings(&s);
    audio_music(vn_title_music); audio_voice(vn_node_audio[0].voice);
    frames(3);
    CHECK(mmEffectActive(music_handle)); CHECK(audio_voice_active());
    mm_sfxhand old = music_handle;
    audio_music(vn_title_music); CHECK(old==music_handle);
    s.music_volume=2; audio_settings(&s);
    CHECK(music_volume==51 && voice_volume==255);
    s.music_enabled=false; audio_settings(&s);
    CHECK(music_volume==0 && audio_voice_active());
    s.voice_enabled=false; audio_settings(&s);
    CHECK(voice_volume==0 && !audio_voice_active());
    s.voice_enabled=true; s.music_enabled=true; audio_settings(&s);
    CHECK(audio_voice_active());
    frames(180);
    CHECK(mmEffectActive(music_handle)); /* WAV loop survives many sample lengths. */
    CHECK(!audio_voice_active());
    audio_voice(vn_node_audio[0].voice); frames(2); CHECK(audio_voice_active());
    audio_voice(-1); CHECK(!audio_voice_active() && mmEffectActive(music_handle));
    audio_music(-1); CHECK(!music_handle);
    volatile uint8_t *ram=(volatile uint8_t *)0x0E000000;
    const char result[]="AUDIO_PASS";
    for (unsigned i=0; i<sizeof(result); ++i) ram[64+i]=result[i];
    ram[80]=checks; ram[81]=checks>>8;
    while (true) frames(1);
}
