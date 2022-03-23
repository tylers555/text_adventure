
//~ Command processing
internal char **
TokenizeCommand(memory_arena *Arena, const char *Command, u32 *TokenCount){
    char **Result = PushArray(Arena, char *, MAX_COMMAND_TOKENS);
    u32 Count = 0;
    
    u32 CommandLength = CStringLength(Command);
    
    u32 CurrentIndex = 0;
    while(CurrentIndex < CommandLength){
        while(StopSeeking(Command[CurrentIndex])) CurrentIndex++;
        
        u32 Next = SeekForward(Command, CommandLength, CurrentIndex);
        u32 WordLength = Next-CurrentIndex;
        Assert(Count < MAX_COMMAND_TOKENS);
        Result[Count] = PushArray(Arena, char, WordLength+1);
        CopyMemory(Result[Count], &Command[CurrentIndex], WordLength);
        Result[Count][WordLength] = 0;
        Count++;
        
        CurrentIndex = Next;
    }
    
    *TokenCount = Count;
    return Result;
}

internal inline b8
CompareWords(const char *A, const char *B){
    while(*A && *B){
        if(*A != *B){
            if(CharToLower(*A) != CharToLower(*B)) return false;
        }
        A++;
        B++;
    }
    if(*A != *B) return false;
    
    return true;
}

internal inline b8
TAItemsHaveSameAliases(ta_item *A, ta_item *B){
    for(u32 I=0; I<A->Aliases.Count; I++){
        for(u32 J=0; J<B->Aliases.Count; J++){
            if(CompareWords(A->Aliases[I], B->Aliases[J])) return true;
        }
    }
    
    return false;
}

#if 0

internal void
TAContinueFindItem(ta_system *TA, array<string> *Items, char **Words, u32 WordCount, ta_found_item *Found){
    for(u32 ItemIndex=0; ItemIndex<Items->Count; ItemIndex++){
        ta_item *Item = FindInHashTablePtr(&TA->ItemTable, ArrayGet(Items, ItemIndex));
        if(!Item) continue;
        
        s32 FoundAliasIndex = -1;
        s32 Weight = 0;
        for(u32 WordIndex=0; WordIndex<WordCount; WordIndex++){
            const char *Word = Words[WordIndex];
            
            b8 JustFoundAdjective = false;
            for(u32 AdjectiveIndex=0; AdjectiveIndex<Item->Adjectives.Count; AdjectiveIndex++){
                const char *Adjective = Item->Adjectives[AdjectiveIndex];
                if(CompareWords(Word, Adjective)){
                    JustFoundAdjective = true;
                    break;
                }
            }
            
            b8 JustFoundAlias = false;
            for(u32 AliasIndex=0; AliasIndex<Item->Aliases.Count; AliasIndex++){
                const char *Alias = Item->Aliases[AliasIndex];
                if(CompareWords(Word, Alias)){
                    FoundAliasIndex = AliasIndex;
                    JustFoundAlias = true;
                    break;
                }
            }
            
            if(JustFoundAlias  || JustFoundAdjective) Weight++;
            if(!JustFoundAlias && (FoundAliasIndex >= 0)) break;
        }
        
        if(FoundAliasIndex >= 0){
            if(Weight > Found->Weight){
                Found->Item = Item;
                Found->ItemIndex = ItemIndex;
                Found->WordIndex = FoundAliasIndex;
                Found->Weight = Weight;
                Found->IsAmbiguous = false;
            }else if(Weight == Found->Weight){
                Found->IsAmbiguous = true;
            }
        }
    }
}

internal ta_found_item
TAFindItem(ta_system *TA, array<string> *Items, char **Words, u32 WordCount){
    ta_found_item Result = {};
    Result.Weight = -1;
    TAContinueFindItem(TA, Items, Words, WordCount, &Result);
    
    return Result;
}
#endif

struct ta_found_item {
    ta_item *Item;
    u32 ItemIndex;
    s32 Weight;
    u32 WordIndex;
};

