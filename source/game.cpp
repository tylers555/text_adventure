
//~ Commands
void CommandMove(char **Words, u32 WordCount){
    ta_room *CurrentRoom = TextAdventure.CurrentRoom;
    ta_room *NextRoom = 0;
    
    for(u32 I=0; I < WordCount; I++){
        char *Word = Words[I];
        direction Direction = FindInHashTable(&TextAdventure.DirectionTable, (const char *)Word);
        if(Direction){
            string NextRoomString = CurrentRoom->Adjacents[Direction];
            NextRoom = FindInHashTablePtr(&TextAdventure.RoomTable, NextRoomString);
            if(NextRoom){
                break;
            }else{
                CopyCString(TextAdventure.ResponseBuffer, "Why would you want move that way!?\nDoing so would be quite \002\002foolish\002\001!", DEFAULT_BUFFER_SIZE);
                return;
            }
        }
    }
    
    if(!NextRoom){
        CopyCString(TextAdventure.ResponseBuffer, "Don't you understand how to move!?\nYou need to specify a direction, \002\002pal\002\001!", DEFAULT_BUFFER_SIZE);
        return;
    }
    TextAdventure.CurrentRoom = NextRoom;
    TextAdventure.ResponseBuffer[0] = 0;
}

void CommandTake(char **Words, u32 WordCount){
    CopyCString(TextAdventure.ResponseBuffer, "This command is not yet implemented, buddy-o!\n\002\002Shouldn't you know this?!", DEFAULT_BUFFER_SIZE);
}

//~ Text adventure system
void
ta_system::Initialize(memory_arena *Arena){
    CommandTable = PushHashTable<const char *, command_func *>(Arena, 64);
    InsertIntoHashTable(&CommandTable, "go",   CommandMove);
    InsertIntoHashTable(&CommandTable, "move", CommandMove);
    InsertIntoHashTable(&CommandTable, "take", CommandTake);
    
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
    Inventory = MakeArray<string>(Arena, 10);
    ArrayAdd(&Inventory, String("Red roses"));
    ArrayAdd(&Inventory, String("White roses"));
    ArrayAdd(&Inventory, String("Pink tulips"));
}

char **
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

//~ 
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
    fancy_font_format BasicFancy = MakeFancyFormat(WHITE, 0.0, 0.0, 0.0);
    
    //~ Room display
    {
        fancy_font_format RoomNameFancy = MakeFancyFormat(YELLOW, 1.0,  5.0, 1.0);
        fancy_font_format ItemFancy = MakeFancyFormat(RED, 1.0,  7.0, 1.0);
        fancy_font_format RoomFancy = MakeFancyFormat(ORANGE, 1.0,  7.0, 1.0);
        fancy_font_format DirectionFancy = MakeFancyFormat(PINK, 0.0,  0.0, 0.0);
        
        v2 RoomP = V2(10, WindowSize.Y-10);
        f32 DescriptionWidth = WindowSize.X-150;
        
        RoomP.Y -= FontRenderFancyString(BoldFont, &RoomNameFancy, 1, RoomP, TextAdventure.CurrentRoom->Name);
        
        fancy_font_format DescriptionFancies[] = {BasicFancy, DirectionFancy, RoomFancy, ItemFancy};
        RoomP.Y -= FontRenderFancyString(Font, DescriptionFancies, ArrayCount(DescriptionFancies), 
                                         RoomP, TextAdventure.CurrentRoom->Description, DescriptionWidth);
    }
    
    //~ Text input rendering
    {
        fancy_font_format ResponseFancy = MakeFancyFormat(GREEN,  0.0,  0.0, 0.0);
        fancy_font_format EmphasisFancy = MakeFancyFormat(PURPLE, 1.0, 13.0, 3.0);
        
        f32 InputHeight = 50;
        v2 InputP = V2(10, InputHeight);
        
        fancy_font_format ResponseFancies[2] = {ResponseFancy, EmphasisFancy};
        InputP.Y -= FontRenderFancyString(Font, ResponseFancies, ArrayCount(ResponseFancies), InputP, TextAdventure.ResponseBuffer);
        
        char *Text = OSInput.Buffer;
        FontRenderFancyString(Font, &BasicFancy, 1, InputP, Text);
        
        v2 CursorP = InputP+FontStringAdvance(Font, OSInput.CursorPosition, Text);
        if(((FrameCounter+30) / 30) % 3){
            RenderLine(&GameRenderer, CursorP, CursorP+V2(0, 5), 0.0, 1, WHITE);
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
                CStringToLower(Word);
                Func = FindInHashTable(&TextAdventure.CommandTable, (const char *)Word);
                if(Func) break;
            }
            if(Func){
                (*Func)(&Tokens[1], TokenCount-1);
            }else{
                CopyCString(TextAdventure.ResponseBuffer, "That is not a valid command!\n\002\002You fool!!!", DEFAULT_BUFFER_SIZE);
            }
            
            OSInput.BeginTextInput();
        }
    }
    
    //~ Inventory
    {
        fancy_font_format ItemFancy = MakeFancyFormat(RED, 0.0, 0.0, 0.0);
        
        f32 InventoryWidth = 100;
        v2 InventoryP = V2(WindowSize.X-InventoryWidth, WindowSize.Y-10);
        InventoryP.Y -= FontRenderFancyString(BoldFont, &BasicFancy, 1, InventoryP, "Inventory:");
        
        for(u32 I=0; I<TextAdventure.Inventory.Count; I++){
            const char *Item = Strings.GetString(TextAdventure.Inventory[I]);
            InventoryP.Y -= FontRenderFancyString(Font, &ItemFancy, 1, InventoryP, Item);
        }
    }
}

