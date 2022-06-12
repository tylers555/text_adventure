
#if defined(SNAIL_JUMPY_DEBUG_BUILD)
struct debug_info_display {
    u64 Start;
    debug_info_display(){
        Start = __rdtsc();
    }
    
    ~debug_info_display(){
        asset_font *Font = GetFont(&DEBUG->Assets, AssetID(font_basic));
        v2 DebugP = V2(10, 10);
        {
            u64 Elapsed = __rdtsc() - Start;
            char Buffer[DEFAULT_BUFFER_SIZE];
            DebugP.Y -= FontRenderString(&DEBUG->Renderer, Font, DebugP, WHITE, 
                                         "Counter: %.2f | Cycles: %08llu | FPS: %.2f | Scale: %.1f", 
                                         Counter, Elapsed, 1.0/OSInput.dTime, DEBUG->Renderer.CameraScale);
        }
        
        DebugP = V2(200, 230);
        for(u32 I=0; I<TextAdventure.Ghosts.Count; I++){
            ta_room *Room = HashTableFindPtr(&TextAdventure.RoomTable, TextAdventure.Ghosts[I].CurrentRoom);
            DebugP.Y -= FontRenderString(&DEBUG->Renderer, Font, DebugP, WHITE,
                                         "Ghost[%u] Room: %s", I, Room->Name);
        }
        
#if 0        
        u32 I = 0;
        text_input_history_node *Node = &OSInput.HistorySentinel;
        do{
            DebugP.Y -= FontRenderString(&DEBUG->Renderer, Font, DebugP, PINK, "[%p]: '%.*s' %s", 
                                         Node, Node->BufferLength, Node->Buffer,
                                         (Node == OSInput.CurrentHistoryNode) ? "current" : "");
            Node = Node->Next;
            I++;
        }while((Node != &OSInput.HistorySentinel) || (I > 30));
#endif
    }
};

#define DO_DEBUG_INFO() debug_info_display DebugDisplay##__FUNCTION__
#else
#define DO_DEBUG_INFO()
#endif