
//~ Text adventure system
void
ta_system::Initialize(asset_system *Assets, memory_arena *Arena){
    AssetTableInit(Room, Arena, ROOM_TABLE_SIZE);
    AssetTableInit(Item, Arena, ITEM_TABLE_SIZE);
    
    Inventory = MakeArray<asset_id>(Arena, INVENTORY_ITEM_COUNT);
    
    CommandMemory = MakeArena(Arena, Megabytes(1));
    CommandStack = MakeStack<const char *>(Arena, 512);
    
    Memory = &Assets->Memory;
    
    ResponseBuilder = BeginStringBuilder(&GlobalTickMemory, DEFAULT_BUFFER_SIZE);
    DLIST_INIT(&EditingCommandSentinel);
    EditingCommandSentinel.Context.Initialize(&GlobalTickMemory);
    CurrentEditingCommand = &EditingCommandSentinel;
    
    //~ Game specific data
    HauntedItems = MakeArray<asset_id>(Arena, 16);
    Ghosts = MakeArray<entity_ghost>(Arena, 16);
    OrganState = AssetTag_Broken;
    PrayStatus = PrayStatus_None;
}

inline void
ta_system::DispatchCommand(audio_mixer *Mixer, asset_system *Assets, word_array Tokens){
    ta_command_func *Func = 0;
    f32 HighestMatch = 0.0f;
    for(u32 I=0; I < Tokens.Count; I++){
        char *Word = Tokens[I];
        CStringMakeLower(Word);
        
        // TODO(Tyler): This should try running the command and if it fails,
        // go for the next closest match.
#define TA_COMMAND(Name, Command) { \
f32 Match = CompareWordsPercentage(Word, Name); \
if(Match > HighestMatch){ \
Func = Command; \
HighestMatch = Match; \
} \
}
        
        //-
        TA_COMMAND("go",       CommandMove);
        TA_COMMAND("move",     CommandMove);
        TA_COMMAND("exit",     CommandExit);
        TA_COMMAND("leave",    CommandExit);
        TA_COMMAND("enter",    CommandEnter);
        TA_COMMAND("take",     CommandTake);
        TA_COMMAND("pick",     CommandTake);
        TA_COMMAND("grab",     CommandTake);
        TA_COMMAND("drop",     CommandDrop);
        TA_COMMAND("leave",    CommandDrop);
        TA_COMMAND("buy",      CommandBuy);
        TA_COMMAND("purchase", CommandBuy);
        TA_COMMAND("eat",      CommandEat);
        TA_COMMAND("consume",  CommandEat);
        TA_COMMAND("ingest",   CommandEat);
        TA_COMMAND("swallow",  CommandEat);
        TA_COMMAND("bite",     CommandEat);
        TA_COMMAND("munch",    CommandEat);
        TA_COMMAND("play",     CommandPlay);
        TA_COMMAND("examine",  CommandExamine);
        TA_COMMAND("inspect",  CommandExamine);
        TA_COMMAND("observe",  CommandExamine);
        TA_COMMAND("look",     CommandExamine);
        TA_COMMAND("see",      CommandExamine);
        TA_COMMAND("unlock",   CommandUnlock);
        TA_COMMAND("repair",   CommandRepair);
        TA_COMMAND("fix",      CommandRepair);
        TA_COMMAND("use",      CommandUse);
        TA_COMMAND("read",     CommandRead);
        
        //- Game commands
        TA_COMMAND("pray",     CommandPray);
        TA_COMMAND("feed",     CommandFeed);
        
        TA_COMMAND("perform",  CommandPerform);
        TA_COMMAND("setup",    CommandSetupCandles);
        TA_COMMAND("prepare",  CommandSetupCandles);
        TA_COMMAND("place",    CommandSetupCandles);
        
        //- Testing commands
        TA_COMMAND("testaddmoney",      CommandTestAddMoney);
        TA_COMMAND("testsubmoney",      CommandTestSubMoney);
        TA_COMMAND("testcarillonpages", CommandTestCarillonPages);
        
        //- Meta commands
        TA_COMMAND("music", CommandMusic);
        TA_COMMAND("undo",  CommandUndo);
        TA_COMMAND("redo",  CommandRedo);
#undef TA_COMMAND
        
        if(HighestMatch > WORD_MATCH_THRESHOLD){
            (*Func)(Mixer, this, Assets, Tokens);
            return;
        }
    }
    
    Respond(GetVar(Assets, invalid_command));
}

