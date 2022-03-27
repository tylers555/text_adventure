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
global state_change_data StateChangeData;

global menu_state MenuState;

global string_manager Strings;

global asset_system AssetSystem;

global game_renderer GameRenderer;

global audio_mixer AudioMixer;

global ta_system TextAdventure;

global u64 DebugInitTime;

global game_mode GameMode = GameMode_None;

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
#include "asset.cpp"
#include "asset_loading.cpp"
#include "audio_mixer.cpp"
#include "text_adventure.cpp"
#include "commands.cpp"

#include "debug.cpp"
#include "game.cpp"
#include "map.cpp"
#include "menu.cpp"

//~ 

internal void
InitializeGame(){
    u64 Start = OSGetMicroseconds();
    
    stbi_set_flip_vertically_on_load(true);
    {
        umw Size = Megabytes(256);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&PermanentStorageArena, Memory, Size);
    }{
        umw Size = Megabytes(512);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&TransientStorageArena, Memory, Size);
    }
    InitializeRendererBackend();
    GameRenderer.Initialize(&PermanentStorageArena, OSInput.WindowSize);
    
    
    Strings.Initialize(&PermanentStorageArena);
    
    TextAdventure.Initialize(&PermanentStorageArena);
    
    //~ Load things
    LoadedImageTable = PushHashTable<const char *, image>(&PermanentStorageArena, 256);
    AssetSystem.Initialize(&PermanentStorageArena);
    
    AudioMixer.Initialize(&PermanentStorageArena);
    AssetSystem.LoadAssetFile(ASSET_FILE_PATH);
    //AudioMixer.PlaySound(AssetSystem.GetSoundEffect(String("test_music")), MixerSoundFlag_Music|MixerSoundFlag_Loop, 1.0f);
    
    GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, PINK);
    DebugInitTime = OSGetMicroseconds()-Start;
}

//~

internal void 
DoDefaultHotkeys(){
    if(OSInput.KeyJustDown(PAUSE_KEY, KeyFlag_Any) && (GameMode != GameMode_Menu)) OpenPauseMenu();
}

internal void
GameUpdateAndRender(){
    if(GameMode == GameMode_None){
        GameMode = GameMode_MainGame;
    }
    
    u64 Start = OSGetMicroseconds();
    ArenaClear(&TransientStorageArena);
    
    OSProcessInput(&OSInput);
    
    switch(GameMode){
        case GameMode_Menu: {
            UpdateAndRenderMenu(&GameRenderer);
        }break;
        case GameMode_MainGame: {
            UpdateAndRenderMainGame(&GameRenderer, &AudioMixer, &AssetSystem, &OSInput);
        }break;
        case GameMode_Map: {
            UpdateAndRenderMap(&GameRenderer, &AudioMixer, &AssetSystem, &OSInput);
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
            case GameMode_Map: {
                GameMode = GameMode_Map;
            }break;
        }
        
        StateChangeData = {};
    }
    
    if(FrameCounter == 1){
        u64 FirstFrameTime = OSGetMicroseconds()-Start;
        DebugInitTime += FirstFrameTime;
        f32 Time = (f32)DebugInitTime/1000000.0f;
        LogMessage("Time to initialize: %f | First frame: %f", Time, FirstFrameTime);
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

inline void
os_input::AddToBuffer(os_key_code Key){
    if(!(InputFlags & OSInputFlag_DoTextInput)) return;
    
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
                Begin = SeekBackward(Buffer, CursorPosition);
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
            End = SeekForward(Buffer, BufferLength, CursorPosition);
        }
        DeleteFromBuffer(Begin, End);
    }else if(Key == KeyCode_Left){
        if(!TestModifier(KeyFlag_Shift|KeyFlag_Any)){
            SelectionMark = -1;
        }else if(SelectionMark < 0){
            SelectionMark = CursorPosition;
        }
        
        if(TestModifier(KeyFlag_Control|KeyFlag_Any)){
            CursorPosition = SeekBackward(Buffer, CursorPosition);
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
            CursorPosition = SeekForward(Buffer, BufferLength, CursorPosition);
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
    }else if(Key == KeyCode_Return){
        InputFlags |= OSInputFlag_EndTextInput;
    }else if(Key == KeyCode_Escape){
        SelectionMark = -1;
    }
    
    BufferLength = CStringLength(Buffer);
    CursorPosition = Minimum(CursorPosition, BufferLength);
}

inline void
os_input::BeginTextInput(){
    ZeroMemory(Buffer, DEFAULT_BUFFER_SIZE);
    BufferLength = 0;
    SelectionMark = -1;
    CursorPosition = 0;
    InputFlags |= OSInputFlag_DoTextInput;
}

inline b8
os_input::MaybeEndTextInput(){
    b8 Result = (InputFlags & OSInputFlag_EndTextInput);
    if(Result){
        InputFlags &= !OSInputFlag_DoTextInput;
    }
    return Result;
}
