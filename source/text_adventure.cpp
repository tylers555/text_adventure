
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
    
    Result.DescriptionFancies[0] = Result.BasicFancy;
    Result.DescriptionFancies[1] = Result.DirectionFancy;
    Result.DescriptionFancies[2] = Result.RoomFancy; 
    Result.DescriptionFancies[3] = Result.ItemFancy;
    Result.DescriptionFancies[4] = Result.MiscFancy;
    Result.DescriptionFancies[5] = Result.MoodFancy;
    
    
    return Result;
}

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

internal inline constexpr b8
operator==(ta_id A, ta_id B){
    b8 Result = (A.ID == B.ID);
    return Result;
}

internal inline constexpr b8
operator!=(ta_id A, ta_id B){
    b8 Result = (A.ID != B.ID);
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
internal array<char *>
TokenizeCommand(memory_arena *Memory, const char *Command){
    dynamic_array<char *> Array = MakeDynamicArray<char *>(Memory, 10);
    
    u32 CommandLength = CStringLength(Command);
    
    u32 CurrentIndex = 0;
    while(CurrentIndex < CommandLength){
        while(StopSeeking(Command[CurrentIndex])) CurrentIndex++;
        
        u32 Next = SeekForward(Command, CommandLength, CurrentIndex).End;
        u32 WordLength = Next-CurrentIndex;
        if(WordLength > 0){
            char *Token = PushArray(Memory, char, WordLength+1);
            CopyMemory(Token, &Command[CurrentIndex], WordLength);
            ArrayAdd(&Array, Token);
        }
        
        CurrentIndex = Next;
    }
    
    array<char *> Result = MakeArray(&Array);
    return Result;
}

internal inline f32
CompareWordsPercentage(const char *A, const char *B){
#if 0
    while(*A && *B){
        if(*A != *B){
            if(CharToLower(*A) != CharToLower(*B)) return false;
        }
        A++;
        B++;
    }
    if(*A != *B) return false;
#endif
    
    // Wagner–Fischer algorithm
    // From: https://en.wikipedia.org/wiki/Levenshtein_distance
    u32 M = CStringLength(A);
    u32 N = CStringLength(B);
    
    s32 *V0 = PushArray(&GlobalTransientMemory, s32, N+1);
    s32 *V1 = PushArray(&GlobalTransientMemory, s32, N+1);
    
    for(u32 I=0; I<=N; I++) V0[I] = I;
    
    for(u32 I=0; I<=M-1; I++){
        V1[0] = I+1;
        
        for(u32 J=0; J<=N-1; J++){
            u32 DeletionCost = V0[J+1] + 1;
            u32 InsertionCost = V1[J] + 1;
            u32 SubstitutionCost = (CharToLower(A[I]) == CharToLower(B[J])) ? V0[J] : (V0[J] + 1);
            V1[J+1] = Minimum(Minimum(DeletionCost, InsertionCost), SubstitutionCost);
        }
        
        Swap(V0, V1);
    }
    
    f32 Result = 1.0f - ((f32)V0[N] / (f32)Maximum(M, N));
    return Result;
}

internal inline b8
CompareWords(const char *A, const char *B){
    b8 Result = CompareWordsPercentage(A, B) > WORD_MATCH_THRESHOLD;
    return Result;
}

struct ta_found_item {
    ta_item *Item;
    u32 ItemIndex;
    f32 Weight;
    u32 WordIndex;
};

struct ta_found_items {
    dynamic_array<ta_found_item> Items;
    b8 IsAmbiguous;
};

struct ta_name_comparison {
    s32 FoundWordIndex;
    s32 MatchingWords;
    f32 Weight; // Sum of all the match percentages
};

internal inline ta_name_comparison
MakeTANameComparison(){
    ta_name_comparison Result = {};
    Result.FoundWordIndex = -1;
    return Result;
}

internal inline ta_name_comparison
TACompareWordsAndName(ta_name *NameData, word_array Words){
    ta_name_comparison Result = MakeTANameComparison();
    for(u32 WordIndex=0; WordIndex<Words.Count; WordIndex++){
        const char *Word = Words[WordIndex];
        
        f32 AdjectiveMatch = 0.0f;
        for(u32 AdjectiveIndex=0; AdjectiveIndex<NameData->Adjectives.Count; AdjectiveIndex++){
            const char *Adjective = NameData->Adjectives[AdjectiveIndex];
            f32 M = CompareWordsPercentage(Word, Adjective);
            if(M > AdjectiveMatch){
                AdjectiveMatch = M;
            }
        }
        
        f32 AliasMatch = 0.0f;
        for(u32 AliasIndex=0; AliasIndex<NameData->Aliases.Count; AliasIndex++){
            const char *Alias = NameData->Aliases[AliasIndex];
            f32 M = CompareWordsPercentage(Word, Alias);
            if(M > AliasMatch){
                AliasMatch = M;
                if(M > WORD_MATCH_THRESHOLD) Result.FoundWordIndex = WordIndex;
            }
        }
        
        if((AliasMatch > WORD_MATCH_THRESHOLD) || (AdjectiveMatch > WORD_MATCH_THRESHOLD)){
            Result.MatchingWords++;
            Result.Weight += Maximum(AliasMatch, AdjectiveMatch);
        }
        if((AliasMatch <= WORD_MATCH_THRESHOLD) && (Result.FoundWordIndex >= 0)) break;
    }
    
    return Result;
}

internal void 
TAContinueFindItems(ta_system *TA, array<ta_id> *Items, word_array Words, ta_found_items *Founds){
    for(u32 ItemIndex=0; ItemIndex<Items->Count; ItemIndex++){
        ta_item *Item = HashTableFindPtr(&TA->ItemTable, ArrayGet(Items, ItemIndex));
        if(!Item) continue;
        
        ta_name_comparison Comparison = TACompareWordsAndName(&Item->NameData, Words);
        
        if(Comparison.FoundWordIndex >= 0){
            b8 FoundSomething = false;
            for(u32 J=0; J<Founds->Items.Count; J++){
                ta_found_item *Found = &Founds->Items[J];
                if(Found->WordIndex == (u32)Comparison.FoundWordIndex){
                    if(Comparison.Weight > Found->Weight){
                        Found->Item = Item;
                        Found->ItemIndex = ItemIndex;
                        Found->WordIndex = Comparison.FoundWordIndex;
                        Found->Weight = Comparison.Weight;
                        FoundSomething = true;
                        break;
                    }else if(Comparison.Weight == Found->Weight){
                        //FoundSomething = true;
                        Founds->IsAmbiguous = true;
                        break;
                    }
                }
            }
            
            if(!FoundSomething){
                ta_found_item *Found = ArrayAlloc(&Founds->Items);
                Found->Item = Item;
                Found->ItemIndex = ItemIndex;
                Found->WordIndex = Comparison.FoundWordIndex;
                Found->Weight = Comparison.Weight;
            }
        }
    }
}

#define HANDLE_AMBIGUOUS_FOUND_ITEMS(FoundItems, Function, Verb) \
if(FoundItems.IsAmbiguous){ \
TA->Respond(GetVar(Assets, ambiguous_items)); \
TA->Callback = Function; \
return false; \
}

#define HANDLE_FOUND_ITEMS(FoundItems, Function, Verb) \
HANDLE_AMBIGUOUS_FOUND_ITEMS(FoundItems, Function, Verb) \

internal ta_found_items
TAFindItems(ta_system *TA, array<ta_id> *Items, word_array Words){
    ta_found_items Result = {};
    Result.Items = MakeDynamicArray<ta_found_item>(&GlobalTransientMemory, 2);
    TAContinueFindItems(TA, Items, Words, &Result);
    
    return Result;
}

internal inline s32
TAFindItemByTag(ta_system *TA, array<ta_id> *Items, asset_tag Tag){
    for(u32 J=0; J<Items->Count; J++){
        ta_item *Item = HashTableFindPtr(&TA->ItemTable, ArrayGet(Items, J));
        if(!Item) continue;
        if(CompareTags(Item->Tag, Tag)) return J;
    }
    
    return -1;
}

internal inline b8
DoTANameComparisonsOverlap(ta_name_comparison Old, ta_name_comparison New){
    b8 Result = ((New.FoundWordIndex >= Old.FoundWordIndex) && 
                 (New.FoundWordIndex < Old.FoundWordIndex+Old.Weight));
    return Result;
}

//~ Other
inline ta_area
MakeTAArea(ta_id Name, v2 Offset){
    ta_area Result = {};;
    Result.Name = Name;
    Result.Offset = Offset;
    return Result;
}

inline void
DoString(game_renderer *Renderer, asset_font *Font, fancy_font_format *Fancies, u32 FancyCount, 
         const char *S, rect *R){
    R->Y1 -= FontRenderFancyString(Renderer, Font, Fancies, FancyCount, S, *R);
}

inline void 
ta_system::Respond(const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    StringBuilderVAdd(&ResponseBuilder, Format, VarArgs);
    StringBuilderAdd(&ResponseBuilder, '\n');
    va_end(VarArgs);
}

inline ta_room *
ta_system::FindRoom(ta_id Room){
    ta_room *Result = HashTableFindPtr(&RoomTable, Room);
    return Result;
}

inline ta_item *
ta_system::FindItem(ta_id Item){
    ta_item *Result = HashTableFindPtr(&ItemTable, Item);
    return Result;
}

inline ta_data *
ta_system::FindData(array<ta_data *> *Datas, ta_data_type Type, asset_tag Tag){
    ta_data *Result = 0;
    for(u32 I=0; I<Datas->Count; I++){
        ta_data *Data = ArrayGet(Datas, I);
        if((Data->Type == Type) && CompareTags(Data->Tag, Tag)){
            Result = Data;
            break;
        }
    }
    return Result;
}

inline ta_data *
ta_system::FindDescription(array<ta_data *> *Descriptions, asset_tag Tag){
    ta_data *Result = FindData(Descriptions, TADataType_Description, Tag);
    
    return Result;
}

void 
ta_system::Unlock(audio_mixer *Mixer, asset_system *Assets, ta_room *Room, asset_tag *Locked){
    Room->Flags |= RoomFlag_Dirty;
    *Locked = AssetTag();
    Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_open_door)));
}

