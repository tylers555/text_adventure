#ifndef TEXT_ADVENTURE_H
#define TEXT_ADVENTURE_H

//~ Text adventure stuff

global_constant u32 TA_ROOM_DEFAULT_ITEM_COUNT = 8;
global_constant u32 INVENTORY_ITEM_COUNT = 10;

struct ta_area {
    asset_id Name;
    v2 Offset;
};

struct ta_map {
    render_texture Texture;
    v2 Size;
    array<ta_area> Areas;
};

struct ta_item {
    asset_loading_data LoadingData;
    
    b8 IsDirty;
    
    asset_id ID;
    ta_name NameData;
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
    asset_loading_data LoadingData;
    
    asset_id ID;
    room_flags Flags;
    ta_name NameData;
    asset_id Area;
    asset_tag Tag;
    array<ta_data *> Datas;
    array<asset_id> Items;
    asset_id     Adjacents[Direction_TOTAL];
    asset_tag AdjacentTags[Direction_TOTAL];
    
    asset_id Ghost;
};

//~
typedef array<char *> word_array;

struct ta_editing_command_node {
    ta_editing_command_node *Next;
    ta_editing_command_node *Prev;
    text_input_context Context;
};

struct ta_system;
typedef b8 ta_command_func(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words);

struct ta_system {
    asset_table(Room, ta_room);
    asset_table(Item, ta_item);
    
    // Used for processed assets
    hash_table<const char *, asset_id> ItemNameTable;
    
    array<asset_id> Inventory;
    ta_map Map;
    
    ta_room *CurrentRoom;
    
    memory_arena *Memory;
    
    string_builder ResponseBuilder;
    ta_command_func *Callback;
    union {
        u32 BuyItemIndex;
    };
    
    inline b8 MaybeMarkItemDirty(asset_id ItemID);
    
    void Initialize(asset_system *Assets, memory_arena *Arena, void *Data=0, u32 DataSize=0);
    inline b8 InventoryAddItem(asset_id Item);
    inline b8 InventoryRemoveItem(u32 Index);
    inline b8 InventoryRemoveItemByID(asset_id ID);
    inline b8 InventoryHasItem(asset_id ID);
    
    inline ta_room *FindRoom(asset_id Room);
    inline b8 RoomAddItem(ta_room *Room, asset_id Item);
    inline b8 RoomDropItem(asset_system *Assets, ta_room *Room, asset_id Item);
    inline b8 RoomRemoveItem(ta_room *Room, u32 Index);
    inline b8 RoomRemoveItemByID(ta_room *Room, asset_id ID);
    inline b8 RoomHasItem(ta_room *Room, asset_id ID);
    inline b8 RoomEnsureItem(ta_room *Room, asset_id ID);
    
    inline ta_item *FindItem(asset_id Item);
    
    inline ta_data *FindData(array<ta_data *> *Datas, ta_data_type Type, asset_tag Tag);
    inline ta_data *FindDescription(array<ta_data *> *Descriptions, asset_tag Tag);
    void Unlock(audio_mixer *Mixer, asset_system *Assets, ta_room *Room, asset_tag *Locked);
    b8 AttemptToUnlock(audio_mixer *Mixer, asset_system *Assets, ta_room *Room, asset_tag *Locked);
    b8 IsClosed(asset_tag Tag);
    inline void Respond(const char *Format, ...);
    
    //~ Command
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
    
    //~ Debug
    inline b8 CheckAndLogItemID(asset_id ItemID);
    inline b8 CheckAndLogRoomID(asset_id RoomID);
    
    //~ Game specific data
    array<entity_ghost> Ghosts;
    array<asset_id> HauntedItems;
    pray_status PrayStatus;
    u32 Money;
    murkwell_event Event;
    
    //- Quests
    asset_tag_id OrganState;
    carillon_pages_state CarillonPages;
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
WORD("yay") \
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