struct ta_found_items {
    dynamic_array<ta_found_item> Items;
    b8 IsAmbiguous;
};

internal void 
TAContinueFindItems(ta_system *TA, array<string> *Items, char **Words, u32 WordCount, ta_found_items *Founds){
    for(u32 ItemIndex=0; ItemIndex<Items->Count; ItemIndex++){
        ta_item *Item = FindInHashTablePtr(&TA->ItemTable, ArrayGet(Items, ItemIndex));
        if(!Item) continue;
        
        s32 FoundWordIndex = -1;
        s32 Weight = 0;
        for(u32 WordIndex=0; WordIndex<WordCount; WordIndex++){
            const char *Word = Words[WordIndex];
            
            b8 JustFoundAdjective = false;
            for(u32 AdjectiveIndex=0; AdjectiveIndex<Item->Adjectives.Count; AdjectiveIndex++){
                const char *Adjective = Item->Adjectives[AdjectiveIndex];
                if(CompareWords(Word, Adjective)){
                    JustFoundAdjective = true;
                    break;
                }
            }
            
            b8 JustFoundAlias = false;
            for(u32 AliasIndex=0; AliasIndex<Item->Aliases.Count; AliasIndex++){
                const char *Alias = Item->Aliases[AliasIndex];
                if(CompareWords(Word, Alias)){
                    FoundWordIndex = WordIndex;
                    JustFoundAlias = true;
                    break;
                }
            }
            
            if(JustFoundAlias  || JustFoundAdjective) Weight++;
            if(!JustFoundAlias && (FoundWordIndex >= 0)) break;
        }
        
        if(FoundWordIndex >= 0){
            b8 FoundSomething = false;
            for(u32 J=0; J<Founds->Items.Count; J++){
                ta_found_item *Found = &Founds->Items[J];
                if(Found->WordIndex == (u32)FoundWordIndex){
                    if(Weight > Found->Weight){
                        Found->Item = Item;
                        Found->ItemIndex = ItemIndex;
                        Found->WordIndex = FoundWordIndex;
                        Found->Weight = Weight;
                        FoundSomething = true;
                        break;
                    }else{
                        FoundSomething = true;
                        Founds->IsAmbiguous = true;
                        break;
                    }
                }
            }
            if(!FoundSomething){
                ta_found_item *Found = ArrayAlloc(&Founds->Items);
                Found->Item = Item;
                Found->ItemIndex = ItemIndex;
                Found->WordIndex = FoundWordIndex;
                Found->Weight = Weight;
            }
        }
    }
}

internal ta_found_items
TAFindItems(ta_system *TA, array<string> *Items, char **Words, u32 WordCount){
    ta_found_items Result = {};
    Result.Items = MakeDynamicArray<ta_found_item>(&TransientStorageArena, 2);
    TAContinueFindItems(TA, Items, Words, WordCount, &Result);
    
    return Result;
}

internal inline s32
TAFindItemByTag(ta_system *TA, array<string> *Items, asset_tag Tag){
    for(u32 J=0; J<Items->Count; J++){
        ta_item *Item = FindInHashTablePtr(&TA->ItemTable, ArrayGet(Items, J));
        if(!Item) continue;
        if(Item->Tag == Tag) return J;
    }
    
    return -1;
}

internal inline s32
TACalculateItemChoiceWeight(ta_item *Item, char **Words, u32 WordEnd){
    s32 Weight = 0;
    return Weight;
}


//~
internal inline b8
TARoomAddItem(ta_system *TA, ta_room *Room, string Item){
    if(!Room->Items){
        memory_arena *Arena = &AssetSystem.Memory;
        Room->Items = MakeArray<string>(Arena, TA_ROOM_DEFAULT_ITEM_COUNT);
    }
    
    b8 Result = ArrayMaybeAdd(&Room->Items, Item);
    if(!Result) TA->Respond("This room is much too \002\002small\002\001!");
    Room->Dirty = true;
    return Result;
    
}

