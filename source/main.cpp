
#ifdef DO_RELEASE_BUILD
//#define SNAIL_JUMPY_USE_PROCESSED_ASSETS
#else
#define SNAIL_JUMPY_DEBUG_BUILD
//#define SNAIL_JUMPY_USE_PROCESSED_ASSETS
#endif

#include "main.h"

//~ Engine variables
global settings_state SettingsState;

global memory_arena GlobalTickMemory;

//~ Includes
#include "os.cpp"
#include "logging.cpp"
#include "stream.cpp"
#include "file_processing.cpp"
#include "render.cpp"
#include "wav.cpp"
#include "asset.cpp"
#include "debug.cpp"
#include "text_adventure.cpp"
#include "asset_loading.cpp"
#include "audio_mixer.cpp"

#include "murkwell.cpp"
#include "commands.cpp"

#include "game.cpp"

//~ 
internal void
MainStateInitialize(main_state *State){
    DEBUG_DATA_INITIALIZE(State);
    
    {
        umw Size = Gigabytes(1);
        void *Memory = OSVirtualAlloc(Size);
        Assert(Memory);
        InitializeArena(&GlobalPermanentMemory, Memory, Size);
    }
    GlobalTransientMemory = MakeArena(&GlobalPermanentMemory, Megabytes(256));
    GlobalTickMemory      = MakeArena(&GlobalPermanentMemory, Megabytes(256));
    
    State->Renderer.Initialize(&GlobalPermanentMemory, State->Input.WindowSize);
    State->Renderer.NewFrame(&State->Input, &GlobalTransientMemory, State->Input.WindowSize);
    State->Mixer.Initialize(&GlobalPermanentMemory);
    
    Strings.Initialize(&GlobalPermanentMemory);
    State->TextAdventure.Initialize(&State->Assets, &GlobalPermanentMemory);
    State->Assets.Initialize(&GlobalPermanentMemory);
    
    State->AssetLoader.Initialize(&GlobalPermanentMemory, &State->Mixer, &State->Assets, &State->TextAdventure);
    State->AssetLoader.LoadAssetFile(ASSET_FILE_PATH);
}

//~
internal void
MainStateDoFrame(main_state *State){
    ArenaClear(&GlobalTransientMemory);
    
    OSProcessInput(&State->Input);
    
    asset_loading_status LoadingStatus = State->AssetLoader.LoadAssetFile(ASSET_FILE_PATH);
    DebugInfo.AssetLoadingStatus = LoadingStatus;
    
    if(LoadingStatus != AssetLoadingStatus_Errors){
        State->Renderer.NewFrame(&State->Input, &GlobalTransientMemory, State->Input.WindowSize);
        
        GameDoFrame(&State->Renderer, &State->Mixer, &State->Assets, &State->Input, &State->TextAdventure);
        
        RendererBackendRenderAll(&State->Renderer);
    }
    
    Counter += State->Input.dTime;
    FrameCounter++;
}
