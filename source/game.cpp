
global_constant fancy_font_format BasicFancy = MakeFancyFormat(WHITE, 0.0, 0.0, 0.0);
global_constant fancy_font_format RoomNameFancy = MakeFancyFormat(YELLOW, 1.0,  6.0, 1.0);
global_constant fancy_font_format ItemFancy = MakeFancyFormat(RED, 1.0,  7.0, 1.0);
global_constant fancy_font_format RoomFancy = MakeFancyFormat(ORANGE, 1.0,  7.0, 1.0);
global_constant fancy_font_format DirectionFancy = MakeFancyFormat(PINK, 0.0,  0.0, 0.0);
global_constant fancy_font_format DescriptionFancies[] = {BasicFancy, DirectionFancy, RoomFancy, ItemFancy};

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

internal inline s32
TAFindItem(ta_system *TA, array<string> *Items, const char *Word){
    for(u32 J=0; J<Items->Count; J++){
        const char *ItemName = Strings.GetString(ArrayGet(Items, J));
        if(CompareWords(ItemName, Word)){
            return J;
        }
        
        ta_item *Item = FindInHashTablePtr(&TA->ItemTable, ArrayGet(Items, J));
        if(!Item) continue;
        for(u32 K=0; K<Item->Aliases.Count; K++){
            const char *Alias = ArrayGet(&Item->Aliases, K);
            if(CompareWords(Alias, Word)){
                return J;
            }
        }
    }
    
    return -1;
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
    return Result;
    
}