internal inline void
TARoomRemoveItem(ta_system *TA, ta_room *Room, u32 Index){
    Room->Dirty = true;
    ArrayOrderedRemove(&Room->Items, Index);
}

internal inline ta_string *
TAFindDescription(array<ta_string *> *Descriptions, asset_tag Tag){
    ta_string *Result = 0;
    for(u32 I=0; I<Descriptions->Count; I++){
        ta_string *Description = ArrayGet(Descriptions, I);
        if(Description->Tag == Tag){
            Result = Description;
            break;
        }
    }
    
    return Result;
}

internal inline ta_string *
TARoomFindDescription(ta_room *Room, asset_tag Tag){
    ta_string *Result = TAFindDescription(&Room->Descriptions, Tag);
    return Result;
}

internal b8
TAAttemptToUnlock(ta_system *TA, ta_room *Room, asset_tag *Locked){
    for(u32 J=0; J<TA->Inventory.Count; J++){
        ta_item *Item = FindInHashTablePtr(&TA->ItemTable, ArrayGet(&TA->Inventory, J));
        if(!Item) continue;
        if(HasTag(Item->Tag, AssetTag_Key)){
            if(HasTag(*Locked, AssetTag_Organ) && HasTag(Item->Tag, AssetTag_Organ)){
                Room->Dirty = true;
                *Locked = AssetTag();
                return true;
            }
        }
    }
    
    return false;
}

internal b8
TAIsClosed(ta_system *TA, asset_tag Tag){
    if(HasTag(Tag, AssetTag_OpenDawn)){
        
    }else if(HasTag(Tag, AssetTag_OpenNoon)){
        
    }else if(HasTag(Tag, AssetTag_OpenDusk)){
        
    }else if(HasTag(Tag, AssetTag_OpenNight)){
        
    }
    
    return false;
}

//~ Commands
void CommandMove(ta_system *TA, char **Words, u32 WordCount){
    ta_room *CurrentRoom = TA->CurrentRoom;
    ta_room *NextRoom = 0;
    
    for(u32 I=0; I < WordCount; I++){
        char *Word = Words[I];
        direction Direction = FindInHashTable(&TA->DirectionTable, (const char *)Word);
        if(!Direction) continue;
        
        string NextRoomString = CurrentRoom->Adjacents[Direction];
        NextRoom = FindInHashTablePtr(&TA->RoomTable, NextRoomString);
        if(!NextRoom){
            TA->Respond("Why would you want move that way!?\nDoing so would be quite \002\002foolish\002\001!");
            return;
        }
        
        asset_tag Tag = CurrentRoom->AdjacentTags[Direction];
        if(HasTag(Tag, AssetTag_Locked)){
            if(TAAttemptToUnlock(TA, CurrentRoom, &Tag)){
                TA->Respond("(unlocked)");
                break;
            }else{
                TA->Respond("That way is locked, \002\002buddy-o\002\001!");
                return;
            }
        }else if(TAIsClosed(TA, Tag)){
            return;
        }
        
        break;
    }
    
    if(!NextRoom){
        TA->Respond("Don't you understand how to move!?\nYou need to specify a direction, \002\002pal\002\001!");
        return;
    }
    TA->CurrentRoom = NextRoom;
    
    AudioMixer.PlaySound(AssetSystem.GetSoundEffect(String("room_change")));
}

void CommandTake(ta_system *TA, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &Room->Items, Words, WordCount);
    if(FoundItems.IsAmbiguous){
        TA->Respond("You will have to be more specific, what item do you want to take?\nEnter the item: ");
        TA->Callback = CommandTake;
        return;
    }
    
    if(!FoundItems.Items.Count){
        TA->Respond("I have no idea what you want to buy!");
        return;
    }
    
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        if(HasTag(Item->Tag, AssetTag_Static)){
            TA->Respond("You couldn't possible hope to take that!");
            continue;
        }else if(Item->Cost > 0){
            TA->Respond("You're going to have to \002\002buy\002\001 for that!");
            continue;
        }
        
        if(TA->AddItem(Room->Items[Index])) TARoomRemoveItem(TA, Room, Index);
        
        ta_string *Description = TAFindDescription(&Item->Descriptions, AssetTag(AssetTag_Examine));
        if(!Description) return;
        TA->Respond(Description->Data);
        AudioMixer.PlaySound(AssetSystem.GetSoundEffect(String("item_taken")));
    }
}

