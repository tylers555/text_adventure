
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
    {
        ta_room Room = {};
        Room.Name = "Main hall";
        Room.Adjacents[Direction_North] = String("Room 1");
        Room.Adjacents[Direction_South] = String("Room 2");
        Room.Description = "This is the main hall\nThere is a room to the north and--you guessed it!--another room to the south";
        InsertIntoHashTable(&RoomTable, String(Room.Name), Room);
    }
    {
        ta_room Room = {};
        Room.Name = "Room 1";
        Room.Adjacents[Direction_South] = String("Main hall");
        Room.Adjacents[Direction_West] = String("Room 2");
        Room.Description = "You find yourself in a small room.\nThere is not yet much in the room";
        InsertIntoHashTable(&RoomTable, String(Room.Name), Room);
    }
    {
        ta_room Room = {};
        Room.Name = "Room 2";
        Room.Adjacents[Direction_North] = String("Main hall");
        Room.Adjacents[Direction_East] = String("Room 1");
        Room.Description = "You look around and see the walls are a hot bright pink.\nThere are signs everywhere that read WIP";
        InsertIntoHashTable(&RoomTable, String(Room.Name), Room);
    }
    
    CurrentRoom = FindInHashTablePtr(&RoomTable, String("Main hall"));
    
    
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
    if(FrameCounter == 0){
        OSInput.BeginTextInput();
    }
    
    //RenderTexture(Renderer, MakeRect(V2(0), V2(30)), 10.0, RED);
    asset_font *Font = AssetSystem.GetFont(String("basic"));
    Assert(Font);
    
    v2 WindowSize = GameRenderer.ScreenToWorld(OSInput.WindowSize);
    v2 P = V2(10, WindowSize.Y-10);
    
    fancy_font_format Fancy0 = MakeFancyFormat(YELLOW, 1.0,  7.0, 2.0);
    fancy_font_format Fancy1 = MakeFancyFormat(GREEN,  0.0,  0.0, 0.0);
    fancy_font_format Fancy2 = MakeFancyFormat(PURPLE, 0.0,  0.0, 0.0);
    fancy_font_format Fancy3 = MakeFancyFormat(PINK, 1.0, 13.0, 3.0);
    P.Y -= FontRenderFancyString(Font, &Fancy0, 1, P, TextAdventure.CurrentRoom->Name);
    P.Y -= FontRenderFancyString(Font, &Fancy1, 1, P, TextAdventure.CurrentRoom->Description);
    
    fancy_font_format ResponseFancies[2] = {Fancy2, Fancy3};
    P.Y -= FontRenderFancyString(Font, ResponseFancies, ArrayCount(ResponseFancies), P, TextAdventure.ResponseBuffer);
    
    char *Text = OSInput.Buffer;
    FontRenderString(Font, P, WHITE, "%s", Text);
    
    v2 CursorP = P;
    CursorP.X += FontStringAdvance(Font, OSInput.CursorPosition, "%s", Text);
    if(((FrameCounter+30) / 30) % 3){
        RenderLine(&GameRenderer, CursorP, CursorP+V2(0, 5), 0.0, 1, WHITE);
    }
    
    if(OSInput.SelectionMark >= 0){
        f32 SelectionX = P.X+FontStringAdvance(Font, OSInput.SelectionMark, Text);
        f32 Width = SelectionX-CursorP.X;
        RenderRect(&GameRenderer, RectFix(SizeRect(V2(CursorP.X, CursorP.Y-1), V2(Width, Font->Height+2))), 1.0, BLUE);
    }
    
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

