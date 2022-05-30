
//~ Text adventure system
void
ta_system::Initialize(memory_arena *Arena){
    RoomTable = MakeHashTable<ta_id, ta_room>(Arena, ROOM_TABLE_SIZE);
    ItemTable = MakeHashTable<ta_id, ta_item>(Arena, ITEM_TABLE_SIZE);
    Inventory = MakeArray<ta_id>(Arena, INVENTORY_ITEM_COUNT);
    ResponseBuilder = BeginStringBuilder(Arena, DEFAULT_BUFFER_SIZE);
    
    ThemeTable = MakeHashTable<ta_id, console_theme>(Arena, 8);
    
    //~ Game specific data
    OrganState = AssetTag_Broken;
}


internal inline void
TADispatchCommand(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Tokens, u32 TokenCount){
    command_func *Func = 0;
    f32 HighestMatch = 0.0f;
    for(u32 I=0; I < TokenCount; I++){
        char *Word = Tokens[I];
        CStringMakeLower(Word);
        
#define TEST_COMMAND(Name, Command) { f32 Match = CompareWordsPercentage(Word, Name); \
if(Match > HighestMatch){ \
Func = Command; \
HighestMatch = Match; \
} \
}
        TEST_COMMAND("go",       CommandMove);
        TEST_COMMAND("move",     CommandMove);
        TEST_COMMAND("exit",     CommandExit);
        TEST_COMMAND("leave",    CommandExit);
        TEST_COMMAND("enter",    CommandEnter);
        TEST_COMMAND("take",     CommandTake);
        TEST_COMMAND("pick",     CommandTake);
        TEST_COMMAND("grab",     CommandTake);
        TEST_COMMAND("drop",     CommandDrop);
        TEST_COMMAND("leave",    CommandDrop);
        TEST_COMMAND("buy",      CommandBuy);
        TEST_COMMAND("purchase", CommandBuy);
        TEST_COMMAND("eat",      CommandEat);
        TEST_COMMAND("consume",  CommandEat);
        TEST_COMMAND("ingest",   CommandEat);
        TEST_COMMAND("swallow",  CommandEat);
        TEST_COMMAND("bite",     CommandEat);
        TEST_COMMAND("munch",    CommandEat);
        TEST_COMMAND("play",     CommandPlay);
        TEST_COMMAND("examine",  CommandExamine);
        TEST_COMMAND("inspect",  CommandExamine);
        TEST_COMMAND("observe",  CommandExamine);
        TEST_COMMAND("look",     CommandExamine);
        TEST_COMMAND("see",      CommandExamine);
        TEST_COMMAND("unlock",   CommandUnlock);
        TEST_COMMAND("repair",   CommandRepair);
        TEST_COMMAND("fix",      CommandRepair);
        TEST_COMMAND("use",      CommandUse);
        
        // Testing commands
        TEST_COMMAND("testaddmoney", CommandTestAddMoney);
        TEST_COMMAND("testsubmoney", CommandTestSubMoney);
#undef TEST_COMMAND
        
        if(HighestMatch > WORD_MATCH_THRESHOLD){
            (*Func)(Mixer, TA, Assets, Tokens, TokenCount);
            return;
        }
    }
    
    TA->Respond(GetVar(Assets, invalid_command));
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
    console_theme *Theme = HashTableFindPtr(&TA->ThemeTable, GetVarTAID(Assets, theme));
    Renderer->NewFrame(&TransientStorageArena, Input->WindowSize, Theme->BackgroundColor);
    
    if(!TA->CurrentRoom){
        Input->BeginTextInput();
        TA->CurrentRoom = HashTableFindPtr(&TA->RoomTable, GetVarTAID(Assets, start_room));
        if(!TA->CurrentRoom){
            TA->CurrentRoom = HashTableFindPtr(&TA->RoomTable, TAIDByName(TA, "Southeast plaza"));
            if(!TA->CurrentRoom){
                LogMessage("CurrentRoom is not set!");
                return;
            }else{
                LogMessage("Room: '%s' does not exist!", GetVar(Assets, start_room));
            }
        }
        TA->Money = 10;
    }
    
    //RenderTexture(Renderer, MakeRect(V2(0), V2(30)), 10.0, RED);
    asset_font *BoldFont = GetFont(Assets, Theme->TitleFont);
    asset_font *Font = GetFont(Assets, Theme->BasicFont);
    Assert(Font);
    
    v2 WindowSize = RoundV2(Renderer->ScreenToWorld(Input->WindowSize));
    
    //~ Area positioning
    f32 Padding = 10;
    rect WindowRect = MakeRect(V2(Padding), WindowSize-V2(Padding));
    rect RoomDescriptionRect = RectRound(RectPercent(WindowRect, 0.0f, 0.5f, 0.7f, 1.0f));
    RoomDescriptionRect.X1 -= Padding;
    rect InventoryRect       = RectRound(RectPercent(WindowRect, 0.7f, 0.5f, 1.0f, 1.0f));
    rect InputRect           = RectRound(RectPercent(WindowRect, 0.0f, 0.0f, 0.7f, 0.5f));
    rect MapRect             = RectRound(RectPercent(WindowRect, 0.7f, 0.0f, 1.0f, 0.5f));
    
    ta_room *Room = TA->CurrentRoom;
    //~ Room display
    {
        DoString(Renderer, BoldFont, &Theme->RoomTitleFancy, 1, Room->Name, &RoomDescriptionRect);
        
        ta_data *Description = Room->Datas[0];
        if(HasTag(Room->Tag, AssetTag_Organ)){
            ta_data *New = TARoomFindDescription(Room, AssetTag(TA->OrganState));
            if(New) Description = New;
        }
        
        DoString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies), 
                 Description->Data, &RoomDescriptionRect);
        
        ta_data *Adjacents = TARoomFindDescription(Room, AssetTag(AssetTag_Adjacents));
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
                ta_item *Item = HashTableFindPtr(&TA->ItemTable, Room->Items[I]);
                if(!Item) continue;
                if(!HasTag(Item->Tag, AssetTag_Static)){
                    HasNonStatic = true; 
                    break;
                }
            }
            
            if(HasNonStatic){
                InventoryRect.Y1 -= FontLineHeight(BoldFont);
                
                ta_data *Items = TARoomFindDescription(Room, AssetTag(AssetTag_Items));
                if(Items){
                    f32 L = FontStringAdvance(Font, Items->Data, RectSize(InventoryRect).X).X;
                    DoString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies), 
                             Items->Data, &InventoryRect);
                }else{
                    DoString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies), 
                             "Items:", &InventoryRect);
                }
                
                for(u32 I=0; I<Room->Items.Count; I++){
                    ta_item *Item = HashTableFindPtr(&TA->ItemTable, Room->Items[I]);
                    if(!Item) continue;
                    if(HasTag(Item->Tag, AssetTag_Static)) continue;
                    DoString(Renderer, Font, &Theme->ItemFancy, 1, Item->Name, &InventoryRect);
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
            ta_item *Item = HashTableFindPtr(&TA->ItemTable, TA->Inventory[I]);
            Assert(Item);
            DoString(Renderer, Font, &Theme->ItemFancy, 1, Item->Name, &InventoryRect);
        }
    }
    
    //~ Map
    {
        s32 MapIndex = TAFindItemByTag(TA, &TA->Inventory, AssetTag(AssetTag_Map));
        if(MapIndex >= 0 || true){
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
                color C = MixColor(PINK, WHITE, 0.5f*(Sin(5.0f*Counter)+1.0f));
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
                TADispatchCommand(Mixer, TA, Assets, Tokens, TokenCount);
            }
            Input->BeginTextInput();
        }
    }
}