void CommandDrop(ta_system *TA, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &Room->Items, Words, WordCount);
    if(FoundItems.IsAmbiguous){
        TA->Respond("You will have to be more specific, what item do you want to drop?\nEnter the item: ");
        TA->Callback = CommandDrop;
        return;
    }
    
    if(!FoundItems.Items.Count){
        TA->Respond("I have no idea what you want to buy!");
        return;
    }
    
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        if(TARoomAddItem(TA, Room, TA->Inventory[Index])) ArrayOrderedRemove(&TA->Inventory, Index);
        
    }
    AudioMixer.PlaySound(AssetSystem.GetSoundEffect(String("item_dropped")));
}

void CallbackConfirmBuy(ta_system *TA, char **Words, u32 WordCount){
    Assert(0);
}

void CommandBuy(ta_system *TA, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &Room->Items, Words, WordCount);
    if(FoundItems.IsAmbiguous){
        TA->Respond("You will have to be more specific, what item do you want to buy?\nEnter the item: ");
        TA->Callback = CommandBuy;
        return;
    }
    
    if(!FoundItems.Items.Count){
        TA->Respond("I have no idea what you want to buy!");
        return;
    }
    
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        if(HasTag(Item->Tag, AssetTag_Static)){
            TA->Respond("You couldn't possible hope to buy that!");
            continue;
        }
        if(TA->Money >= Item->Cost){
            if(TA->AddItem(Room->Items[Index])) TARoomRemoveItem(TA, Room, Index);
            else return;
            TA->Money -= Item->Cost;
            Item->Dirty = true;
            Item->Cost = 0;
        }else{
            TA->Respond("You don't have enough \002\002money\002\001 for that!");
            continue;
        }
        
        ta_string *Description = TAFindDescription(&Item->Descriptions, AssetTag(AssetTag_Examine));
        if(!Description) continue;
        TA->Respond(Description->Data);
    }
    
    AudioMixer.PlaySound(AssetSystem.GetSoundEffect(String("item_bought")));
}

void CommandEat(ta_system *TA, char **Words, u32 WordCount){
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words, WordCount);
    
    if(FoundItems.IsAmbiguous){
        TA->Respond("You will have to be more specific, what item do you want to eat?\nEnter the item: ");
        TA->Callback = CommandEat;
        return;
    }
    
    if(!FoundItems.Items.Count){
        TA->Respond("I have no idea what you want to buy!");
        return;
    }
    
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        ta_string *Description = TAFindDescription(&Item->Descriptions, AssetTag(AssetTag_Eat));
        if(!Description){
            TA->Respond("You can't eat \002\002that\002\001!");
            continue;
        }
        TA->Respond(Description->Data);
        
        if(HasTag(Item->Tag, AssetTag_Bread)){
            TA->AddItem(String("bread crumbs"));
        }
        
        ArrayOrderedRemove(&TA->Inventory, Index);
    }
    
    AudioMixer.PlaySound(AssetSystem.GetSoundEffect(String("item_eaten")));
}

