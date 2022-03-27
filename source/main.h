#if !defined(SNAIL_JUMPY_H)
#define SNAIL_JUMPY_H

// TODO(Tyler): Do this in build.bat
#define SNAIL_JUMPY_DEBUG_BUILD

#include "basics.h"
#include "math.h"
#include "intrinsics.h"
#include "generated.h"

//~ Constants TODO(Tyler): Several of these should be hotloaded in a variables file
global_constant u32 DEFAULT_BUFFER_SIZE = 512;

global_constant f32 MAXIMUM_SECONDS_PER_FRAME = (1.0f / 20.0f);
global_constant f32 MINIMUM_SECONDS_PER_FRAME = (1.0f / 60.0f);
global_constant f32 SECONDS_PER_TICK = (1.0f / 75.0f);

global_constant u32 PHYSICS_ITERATIONS_PER_OBJECT = 4;
global_constant f32 WALKABLE_STEEPNESS    = 0.1f;
global_constant u32 MAX_ENTITY_BOUNDARIES = 8;

global_constant f32 TILE_SIDE = 16;
global_constant v2  TILE_SIZE = V2(TILE_SIDE, TILE_SIDE);

global_constant char *ASSET_FILE_PATH = "assets.sja";
global_constant char *STARTUP_LEVEL = "Debug";

global_constant u32 MINIMUM_WINDOW_WIDTH  = 800;
global_constant u32 MINIMUM_WINDOW_HEIGHT = 600;
global_constant const char *WINDOW_NAME = "Toe Tac Tic";

//~ TODO(Tyler): Things that need a better place to go

enum direction {
    Direction_None,
    
    Direction_North,
    Direction_NorthEast,
    Direction_East,
    Direction_SouthEast,
    Direction_South,
    Direction_SouthWest,
    Direction_West,
    Direction_NorthWest,
    
    Direction_Up,
    Direction_Down,
    
    Direction_TOTAL,
};

//~ Enum to string tables
local_constant char *TRUE_FALSE_TABLE[2] = {
    "false",
    "true",
};

local_constant char *DIRECTION_TABLE[Direction_TOTAL] = {
    "Direction none",
    "Direction north",
    "Direction northeast",
    "Direction east",
    "Direction southeast",
    "Direction south",
    "Direction southwest",
    "Direction west",
    "Direction northwest",
    "Direction up",
    "Direction down"
};

//~ Miscallaneous
enum game_mode {
    GameMode_None,
    GameMode_Menu,
    GameMode_MainGame,
    GameMode_Map,
};

struct state_change_data {
    b8 DidChange;
    game_mode NewMode;
    const char *NewLevel;
};

//~ Includes
#include "random.h"
#include "helpers.cpp"
#include "os.h"
#include "memory_arena.cpp"
#include "array.cpp"
#include "stack.cpp"
#include "hash_table.cpp"
#include "strings.cpp"
#include "render.h"
#include "file_processing.h"
#include "asset.h" 
#include "audio_mixer.h"

#include "game.h"
#include "menu.h"

//~ Forward declarations
internal inline void ChangeState(game_mode NewMode, string NewLevel);

#endif