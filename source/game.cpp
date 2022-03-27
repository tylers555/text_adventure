
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
    
    ThemeTable = PushHashTable<string, console_theme>(Arena, 8);
    Theme = MakeDefaultConsoleTheme();
    
    //~ Game specific data
    OrganState = AssetTag_Broken;
}

//~ 

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
    
    //~ Room display
    ta_room *Room = TA->CurrentRoom;
    v2 StartRoomP = V2(10, WindowSize.Y-15);
    f32 RoomDescriptionWidth = WindowSize.X-150;
    v2 StartItemP = V2(StartRoomP.X+RoomDescriptionWidth+10, StartRoomP.Y);
    f32 ItemDescriptionWidth = WindowSize.X-25-RoomDescriptionWidth;
    f32 InventoryWidth = ItemDescriptionWidth;
    f32 RoomTitleHeight = 0;
    
    {
        v2 RoomP = StartRoomP;
        
        RoomTitleHeight = FontRenderFancyString(Renderer, BoldFont, &Theme->RoomTitleFancy, 1, RoomP, Room->Name);
        RoomP.Y -= RoomTitleHeight;
        
        ta_string *Description = Room->Descriptions[0];
        if(HasTag(Room->Tag, AssetTag_Organ)){
            ta_string *New = TARoomFindDescription(Room, AssetTag(TA->OrganState));
            if(New) Description = New;
        }
        
        RoomP.Y -= FontRenderFancyString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies), 
                                         RoomP, Description->Data, RoomDescriptionWidth);
        
        ta_string *Adjacents = TARoomFindDescription(Room, AssetTag(AssetTag_Adjacents));
        if(Adjacents){
            RoomP.Y -= FontRenderFancyString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies), 
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
                if(!Item) continue;
                if(!HasTag(Item->Tag, AssetTag_Static)){
                    HasNonStatic = true; 
                    break;
                }
            }
            
            if(HasNonStatic){
                ItemP.Y -= RoomTitleHeight;
                
                ta_string *Items = TARoomFindDescription(Room, AssetTag(AssetTag_Items));
                if(Items){
                    ItemP.Y -= FontRenderFancyString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies), 
                                                     ItemP, Items->Data, ItemDescriptionWidth);
                }else{
                    ItemP.Y -= FontRenderFancyString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies), 
                                                     ItemP, "Items:", ItemDescriptionWidth);
                }
                
                for(u32 I=0; I<Room->Items.Count; I++){
                    ta_item *Item = FindInHashTablePtr(&TA->ItemTable, Room->Items[I]);
                    if(!Item) continue;
                    if(HasTag(Item->Tag, AssetTag_Static)) continue;
                    ItemP.Y -= FontRenderFancyString(Renderer, Font, &Theme->ItemFancy, 1, 
                                                     ItemP, Strings.GetString(Room->Items[I]), ItemDescriptionWidth);
                }
                ItemP.Y -= 10;
            }
        }
        
        v2 InventoryP = ItemP;
        InventoryP.Y -= FontRenderFancyString(Renderer, BoldFont, &Theme->BasicFancy, 1, InventoryP, "Inventory:");
        
        {
            char Buffer[DEFAULT_BUFFER_SIZE];
            stbsp_snprintf(Buffer, DEFAULT_BUFFER_SIZE, "%u coins", TA->Money);
            InventoryP.Y -= FontRenderFancyString(Renderer, Font, &Theme->ItemFancy, 1, InventoryP, Buffer);
        }
        
        for(u32 I=0; I<TA->Inventory.Count; I++){
            const char *Item = Strings.GetString(TA->Inventory[I]);
            InventoryP.Y -= FontRenderFancyString(Renderer, Font, &Theme->ItemFancy, 1, InventoryP, Item);
        }
    }
    
    //~ Text input rendering
    {
        
        f32 InputHeight = 100;
        f32 ResponseWidth = RoomDescriptionWidth;
        v2 InputP = V2(10, InputHeight);
        
        const char *Response = TA->ResponseBuilder.Buffer;
        InputP.Y -= FontRenderFancyString(Renderer, Font, Theme->ResponseFancies, ArrayCount(Theme->ResponseFancies), InputP, 
                                          Response, ResponseWidth);
        
        char *Text = Input->Buffer;
        FontRenderFancyString(Renderer, Font, &Theme->BasicFancy, 1, InputP, Text);
        
        f32 CursorHeight = Font->Height-Font->Descent;
        v2 CursorP = InputP+FontStringAdvance(Font, Input->CursorPosition, Text);
        if(((FrameCounter+30) / 30) % 3){
            RenderLine(Renderer, CursorP, CursorP+V2(0, CursorHeight), 0.0, 1, WHITE);
        }
        
        //~ Selection
        if(Input->SelectionMark >= 0){
            v2 SelectionP = InputP+FontStringAdvance(Font, Input->SelectionMark, Text);
            f32 Width = SelectionP.X-CursorP.X;
            RenderRect(Renderer, RectFix(SizeRect(V2(CursorP.X, CursorP.Y-1), V2(Width, Font->Height+2))), 1.0, BLUE);
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
                    (*Func)(Mixer, TA, Assets, &Tokens[1], TokenCount-1);
                }else{
                    TA->Respond("That is not a valid command!\n\002\002You fool\002\001!!!");
                }
            }
            Input->BeginTextInput();
        }
    }
}
