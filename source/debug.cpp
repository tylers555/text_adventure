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
struct debug_info {
    game_state *State;
    u64 InitTime;
    
    inline void Display(debug_scope_time_elapsed Elapsed);
};

global debug_info DebugInfo;

#define DEBUG_DATA_INITIALIZE(State) debug_info_initializer DEBUG_INFO_INITIALIZER(State);
struct debug_info_initializer {
    u64 StartTime;
    game_state *State;
    debug_info_initializer(game_state *State_){
        State = State_;
        StartTime = OSGetMicroseconds();
    }
    
    ~debug_info_initializer(){
        DebugInfo.State = State;
        DebugInfo.InitTime = OSGetMicroseconds()-StartTime;
    }
};

inline void
debug_info::Display(debug_scope_time_elapsed Elapsed) {
    asset_system *Assets = &State->Assets;
    game_renderer *Renderer = &State->Renderer;
    ta_system *TA = &State->TextAdventure;
    os_input *Input = &State->Input;
    
    asset_font *Font = GetFont(Assets, AssetID(font_basic));
    v2 DebugP = V2(10, 10);
    {
        char Buffer[DEFAULT_BUFFER_SIZE];
        DebugP.Y -= FontRenderString(Renderer, Font, DebugP, WHITE, 
                                     "Counter: %.2f | Cycles: %08llu | FPS: %.2f | Scale: %.1f", 
                                     Counter, Elapsed.Cycles, 1.0/Input->dTime, Renderer->CameraScale);
    }
    
    DebugP = V2(200, 230);
    for(u32 I=0; I<TA->Ghosts.Count; I++){
        ta_room *Room = TA->FindRoom(TA->Ghosts[I].CurrentRoom);
        DebugP.Y -= FontRenderString(Renderer, Font, DebugP, WHITE,
                                     "Ghost[%u] Room: %s", I, Room->Name);
    }
    
    //~ @debug_text_input_undo
#ifdef DEBUG_DISPLAY_TEXT_INPUT_UNDO
    text_input_context *Context = OSInput.TextInput;
    if(!Context) return;
    
    u32 I = 0;
    text_input_history_node *Node = &Context->HistorySentinel;
    do{
        DebugP.Y -= FontRenderString(Renderer, Font, DebugP, PINK, "%s [%p]: '%.*s'", 
                                     (Node == Context->CurrentHistoryNode) ? ">" : " ",
                                     Node, Node->BufferLength, Node->Buffer);
        
        Node = Node->Next;
        I++;
    }while((Node != &Context->HistorySentinel) || (I > 30));
#endif
}

//~ 
struct debug_info_display {
    debug_scope_timer Timer;
    debug_info_display() : Timer() {
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