void CommandPlay(ta_system *TA, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words, WordCount);
    TAContinueFindItems(TA, &Room->Items, Words, WordCount, &FoundItems);
    
    if(FoundItems.IsAmbiguous){
        TA->Respond("You will have to be more specific, what item do you want to play?\nEnter the item: ");
        TA->Callback = CommandPlay;
        return;
    }
    
    if(!FoundItems.Items.Count){
        TA->Respond("I have no idea what you want to buy!");
        return;
    }
    
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        
        asset_tag Tag = AssetTag(AssetTag_Play);
        if(HasTag(Item->Tag, AssetTag_Organ)){
            string Sound;
            if(TA->OrganState == AssetTag_Broken) Sound = String("organ_play_broken");
            else                                  Sound = String("organ_play_repaired");
            AudioMixer.PlaySound(AssetSystem.GetSoundEffect(Sound));
            Tag = AssetTag(AssetTag_Play, TA->OrganState);
        }
        
        ta_string *Description = TAFindDescription(&Item->Descriptions, Tag);
        if(!Description) continue;;
        TA->Respond(Description->Data);
    }
}

void CommandExamine(ta_system *TA, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words, WordCount);
    TAContinueFindItems(TA, &Room->Items, Words, WordCount, &FoundItems);
    
    if(FoundItems.IsAmbiguous){
        TA->Respond("You will have to be more specific, what item do you want to examine?\nEnter the item: ");
        TA->Callback = CommandExamine;
        return;
    }
    
    if(!FoundItems.Items.Count){
        TA->Respond("I have no idea what you want to buy!");
        return;
    }
    
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        
        asset_tag Tag = AssetTag(AssetTag_Examine);
        if(HasTag(Item->Tag, AssetTag_Organ)){
            Tag = AssetTag(AssetTag_Examine, TA->OrganState);
        }
        
        ta_string *Description = TAFindDescription(&Item->Descriptions, Tag);
        if(!Description) continue;
        TA->Respond(Description->Data);
    }
}

void CommandMap(ta_system *TA, char **Words, u32 WordCount){
    TA->Respond("Not implemented yet!");
    Assert(0);
}

void CommandUnlock(ta_system *TA, char **Words, u32 WordCount){
    ta_room *CurrentRoom = TA->CurrentRoom;
    for(u32 I=0; I<Direction_TOTAL; I++){
        asset_tag Tag = CurrentRoom->AdjacentTags[I];
        if(!HasTag(Tag, AssetTag_Locked)) continue;
        if(TAAttemptToUnlock(TA, CurrentRoom, &Tag)){
            TA->Respond("Unlocked!");
        }else{
            TA->Respond("you can't unlock that!");
        }
    }
}

//~ Testing commands
void CommandTestRepair(ta_system *TA, char **Words, u32 WordCount){
    if(TA->OrganState == AssetTag_Broken)        TA->OrganState = AssetTag_Repaired;
    else if(TA->OrganState == AssetTag_Repaired) TA->OrganState = AssetTag_Broken;
}

void CommandTestAddMoney(ta_system *TA, char **Words, u32 WordCount){
    TA->Money += 10;
}

void CommandTestSubMoney(ta_system *TA, char **Words, u32 WordCount){
    if(TA->Money >= 10) TA->Money -= 10;
}

