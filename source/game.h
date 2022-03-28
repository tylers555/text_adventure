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
    string BasicFont;
    string TitleFont;
    
    color BackgroundColor;
    color CursorColor;
    color SelectionColor;
    
    fancy_font_format BasicFancy;
    fancy_font_format RoomTitleFancy;
    fancy_font_format ItemFancy;
    fancy_font_format RoomFancy;
    fancy_font_format DirectionFancy;
    fancy_font_format MiscFancy;
    fancy_font_format DescriptionFancies[5];
    fancy_font_format ResponseFancies[2];
};
internal inline console_theme MakeDefaultConsoleTheme();

//~ Text adventure stuff

global_constant u32 MAX_COMMAND_TOKENS = 64;
global_constant u32 TA_ROOM_DEFAULT_ITEM_COUNT = 8;
global_constant u32 INVENTORY_ITEM_COUNT = 10;

struct ta_string {
    asset_tag Tag;
    const char Data[];
};

struct ta_area {
    string Name;
    v2 Offset;
};

struct ta_map {
    render_texture Texture;
    v2 Size;
    array<ta_area> Areas;
};

struct ta_item {
    b8 Dirty;
    asset_tag Tag;
    u32 Cost;
    array<const char *> Aliases;
    array<const char *> Adjectives;
    array<ta_string *> Descriptions;
};

struct ta_room {
    b8 Dirty;
    const char *Name;
    string Area;
    asset_tag Tag;
    array<ta_string *> Descriptions;
    array<string> Items;
    string    Adjacents[Direction_TOTAL];
    asset_tag AdjacentTags[Direction_TOTAL];
};

struct ta_system;
typedef void command_func(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount);

struct ta_system {
    hash_table<string, console_theme> ThemeTable;
    console_theme Theme;
    
    hash_table<const char *, command_func *> CommandTable;
    hash_table<const char *, direction>      DirectionTable;
    
    hash_table<string, ta_room> RoomTable;
    hash_table<string, ta_item> ItemTable;
    
    array<string> Inventory;
    
    string StartRoom;
    ta_room *CurrentRoom;
    
    command_func *Callback;
    string_builder ResponseBuilder;
    
    void Initialize(memory_arena *Arena);
    inline b8 AddItem(string Item);
    inline void ClearResponse();
    inline void Respond(const char *Format, ...);
    
    //~ Game specific data
    asset_tag_id OrganState;
    u32 Money;
    
    ta_map Map;
};

#endif //GAME_H
