#include <genesis.h>

#include "../graphics.h"

#define kPALETTE_INDEX_BACKGROUND          0
#define kPALETTE_INDEX_BLUE_BUILDINGS      1
#define kPALETTE_INDEX_RED_BUILDINGS       2
#define kPALETTE_INDEX_GUI                 3

#define kMAX_BASE_NEIGHBORS                4
#define kMAX_MAP_BASES                     2

extern const u8 test_map_tiles[64*64];
extern const s16 test_map_buildings[64*64];

typedef enum game_movement_state {
    kGAME_MOVEMENT_STATE_SCROLL,
    kGAME_MOVEMENT_STATE_SELECT,
} TGAME_MOVEMENT_STATE;

struct vector {
    s16 x;
    s16 y;
};

struct __attribute__((packed)) base {
    struct vector position;
    struct base *neighbors[kMAX_BASE_NEIGHBORS];
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
static TGAME_MOVEMENT_STATE movement_state = kGAME_MOVEMENT_STATE_SELECT;
static u8 previous_pad_state;
static Sprite *selection_marker[4];
static struct vector selection_marker_position;
static struct base map_bases[kMAX_MAP_BASES];
static u8 num_map_bases = sizeof(map_bases) / sizeof(struct base);

static inline void initSystem(void)
{
    SYS_disableInts();
    SPR_init(0,0,0);
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    VDP_setPlanSize(64, 64);
    VDP_setTextPlan(PLAN_WINDOW);
    SYS_enableInts();
}


static inline void drawBackground(void)
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

            VDP_setTileMapXY(PLAN_B, TILE_ATTR_FULL(PAL0 + kPALETTE_INDEX_BLUE_BUILDINGS,TRUE,FALSE,FALSE,
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


static inline void changeSelection(u8 pad_state)
{

}

static inline void hideSelectioMarker(void)
{
    SPR_setVisibility(selection_marker[0], HIDDEN);
    SPR_setVisibility(selection_marker[1], HIDDEN);
    SPR_setVisibility(selection_marker[2], HIDDEN);
    SPR_setVisibility(selection_marker[3], HIDDEN);
}

static inline void showSelectioMarker(void)
{
    SPR_setVisibility(selection_marker[0], VISIBLE);
    SPR_setVisibility(selection_marker[1], VISIBLE);
    SPR_setVisibility(selection_marker[2], VISIBLE);
    SPR_setVisibility(selection_marker[3], VISIBLE);
}

static void switchMovementState(void)
{
    if (movement_state == kGAME_MOVEMENT_STATE_SCROLL) {
        movement_state = kGAME_MOVEMENT_STATE_SELECT;
        showSelectioMarker();
    } else {
        movement_state = kGAME_MOVEMENT_STATE_SCROLL;
        hideSelectioMarker();
    }
}


static inline void initSelectionMarker(void)
{
    selection_marker_position.x = map_bases[0].position.x;
    selection_marker_position.y = map_bases[0].position.y;

    selection_marker[0] = SPR_addSprite(&selection_marker_spr, selection_marker_position.x, selection_marker_position.y, TILE_ATTR(PAL0 + kPALETTE_INDEX_GUI, TRUE, FALSE, FALSE));
    selection_marker[1] = SPR_addSprite(&selection_marker_spr, selection_marker_position.x+64, selection_marker_position.y, TILE_ATTR(PAL0 + kPALETTE_INDEX_GUI, TRUE, FALSE, TRUE));
    selection_marker[2] = SPR_addSprite(&selection_marker_spr, selection_marker_position.x+64, selection_marker_position.y+64, TILE_ATTR(PAL0 + kPALETTE_INDEX_GUI, TRUE, TRUE, TRUE));
    selection_marker[3] = SPR_addSprite(&selection_marker_spr, selection_marker_position.x, selection_marker_position.y+64, TILE_ATTR(PAL0 + kPALETTE_INDEX_GUI, TRUE, TRUE, FALSE));
}


static inline void initMapBases(void)
{
    map_bases[0].position.x = 8;
    map_bases[0].position.y = 16;
    map_bases[0].neighbors[0] = &map_bases[1];

    map_bases[1].position.x = 216;
    map_bases[1].position.y = 40;
    map_bases[1].neighbors[0] = &map_bases[0];
}


static inline void initPalettes(void)
{
    VDP_setPalette(PAL0 + kPALETTE_INDEX_BACKGROUND, background_pal.data);
    VDP_setPalette(PAL0 + kPALETTE_INDEX_BLUE_BUILDINGS, blue_pal.data);
    VDP_setPalette(PAL0 + kPALETTE_INDEX_GUI, gui_pal.data);
}




int main()
{
    u8 pad_state;

    initSystem();
    initPalettes();
    initMapBases();
    initSelectionMarker();
    drawBackground();

    while (1) {
        pad_state = JOY_readJoypad(JOY_1);

        if (movement_state == kGAME_MOVEMENT_STATE_SCROLL) {
            scrollMap(pad_state);
        }

        if (pad_state & BUTTON_C && !(previous_pad_state & BUTTON_C)) {
            switchMovementState();
        }

        SPR_update();
        VDP_waitVSync();

        previous_pad_state = pad_state;
    }
}

