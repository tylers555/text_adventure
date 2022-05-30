
#ifdef DO_RELEASE_BUILD
#define SNAIL_JUMPY_USE_PROCESSED_ASSETS
#else
#define SNAIL_JUMPY_DEBUG_BUILD
//#define SNAIL_JUMPY_USE_PROCESSED_ASSETS
#endif

#include "main.h"

//~ Engine variables
global game_state *DEBUG;
global u64 DebugInitTime;

global menu_state MenuState;
global ta_system TextAdventure;

global game_mode GameMode = GameMode_None;

//~ Helpers
internal inline string
String(const char *S){
    return Strings.GetString(S);
}

//~ Includes
#include "logging.cpp"
#include "stream.cpp"
#include "file_processing.cpp"
#include "render.cpp"
#include "wav.cpp"
#include "asset.cpp"
#include "text_adventure.cpp"
#include "asset_loading.cpp"
#include "audio_mixer.cpp"
#include "commands.cpp"

#include "debug.cpp"
#include "game.cpp"
#include "map.cpp"
#include "menu.cpp"

//~ 

internal void
InitializeGame(game_state *State){
    DEBUG = State;
    u64 Start = OSGetMicroseconds();
    
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
    State->Renderer.Initialize(&PermanentStorageArena, OSInput.WindowSize);
    State->Renderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, PINK);
    State->Mixer.Initialize(&PermanentStorageArena);
    
    Strings.Initialize(&PermanentStorageArena);
    TextAdventure.Initialize(&PermanentStorageArena);
    State->Assets.Initialize(&PermanentStorageArena);
    
    State->Assets.LoadAssetFile(ASSET_FILE_PATH);
    
    DebugInitTime = OSGetMicroseconds()-Start;
    
}

//~

internal void 
DoDefaultHotkeys(){
    if(OSInput.KeyJustDown(PAUSE_KEY, KeyFlag_Any) && (GameMode != GameMode_Menu)) OpenPauseMenu();
}

internal void
GameUpdateAndRender(game_state *State){
    if(GameMode == GameMode_None){
        const char *S = GetVar(&State->Assets, start_game_mode);
        if(CompareStrings(S, "main game")) GameMode = GameMode_MainGame;
        else if(CompareStrings(S, "menu")) GameMode = GameMode_Menu;
        else                               GameMode = GameMode_MainGame;
    }
    
    u64 Start = OSGetMicroseconds();
    ArenaClear(&TransientStorageArena);
    
    OSProcessInput(&OSInput);
    
    switch(GameMode){
        case GameMode_Menu: {
            UpdateAndRenderMenu(&State->Renderer);
        }break;
        case GameMode_MainGame: {
            UpdateAndRenderMainGame(&State->Renderer, &State->Mixer, &State->Assets, &OSInput);
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

inline void
os_input::EndTextInput(){
    InputFlags &= !OSInputFlag_DoTextInput;
}
