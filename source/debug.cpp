
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
    }
};

#define DO_DEBUG_INFO() debug_info_display DebugDisplay##__FUNCTION__
#else
#define DO_DEBUG_INFO()
#endif