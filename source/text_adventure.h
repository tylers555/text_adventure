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

//~
typedef array<char *> word_array;

struct ta_editing_command_node {
    ta_editing_command_node *Next;
    ta_editing_command_node *Prev;
    text_input_context Context;
};

struct ta_system;
typedef b8 command_func(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words);

struct ta_system {
    hash_table<ta_id, console_theme> ThemeTable;
    
    hash_table<ta_id, ta_room> RoomTable;
    hash_table<ta_id, ta_item> ItemTable;
    
    // Used for processed assets
    hash_table<const char *, ta_id> ItemNameTable;
    
    array<ta_id> Inventory;
    ta_map Map;
    
    ta_room *CurrentRoom;
    
    memory_arena *Memory;
    
    string_builder ResponseBuilder;
    command_func *Callback;
    union {
        u32 BuyItemIndex;
    };
    
    void Initialize(asset_system *Assets, memory_arena *Arena);
    inline b8 InventoryAddItem(ta_id Item);
    inline b8 InventoryRemoveItem(u32 Index);
    inline b8 InventoryRemoveItemByID(ta_id ID);
    
    inline ta_room *FindRoom(ta_id Room);
    inline b8 RoomAddItem(ta_room *Room, ta_id Item);
    inline b8 RoomDropItem(asset_system *Assets, ta_room *Room, ta_id Item);
    inline b8 RoomRemoveItem(ta_room *Room, u32 Index);
    inline b8 RoomRemoveItemByID(ta_room *Room, ta_id ID);
    
    inline ta_item *FindItem(ta_id Item);
    
    inline ta_data *FindData(array<ta_data *> *Datas, ta_data_type Type, asset_tag Tag);
    inline ta_data *FindDescription(array<ta_data *> *Descriptions, asset_tag Tag);
    void Unlock(audio_mixer *Mixer, asset_system *Assets, ta_room *Room, asset_tag *Locked);
    b8 AttemptToUnlock(audio_mixer *Mixer, asset_system *Assets, ta_room *Room, asset_tag *Locked);
    b8 IsClosed(asset_tag Tag);
    inline void Respond(const char *Format, ...);
    
    memory_arena CommandMemory;
    stack<const char *> CommandStack;
    ta_editing_command_node EditingCommandSentinel;
    ta_editing_command_node *CurrentEditingCommand;
    u32 CurrentPeekedCommand;
    
    inline array<char *> EndCommand();
    inline void DispatchCommand(audio_mixer *Mixer, asset_system *Assets, word_array Tokens);
    
    inline ta_editing_command_node *AllocEditingCommand();
    void EditingCommandCycleUp(os_input *Input);
    void EditingCommandCycleDown(os_input *Input);
    
    //~ Game specific data
    asset_tag_id OrganState;
    array<entity_ghost> Ghosts;
    array<ta_id> HauntedItems;
    pray_state PrayState;
    u32 Money;
    murkwell_event Event;
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

#endif //TEXT_ADVENTURE_H
