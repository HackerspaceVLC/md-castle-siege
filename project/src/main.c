#include <genesis.h>

#include "../graphics.h"

#define kPALETTE_INDEX_BACKGROUND          0
#define kPALETTE_INDEX_BLUE_BUILDINGS      1


extern const u8 test_map_tiles[64*64];
extern const s8 test_map_buildings[64*64];

struct vector {
    s16 x;
    s16 y;
};

static const struct vector scroll_by_direction[0xF] = {
    [BUTTON_UP] = {
        .x = 0,
        .y = -2,
    },
    [BUTTON_DOWN] = {
        .x = 0,
        .y = 2,
    },
    [BUTTON_UP | BUTTON_DOWN] = {
        .x = 0,
        .y = 0,
    },
    [BUTTON_LEFT] = {
        .x = 2,
        .y = 0,
    },
    [BUTTON_RIGHT] = {
        .x = -2,
        .y = 0,
    },
    [BUTTON_LEFT | BUTTON_RIGHT] = {
        .x = 0,
        .y = 0,
    },
    [BUTTON_DOWN | BUTTON_LEFT] = {
        .x = 1,
        .y = 1,
    },
    [BUTTON_DOWN | BUTTON_RIGHT] = {
        .x = -1,
        .y = 1,
    },
    [BUTTON_UP | BUTTON_RIGHT] = {
        .x = -1,
        .y = -1,
    },
    [BUTTON_UP | BUTTON_LEFT] = {
        .x = 1,
        .y = -1,
    },
};

static struct vector scroll_position;


static inline void initSystem(void)
{
    SYS_disableInts();
    SPR_init(0,0,0);
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    VDP_setPlanSize(64, 64);
    SYS_enableInts();
}


static void drawBackground(void)
{
    u8 i, j;
    s16 current;

    SYS_disableInts();

    VDP_loadTileSet(&grass_tileset, TILE_USERINDEX, TRUE);

    for (i=0; i<64; ++i) {
        for (j=0; j<64; ++j) {
            VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(PAL0 + kPALETTE_INDEX_BACKGROUND,FALSE,FALSE,FALSE,
                TILE_USERINDEX + test_map_tiles[i+64*j]),
                i, j);
        }
    }

    VDP_loadTileSet(&blue_tileset, TILE_USERINDEX + grass_tileset.numTile, TRUE);

    for (i=0; i<64; ++i) {
        for (j=0; j<64; ++j) {
            current = test_map_buildings[i+64*j];

            if (current == -1) {
                continue;
            }

            VDP_setTileMapXY(PLAN_B, TILE_ATTR_FULL(PAL0 + kPALETTE_INDEX_BLUE_BUILDINGS,FALSE,FALSE,FALSE,
                TILE_USERINDEX + grass_tileset.numTile + current),
                i, j);
        }
    }

    SYS_enableInts();
}


static inline void scrollMap(u8 pad_state)
{
    struct vector scroll;

    scroll = scroll_by_direction[pad_state & 0xF];
    scroll_position.x += scroll.x;
    scroll_position.y += scroll.y;
        
    VDP_setHorizontalScroll(PLAN_A, scroll_position.x);
    VDP_setVerticalScroll(PLAN_A, scroll_position.y);
    VDP_setHorizontalScroll(PLAN_B, scroll_position.x);
    VDP_setVerticalScroll(PLAN_B, scroll_position.y);
}


int main()
{
    u8 pad_state;

    initSystem();

    VDP_setPalette(PAL0 + kPALETTE_INDEX_BACKGROUND, background_pal.data);
    VDP_setPalette(PAL0 + kPALETTE_INDEX_BLUE_BUILDINGS, blue_pal.data);

    drawBackground();

    while (1) {
        pad_state = JOY_readJoypad(JOY_1);

        scrollMap(pad_state);

        SPR_update();
        VDP_waitVSync();
    }
}