internal inline ta_string *
TAFindDescription(array<ta_string *> *Descriptions, string Tag){
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
TARoomFindDescription(ta_room *Room, string Tag){
    ta_string *Result = TAFindDescription(&Room->Descriptions, Tag);
    return Result;
}

//~ Commands
void CommandMove(char **Words, u32 WordCount){
    ta_system *TA = &TextAdventure;
    
    ta_room *CurrentRoom = TA->CurrentRoom;
    ta_room *NextRoom = 0;
    
    for(u32 I=0; I < WordCount; I++){
        char *Word = Words[I];
        direction Direction = FindInHashTable(&TA->DirectionTable, (const char *)Word);
        if(Direction){
            string NextRoomString = CurrentRoom->Adjacents[Direction];
            NextRoom = FindInHashTablePtr(&TA->RoomTable, NextRoomString);
            if(NextRoom){
                break;
            }else{
                TA->Respond("Why would you want move that way!?\nDoing so would be quite \002\002foolish\002\001!");
                return;
            }
        }
    }
    
    if(!NextRoom){
        TA->Respond("Don't you understand how to move!?\nYou need to specify a direction, \002\002pal\002\001!");
        return;
    }
    TA->CurrentRoom = NextRoom;
    TA->ResponseBuffer[0] = 0;
    
    AudioMixer.PlaySound(AssetSystem.GetSoundEffect(String("room_change")));
}

// TODO(Tyler): This is an incredibly crude way of doing this
void CommandTake(char **Words, u32 WordCount){
    ta_system *TA = &TextAdventure;
    
    ta_room *Room = TA->CurrentRoom;
    
    b8 TookSomething = false;
    for(u32 I=0; I<WordCount; I++){
        CStringMakeLower(Words[I]);
        char *Word = Words[I];
        
        if(CompareStrings(Word, "all") ||
           CompareStrings(Word, "everything")){
            for(u32 J=0; J<Room->Items.Count; J++){
                if(TA->AddItem(Room->Items[J])) ArrayOrderedRemove(&Room->Items, J);
                else break;
                TookSomething = true;
            }
            break;
        }
        
        s32 Index = TAFindItem(TA, &Room->Items, Word);
        if(Index < 0) continue;
        
        ta_item *Item = FindInHashTablePtr(&TA->ItemTable, Room->Items[Index]);
        ta_string *Description = TAFindDescription(&Item->Descriptions, String("examine"));
        TA->Respond(Description->Data);
        
        if(TA->AddItem(Room->Items[Index])) ArrayOrderedRemove(&Room->Items, Index);
        TookSomething = true;
        
    }
    
    if(!TookSomething){
        TA->Respond("I have no idea what you want to take!");
        return;
    }
    AudioMixer.PlaySound(AssetSystem.GetSoundEffect(String("item_taken")));
}

// TODO(Tyler): This is an incredibly crude way of doing this
void CommandDrop(char **Words, u32 WordCount){
    ta_system *TA = &TextAdventure;
    
    ta_room *Room = TA->CurrentRoom;
    
    b8 DroppedSomething = false;
    for(u32 I=0; I<WordCount; I++){
        CStringMakeLower(Words[I]);
        char *Word = Words[I];
        
        if(CompareStrings(Word, "all") ||
           CompareStrings(Word, "everything")){
            for(u32 J=0; J<TA->Inventory.Count; J++){
                if(TARoomAddItem(TA, Room, TA->Inventory[J])) ArrayOrderedRemove(&TA->Inventory, J);
                else break;
                DroppedSomething = true;
            }
            
            break;
        }
        
        s32 Index = TAFindItem(TA, &TA->Inventory, Word);
        if(Index < 0) continue;
        if(TARoomAddItem(TA, Room, TA->Inventory[Index])) ArrayOrderedRemove(&TA->Inventory, Index);
        DroppedSomething = true;
    }
    
    if(!DroppedSomething){
        TA->Respond("I have no idea what you want to drop!");
        return;
    }
    
    AudioMixer.PlaySound(AssetSystem.GetSoundEffect(String("item_dropped")));
    TextAdventure.ResponseBuffer[0] = 0;
}

void CommandEat(char **Words, u32 WordCount){
    ta_system *TA = &TextAdventure;
    
    for(u32 I=0; I<WordCount; I++){
        char *Word = Words[I];
        
        s32 Index = TAFindItem(TA, &TA->Inventory, Word);
        if(Index < 0) continue;
        
        ta_item *Item = FindInHashTablePtr(&TA->ItemTable, TA->Inventory[Index]);
        ta_string *Description = TAFindDescription(&Item->Descriptions, String("eat"));
        if(!Description){
            TA->Respond("You can't eat \002\002that\002\001!");
            return; // TODO(Tyler): I don't like this return, but we need a better response system first.
        }
        TA->Respond(Description->Data);
        
        if(Item->Tag == String("bread")){
            TA->AddItem(String("bread crumbs"));
        }
        
        ArrayOrderedRemove(&TA->Inventory, Index);
    }
}

void CommandPlay(char **Words, u32 WordCount){
    ta_system *TA = &TextAdventure;
    
    if(TA->CurrentRoom->Tag == String("organ")){
        string Tag = (TA->OrganState == String("broken")) ? String("play-broken") : String("play-repaired");
        ta_string *Description = TARoomFindDescription(TA->CurrentRoom, Tag);
        if(!Description) return;
        TA->Respond(Description->Data);
    }
}

void CommandExamine(char **Words, u32 WordCount){
    ta_system *TA = &TextAdventure;
    
    // TODO(Tyler): This is a bit hacky right now
    if(TA->CurrentRoom->Tag == String("organ")){
        string Tag = (TA->OrganState == String("broken")) ? String("examine-broken") : String("examine-repaired");
        ta_string *Description = TARoomFindDescription(TA->CurrentRoom, Tag);
        if(!Description) return;
        TA->Respond(Description->Data);
        return; 
    }
    
    b8 FoundSomething = false;
    for(u32 I=0; I<WordCount; I++){
        const char *Word = Words[I];
        
        s32 Index = TAFindItem(TA, &TA->Inventory, Word);
        if(Index < 0) continue;
        
        ta_item *Item = FindInHashTablePtr(&TA->ItemTable, TA->Inventory[Index]);
        ta_string *Description = TAFindDescription(&Item->Descriptions, String("examine"));
        if(!Description) continue;
        TA->Respond(Description->Data);
        FoundSomething = true;
    }
    
    if(FoundSomething) return;
    ta_room *Room = TA->CurrentRoom;
    for(u32 I=0; I<WordCount; I++){
        const char *Word = Words[I];
        
        s32 Index = TAFindItem(TA, &Room->Items, Word);
        if(Index < 0) continue;
        
        ta_item *Item = FindInHashTablePtr(&TA->ItemTable, Room->Items[Index]);
        ta_string *Description = TAFindDescription(&Item->Descriptions, String("examine"));
        if(!Description) continue;
        TA->Respond(Description->Data);
        FoundSomething = true;
    }
    
    if(FoundSomething) return;
    TA->Respond("I have no idea what you want to examine!");
}

//~ Testing commands
void CommandTestRepair(char **Words, u32 WordCount){
    ta_system *TA = &TextAdventure;
    
    if(TA->OrganState == String("broken"))        TA->OrganState = String("repaired");
    else if(TA->OrganState == String("repaired")) TA->OrganState = String("broken");
}

//~ Text adventure system
void
ta_system::Initialize(memory_arena *Arena){
    CommandTable = PushHashTable<const char *, command_func *>(Arena, 64);
    InsertIntoHashTable(&CommandTable, "go",   CommandMove);
    InsertIntoHashTable(&CommandTable, "move", CommandMove);
    InsertIntoHashTable(&CommandTable, "take", CommandTake);
    InsertIntoHashTable(&CommandTable, "pick", CommandTake);
    InsertIntoHashTable(&CommandTable, "grab", CommandTake);
    InsertIntoHashTable(&CommandTable, "drop",  CommandDrop);
    InsertIntoHashTable(&CommandTable, "leave", CommandDrop);
    InsertIntoHashTable(&CommandTable, "eat",     CommandEat);
    InsertIntoHashTable(&CommandTable, "consume", CommandEat);
    InsertIntoHashTable(&CommandTable, "ingest",  CommandEat);
    InsertIntoHashTable(&CommandTable, "swallow", CommandEat);
    InsertIntoHashTable(&CommandTable, "bite",    CommandEat);
    InsertIntoHashTable(&CommandTable, "munch",   CommandEat);
    InsertIntoHashTable(&CommandTable, "play",    CommandPlay);
    InsertIntoHashTable(&CommandTable, "examine", CommandExamine);
    InsertIntoHashTable(&CommandTable, "inspect", CommandExamine);
    InsertIntoHashTable(&CommandTable, "observe", CommandExamine);
    InsertIntoHashTable(&CommandTable, "look",    CommandExamine);
    
    InsertIntoHashTable(&CommandTable, "testrepair", CommandTestRepair);
    
    DirectionTable = PushHashTable<const char *, direction>(Arena, 2*Direction_TOTAL);
    InsertIntoHashTable(&DirectionTable, "north",     Direction_North);
    InsertIntoHashTable(&DirectionTable, "northeast", Direction_NorthEast);
    InsertIntoHashTable(&DirectionTable, "east",      Direction_East);
    InsertIntoHashTable(&DirectionTable, "southeast", Direction_SouthEast);
    InsertIntoHashTable(&DirectionTable, "south",     Direction_South);
    InsertIntoHashTable(&DirectionTable, "southwest", Direction_SouthWest);
    InsertIntoHashTable(&DirectionTable, "west",      Direction_West);
    InsertIntoHashTable(&DirectionTable, "northwest", Direction_NorthWest);
    InsertIntoHashTable(&DirectionTable, "n",  Direction_North);
    InsertIntoHashTable(&DirectionTable, "ne", Direction_NorthEast);
    InsertIntoHashTable(&DirectionTable, "e",  Direction_East);
    InsertIntoHashTable(&DirectionTable, "se", Direction_SouthEast);
    InsertIntoHashTable(&DirectionTable, "s",  Direction_South);
    InsertIntoHashTable(&DirectionTable, "sw", Direction_SouthWest);
    InsertIntoHashTable(&DirectionTable, "w",  Direction_West);
    InsertIntoHashTable(&DirectionTable, "nw", Direction_NorthWest);
    
    RoomTable = PushHashTable<string, ta_room>(Arena, 64);
    ItemTable = PushHashTable<string, ta_item>(Arena, 128);
    Inventory = MakeArray<string>(Arena, 10);
    
    //~ Game specific data
    OrganState = String("broken");
}

inline b8
ta_system::AddItem(string Item){
    b8 Result = ArrayMaybeAdd(&Inventory, Item);
    if(!Result) Respond("You are far too \002\002weak\002\001 to carry that many items!!!");
    return Result;
}

inline void 
ta_system::Respond(const char *Response){
    if(!Response) return;
    if((size_t)Response == offsetof(ta_string, Data)) return; // TODO(Tyler): Hacky solution
    CopyCString(ResponseBuffer, Response, DEFAULT_BUFFER_SIZE);
}

//~ 
internal inline f32
DoDescription(ta_system *TA, ta_room *Room, asset_font *Font, v2 P, f32 DescriptionWidth){
    ta_string *Description = Room->Descriptions[0];
    if(Room->Tag == String("organ")){
        ta_string *New = TARoomFindDescription(Room, TA->OrganState);
        if(New) Description = New;
    }
    
    f32 Result = FontRenderFancyString(Font, DescriptionFancies, ArrayCount(DescriptionFancies), 
                                       P, Description->Data, DescriptionWidth);
    P.Y -= Result;
    
    ta_string *Items = TARoomFindDescription(Room, String("items"));
    if(Items){
        Result += FontRenderFancyString(Font, DescriptionFancies, ArrayCount(DescriptionFancies), 
                                        P, Items->Data, DescriptionWidth);
    }
    
    return Result;
}

internal void
UpdateAndRenderMainGame(game_renderer *Renderer){
    if(!TextAdventure.CurrentRoom){
        OSInput.BeginTextInput();
        TextAdventure.CurrentRoom = FindInHashTablePtr(&TextAdventure.RoomTable, String("Shop front"));
    }
    
    //RenderTexture(Renderer, MakeRect(V2(0), V2(30)), 10.0, RED);
    asset_font *BoldFont = AssetSystem.GetFont(String("font_bold"));
    asset_font *Font = AssetSystem.GetFont(String("basic"));
    Assert(Font);
    
    v2 WindowSize = GameRenderer.ScreenToWorld(OSInput.WindowSize);
    
    //~ Room display
    {
        ta_room *Room = TextAdventure.CurrentRoom;
        
        v2 RoomP = V2(10, WindowSize.Y-15);
        f32 DescriptionWidth = WindowSize.X-150;
        
        RoomP.Y -= FontRenderFancyString(BoldFont, &RoomNameFancy, 1, RoomP, Room->Name);
        
        RoomP.Y -= DoDescription(&TextAdventure, Room, Font, RoomP, DescriptionWidth);
        
        for(u32 I=0; I<Room->Items.Count; I++){
            RoomP.Y -= FontRenderFancyString(Font, &ItemFancy, 1, 
                                             RoomP, Strings.GetString(Room->Items[I]), DescriptionWidth);
        }
    }
    
    //~ Text input rendering
    {
        
        fancy_font_format ResponseFancy = MakeFancyFormat(GREEN,  0.0,  0.0, 0.0);
        fancy_font_format EmphasisFancy = MakeFancyFormat(PURPLE, 1.0, 13.0, 3.0);
        
        f32 InputHeight = 50;
        f32 ResponseWidth = WindowSize.X-20;
        v2 InputP = V2(10, InputHeight);
        
        fancy_font_format ResponseFancies[2] = {ResponseFancy, EmphasisFancy};
        InputP.Y -= FontRenderFancyString(Font, ResponseFancies, ArrayCount(ResponseFancies), InputP, TextAdventure.ResponseBuffer, ResponseWidth);
        
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
            command_func *Func = 0;
            u32 TokenCount;
            char **Tokens = TokenizeCommand(&TransientStorageArena, OSInput.Buffer, &TokenCount);
            for(u32 I=0; I < TokenCount; I++){
                char *Word = Tokens[I];
                CStringMakeLower(Word);
                Func = FindInHashTable(&TextAdventure.CommandTable, (const char *)Word);
                if(Func) break;
            }
            if(Func){
                (*Func)(&Tokens[1], TokenCount-1);
            }else{
                TextAdventure.Respond("That is not a valid command!\n\002\002You fool\002\001!!!");
            }
            
            OSInput.BeginTextInput();
        }
    }
    
    //~ Inventory
    {
        
        f32 InventoryWidth = 100;
        v2 InventoryP = V2(WindowSize.X-InventoryWidth, WindowSize.Y-15);
        InventoryP.Y -= FontRenderFancyString(BoldFont, &BasicFancy, 1, InventoryP, "Inventory:");
        
        for(u32 I=0; I<TextAdventure.Inventory.Count; I++){
            const char *Item = Strings.GetString(TextAdventure.Inventory[I]);
            InventoryP.Y -= FontRenderFancyString(Font, &ItemFancy, 1, InventoryP, Item);
        }
    }
}