b8
ta_system::AttemptToUnlock(audio_mixer *Mixer, asset_system *Assets, ta_room *Room, asset_tag *Locked){
    for(u32 J=0; J<Inventory.Count; J++){
        ta_item *Item = FindItem(ArrayGet(&Inventory, J));
        if(!Item) continue;
        if(HasTag(Item->Tag, AssetTag_Key)){
            if(HasTag(*Locked, AssetTag_Organ) && HasTag(Item->Tag, AssetTag_Organ)){
                Unlock(Mixer, Assets, Room, Locked);
                return true;
            }else if(HasTag(*Locked, AssetTag_BellTower) && HasTag(Item->Tag, AssetTag_BellTower)){
                Unlock(Mixer, Assets, Room, Locked);
                return true;
            }
        }
    }
    
    return false;
}

b8
ta_system::IsClosed(asset_tag Tag){
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

//~ Inventory
inline b8
ta_system::InventoryAddItem(ta_id Item){
    b8 Result = ArrayMaybeAdd(&Inventory, Item);
    if(!Result) Respond("You are far too \002\002weak\002\001 to carry that many items!!!");
    return Result;
}

inline b8
ta_system::InventoryRemoveItem(u32 Index){
    ArrayOrderedRemove(&Inventory, Index);
    return (Index < Inventory.Count);
}

inline b8
ta_system::InventoryRemoveItemByID(ta_id ID){
    b8 Result = ArrayRemoveByValue(&Inventory, ID);
    Assert(Result);
    
    return Result;
}

//~ Room

inline b8
ta_system::RoomAddItem(ta_room *Room, ta_id Item){
    if(!Room->Items){
        Room->Items = MakeArray<ta_id>(Memory, TA_ROOM_DEFAULT_ITEM_COUNT);
    }
    
    b8 Result = ArrayMaybeAdd(&Room->Items, Item);
    Room->Flags |= RoomFlag_Dirty;
    return Result;
}

inline b8
ta_system::RoomDropItem(asset_system *Assets, ta_room *Room, ta_id Item){
    b8 Result = RoomAddItem(Room, Item);
    if(!Result) Respond(GetVar(Assets, room_too_small));
    return Result;
}

inline b8
ta_system::RoomRemoveItem(ta_room *Room, u32 Index){
    Room->Flags |= RoomFlag_Dirty;
    ArrayOrderedRemove(&Room->Items, Index);
    return (Index < Room->Items.Count);
}

inline b8
ta_system::RoomRemoveItemByID(ta_room *Room, ta_id ID){
    b8 Result = ArrayRemoveByValue(&Room->Items, ID);
    Assert(Result);
    
    return Result;
}

internal inline void
TAUpdateOrganState(ta_system *TA){
    ta_item *Bellows = HashTableFindPtr(&TA->ItemTable, TAItemByName(TA, "organ bellows static"));
    ta_item *Pipes   = HashTableFindPtr(&TA->ItemTable, TAItemByName(TA, "organ pipes static"));
    if(HasTag(Bellows->Tag, AssetTag_Repaired) &&
       HasTag(Pipes->Tag, AssetTag_Repaired)){
        TA->OrganState = AssetTag_Repaired;
    }else{
        TA->OrganState = AssetTag_Broken;
    }
}

//~ 
inline array<char *>
ta_system::EndCommand(){
    const char *Command = CurrentEditingCommand->Context.Buffer;
    const char *SavedCommand = ArenaPushCString(&CommandMemory, Command);
    StackPushSafe(&CommandStack, SavedCommand);
    
    ArenaClear(&GlobalTickMemory);
    DLIST_INIT(&EditingCommandSentinel);
    EditingCommandSentinel.Context.Reset();
    CurrentEditingCommand = &EditingCommandSentinel;
    CurrentPeekedCommand = 0;
    
    ResponseBuilder = BeginStringBuilder(&GlobalTickMemory, DEFAULT_BUFFER_SIZE);
    
    array<char *> Result = TokenizeCommand(&GlobalTransientMemory, SavedCommand);
    
    return Result;
}

inline ta_editing_command_node *
ta_system::AllocEditingCommand(){
    ta_editing_command_node *Node = PushStruct(&GlobalTickMemory, ta_editing_command_node);
    DLIST_ADD_LAST(&EditingCommandSentinel, Node);
    Node->Context.Initialize(&GlobalTickMemory);
    return Node;
}

void
ta_system::EditingCommandCycleUp(os_input *Input){
    if(CurrentPeekedCommand >= CommandStack.Count) return;
    CurrentPeekedCommand++;
    if(CurrentEditingCommand->Next == &EditingCommandSentinel){
        ta_editing_command_node *Node = AllocEditingCommand();
        Node->Context.LoadToBuffer(StackPeek(&CommandStack, CurrentPeekedCommand-1));
        Input->BeginTextInput(&Node->Context);
    }else{
        Input->BeginTextInput(&CurrentEditingCommand->Next->Context);
    }
    CurrentEditingCommand = CurrentEditingCommand->Next;
}

void
ta_system::EditingCommandCycleDown(os_input *Input){
    if(CurrentPeekedCommand == 0) return;
    CurrentPeekedCommand--;
    
    Input->BeginTextInput(&CurrentEditingCommand->Prev->Context);
    CurrentEditingCommand = CurrentEditingCommand->Prev;
}