/* Runs the production input and save code on GBA hardware / an emulator. */
#define main game_main
#include "../source/main.c"
#undef main
#include <gba_console.h>
#include <gba_systemcalls.h>
#include <gba_video.h>
#include <gba_timers.h>
#include <stdio.h>
#include <string.h>

static unsigned checks;
static void require(bool passed, unsigned line) {
    if (passed) { ++checks; return; }
    volatile uint8_t *failure = (volatile uint8_t *)0x0E000000;
    failure[64] = 'F'; failure[88] = line; failure[89] = line >> 8;
    consoleDemoInit();
    iprintf("\x1b[2JTEST FAILED\nline %u\n%u checks passed", line, checks);
    while (true) VBlankIntrWait();
}
#define CHECK(condition) require((condition), __LINE__)

static void reset_game(void) {
    animation_finish(); visual_valid=false;
    save_defaults(&save_data); story_reset(); node_recorded=false; node_frames=0; dialogue_hidden=false;
    mode = TITLE;
    active_node = VN_START;
    revealed = total_chars = wait_frames = selected_choice = tick = 0;
    title_selection = MENU_START;
    autoplay = replay = false;
}

static void finish_route(unsigned choice) {
    for (unsigned i = 0; i < VN_NODE_COUNT && mode == READING; ++i) {
        revealed = total_chars; node_frames=3600;
        selected_choice = vn_story[active_node].choice_count ? choice : 0;
        handle_input(KEY_A, 0);
    }
    CHECK(mode == ENDING);
}

static unsigned packed_pixel(const uint16_t *data, unsigned i) {
    return (data[i/2] >> ((i & 1) * 8)) & 255;
}

static void check_title_pixels(bool unlocked, unsigned selection) {
    graphics_title(unlocked, selection);
    graphics_present();
    const uint16_t *visible = (const uint16_t *)((REG_DISPCNT & BACKBUFFER) ? 0x0600A000 : 0x06000000);
    bool matches = true;
    for (unsigned i = 0; i < 38400; ++i) {
        unsigned expected = packed_pixel(vn_title_backgrounds[unlocked], i);
        int x = i % 240 - VN_MENU_X, y = i / 240 - VN_MENU_Y;
        if (x >= 0 && x < VN_BUTTON_W && y >= 0) {
            unsigned row = y / VN_MENU_SPACING, by = y % VN_MENU_SPACING;
            if (row < 7 && row >= (unlocked ? 0u : 1u) && by < VN_BUTTON_H) {
                unsigned color = packed_pixel(vn_title_buttons[row][row == selection], by*VN_BUTTON_W+x);
                if (color) expected = color;
            }
        }
        if (packed_pixel(visible, i) != expected) matches = false;
    }
    CHECK(matches);
    matches = true;
    for (unsigned i = 0; i < 256; ++i) if (BG_COLORS[i] != vn_title_palette[i]) matches = false;
    CHECK(matches);
}