//~ Text adventure system
void
ta_system::Initialize(memory_arena *Arena){
    CommandTable = PushHashTable<const char *, command_func *>(Arena, 64);
    InsertIntoHashTable(&CommandTable, "go",       CommandMove);
    InsertIntoHashTable(&CommandTable, "move",     CommandMove);
    InsertIntoHashTable(&CommandTable, "take",     CommandTake);
    InsertIntoHashTable(&CommandTable, "pick",     CommandTake);
    InsertIntoHashTable(&CommandTable, "grab",     CommandTake);
    InsertIntoHashTable(&CommandTable, "drop",     CommandDrop);
    InsertIntoHashTable(&CommandTable, "leave",    CommandDrop);
    InsertIntoHashTable(&CommandTable, "buy",      CommandBuy);
    InsertIntoHashTable(&CommandTable, "purchase", CommandBuy);
    InsertIntoHashTable(&CommandTable, "eat",      CommandEat);
    InsertIntoHashTable(&CommandTable, "consume",  CommandEat);
    InsertIntoHashTable(&CommandTable, "ingest",   CommandEat);
    InsertIntoHashTable(&CommandTable, "swallow",  CommandEat);
    InsertIntoHashTable(&CommandTable, "bite",     CommandEat);
    InsertIntoHashTable(&CommandTable, "munch",    CommandEat);
    InsertIntoHashTable(&CommandTable, "play",     CommandPlay);
    InsertIntoHashTable(&CommandTable, "examine",  CommandExamine);
    InsertIntoHashTable(&CommandTable, "inspect",  CommandExamine);
    InsertIntoHashTable(&CommandTable, "observe",  CommandExamine);
    InsertIntoHashTable(&CommandTable, "look",     CommandExamine);
    InsertIntoHashTable(&CommandTable, "map",      CommandMap);
    InsertIntoHashTable(&CommandTable, "unlock",   CommandUnlock);
    
    InsertIntoHashTable(&CommandTable, "testrepair",   CommandTestRepair);
    InsertIntoHashTable(&CommandTable, "testaddmoney", CommandTestAddMoney);
    InsertIntoHashTable(&CommandTable, "testsubmoney", CommandTestSubMoney);
    
    DirectionTable = PushHashTable<const char *, direction>(Arena, 2*Direction_TOTAL);
    InsertIntoHashTable(&DirectionTable, "north",     Direction_North);
    InsertIntoHashTable(&DirectionTable, "northeast", Direction_NorthEast);
    InsertIntoHashTable(&DirectionTable, "east",      Direction_East);
    InsertIntoHashTable(&DirectionTable, "southeast", Direction_SouthEast);
    InsertIntoHashTable(&DirectionTable, "south",     Direction_South);
    InsertIntoHashTable(&DirectionTable, "southwest", Direction_SouthWest);
    InsertIntoHashTable(&DirectionTable, "west",      Direction_West);
    InsertIntoHashTable(&DirectionTable, "northwest", Direction_NorthWest);
    InsertIntoHashTable(&DirectionTable, "up",        Direction_Up);
    InsertIntoHashTable(&DirectionTable, "down",      Direction_Down);
    InsertIntoHashTable(&DirectionTable, "n",  Direction_North);
    InsertIntoHashTable(&DirectionTable, "ne", Direction_NorthEast);
    InsertIntoHashTable(&DirectionTable, "e",  Direction_East);
    InsertIntoHashTable(&DirectionTable, "se", Direction_SouthEast);
    InsertIntoHashTable(&DirectionTable, "s",  Direction_South);
    InsertIntoHashTable(&DirectionTable, "sw", Direction_SouthWest);
    InsertIntoHashTable(&DirectionTable, "w",  Direction_West);
    InsertIntoHashTable(&DirectionTable, "nw", Direction_NorthWest);
    InsertIntoHashTable(&DirectionTable, "u",  Direction_Up);
    InsertIntoHashTable(&DirectionTable, "d",  Direction_Down);
    
    RoomTable = PushHashTable<string, ta_room>(Arena, 64);
    ItemTable = PushHashTable<string, ta_item>(Arena, 128);
    Inventory = MakeArray<string>(Arena, INVENTORY_ITEM_COUNT);
    ResponseBuilder = BeginStringBuilder(Arena, DEFAULT_BUFFER_SIZE);
    
    //~ Game specific data
    OrganState = AssetTag_Broken;
}

inline b8
ta_system::AddItem(string Item){
    b8 Result = ArrayMaybeAdd(&Inventory, Item);
    if(!Result) Respond("You are far too \002\002weak\002\001 to carry that many items!!!");
    return Result;
}

inline void
ta_system::ClearResponse(){
    ResponseBuilder.Buffer[0] = 0;
    ResponseBuilder.BufferSize = 0;
}

inline void 
ta_system::Respond(const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    StringBuilderVAdd(&ResponseBuilder, Format, VarArgs);
    StringBuilderAdd(&ResponseBuilder, '\n');
    va_end(VarArgs);
}

