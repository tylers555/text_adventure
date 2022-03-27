
struct debug_info_display {
    u64 Start;
    debug_info_display(){
        Start = __rdtsc();
    }
    
    ~debug_info_display(){
        asset_font *Font = AssetSystem.GetFont(String("basic"));
        v2 DebugP = V2(10, 10);
        {
            u64 Elapsed = __rdtsc() - Start;
            char Buffer[DEFAULT_BUFFER_SIZE];
            DebugP.Y -= FontRenderString(&GameRenderer, Font, DebugP, WHITE, 
                                         "Counter: %.2f | Cycles: %08llu | FPS: %.2f | Scale: %.1f", 
                                         Counter, Elapsed, 1.0/OSInput.dTime, GameRenderer.CameraScale);
        }
    }
};

#define DO_DEBUG_INFO() debug_info_display DebugDisplay##__FUNCTION__
