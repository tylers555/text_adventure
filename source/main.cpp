
#ifdef DO_RELEASE_BUILD
//#define SNAIL_JUMPY_USE_PROCESSED_ASSETS
#else
#define SNAIL_JUMPY_DEBUG_BUILD
//#define SNAIL_JUMPY_USE_PROCESSED_ASSETS
#endif

#include "main.h"

//~ Engine variables
global game_state *DEBUG;
global u64 DebugInitTime;

global ta_system TextAdventure;
global settings_state SettingsState;

global memory_arena GlobalTickMemory;

global game_mode GameMode = GameMode_None;

//~ Helpers
internal inline string
String(const char *S){
    return Strings.GetString(S);
}

//~ Includes
#include "os.cpp"
#include "logging.cpp"
#include "stream.cpp"
#include "file_processing.cpp"
#include "render.cpp"
#include "wav.cpp"
#include "asset.cpp"
#include "text_adventure.cpp"
#include "asset_loading.cpp"
#include "audio_mixer.cpp"

#include "murkwell.cpp"
#include "commands.cpp"

#include "debug.cpp"
#include "game.cpp"

//~ 

internal void
StateInitialize(game_state *State){
    DEBUG = State;
    u64 Start = OSGetMicroseconds();
    
    {
        umw Size = Gigabytes(1);
        void *Memory = OSVirtualAlloc(Size);
        Assert(Memory);
        InitializeArena(&GlobalPermanentMemory, Memory, Size);
    }
    GlobalTransientMemory = MakeArena(&GlobalPermanentMemory, Megabytes(512));
    GlobalTickMemory      = MakeArena(&GlobalPermanentMemory, Megabytes(256));
    
    InitializeRendererBackend();
    State->Renderer.Initialize(&GlobalPermanentMemory, OSInput.WindowSize);
    State->Renderer.NewFrame(&GlobalTransientMemory, OSInput.WindowSize, PINK);
    State->Mixer.Initialize(&GlobalPermanentMemory);
    
    Strings.Initialize(&GlobalPermanentMemory);
    TextAdventure.Initialize(&State->Assets, &GlobalPermanentMemory);
    State->Assets.Initialize(&GlobalPermanentMemory);
    State->Assets.LoadAssetFile(ASSET_FILE_PATH);
    
    DebugInitTime = OSGetMicroseconds()-Start;
}

//~

internal void 
DoDefaultHotkeys(){
}

internal void
StateDoFrame(game_state *State){
    if(GameMode == GameMode_None){
        const char *S = GetVar(&State->Assets, start_game_mode);
        if(CompareCStrings(S, "game"))      GameMode = GameMode_Game;
        else if(CompareCStrings(S, "menu")) GameMode = GameMode_Menu;
        else                                GameMode = GameMode_Game;
    }
    
    u64 Start = OSGetMicroseconds();
    ArenaClear(&GlobalTransientMemory);
    
    OSProcessInput(&OSInput);
    
    switch(GameMode){
        case GameMode_Game: {
            GameDoFrame(&State->Renderer, &State->Mixer, &State->Assets, &OSInput);
        }break;
    }
    
    RendererRenderAll(&State->Renderer);
    
    State->Assets.LoadAssetFile(ASSET_FILE_PATH);
    Counter += OSInput.dTime;
    FrameCounter++;
    
    if(FrameCounter == 1){
        u64 FirstFrameTime = OSGetMicroseconds()-Start;
        DebugInitTime += FirstFrameTime;
        f32 Time = (f32)DebugInitTime/1000000.0f;
        LogMessage("Time to initialize: %f | First frame: %f", Time, FirstFrameTime);
    }
}
