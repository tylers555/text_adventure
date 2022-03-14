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

global game_renderer GameRenderer;

global audio_mixer AudioMixer;

global game_settings GameSettings;

global f32 Accumulator;

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
    //AudioMixer.PlaySound(AssetSystem.GetSoundEffect(String("test_music")), MixerSoundFlag_Music|MixerSoundFlag_Loop, 1.0f);
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
    GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, MakeColor(0.0f, 0.0f, 0.0f));
    
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

//~ Text input
inline void
os_input::DeleteFromBuffer(u32 Begin, u32 End){
    MoveMemory(&Buffer[Begin],
               &Buffer[End], BufferLength-(End-1));
    u32 Size = End-Begin;
    ZeroMemory(&Buffer[BufferLength-Size], Size);
    CursorPosition = Begin;
}

inline u32 
os_input::SeekForward(u32 Start){
    u32 Result=Start;
    b8 HitAlphabetic = false;
    for(u32 I=Start; I<=BufferLength; I++){
        char C = Buffer[I];
        Result = I;
        if(IsWhiteSpace(C)){
            if(HitAlphabetic) break;
        }else HitAlphabetic = true;
    }
    
    return Result;
}

inline u32 
os_input::SeekBackward(u32 Start){
    u32 Result = Start;
    b8 HitAlphabetic = false;
    for(s32 I=CursorPosition-1; I>=0; I--){
        char C = Buffer[I];
        if(IsWhiteSpace(C)){
            if(HitAlphabetic) break;
        }else HitAlphabetic = true;
        Result = I;
    }
    
    return Result;
}

inline void
os_input::AddToBuffer(os_key_code Key){
    if(!DoTextInput) return;
    
    if(Key == KeyCode_NULL){
    }else if(Key < U8_MAX){
        
        char Char = (char)Key;
        if(('A' <= Char) && (Char <= 'Z')){
            Char += 'a'-'A';
        }
        if(OSInput.KeyFlags & KeyFlag_Shift){
            Char = KEYBOARD_SHIFT_TABLE[Char];
        }
        Assert(Char);
        
        if(SelectionMark >= 0){
            u32 Begin = Minimum(CursorPosition, (u32)SelectionMark);
            u32 End   = Maximum(CursorPosition, (u32)SelectionMark);
            DeleteFromBuffer(Begin, End);
            SelectionMark = -1;
        }
        
        if(BufferLength < DEFAULT_BUFFER_SIZE-1){
            MoveMemory(&Buffer[CursorPosition+1],
                       &Buffer[CursorPosition],
                       BufferLength-CursorPosition);
            
            Buffer[CursorPosition++] = Char;
            BufferLength++;
        }
        
    }else if(Key == KeyCode_BackSpace){
        if(SelectionMark >= 0){
            u32 Begin = Minimum(CursorPosition, (u32)SelectionMark);
            u32 End   = Maximum(CursorPosition, (u32)SelectionMark);
            SelectionMark = -1;
            DeleteFromBuffer(Begin, End);
        }else if(CursorPosition > 0){
            u32 Begin = CursorPosition-1;
            u32 End = CursorPosition;
            if(TestModifier(KeyFlag_Control|KeyFlag_Any)){
                Begin = SeekBackward(CursorPosition);
            }
            DeleteFromBuffer(Begin, End);
        }
    }else if(Key == KeyCode_Delete){
        u32 Begin = CursorPosition;
        u32 End = CursorPosition+1;
        if(SelectionMark >= 0){
            Begin = Minimum(CursorPosition, (u32)SelectionMark);
            End   = Maximum(CursorPosition, (u32)SelectionMark);
            SelectionMark = -1;
        }else if(TestModifier(KeyFlag_Control|KeyFlag_Any)){
            End = SeekForward(CursorPosition);
        }
        DeleteFromBuffer(Begin, End);
    }else if(Key == KeyCode_Left){
        if(!TestModifier(KeyFlag_Shift|KeyFlag_Any)){
            SelectionMark = -1;
        }else if(SelectionMark < 0){
            SelectionMark = CursorPosition;
        }
        
        if(TestModifier(KeyFlag_Control|KeyFlag_Any)){
            CursorPosition = SeekBackward(CursorPosition);
        }else if(CursorPosition > 0){
            CursorPosition--;
        }
    }else if(Key == KeyCode_Right){
        if(!TestModifier(KeyFlag_Shift|KeyFlag_Any)){
            SelectionMark = -1;
        }else if(SelectionMark < 0){
            SelectionMark = CursorPosition;
        }
        
        if(TestModifier(KeyFlag_Control|KeyFlag_Any)){
            CursorPosition = SeekForward(CursorPosition);
        }else{
            CursorPosition++;
        }
    }else if(Key == KeyCode_Home){
        if(!TestModifier(KeyFlag_Shift|KeyFlag_Any)){
            SelectionMark = -1;
        }else if(SelectionMark < 0){
            SelectionMark = CursorPosition;
        }
        
        CursorPosition = 0;
    }else if(Key == KeyCode_End){
        if(!TestModifier(KeyFlag_Shift|KeyFlag_Any)){
            SelectionMark = -1;
        }else if(SelectionMark < 0){
            SelectionMark = CursorPosition;
        }
        
        CursorPosition = BufferLength;
    }
    
    CursorPosition = Minimum(CursorPosition, BufferLength);
}

inline void
os_input::BeginTextInput(){
    ZeroMemory(Buffer, DEFAULT_BUFFER_SIZE);
    BufferLength = 0;
    SelectionMark = -1;
    CursorPosition = 0;
    DoTextInput = true;
}

inline void
os_input::EndTextInput(){
    DoTextInput = false;
}
