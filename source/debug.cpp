#if defined(SNAIL_JUMPY_DEBUG_BUILD)

//#define DEBUG_DISPLAY_TEXT_INPUT_UNDO

//~ 
struct debug_scope_time_elapsed {
    u64 Microseconds;
    u64 Cycles;
};

struct debug_scope_timer {
    u64 StartCycle;
    u64 StartTime;
    debug_scope_timer(){
        StartCycle = __rdtsc();
        StartTime = OSGetMicroseconds();
    }
    
    inline debug_scope_time_elapsed GetElapsed() {
        debug_scope_time_elapsed Result = {};
        Result.Cycles = __rdtsc()-StartCycle;
        Result.Microseconds = OSGetMicroseconds()-StartTime;
        return Result;
    }
};

//~ 
typedef u32 debug_display_flags;
enum debug_display_flags_ {
    DebugDisplay_None          = (0 << 0),
    DebugDisplay_Basic         = (1 << 0),
    DebugDisplay_Ghosts        = (1 << 1),
    DebugDisplay_TextInputUndo = (1 << 2),
    DebugDisplay_StaticItems   = (1 << 3),
    DebugDisplay_AssetLoading  = (1 << 4),
};

struct debug_info {
    main_state *State;
    u64 InitTime;
    debug_display_flags DisplayFlags = (DebugDisplay_Basic|DebugDisplay_AssetLoading);
    asset_loading_status AssetLoadingStatus;
    
    inline void Display(debug_scope_time_elapsed Elapsed);
    inline void DoDebugHotkeys();
};

global debug_info DebugInfo;

#define DEBUG_DATA_INITIALIZE(State) debug_info_initializer DEBUG_INFO_INITIALIZER(State);
struct debug_info_initializer {
    u64 StartTime;
    main_state *State;
    debug_info_initializer(main_state *State_){
        State = State_;
        StartTime = OSGetMicroseconds();
    }
    
    ~debug_info_initializer(){
        DebugInfo.State = State;
        DebugInfo.InitTime = OSGetMicroseconds()-StartTime;
    }
};

inline void
debug_info::DoDebugHotkeys(){
    os_input *Input = &State->Input;
    if(Input->KeyJustDown(KeyCode_F1)) DisplayFlags ^= DebugDisplay_Basic;
    if(Input->KeyJustDown(KeyCode_F2)) DisplayFlags ^= DebugDisplay_Ghosts;
    if(Input->KeyJustDown(KeyCode_F3)) DisplayFlags ^= DebugDisplay_TextInputUndo;
    if(Input->KeyJustDown(KeyCode_F4)) DisplayFlags ^= DebugDisplay_StaticItems;
    if(Input->KeyJustDown(KeyCode_F5)) DisplayFlags ^= DebugDisplay_AssetLoading;
}

inline void
debug_info::Display(debug_scope_time_elapsed Elapsed) {
    asset_system *Assets = &State->Assets;
    game_renderer *Renderer = &State->Renderer;
    ta_system *TA = &State->TextAdventure;
    os_input *Input = &State->Input;
    
    
    asset_font *Font = GetFont(Assets, AssetID(font_basic));
    
    v2 DebugP = V2(10, 10);
    if(DisplayFlags & DebugDisplay_Basic){
        DebugP.Y += FontRenderString(Renderer, Font, DebugP, WHITE, 
                                     "Counter: %.2f | Cycles: %08llu | Time: %04llu | FPS: %.2f | Scale: %.1f", 
                                     Counter, Elapsed.Cycles, Elapsed.Microseconds, 1.0/Input->dTime, Renderer->CameraScale);
    }
    
    if(DisplayFlags & DebugDisplay_AssetLoading){
        if(AssetLoadingStatus == AssetLoadingStatus_Warnings){
            DebugP.Y += FontRenderString(Renderer, Font, DebugP, WHITE, 
                                         "Asset loader warnings!", 
                                         Counter, Elapsed.Cycles, Elapsed.Microseconds, 1.0/Input->dTime, Renderer->CameraScale);
        }
    }
    
    DebugP = V2(200, 230);
    if(DisplayFlags & DebugDisplay_Ghosts){
        FOR_EACH_PTR_(Ghost, Index, &TA->Ghosts){
            ta_room *Room = TA->FindRoom(Ghost->CurrentRoom);
            DebugP.Y -= FontRenderString(Renderer, Font, DebugP, WHITE,
                                         "Ghost[%u] Room: %s", Index, Room->NameData.Name);
        }
    }
    
    if(DisplayFlags & DebugDisplay_TextInputUndo){
        text_input_context *Context = Input->TextInput;
        if(!Context) return;
        
        u32 I = 0;
        text_input_history_node *Node = &Context->HistorySentinel;
        do{
            DebugP.Y -= FontRenderString(Renderer, Font, DebugP, WHITE, "%s [%p]: '%.*s'", 
                                         (Node == Context->CurrentHistoryNode) ? ">" : " ",
                                         Node, Node->BufferLength, Node->Buffer);
            
            Node = Node->Next;
            I++;
        }while((Node != &Context->HistorySentinel) || (I > 30));
    }
    
    if(DisplayFlags & DebugDisplay_StaticItems){
        ta_room *Room = TA->CurrentRoom;
        FOR_EACH(ItemID, &Room->Items){
            ta_item *Item = TA->FindItem(ItemID);
            if(HasTag(Item->Tag, AssetTag_Static)){
                DebugP.Y -= FontRenderString(Renderer, Font, DebugP, WHITE, "%s", Item->NameData.Name);
            }
        }
    }
}

//~ 
struct debug_info_display {
    debug_scope_timer Timer;
    debug_info_display() : Timer() {
        DebugInfo.DoDebugHotkeys();
    }
    
    ~debug_info_display(){ 
        DebugInfo.Display(Timer.GetElapsed()); 
    }
};

#define DO_DEBUG_INFO() debug_info_display DebugDisplay##__FUNCTION__
#else
#define DO_DEBUG_INFO()
#define DEBUG_DATA_INITIALIZE()
#endif