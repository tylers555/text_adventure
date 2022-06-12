#ifndef TEXT_ADVENTURE_H
#define TEXT_ADVENTURE_H

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

typedef u32 room_flags;
enum room_flags_ {
    RoomFlag_None     = (0 << 0),
    RoomFlag_Dirty    = (1 << 0),
    RoomFlag_HadGhost = (1 << 1),
};

struct ta_room {
    room_flags Flags;
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
    
    ta_id Ghost;
};

struct ta_system;
typedef b8 command_func(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount);

struct ta_system {
    hash_table<ta_id, console_theme> ThemeTable;
    
    hash_table<ta_id, ta_room> RoomTable;
    hash_table<ta_id, ta_item> ItemTable;
    
    // Used for processed assets
    hash_table<const char *, ta_id> ItemNameTable;
    
    array<ta_id> Inventory;
    
    ta_room *CurrentRoom;
    
    memory_arena *Memory;
    
    string_builder ResponseBuilder;
    command_func *Callback;
    union {
        u32 BuyItemIndex;
    };
    
    void Initialize(asset_system *Assets, memory_arena *Arena);
    inline b8 AddItem(ta_id Item);
    
    inline void ClearResponse();
    inline void Respond(const char *Format, ...);
    
    memory_arena CommandMemory;
    stack<const char *> CommandStack;
    array<char[DEFAULT_BUFFER_SIZE]> EditingCommands;
    u32 CurrentPeekedCommand;
    inline void SaveCommand(const char *Command);
    
    //~ Game specific data
    asset_tag_id OrganState;
    array<entity_ghost> Ghosts;
    array<ta_id> HauntedItems;
    pray_state PrayState;
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
DIRECTION("northward",     Direction_North) \
DIRECTION("northeastward", Direction_NorthEast) \
DIRECTION("eastward",      Direction_East) \
DIRECTION("southeastward", Direction_SouthEast) \
DIRECTION("southward",     Direction_South) \
DIRECTION("southwestward", Direction_SouthWest) \
DIRECTION("westward",      Direction_West) \
DIRECTION("northwestward", Direction_NorthWest) \
DIRECTION("upward",        Direction_Up) \
DIRECTION("downward",      Direction_Down) \

#define POSITIVES \
WORD("y") \
WORD("yes") \
WORD("yeah") \
WORD("yep") \
WORD("sure") \
WORD("absolutely") \
WORD("ok") \
WORD("nay") \
WORD("positive") \

#define NEGATIVES \
WORD("n") \
WORD("no") \
WORD("nah") \
WORD("nay") \
WORD("nope") \
WORD("nada") \
WORD("negative")

global_constant f32 WORD_MATCH_THRESHOLD = 0.5f;

internal inline void 
TADispatchCommand(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Tokens, u32 TokenCount);

#endif //TEXT_ADVENTURE_H