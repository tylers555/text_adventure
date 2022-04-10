
//~ TA IDs
internal inline ta_id 
MakeTAID(u64 ID){
    ta_id Result;
    Result.ID = ID;
    return Result;
}

internal inline ta_id 
MakeTAID(string S){
    ta_id Result;
    Result.ID = S.ID;
    return Result;
}

#if defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)
internal inline ta_id
TAItemByName(ta_system *TA, const char *S){
    ta_id ItemID = HashTableFind(&TA->ItemNameTable, S);
    return ItemID;
}

internal inline ta_id
TAIDByName(ta_system *TA, const char *S){
    ta_id ItemID = {};
    return ItemID;
}
#else
internal inline ta_id
TAIDByName(ta_system *TA, const char *S){
    ta_id Result = MakeTAID(Strings.GetString(S));
    return Result;
}

internal inline ta_id
TAItemByName(ta_system *TA, const char *S){
    ta_id ItemID = TAIDByName(TA, S);
    return ItemID;
}
#endif

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
TAContinueFindItems(ta_system *TA, array<ta_id> *Items, char **Words, u32 WordCount, ta_found_items *Founds){
    for(u32 ItemIndex=0; ItemIndex<Items->Count; ItemIndex++){
        ta_item *Item = HashTableFindPtr(&TA->ItemTable, ArrayGet(Items, ItemIndex));
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
                    }else if(Weight == Found->Weight){
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
TAFindItems(ta_system *TA, array<ta_id> *Items, char **Words, u32 WordCount){
    ta_found_items Result = {};
    Result.Items = MakeDynamicArray<ta_found_item>(&TransientStorageArena, 2);
    TAContinueFindItems(TA, Items, Words, WordCount, &Result);
    
    return Result;
}

internal inline s32
TAFindItemByTag(ta_system *TA, array<ta_id> *Items, asset_tag Tag){
    for(u32 J=0; J<Items->Count; J++){
        ta_item *Item = HashTableFindPtr(&TA->ItemTable, ArrayGet(Items, J));
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
TARoomAddItem(ta_system *TA, asset_system *Assets, ta_room *Room, ta_id Item){
    if(!Room->Items){
        memory_arena *Arena = &Assets->Memory;
        Room->Items = MakeArray<ta_id>(Arena, TA_ROOM_DEFAULT_ITEM_COUNT);
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

internal void 
TAUnlock(audio_mixer *Mixer, asset_system *Assets, ta_room *Room, asset_tag *Locked){
    Room->Dirty = true;
    *Locked = AssetTag();
    Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_open_door)));
}

internal b8
TAAttemptToUnlock(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, ta_room *Room, asset_tag *Locked){
    for(u32 J=0; J<TA->Inventory.Count; J++){
        ta_item *Item = HashTableFindPtr(&TA->ItemTable, ArrayGet(&TA->Inventory, J));
        if(!Item) continue;
        if(HasTag(Item->Tag, AssetTag_Key)){
            if(HasTag(*Locked, AssetTag_Organ) && HasTag(Item->Tag, AssetTag_Organ)){
                TAUnlock(Mixer, Assets, Room, Locked);
                return true;
            }else if(HasTag(*Locked, AssetTag_BellTower) && HasTag(Item->Tag, AssetTag_BellTower)){
                TAUnlock(Mixer, Assets, Room, Locked);
                return true;
            }
        }
    }
    
    return false;
}

internal b8
TAIsClosed(ta_system *TA, asset_tag Tag){
    if(HasTag(Tag, AssetTag_OpenDawn)){
        NOT_IMPLEMENTED_YET;
    }else if(HasTag(Tag, AssetTag_OpenNoon)){
        NOT_IMPLEMENTED_YET;
    }else if(HasTag(Tag, AssetTag_OpenDusk)){
        NOT_IMPLEMENTED_YET;
    }else if(HasTag(Tag, AssetTag_OpenNight)){
        NOT_IMPLEMENTED_YET;
    }
    
    return false;
}

internal inline ta_area
MakeTAArea(ta_id Name, v2 Offset){
    ta_area Result = {};;
    Result.Name = Name;
    Result.Offset = Offset;
    return Result;
}

internal inline constexpr b8
operator==(ta_id A, ta_id B){
    b8 Result = (A.ID == B.ID);
    return Result;
}

internal constexpr u64
HashKey(ta_id Value) {
    u64 Result = Value.ID;
    return(Result);
}

internal constexpr b32
CompareKeys(ta_id A, ta_id B){
    b32 Result = (A == B);
    return(Result);
}


//~ Theme
internal inline console_theme
MakeDefaultConsoleTheme(){
    console_theme Result = {};
    Result.BasicFont = AssetID(font_basic);
    Result.TitleFont = AssetID(font_basic_bold);
    Result.BackgroundColor = MakeColor(0x0a0d4aff);
    Result.CursorColor     = MakeColor(0xf2f2f2ff);
    Result.SelectionColor = MakeColor(0x232c8cff);
    Result.BasicFancy     = MakeFancyFormat(MakeColor(0xf2f2f2ff), 0.0, 0.0, 0.0);
    Result.RoomTitleFancy = MakeFancyFormat(MakeColor(0xff6969ff), 1.0, 4.0, 2.0);
    Result.ItemFancy      = MakeFancyFormat(MakeColor(0x24e3e3ff), 0.0, 0.0, 0.0);
    Result.RoomFancy      = MakeFancyFormat(MakeColor(0xff6969ff), 0.0, 0.0, 0.0);
    Result.DirectionFancy = MakeFancyFormat(MakeColor(0x5eeb82ff), 0.0, 0.0, 0.0);
    Result.MiscFancy      = MakeFancyFormat(MakeColor(0xf5d756ff), 0.0, 0.0, 0.0);
    Result.MoodFancy      = MakeFancyFormat(MakeColor(0x59ab8aff), 0.0, 0.0, 0.0);
    Result.ResponseFancies[0] = MakeFancyFormat(MakeColor(0x9063ffff), 0.0, 0.0, 0.0);
    Result.ResponseFancies[1] = MakeFancyFormat(MakeColor(0xe64eccff), MakeColor(0x9063ffff), 0.0, 0.0, 0.0, 2.0, 0.2f, 0.0);
    
    Result.DescriptionFancies[0]= Result.BasicFancy;
    Result.DescriptionFancies[1] = Result.DirectionFancy;
    Result.DescriptionFancies[2] = Result.RoomFancy; 
    Result.DescriptionFancies[3] = Result.ItemFancy;
    Result.DescriptionFancies[4] = Result.MiscFancy;
    Result.DescriptionFancies[5] = Result.MoodFancy;
    
    
    return Result;
}

//~ 
inline b8
ta_system::AddItem(ta_id Item){
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