//~ 
internal inline b8
DoDisplayItem(ta_item *Item){
    b8 Result = !(HasTag(Item->Tag, AssetTag_Static));
    return Result;
}

internal inline void
RenderTextInput(game_renderer *Renderer, console_theme *Theme, asset_font *Font, 
                text_input_context *Input, rect *InputRect){
    char *Text = Input->Buffer;
    f32 LineHeight = Font->Height+FONT_VERTICAL_SPACE;
    f32 TotalWidth = RectWidth(*InputRect);
    v2 InputP = V2(InputRect->X0, InputRect->Y1);
    
    DoString(Renderer, Font, &Theme->BasicFancy, 1, Text, InputRect);
    
    f32 CursorHeight = Font->Height-Font->Descent;
    v2 CursorP = InputP+FontStringAdvance(Font, Input->CursorPosition, Text, TotalWidth);
    if((1 + (FrameCounter / 30)) % 3){
        RenderLine(Renderer, CursorP, CursorP+V2(0, CursorHeight), 0.0, 1, Theme->CursorColor);
    }
    
    //- Text input selection
    if(Input->SelectionMark < 0) return;
    range_s32 Range = Input->GetSelectionRange();
    if(RangeSize(Range) == 0) return;
    
    font_string_metrics Metrics = FontStringMetricsRange(Font, Range, Text, TotalWidth);
    v2 StartP = V2(InputP.X, InputP.Y-Font->Descent);
    
    if(Metrics.LineCount == 0){
        f32 Width = Metrics.Advance.X-Metrics.StartAdvance.X;
        RenderRect(Renderer, SizeRect(StartP+Metrics.StartAdvance, V2(Width, LineHeight)), 
                   1.0, Theme->SelectionColor);
    }else{
        f32 FirstWidth = Metrics.LineWidths[0]-Metrics.StartAdvance.X;
        RenderRect(Renderer, SizeRect(StartP+Metrics.StartAdvance, 
                                      V2(FirstWidth, LineHeight)), 1.0, Theme->SelectionColor);
        for(u32 I=1; I<Metrics.LineCount; I++){
            RenderRect(Renderer, SizeRect(V2(StartP.X, StartP.Y-(LineHeight)*I), 
                                          V2(Metrics.LineWidths[I], LineHeight)), 1.0, Theme->SelectionColor);
        }
        RenderRect(Renderer, SizeRect(V2(StartP.X, StartP.Y+Metrics.Advance.Y), 
                                      V2(Metrics.Advance.X, LineHeight)), 1.0, Theme->SelectionColor);
    }
}

