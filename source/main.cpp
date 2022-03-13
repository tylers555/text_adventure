// TODO(Tyler): Implement an allocator for the stb libraries
#define STB_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "third_party/stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "third_party/stb_truetype.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "third_party/stb_sprintf.h"

#include "main.h"

//~ Engine variables
global debug_config DebugConfig;

global state_change_data StateChangeData;

global menu_state MenuState;

global string_manager Strings;

global asset_system AssetSystem;

global audio_mixer AudioMixer;

global game_renderer GameRenderer;

global game_settings GameSettings;

global f32 Accumulator;

//~ Gameplay variables
global s32 Score;
global f32 CompletionCooldown;
global world_data *CurrentWorld;

//~ Hotloaded variables file!
// TODO(Tyler): Load this from a variables file at startup
global game_mode GameMode = GameMode_MainGame;

//~ Helpers
internal inline string
String(const char *S){
    return Strings.GetString(S);
}

//~ Includes
#include "logging.cpp"
#include "render.cpp"
#include "stream.cpp"
#include "file_processing.cpp"
#include "wav.cpp"
#include "ui.cpp"
#include "asset.cpp"
#include "asset_loading.cpp"
#include "audio_mixer.cpp"

#include "game.cpp"
#include "menu.cpp"

//~ 

internal void
InitializeGame(){
    stbi_set_flip_vertically_on_load(true);
    
    {
        umw Size = Megabytes(500);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&PermanentStorageArena, Memory, Size);
    }{
        umw Size = Gigabytes(1);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&TransientStorageArena, Memory, Size);
    }
    
    LogFile = OpenFile("log.txt", OpenFile_Write | OpenFile_Clear);
    
    InitializeRendererBackend();
    GameRenderer.Initialize(&PermanentStorageArena, OSInput.WindowSize);
    
    Strings.Initialize(&PermanentStorageArena);
    
    //~ Load things
    LoadedImageTable = PushHashTable<const char *, image>(&PermanentStorageArena, 256);
    AssetSystem.Initialize(&PermanentStorageArena);
    
    AudioMixer.Initialize(&PermanentStorageArena);
    
    AssetSystem.LoadAssetFile(ASSET_FILE_PATH);
    AudioMixer.PlaySound(AssetSystem.GetSoundEffect(String("test_music")), MixerSoundFlag_Music|MixerSoundFlag_Loop, 1.0f);
}

//~

internal inline void
ToggleOverlay(_debug_overlay_flags Overlay){
    if(!(DebugConfig.Overlay & Overlay)) DebugConfig.Overlay |= Overlay;
    else DebugConfig.Overlay &= ~Overlay;
}

internal void 
DoDefaultHotkeys(){
#if defined(SNAIL_JUMPY_DEBUG_BUILD)
    if(OSInput.KeyJustDown(KeyCode_F1, KeyFlag_Any)) ToggleOverlay(DebugOverlay_Miscellaneous);
    if(OSInput.KeyJustDown(KeyCode_F2, KeyFlag_Any)) ToggleOverlay(DebugOverlay_Profiler);
    if(OSInput.KeyJustDown(KeyCode_F3, KeyFlag_Any)) ToggleOverlay(DebugOverlay_Boundaries);
#endif
    
    if(OSInput.KeyJustDown(PAUSE_KEY, KeyFlag_Any) && (GameMode != GameMode_Menu)) OpenPauseMenu();
}

internal void
GameUpdateAndRender(){
    ProfileData.CurrentBlockIndex = 0;
    
    TIMED_FUNCTION();
    
    ArenaClear(&TransientStorageArena);
    GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, MakeColor(0.30f, 0.55f, 0.70f));
    
    OSProcessInput(&OSInput);
    
    switch(GameMode){
        case GameMode_Menu: {
            UpdateAndRenderMenu(&GameRenderer);
        }break;
        case GameMode_MainGame: {
            UpdateAndRenderMainGame(&GameRenderer);
        }break;
    }
    
    RendererRenderAll(&GameRenderer);
    
    AssetSystem.LoadAssetFile(ASSET_FILE_PATH);
    Counter += OSInput.dTime;
    FrameCounter++;
    
    //~ Other
    if(StateChangeData.DidChange){
        switch(StateChangeData.NewMode){
            case GameMode_Menu: {
                GameMode = GameMode_Menu;
            }break;
            case GameMode_MainGame: {
                GameMode = GameMode_MainGame; 
            }break;
        }
        
        StateChangeData = {};
    }
}

internal inline void
ChangeState(game_mode NewMode, string NewLevel){
    StateChangeData.DidChange = true;
    StateChangeData.NewMode = NewMode;
    StateChangeData.NewLevel = Strings.GetString(NewLevel);
}
