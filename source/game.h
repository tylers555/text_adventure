#ifndef GAME_H
#define GAME_H

global_constant os_key_code PAUSE_KEY = KeyCode_Escape;

//~ Theme
global_constant color BASE_BACKGROUND_COLOR = MakeColor(0x0a0d4aff);
global_constant color BASIC_COLOR      = MakeColor(0xf2f2f2ff);
global_constant color ROOM_TITLE_COLOR = MakeColor(0xff6969ff);
global_constant color ROOM_COLOR       = MakeColor(0xffe369ff);
global_constant color DIRECTION_COLOR  = MakeColor(0x84d197ff);
global_constant color ITEM_COLOR       = MakeColor(0x24e3e3ff);

global_constant color RESPONSE_COLOR = MakeColor(0x9063ffff);
global_constant color EMPHASIS_COLOR = MakeColor(0xe06ecdff);

struct console_theme {
    asset_id BasicFont;
    asset_id TitleFont;
    
    color BackgroundColor;
    color CursorColor;
    color SelectionColor;
    
    fancy_font_format BasicFancy;
    fancy_font_format RoomTitleFancy;
    fancy_font_format ItemFancy;
    fancy_font_format RoomFancy;
    fancy_font_format DirectionFancy;
    fancy_font_format MiscFancy;
    fancy_font_format MoodFancy;
    fancy_font_format DescriptionFancies[6];
    fancy_font_format ResponseFancies[2];
};
internal inline console_theme MakeDefaultConsoleTheme();

//~ Text adventure stuff

global_constant u32 MAX_COMMAND_TOKENS = 64;
global_constant u32 TA_ROOM_DEFAULT_ITEM_COUNT = 8;
global_constant u32 INVENTORY_ITEM_COUNT = 10;

struct ta_area {
    ta_id Name;
    v2 Offset;
};

struct ta_map {
    render_texture Texture;
    v2 Size;
    array<ta_area> Areas;
};

struct ta_item {
    b8 Dirty;
    union{
        ta_name NameData;
        const char *Name;
    };
    asset_tag Tag;
    u32 Cost;
    array<ta_data *> Datas;
};

struct ta_room {
    b8 Dirty;
    union{
        ta_name NameData;
        const char *Name;
    };
    ta_id Area;
    asset_tag Tag;
    array<ta_data *> Datas;
    array<ta_id> Items;
    ta_id     Adjacents[Direction_TOTAL];
    asset_tag AdjacentTags[Direction_TOTAL];
};

struct ta_system;
typedef b8 command_func(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount);

struct ta_system {
    hash_table<ta_id, console_theme> ThemeTable;
    console_theme Theme;
    
    
    
    hash_table<ta_id, ta_room> RoomTable;
    hash_table<ta_id, ta_item> ItemTable;
    
    // Used for processed assets
    hash_table<const char *, ta_id> ItemNameTable;
    
    array<ta_id> Inventory;
    
    ta_id StartRoomID;
    const char *StartRoomName;
    ta_room *CurrentRoom;
    
    command_func *Callback;
    string_builder ResponseBuilder;
    
    void Initialize(memory_arena *Arena);
    inline b8 AddItem(ta_id Item);
    inline void ClearResponse();
    inline void Respond(const char *Format, ...);
    
    //~ Game specific data
    asset_tag_id OrganState;
    u32 Money;
    
    ta_map Map;
};

#define DIRECTIONS \
DIRECTION("north",     Direction_North) \
DIRECTION("northeast", Direction_NorthEast) \
DIRECTION("east",      Direction_East) \
DIRECTION("southeast", Direction_SouthEast) \
DIRECTION("south",     Direction_South) \
DIRECTION("southwest", Direction_SouthWest) \
DIRECTION("west",      Direction_West) \
DIRECTION("northwest", Direction_NorthWest) \
DIRECTION("up",        Direction_Up) \
DIRECTION("down",      Direction_Down) \
DIRECTION("n",  Direction_North) \
DIRECTION("ne", Direction_NorthEast) \
DIRECTION("e",  Direction_East) \
DIRECTION("se", Direction_SouthEast) \
DIRECTION("s",  Direction_South) \
DIRECTION("sw", Direction_SouthWest) \
DIRECTION("w",  Direction_West) \
DIRECTION("nw", Direction_NorthWest) \
DIRECTION("u",  Direction_Up) \
DIRECTION("d",  Direction_Down) \


internal inline void 
TADispatchCommand(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Tokens, u32 TokenCount);

#endif //GAME_H