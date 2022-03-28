
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
    
    ThemeTable = PushHashTable<string, console_theme>(Arena, 8);
    Theme = MakeDefaultConsoleTheme();
    
    //~ Game specific data
    OrganState = AssetTag_Broken;
}


//~ 
internal inline void
DoString(game_renderer *Renderer, asset_font *Font, fancy_font_format *Fancies, u32 FancyCount, 
         const char *S, rect *R){
    R->Y1 -= FontRenderFancyString(Renderer, Font, Fancies, FancyCount, S, *R);
}

internal void
UpdateAndRenderMainGame(game_renderer *Renderer, audio_mixer *Mixer, asset_system *Assets, os_input *Input){
    DO_DEBUG_INFO();
    
    ta_system *TA = &TextAdventure;
    console_theme *Theme = &TA->Theme;
    Renderer->NewFrame(&TransientStorageArena, Input->WindowSize, Theme->BackgroundColor);
    
    if(!TA->CurrentRoom){
        Input->BeginTextInput();
        TA->CurrentRoom = FindInHashTablePtr(&TA->RoomTable, TA->StartRoom);
        if(!TA->CurrentRoom){
            TA->CurrentRoom = FindInHashTablePtr(&TA->RoomTable, String("Plaza SE"));
            if(!TA->CurrentRoom){
                LogMessage("CurrentRoom is not set!");
                return;
            }else{
                LogMessage("Room: '%s' does not exist!", Strings.GetString(TA->StartRoom));
            }
        }
        TA->Money = 10;
    }
    
    //RenderTexture(Renderer, MakeRect(V2(0), V2(30)), 10.0, RED);
    asset_font *BoldFont = Assets->GetFont(Theme->TitleFont);
    asset_font *Font = Assets->GetFont(Theme->BasicFont);
    Assert(Font);
    
    v2 WindowSize = RoundV2(Renderer->ScreenToWorld(Input->WindowSize));
    
    //~ Area positioning
    f32 Padding = 10;
    rect WindowRect = MakeRect(V2(Padding), WindowSize-V2(Padding));
    rect RoomDescriptionRect = RectRound(RectPercent(WindowRect, 0.0f, 0.5f, 0.6f, 1.0f));
    RoomDescriptionRect.X1 -= Padding;
    rect InventoryRect       = RectRound(RectPercent(WindowRect, 0.6f, 0.5f, 1.0f, 1.0f));
    rect InputRect           = RectRound(RectPercent(WindowRect, 0.0f, 0.0f, 0.6f, 0.5f));
    rect MapRect             = RectRound(RectPercent(WindowRect, 0.6f, 0.0f, 1.0f, 0.5f));
    
    //~ Room display
    ta_room *Room = TA->CurrentRoom;
    
    {
        DoString(Renderer, BoldFont, &Theme->RoomTitleFancy, 1, Room->Name, &RoomDescriptionRect);
        
        ta_string *Description = Room->Descriptions[0];
        if(HasTag(Room->Tag, AssetTag_Organ)){
            ta_string *New = TARoomFindDescription(Room, AssetTag(TA->OrganState));
            if(New) Description = New;
        }
        
        DoString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies), 
                 Description->Data, &RoomDescriptionRect);
        
        ta_string *Adjacents = TARoomFindDescription(Room, AssetTag(AssetTag_Adjacents));
        if(Adjacents){
            DoString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies), 
                     Adjacents->Data, &RoomDescriptionRect);
        }
        
        
    }
    
    //~ Inventory
    {
        if(Room->Items.Count > 0){
            b8 HasNonStatic = false;
            for(u32 I=0; I<Room->Items.Count; I++){
                ta_item *Item = FindInHashTablePtr(&TA->ItemTable, Room->Items[I]);
                if(!Item) continue;
                if(!HasTag(Item->Tag, AssetTag_Static)){
                    HasNonStatic = true; 
                    break;
                }
            }
            
            if(HasNonStatic){
                InventoryRect.Y1 -= FontLineHeight(BoldFont);
                
                ta_string *Items = TARoomFindDescription(Room, AssetTag(AssetTag_Items));
                if(Items){
                    f32 L = FontStringAdvance(Font, Items->Data, RectSize(InventoryRect).X).X;
                    DoString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies), 
                             Items->Data, &InventoryRect);
                }else{
                    DoString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies), 
                             "Items:", &InventoryRect);
                }
                
                for(u32 I=0; I<Room->Items.Count; I++){
                    ta_item *Item = FindInHashTablePtr(&TA->ItemTable, Room->Items[I]);
                    if(!Item) continue;
                    if(HasTag(Item->Tag, AssetTag_Static)) continue;
                    DoString(Renderer, Font, &Theme->ItemFancy, 1, 
                             Strings.GetString(Room->Items[I]), &InventoryRect);
                }
                
                InventoryRect.Y1 -= Padding;
            }
        }
        
        DoString(Renderer, BoldFont, &Theme->BasicFancy, 1, "Inventory:", &InventoryRect);
        
        {
            char Buffer[DEFAULT_BUFFER_SIZE];
            stbsp_snprintf(Buffer, DEFAULT_BUFFER_SIZE, "%u coins", TA->Money);
            DoString(Renderer, Font, &Theme->ItemFancy, 1, Buffer, &InventoryRect);
        }
        
        for(u32 I=0; I<TA->Inventory.Count; I++){
            const char *Item = Strings.GetString(TA->Inventory[I]);
            DoString(Renderer, Font, &Theme->ItemFancy, 1, Item, &InventoryRect);
        }
    }
    
    //~ Map
    {
        s32 MapIndex = TAFindItemByTag(TA, &TA->Inventory, AssetTag(AssetTag_Map));
        if(MapIndex >= 0){
            ta_map *Map = &TA->Map;
            render_texture Texture = Map->Texture;
            v2 BR = V2(MapRect.X1, MapRect.Y0);
            rect R = SizeRect(BR, Map->Size);
            R = RectMoveRight(R, -Map->Size.X);
            RenderTexture(Renderer, R, 0.0, Texture);
            
            ta_area *CurrentArea = 0;
            for(u32 I=0; I<Map->Areas.Count; I++){
                ta_area *Area = &Map->Areas[I];
                if(Area->Name == TA->CurrentRoom->Area){
                    CurrentArea = Area;
                    break;
                }
            }
            
            if(CurrentArea){
                rect Point = R.Min+CenterRect(CurrentArea->Offset, V2(3));
                color C = MixColor(PINK, WHITE, 0.5f*(Sin(3.0f*Counter)+1.0f));
                RenderRect(Renderer, Point, -1.0, C);
            }
        }
    }
    
    //~ Text input rendering
    {
        const char *Response = TA->ResponseBuilder.Buffer;
        DoString(Renderer, Font, Theme->ResponseFancies, ArrayCount(Theme->ResponseFancies),
                 Response, &InputRect);
        
        char *Text = Input->Buffer;
        v2 InputP = V2(InputRect.X0, InputRect.Y1);
        DoString(Renderer, Font, &Theme->BasicFancy, 1, Text, &InputRect);
        
        f32 CursorHeight = Font->Height-Font->Descent;
        v2 CursorP = InputP+FontStringAdvance(Font, Input->CursorPosition, Text);
        if(((FrameCounter+30) / 30) % 3){
            RenderLine(Renderer, CursorP, CursorP+V2(0, CursorHeight), 0.0, 1, Theme->CursorColor);
        }
        
        //~ Selection
        if(Input->SelectionMark >= 0){
            v2 SelectionP = InputP+FontStringAdvance(Font, Input->SelectionMark, Text);
            f32 Width = SelectionP.X-CursorP.X;
            RenderRect(Renderer, SizeRect(V2(CursorP.X, CursorP.Y-1), V2(Width, Font->Height+2)), 1.0, 
                       Theme->SelectionColor);
        }
        
        //~ Command processing
        if(Input->MaybeEndTextInput()){
            TA->ClearResponse();
            u32 TokenCount;
            char **Tokens = TokenizeCommand(&TransientStorageArena, Input->Buffer, &TokenCount);
            if(TA->Callback){
                // NOTE(Tyler): Weird dance, so that the callback can set another callback.
                command_func *Callback = TA->Callback;
                TA->Callback = 0;
                (*Callback)(Mixer, TA, Assets, Tokens, TokenCount);
                
            }else{
                command_func *Func = 0;
                for(u32 I=0; I < TokenCount; I++){
                    char *Word = Tokens[I];
                    CStringMakeLower(Word);
                    Func = FindInHashTable(&TA->CommandTable, (const char *)Word);
                    if(Func) break;
                }
                if(Func){
                    (*Func)(Mixer, TA, Assets, Tokens, TokenCount);
                }else{
                    TA->Respond("That is not a valid command!\n\002\002You fool\002\001!!!");
                }
            }
            Input->BeginTextInput();
        }
    }
}