int main(void) {
    irqInit();
    irqEnable(IRQ_VBLANK);
    reset_game();
    CHECK(!save_true_unlocked(&save_data));
    title_selection = MENU_CONTINUE;
    handle_input(KEY_UP, 0);
    CHECK(title_selection == MENU_EXIT);
    handle_input(KEY_DOWN, 0);
    CHECK(title_selection == MENU_CONTINUE);
    for (unsigned i = 0; i < 12; ++i) {
        handle_input(KEY_DOWN, 0);
        CHECK(title_selection != MENU_TRUE);
    }
    title_selection = MENU_TRUE;
    activate_title();
    CHECK(mode == TITLE && !save_data.resume.valid);
    title_selection = MENU_CONTINUE;
    handle_input(KEY_A, 0);
    CHECK(mode == MESSAGE && message_text == UI_NO_RESUME);
    handle_input(KEY_B, 0);
    CHECK(mode == TITLE);
    title_selection = MENU_LOAD;
    handle_input(KEY_A, 0);
    CHECK(mode == LOAD && !save_slot(&save_data,0)->valid);
    handle_input(KEY_B, 0);

    title_selection = MENU_START;
    handle_input(KEY_A, 0);
    CHECK(mode == READING && active_node == VN_START);
    handle_input(KEY_A, 0);
    CHECK(mode == READING && active_node == VN_START && revealed == total_chars);
    handle_input(KEY_START, 0);
    CHECK(mode == PAUSE && revealed == total_chars && sub_selection == 0);
    handle_input(KEY_UP, 0);
    CHECK(sub_selection == 6);
    handle_input(KEY_UP, 0);
    CHECK(sub_selection == 5);
    bool old_auto = autoplay;
    wait_frames = 100;
    handle_input(KEY_A, 0);
    CHECK(mode == PAUSE && autoplay != old_auto && wait_frames == 0);
    handle_input(KEY_A, 0);
    CHECK(mode == PAUSE && autoplay == old_auto);
    handle_input(KEY_DOWN, 0);
    handle_input(KEY_DOWN, 0);
    CHECK(sub_selection == 0);
    handle_input(KEY_A, 0);
    CHECK(mode == READING && revealed == total_chars);
    handle_input(KEY_L, 0);
    CHECK(mode == MESSAGE && message_text == UI_SAVED);
    handle_input(KEY_A, 0); /* Dismiss save notification. */
    handle_input(KEY_A, 0);
    CHECK(active_node != VN_START);
    handle_input(KEY_R, 0);
    CHECK(mode == LOAD);
    handle_input(KEY_B, 0);
    CHECK(mode == READING && save_data.manual.valid && save_data.manual.node == VN_START);
    handle_input(KEY_A, 0); /* Reveal the new node before checking pause/resume. */
    handle_input(KEY_START, 0);
    CHECK(mode == PAUSE);
    handle_input(KEY_B, 0);
    CHECK(mode == READING && revealed == total_chars);

    finish_route(0);
    CHECK(save_data.endings == 1 && save_true_unlocked(&save_data));
    SaveData reloaded;
    save_load(&reloaded);
    CHECK(reloaded.endings == 1 && reloaded.resume.ending);
    CHECK(reloaded.manual.valid && reloaded.manual.node == VN_START);
    handle_input(KEY_A, 0);
    title_selection = MENU_CONTINUE;
    handle_input(KEY_UP, 0);
    CHECK(title_selection == MENU_TRUE);
    handle_input(KEY_A, 0);
    CHECK(mode == READING && active_node == VN_TRUE_START);
    finish_route(0);
    CHECK(save_data.endings == 5);
    handle_input(KEY_A, 0);
    title_selection = MENU_START;
    handle_input(KEY_A, 0);
    finish_route(1);
    CHECK(save_data.endings == 7);

    handle_input(KEY_A, 0);
    Bookmark resume_before = save_data.resume;
    title_selection = MENU_EXTRA;
    handle_input(KEY_A, 0);
    CHECK(mode == EXTRA);
    handle_input(KEY_A, 0);
    CHECK(mode == ENDING && replay);
    handle_input(KEY_A, 0);
    CHECK(mode == TITLE && !replay);
    CHECK(save_data.resume.node == resume_before.node && save_data.resume.revealed == resume_before.revealed);

    title_selection = MENU_SYSTEM;
    handle_input(KEY_A, 0);
    CHECK(mode == SYSTEM && sub_selection == 0);
    handle_input(KEY_RIGHT, 0);
    CHECK(save_data.text_speed == 2);
    handle_input(KEY_DOWN, 0);
    handle_input(KEY_A, 0);
    CHECK(save_data.auto_default);
    handle_input(KEY_B, 0);
    save_load(&reloaded);
    CHECK(reloaded.text_speed == 2 && reloaded.auto_default && reloaded.endings == 7);
    mode = SYSTEM; sub_selection = 2;
    handle_input(KEY_LEFT, 0); CHECK(save_data.music_volume == 5 && save_data.voice_volume == 10);
    handle_input(KEY_DOWN, 0); handle_input(KEY_A, 0); CHECK(!save_data.music_enabled);
    handle_input(KEY_DOWN, 0); handle_input(KEY_LEFT, 0); CHECK(save_data.voice_volume == 9);
    handle_input(KEY_DOWN, 0); handle_input(KEY_A, 0); CHECK(!save_data.voice_enabled);
    save_load(&reloaded);
    CHECK(reloaded.music_volume == 5 && reloaded.voice_volume == 9 && !reloaded.music_enabled && !reloaded.voice_enabled);
    sub_selection = 2;
    for (unsigned i=0; i<20; ++i) handle_input(KEY_LEFT, 0);
    CHECK(save_data.music_volume == 0);
    for (unsigned i=0; i<20; ++i) handle_input(KEY_RIGHT, 0);
    CHECK(save_data.music_volume == 10);
    sub_selection = 6; handle_input(KEY_A,0); CHECK(mode == TITLE);
    title_selection = MENU_START;
    handle_input(KEY_A, 0);
    CHECK(autoplay && save_data.endings == 7 && save_data.manual.valid);
    handle_input(KEY_START, 0);
    CHECK(mode == PAUSE);
    sub_selection = 6;
    handle_input(KEY_A, 0);
    CHECK(mode == EXITED);
    handle_input(KEY_START, 0);
    CHECK(mode == TITLE);

    /* Variables are captured at save time and node effects are not re-applied on load. */
    reset_game(); title_selection=MENU_START; activate_title();
    int16_t vars[STORY_VARIABLES]; story_get(vars); CHECK(vars[0]==0 && vars[1]==0);
    while(active_node!=3) {revealed=total_chars; handle_input(KEY_A,0);}
    revealed=total_chars; handle_input(KEY_A,0); story_get(vars);
    CHECK(vars[0]==1 && vars[1]==2);
    handle_input(KEY_START,0); sub_selection=1;handle_input(KEY_A,0);CHECK(mode==SAVE);
    handle_input(KEY_DOWN,0);CHECK(slot_selection==1);handle_input(KEY_A,0);CHECK(mode==MESSAGE);
    CHECK(save_slot(&save_data,1)->valid && save_slot(&save_data,1)->variables[0]==1);
    handle_input(KEY_B,0);CHECK(mode==PAUSE);sub_selection=1;handle_input(KEY_A,0);handle_input(KEY_DOWN,0);handle_input(KEY_A,0);CHECK(mode==CONFIRM);
    handle_input(KEY_B,0);CHECK(mode==SAVE);handle_input(KEY_A,0);CHECK(mode==CONFIRM);handle_input(KEY_A,0);CHECK(mode==MESSAGE);
    handle_input(KEY_B,0);CHECK(mode==PAUSE);handle_input(KEY_B,0);CHECK(mode==READING);
    restore(save_slot(&save_data,1));story_get(vars);CHECK(vars[0]==1 && vars[1]==2);
    CHECK(story_history_count()==0);
    revealed=total_chars;handle_input(KEY_START,0);sub_selection=3;handle_input(KEY_A,0);CHECK(mode==HISTORY && story_history_count()==1);
    handle_input(KEY_B,0);CHECK(mode==PAUSE && sub_selection==3);handle_input(KEY_B,0);
    int before=active_node;handle_input(KEY_UP,0);CHECK(dialogue_hidden);handle_input(KEY_A,0);CHECK(!dialogue_hidden && active_node==before);
    vars[0]=0;story_set(vars);CHECK(story_next(5)==7);vars[0]=1;story_set(vars);CHECK(story_next(5)==6);
    /* New game resets variables; read flags survive and held skip does not cross a choice. */
    title();title_selection=MENU_START;activate_title();story_get(vars);CHECK(vars[0]==0 && vars[1]==0);
    CHECK(save_is_read(&save_data,0));revealed=total_chars;handle_input(0,KEY_B);CHECK(active_node!=0);
    CHECK(story_history_count()>0);
    save_mark_read(&save_data,STORY_NODE_LIMIT-1);
    CHECK(save_is_read(&save_data,STORY_NODE_LIMIT-1));
    for(unsigned i=0;i<STORY_HISTORY+10;++i)story_history_push(i%VN_NODE_COUNT);
    CHECK(story_history_count()==STORY_HISTORY && story_history_node(0)==(STORY_HISTORY+9)%VN_NODE_COUNT);
    save_load(&reloaded);CHECK(save_slot(&reloaded,1)->valid && save_slot(&reloaded,1)->variables[0]==1);
    /* Conditional choices and saturating arithmetic use the isolated fixture. */
    vars[0]=-1;story_set(vars);CHECK(!story_choice_visible(3,0) && story_choice_visible(3,1));
    enter_node(3);CHECK(selected_choice==1);revealed=total_chars;handle_input(KEY_UP,0);CHECK(selected_choice==1);
    vars[0]=32767;story_set(vars);story_apply_choice(3,0);story_get(vars);CHECK(vars[0]==32767);
    /* Per-node wait gates manual advance, including after a restored node. */
    replay=false;save_data.endings=7;autoplay=false;enter_node(15);revealed=total_chars;
    handle_input(KEY_A,0);CHECK(active_node==15);node_frames=30;handle_input(KEY_A,0);CHECK(active_node==16);
    /* Navigation is bounded by the history ring and the settings return cursor. */
    story_history_clear();story_history_push(0);story_history_push(1);mode=HISTORY;history_offset=0;
    handle_input(KEY_UP,0);CHECK(history_offset==1);handle_input(KEY_UP,0);CHECK(history_offset==1);
    handle_input(KEY_DOWN,0);CHECK(history_offset==0);
    system_return=PAUSE;mode=SYSTEM;sub_selection=6;handle_input(KEY_A,0);CHECK(mode==PAUSE && sub_selection==4);
    /* A damaged latest journal record falls back to the previous complete write. */
    save_data.music_volume=3;save_write(&save_data);save_data.music_volume=7;save_write(&save_data);
    volatile uint8_t *journal=(volatile uint8_t *)0x0E000000;
    unsigned latest=journal[40]>journal[SAVE_JOURNAL_STRIDE+40]?0:SAVE_JOURNAL_STRIDE; journal[latest+18]^=1;
    save_load(&reloaded);CHECK(reloaded.music_volume==3);
    save_data.music_volume=6;save_write(&save_data);

    uint8_t record[SAVE_RECORD_SIZE];
    save_encode(&save_data, record);
    record[8] = 2; record[18] = record[19] = record[26] = 0;
    uint32_t legacy_sum = 2166136261u;
    for (unsigned i=0; i<28; ++i) legacy_sum = (legacy_sum ^ record[i]) * 16777619u;
    for (unsigned i=0; i<4; ++i) record[28+i] = legacy_sum >> (i*8);
    CHECK(save_decode(&reloaded,record));
    CHECK(reloaded.endings == save_data.endings && reloaded.manual.node == save_data.manual.node);
    CHECK(reloaded.music_enabled && reloaded.voice_enabled && reloaded.music_volume == 6 && reloaded.voice_volume == 10);
    record[8]=3;record[18]=4;record[19]=8;record[26]=1;
    legacy_sum=2166136261u;for(unsigned i=0;i<28;++i)legacy_sum=(legacy_sum^record[i])*16777619u;
    for(unsigned i=0;i<4;++i)record[28+i]=legacy_sum>>(i*8);
    CHECK(save_decode(&reloaded,record));CHECK(reloaded.music_volume==4 && reloaded.voice_volume==8 && reloaded.music_enabled && !reloaded.voice_enabled);
    save_encode(&save_data,record);record[32]^=1;
    legacy_sum=2166136261u;for(unsigned i=0;i<SAVE_RECORD_SIZE-4;++i)legacy_sum=(legacy_sum^record[i])*16777619u;
    for(unsigned i=0;i<4;++i)record[SAVE_RECORD_SIZE-4+i]=legacy_sum>>(i*8);
    CHECK(save_decode(&reloaded,record) && !reloaded.resume.valid && !reloaded.manual.valid && reloaded.endings==save_data.endings);
    save_encode(&save_data, record);
    CHECK(save_decode(&reloaded, record));
    CHECK(save_is_read(&reloaded,STORY_NODE_LIMIT-1));
    /* Check every field byte, a spread of read bits, and all checksum bytes.
       A 4 KiB journal need not rerun a full checksum for every padding byte. */
    for (unsigned i = 0; i < SAVE_RECORD_SIZE; ++i) {
        if(i>=256 && i<SAVE_RECORD_SIZE-4 && i%127)continue;
        record[i] ^= 1;
        CHECK(!save_decode(&reloaded, record));
        CHECK(!save_true_unlocked(&reloaded) && !reloaded.resume.valid && !reloaded.manual.valid);
        record[i] ^= 1;
    }
    memset(record, 0xff, sizeof(record));
    CHECK(!save_decode(&reloaded, record) && !save_true_unlocked(&reloaded));
    memset(record, 0, sizeof(record));
    CHECK(!save_decode(&reloaded, record) && !save_true_unlocked(&reloaded));
    SaveData invalid = save_data;
    invalid.resume.node = VN_NODE_COUNT;
    save_encode(&invalid, record);
    CHECK(!save_decode(&reloaded, record));
    invalid = save_data;
    invalid.resume.revealed = 65535;
    save_encode(&invalid, record);
    CHECK(!save_decode(&reloaded, record));
    invalid = save_data;
    invalid.resume.choice = 2;
    save_encode(&invalid, record);
    CHECK(!save_decode(&reloaded, record));
    invalid = save_data;
    invalid.endings = 0;
    invalid.resume.node = VN_TRUE_START;
    save_encode(&invalid, record);
    CHECK(!save_decode(&reloaded, record));

    reset_game();
    title_selection = MENU_EXTRA;
    handle_input(KEY_A, 0);
    handle_input(KEY_A, 0);
    CHECK(mode == EXTRA && !save_true_unlocked(&save_data));
    handle_input(KEY_B, 0);
    title_selection = MENU_START;
    handle_input(KEY_A, 0);
    finish_route(1);
    CHECK(save_data.endings == 2 && save_true_unlocked(&save_data));

    graphics_init();
    check_title_pixels(false, MENU_START);
    check_title_pixels(false, MENU_EXIT);
    check_title_pixels(true, MENU_TRUE);
    check_title_pixels(true, MENU_CONTINUE);
    graphics_scene(0, false);
    graphics_present();
    const uint16_t *visible = (const uint16_t *)((REG_DISPCNT & BACKBUFFER) ? 0x0600A000 : 0x06000000);
    CHECK(memcmp(visible, vn_backgrounds[0], 38400) == 0);
    bool palette_matches = true;
    for (unsigned i = 0; i < 256; ++i) if (BG_COLORS[i] != vn_palette[i]) palette_matches = false;
    CHECK(palette_matches);
    check_title_pixels(true, MENU_SYSTEM);

    /* Check optimized pair writes against the original per-pixel geometry and lookup. */
    const int boxes[][4] = {{8,103,224,54},{8,87,32,14},{38,12,164,136},{25,29,190,54},{9,31,73,49}};
    for (unsigned b = 0; b < sizeof(boxes)/sizeof(boxes[0]); ++b) {
        graphics_scene(0, false);
        graphics_panel(boxes[b][0], boxes[b][1], boxes[b][2], boxes[b][3]);
        graphics_present();
        visible = (const uint16_t *)((REG_DISPCNT & BACKBUFFER) ? 0x0600A000 : 0x06000000);
        bool same = true;
        for (int y = 0; y < 160; ++y) for (int x = 0; x < 240; ++x) {
            int cx = x-boxes[b][0], cy = y-boxes[b][1];
            unsigned old = packed_pixel(vn_backgrounds[0], y*240+x), expected = old;
            if (cx >= 0 && cy >= 0 && cx < boxes[b][2] && cy < boxes[b][3]) {
                int w = boxes[b][2], h = boxes[b][3];
                int dx = cx < 5 ? 5-cx : cx >= w-5 ? cx-(w-6) : 0;
                int dy = cy < 5 ? 5-cy : cy >= h-5 ? cy-(h-6) : 0;
                if (dx*dx+dy*dy <= 25) {
                    int ix=cx-1, iy=cy-1;
                    dx = ix < 4 ? 4-ix : ix >= w-6 ? ix-(w-7) : 0;
                    dy = iy < 4 ? 4-iy : iy >= h-6 ? iy-(h-7) : 0;
                    bool inner = ix>=0 && iy>=0 && ix<w-2 && iy<h-2 && dx*dx+dy*dy<=16;
                    expected = inner ? vn_panel_blend[old] : BORDER;
                }
            }
            if (packed_pixel(visible,y*240+x) != expected) same = false;
        }
        CHECK(same);
    }
    /* Motion endpoints, phase ordering, no-op transitions and runtime integration. */
    unsigned pose_id=vn_story[16].character;
    const PortraitAsset *pose=&vn_portraits[pose_id];
    CHECK(pose->x==13 && pose->y==21 && pose->w==64 && pose->h==120);
    bool mirrored=true;
    for(unsigned y=0;y<120;++y)for(unsigned x=0;x<64;++x)
        if(packed_pixel(pose->pixels,y*64+x)!=packed_pixel(vn_portraits[1].pixels,y*64+63-x))mirrored=false;
    CHECK(mirrored);
    /* Verify all displayed pixels, including the mask, odd x and nonzero y. */
    graphics_scene(0,pose_id);graphics_present();
    visible=(const uint16_t *)((REG_DISPCNT&BACKBUFFER)?0x0600A000:0x06000000);
    bool placed=true;
    for(int y=0;y<160;++y)for(int x=0;x<240;++x) {
        unsigned expected=packed_pixel(vn_backgrounds[0],y*240+x);
        int sx=x-13,sy=y-21;
        if(sx>=0 && sx<64 && sy>=0 && sy<120) {
            unsigned color=packed_pixel(vn_portraits[1].pixels,sy*64+63-sx);
            if(color)expected=color;
        }
        if(packed_pixel(visible,y*240+x)!=expected)placed=false;
    }
    CHECK(placed);
    enter_node(16);CHECK(visual_portrait==pose_id);
    restore(&save_data.resume);CHECK(visual_portrait==pose_id && !animation_active());
    Transition motion={BG_FADE,BG_FADE,PORTRAIT_LEFT,PORTRAIT_RIGHT,4};
    AnimationFrame frame;
    animation_start(true,0,1,1,1,&motion);
    frame=animation_frame();CHECK(frame.scene==0 && frame.portrait==1 && frame.offset_x==0);
    for(unsigned i=0;i<4;++i)animation_tick();
    frame=animation_frame();CHECK(frame.offset_x==240-(int)vn_portraits[1].x);
    animation_tick();frame=animation_frame();CHECK(frame.scene==0 && frame.portrait==0 && frame.darkness==0);
    for(unsigned i=0;i<4;++i)animation_tick();
    CHECK(animation_frame().darkness==16);
    animation_tick();frame=animation_frame();CHECK(frame.scene==1 && frame.portrait==0 && frame.darkness==16);
    for(unsigned i=0;i<4;++i)animation_tick();
    CHECK(animation_frame().darkness==0);
    animation_tick();frame=animation_frame();CHECK(frame.portrait==1 && frame.offset_x==-(int)(vn_portraits[1].x+vn_portraits[1].w));
    for(unsigned i=0;i<4;++i)animation_tick();
    CHECK(animation_frame().offset_x==0);
    animation_tick();CHECK(!animation_active());
    animation_start(true,0,1,0,1,&motion);CHECK(!animation_active());
    animation_start(true,0,1,0,pose_id,&motion);CHECK(animation_active());
    for(unsigned i=0;i<5;++i)animation_tick();
    frame=animation_frame();CHECK(frame.portrait==pose_id && frame.offset_x==-77);
    animation_finish();
    animation_start(false,0,0,0,1,&motion);CHECK(animation_frame().darkness==16);
    animation_finish();CHECK(!animation_active());
    /* Exit a portrait without changing the background. */
    animation_start(true,0,1,0,0,&motion);
    for(unsigned i=0;i<5;++i)animation_tick();
    CHECK(!animation_active() && animation_frame().portrait==0);
    /* Fully clipped endpoints preserve every background pixel. */
    graphics_animated_scene(0,1,-(int)(vn_portraits[1].x+vn_portraits[1].w),0);
    graphics_present();
    visible=(const uint16_t *)((REG_DISPCNT&BACKBUFFER)?0x0600A000:0x06000000);
    CHECK(memcmp(visible,vn_backgrounds[0],38400)==0);
    graphics_animated_scene(0,1,240-vn_portraits[1].x,16);graphics_present();
    visible=(const uint16_t *)((REG_DISPCNT&BACKBUFFER)?0x0600A000:0x06000000);
    /* BLDY is write-only on GBA: inspect the readable blend control register. */
    CHECK(memcmp(visible,vn_backgrounds[0],38400)==0 && REG_BLDCNT==((1<<2)|(1<<5)|(3<<6)));
    graphics_scene(0,1);graphics_present();CHECK(REG_BLDCNT==0);
    /* Odd offsets must preserve transparent pixels and match a reference blit. */
    const int offsets[]={-191,-37,1,27,81};
    for(unsigned k=0;k<sizeof(offsets)/sizeof(offsets[0]);++k) {
        const PortraitAsset *p=&vn_portraits[1];
        graphics_animated_scene(0,1,offsets[k],0);graphics_present();
        visible=(const uint16_t *)((REG_DISPCNT&BACKBUFFER)?0x0600A000:0x06000000);
        bool correct=true;
        for(int y=0;y<160;++y)for(int x=0;x<240;++x) {
            unsigned expected=packed_pixel(vn_backgrounds[0],y*240+x);
            int sx=x-(int)p->x-offsets[k],sy=y-(int)p->y;
            if(sx>=0 && sx<p->w && sy>=0 && sy<p->h) {
                unsigned color=packed_pixel(p->pixels,sy*p->w+sx);if(color)expected=color;
            }
            if(packed_pixel(visible,y*240+x)!=expected)correct=false;
        }
        CHECK(correct);
    }
    visual_valid=true;visual_scene=0;visual_portrait=1;enter_node(17);
    CHECK(animation_active() && revealed==0);
    handle_input(0,0);CHECK(revealed==0 && node_frames==0);
    handle_input(KEY_A,0);CHECK(!animation_active() && active_node==17 && revealed==0);
    restore(&save_data.resume);CHECK(!animation_active());
    visual_valid=false;enter_node(17);handle_input(KEY_START,0);
    CHECK(mode==PAUSE && !animation_active());handle_input(KEY_B,0);CHECK(mode==READING);
    visual_valid=false;enter_node(17);
    unsigned timeout=0;while(animation_active() && timeout++<100)handle_input(0,0);
    CHECK(!animation_active() && timeout<100 && revealed==0);
    title();CHECK(!visual_valid && !animation_active());
    /* A fully visible odd-offset sample portrait fits a 280,896-cycle GBA frame. */
    graphics_animated_scene(0,1,1,0);
    REG_TM0CNT=0;REG_TM1CNT=0;
    REG_TM1CNT_H=TIMER_START|TIMER_COUNT;REG_TM0CNT_H=TIMER_START;
    graphics_animated_scene(0,1,1,0);
    REG_TM0CNT_H=0;
    unsigned animation_cycles=REG_TM0CNT_L|(REG_TM1CNT_L<<16);
    REG_TM1CNT_H=0;
    CHECK(animation_cycles>0 && animation_cycles<280896);
    active_node = VN_START; revealed = total_chars = graphics_text_length(vn_story[VN_START].text);
    mode = READING; draw(); /* Warm the composition cache. */
    REG_TM0CNT = 0; REG_TM1CNT = 0;
    REG_TM1CNT_H = TIMER_START | TIMER_COUNT;
    REG_TM0CNT_H = TIMER_START;
    draw();
    REG_TM0CNT_H = 0;
    unsigned draw_cycles = REG_TM0CNT_L | (REG_TM1CNT_L << 16);
    REG_TM1CNT_H = 0;
    /* Leave a recognizable result in the test ROM's own SRAM for automated inspection. */
    volatile uint8_t *sram = (volatile uint8_t *)0x0E000000;
    const char passed[] = "VN_TEST_PASS";
    for (unsigned i = 0; i < sizeof(passed); ++i) sram[64+i] = passed[i];
    sram[80] = checks;
    sram[81] = checks >> 8;
    for (unsigned i=0; i<4; ++i) sram[84+i] = draw_cycles >> (i*8);
    for (unsigned i=0; i<4; ++i) sram[92+i] = animation_cycles >> (i*8);
    consoleDemoInit();
    iprintf("\x1b[2JALL TESTS PASSED\n%u checks\n\nTitle navigation\nBoth normal endings\nTrue ending unlock\nContinue / manual load\nSRAM persistence\nCorrupt save rejection\nReplay preserves resume\nSystem settings / exit", checks);
    while (true) VBlankIntrWait();
}
