#if !defined(SNAIL_JUMPY_H)
#define SNAIL_JUMPY_H

// TODO(Tyler): Implement an allocator for the stb libraries
#define STB_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "third_party/stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "third_party/stb_truetype.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "third_party/stb_sprintf.h"

#include "tyler_basics.h"
#include "generated_asset_id.h"

//~ Constants TODO(Tyler): Several of these should be hotloaded in a variables file
global_constant u32 DEFAULT_BUFFER_SIZE = 512;

global_constant f32 MAXIMUM_SECONDS_PER_FRAME = (1.0f / 20.0f);
global_constant f32 MINIMUM_SECONDS_PER_FRAME = (1.0f / 60.0f);

global_constant char *ASSET_FILE_PATH = "assets.sja";

global_constant u32 MINIMUM_WINDOW_WIDTH  = 800;
global_constant u32 MINIMUM_WINDOW_HEIGHT = 600;
global_constant const char *WINDOW_NAME = "Murkwell";
global_constant const char *NORMAL_WINDOW_ICON_PATH = "other_data/map_colored.ico";
global_constant const char *SMALL_WINDOW_ICON_PATH = "other_data/map_colored.ico";

global_constant s32 AUDIO_SAMPLES_PER_SECOND = 48000;

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

//~ Includes
#include "random.h"
#include "helpers.cpp"
#include "os.h"
#include "strings.cpp"
#include "file_processing.h"
#include "render.h"
#include "asset.h" 
#include "audio_mixer.h"

#include "game.h"
#include "text_adventure.h"

//~ 
struct game_state {
    asset_system Assets;
    game_renderer Renderer;
    audio_mixer Mixer;
    ta_system TextAdventure;
    os_input Input;
};

struct settings_state {
    sound_handle MusicHandle;
};

#endif