//~ 
internal void
GameDoFrame(game_renderer *Renderer, audio_mixer *Mixer, asset_system *Assets, os_input *Input, ta_system *TA){
    DO_DEBUG_INFO();
    
    console_theme *Theme = AssetsFind_(Assets, Theme, GetVarAsset(Assets, theme));
    if(!Theme){
        DebugInfo.SubmitMessage(DebugMessage_PerFrame, 
                                "Theme: \"%s\" does not exist does not exist!", GetVar(Assets, theme));
        return;
    }
    Renderer->ClearColor = Theme->BackgroundColor;
    
    if(!TA->CurrentRoom){
        Input->BeginTextInput(&TA->EditingCommandSentinel.Context);
        TA->CurrentRoom = HashTableFindPtr(&TA->RoomTable, GetVarTAID(Assets, start_room));
        if(!TA->CurrentRoom){
            if(!TA->CurrentRoom){
                DebugInfo.SubmitMessage(DebugMessage_PerFrame, 
                                        "The current theme's title font(%s) does not exist does not exist!",
                                        GetVar(Assets, start_room));
                return;
            }else{
                LogMessage("Room: '%s' does not exist!", GetVar(Assets, start_room));
            }
        }
        TA->Money = 10;
    }
    
    asset_font *BoldFont = AssetsFind_(Assets, Font, Theme->TitleFont);
    asset_font *Font = AssetsFind_(Assets, Font, Theme->BasicFont);
    if(!BoldFont){
        DebugInfo.SubmitMessage(DebugMessage_PerFrame, 
                                "The current theme's title font(%s) does not exist does not exist!",
                                AssetIDName(Item, Theme->TitleFont));
        return;
    }
    if(!Font){
        DebugInfo.SubmitMessage(DebugMessage_PerFrame, 
                                "The current theme's basic font(%s) does not exist does not exist!",
                                AssetIDName(Item, Theme->BasicFont));
        return;
    }
    
    v2 WindowSize = V2Round(Renderer->ScreenToWorld(Input->WindowSize));
    
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
        DoString(Renderer, BoldFont, &Theme->RoomTitleFancy, 1, Room->NameData.Name, &RoomDescriptionRect);
        
        if(Room->Datas.Count > 0){
            const char *String = MurkwellOverrideRoomDescription(TA, Assets, Room);
            if(!String) String = Room->Datas[0]->Data;
            
            DoString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies), 
                     String, &RoomDescriptionRect);
            
            ta_data *Adjacents = TA->FindDescription(&Room->Datas, AssetTag(AssetTag_Adjacents));
            if(Adjacents){
                DoString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies), 
                         Adjacents->Data, &RoomDescriptionRect);
            }
            
            const char *Additional = MurkwellAdditionalRoomDescription(TA, Assets, Room);
            if(Additional) DoString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies), 
                                    Additional, &RoomDescriptionRect);
        }
    }
    
    //~ Inventory
    {
        if(Room->Items.Count > 0){
            b8 HasDisplayedItems = false;
            for(u32 I=0; I<Room->Items.Count; I++){
                ta_item *Item = HashTableFindPtr(&TA->ItemTable, Room->Items[I]);
                if(!Item) continue;
                if(DoDisplayItem(Item)){
                    HasDisplayedItems = true; 
                    break;
                }
            }
            
            if(HasDisplayedItems){
                InventoryRect.Y1 -= FontLineHeight(BoldFont);
                
                ta_data *Items = TA->FindDescription(&Room->Datas, AssetTag(AssetTag_Items));
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
                    if(!DoDisplayItem(Item)) continue;
                    DoString(Renderer, Font, &Theme->ItemFancy, 1, Item->NameData.Name, &InventoryRect);
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
            DoString(Renderer, Font, &Theme->ItemFancy, 1, Item->NameData.Name, &InventoryRect);
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
                color C = ColorMix(PINK, WHITE, 0.5f*(Sin(5.0f*Counter)+1.0f));
                RenderRect(Renderer, Point, -1.0, C);
            }
        }
    }
    
    //~ Text input
    {
        //- Previous commmands
        if(Input->KeyJustDown(KeyCode_Up, KeyFlag_Any)){
            TA->EditingCommandCycleUp(Input);
        }else if(Input->KeyJustDown(KeyCode_Down, KeyFlag_Any)){
            TA->EditingCommandCycleDown(Input);
        }
        
        const char *Response = TA->ResponseBuilder.Buffer;
        DoString(Renderer, Font, Theme->ResponseFancies, ArrayCount(Theme->ResponseFancies),
                 Response, &InputRect);
        
        RenderTextInput(Renderer, Theme, Font, &TA->CurrentEditingCommand->Context, &InputRect);
        
        //- Debug
        if(Input->KeyRepeat(KeyCode_F1)){
            MurkwellTick(TA, Assets);
        }
        
        //- Command processing
        if(Input->MaybeEndTextInput()){
            array<char *> Tokens = TA->EndCommand();
            
            if(TA->Callback){
                // NOTE(Tyler): Weird dance, so that the callback can set another callback.
                ta_command_func *Callback = TA->Callback;
                TA->Callback = 0;
                (*Callback)(Mixer, TA, Assets, Tokens);
            }else{
                MurkwellTick(TA, Assets);
                TA->DispatchCommand(Mixer, Assets, Tokens);
            }
            
            Input->BeginTextInput(&TA->EditingCommandSentinel.Context);
        }
    }
}