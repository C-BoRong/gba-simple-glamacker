#!/usr/bin/env bash
set -euo pipefail
export DEVKITPRO="$(cygpath -u "$DEVKITPRO")"
export DEVKITARM="$DEVKITPRO/devkitARM"
mkdir -p generated
"$DEVKITARM/bin/arm-none-eabi-gcc" -std=gnu11 -O2 -Wall -Wextra \
    -mcpu=arm7tdmi -mthumb -mthumb-interwork -g \
    -Iinclude -Igenerated/game-test/generated -I"$DEVKITPRO/libgba/include" \
    scripts/test_game.c source/save.c source/story.c source/animation.c generated/game-test/generated/vn_script.c source/graphics.c source/audio.c generated/game-test/generated/vn_audio.c generated/game-test/generated/vn_assets.c \
    -L"$DEVKITPRO/libgba/lib" -specs=gba.specs -lmm -lgba -o generated/test-game.elf
"$DEVKITARM/bin/arm-none-eabi-objcopy" -O binary generated/test-game.elf generated/test-game.gba
"$DEVKITPRO/tools/bin/gbafix" generated/test-game.gba -tVNTEST