//~ 

internal void
UpdateAndRenderMainGame(game_renderer *Renderer){
    u64 Start = __rdtsc();
    
    if(!TextAdventure.CurrentRoom){
        OSInput.BeginTextInput();
        //TextAdventure.CurrentRoom = FindInHashTablePtr(&TextAdventure.RoomTable, String("Plaza SE"));
        TextAdventure.CurrentRoom = FindInHashTablePtr(&TextAdventure.RoomTable, String("Bakery"));
        TextAdventure.Money = 10;
    }
    
    //RenderTexture(Renderer, MakeRect(V2(0), V2(30)), 10.0, RED);
    asset_font *BoldFont = AssetSystem.GetFont(String("font_bold"));
    asset_font *Font = AssetSystem.GetFont(String("basic"));
    Assert(Font);
    ta_system *TA = &TextAdventure;
    
    v2 WindowSize = GameRenderer.ScreenToWorld(OSInput.WindowSize);
    
    //~ Room display
    ta_room *Room = TA->CurrentRoom;
    v2 StartRoomP = V2(10, WindowSize.Y-15);
    f32 RoomDescriptionWidth = WindowSize.X-150;
    v2 StartItemP = V2(StartRoomP.X+RoomDescriptionWidth+10, StartRoomP.Y);
    f32 ItemDescriptionWidth = WindowSize.X-20-RoomDescriptionWidth;
    f32 InventoryWidth = 100;
    f32 RoomTitleHeight = 0;
    
    {
        v2 RoomP = StartRoomP;
        
        RoomTitleHeight = FontRenderFancyString(BoldFont, &RoomTitleFancy, 1, RoomP, Room->Name);
        RoomP.Y -= RoomTitleHeight;
        
        ta_string *Description = Room->Descriptions[0];
        if(HasTag(Room->Tag, AssetTag_Organ)){
            ta_string *New = TARoomFindDescription(Room, AssetTag(TA->OrganState));
            if(New) Description = New;
        }
        
        RoomP.Y -= FontRenderFancyString(Font, DescriptionFancies, ArrayCount(DescriptionFancies), 
                                         RoomP, Description->Data, RoomDescriptionWidth);
        
        ta_string *Adjacents = TARoomFindDescription(Room, AssetTag(AssetTag_Adjacents));
        if(Adjacents){
            RoomP.Y -= FontRenderFancyString(Font, DescriptionFancies, ArrayCount(DescriptionFancies), 
                                             RoomP, Adjacents->Data, RoomDescriptionWidth);
        }
        
        
    }
    
    //~ Inventory
    {
        v2 ItemP = StartItemP;
        if(Room->Items.Count > 0){
            b8 HasNonStatic = false;
            for(u32 I=0; I<Room->Items.Count; I++){
                ta_item *Item = FindInHashTablePtr(&TA->ItemTable, Room->Items[I]);
                if(!HasTag(Item->Tag, AssetTag_Static)){
                    HasNonStatic = true; 
                    break;
                }
            }
            
            if(HasNonStatic){
                ItemP.Y -= RoomTitleHeight;
                
                ta_string *Items = TARoomFindDescription(Room, AssetTag(AssetTag_Items));
                if(Items){
                    ItemP.Y -= FontRenderFancyString(Font, DescriptionFancies, ArrayCount(DescriptionFancies), 
                                                     ItemP, Items->Data, ItemDescriptionWidth);
                }else{
                    ItemP.Y -= FontRenderFancyString(Font, DescriptionFancies, ArrayCount(DescriptionFancies), 
                                                     ItemP, "Items:", ItemDescriptionWidth);
                }
                
                for(u32 I=0; I<Room->Items.Count; I++){
                    ta_item *Item = FindInHashTablePtr(&TA->ItemTable, Room->Items[I]);
                    if(HasTag(Item->Tag, AssetTag_Static)) continue;
                    ItemP.Y -= FontRenderFancyString(Font, &ItemFancy, 1, 
                                                     ItemP, Strings.GetString(Room->Items[I]), ItemDescriptionWidth);
                }
                ItemP.Y -= 10;
            }
        }
        
        v2 InventoryP = ItemP;
        InventoryP.Y -= FontRenderFancyString(BoldFont, &BasicFancy, 1, InventoryP, "Inventory:");
        
        {
            char Buffer[DEFAULT_BUFFER_SIZE];
            stbsp_snprintf(Buffer, DEFAULT_BUFFER_SIZE, "%u coins", TA->Money);
            InventoryP.Y -= FontRenderFancyString(Font, &ItemFancy, 1, InventoryP, Buffer);
        }
        
        for(u32 I=0; I<TA->Inventory.Count; I++){
            const char *Item = Strings.GetString(TA->Inventory[I]);
            InventoryP.Y -= FontRenderFancyString(Font, &ItemFancy, 1, InventoryP, Item);
        }
    }
    
    //~ Text input rendering
    {
        
        f32 InputHeight = 50;
        f32 ResponseWidth = WindowSize.X-20;
        v2 InputP = V2(10, InputHeight);
        
        fancy_font_format ResponseFancies[2] = {ResponseFancy, EmphasisFancy};
        const char *Response = TA->ResponseBuilder.Buffer;
        InputP.Y -= FontRenderFancyString(Font, ResponseFancies, ArrayCount(ResponseFancies), InputP, 
                                          Response, ResponseWidth);
        
        char *Text = OSInput.Buffer;
        FontRenderFancyString(Font, &BasicFancy, 1, InputP, Text);
        
        f32 CursorHeight = Font->Height-Font->Descent;
        v2 CursorP = InputP+FontStringAdvance(Font, OSInput.CursorPosition, Text);
        if(((FrameCounter+30) / 30) % 3){
            RenderLine(&GameRenderer, CursorP, CursorP+V2(0, CursorHeight), 0.0, 1, WHITE);
        }
        
        //~ Selection
        if(OSInput.SelectionMark >= 0){
            v2 SelectionP = InputP+FontStringAdvance(Font, OSInput.SelectionMark, Text);
            f32 Width = SelectionP.X-CursorP.X;
            RenderRect(&GameRenderer, RectFix(SizeRect(V2(CursorP.X, CursorP.Y-1), V2(Width, Font->Height+2))), 1.0, BLUE);
        }
        
        //~ Command processing
        if(OSInput.MaybeEndTextInput()){
            TA->ClearResponse();
            u32 TokenCount;
            char **Tokens = TokenizeCommand(&TransientStorageArena, OSInput.Buffer, &TokenCount);
            if(TA->Callback){
                // NOTE(Tyler): Weird dance, so that the callback can set another callback.
                command_func *Callback = TA->Callback;
                TA->Callback = 0;
                (*Callback)(TA, Tokens, TokenCount);
                
            }else{
                command_func *Func = 0;
                for(u32 I=0; I < TokenCount; I++){
                    char *Word = Tokens[I];
                    CStringMakeLower(Word);
                    Func = FindInHashTable(&TA->CommandTable, (const char *)Word);
                    if(Func) break;
                }
                if(Func){
                    (*Func)(TA, &Tokens[1], TokenCount-1);
                }else{
                    TA->Respond("That is not a valid command!\n\002\002You fool\002\001!!!");
                }
            }
            OSInput.BeginTextInput();
        }
    }
    
    //~ Debug
    {
        v2 DebugP = V2(10, 10);
        {
            u64 Elapsed = __rdtsc() - Start;
            char Buffer[DEFAULT_BUFFER_SIZE];
            stbsp_snprintf(Buffer, DEFAULT_BUFFER_SIZE, "%08llu | FPS: %.2f |", 
                           Elapsed, 1.0/OSInput.dTime);
            DebugP.Y -= FontRenderFancyString(Font, &BasicFancy, 1, DebugP, Buffer);
        }
    }
}
