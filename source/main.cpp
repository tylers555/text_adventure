
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
global settings_state SettingsState;

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
InitializeState(game_state *State){
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
UpdateAndRenderState(game_state *State){
    if(GameMode == GameMode_None){
        const char *S = GetVar(&State->Assets, start_game_mode);
        if(CompareStrings(S, "game")) GameMode = GameMode_Game;
        else if(CompareStrings(S, "menu")) GameMode = GameMode_Menu;
        else                               GameMode = GameMode_Game;
    }
    
    u64 Start = OSGetMicroseconds();
    ArenaClear(&TransientStorageArena);
    
    OSProcessInput(&OSInput);
    
    switch(GameMode){
        case GameMode_Menu: {
            UpdateAndRenderMenu(&State->Renderer);
        }break;
        case GameMode_Game: {
            UpdateAndRenderGame(&State->Renderer, &State->Mixer, &State->Assets, &OSInput);
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
os_input::DeleteFromBuffer(s32 Begin, s32 End){
    if(Begin < 0) return;
    MoveMemory(&Buffer[Begin],
               &Buffer[End], BufferLength-(End-1));
    u32 Size = End-Begin;
    ZeroMemory(&Buffer[BufferLength-Size], Size);
    CursorPosition = Begin;
}

inline b8
os_input::TryDeleteSelection(){
    if(SelectionMark >= 0){
        s32 Begin = Minimum(CursorPosition, SelectionMark);
        s32 End   = Maximum(CursorPosition, SelectionMark);
        DeleteFromBuffer(Begin, End);
        SelectionMark = -1;
        return true;
    }
    
    return false;
}

inline void
os_input::MaybeSetSelection(){
    if(!TestModifier(KeyFlag_Shift|KeyFlag_Any)){
        SelectionMark = -1;
    }else if(SelectionMark < 0){
        SelectionMark = CursorPosition;
    }
}

inline u32
os_input::InsertCharsToBuffer(u32 Position, char *Chars, u32 CharCount){
    if(BufferLength+CharCount < DEFAULT_BUFFER_SIZE){
        MoveMemory(&Buffer[Position+CharCount],
                   &Buffer[Position],
                   BufferLength-Position);
        CopyMemory(&Buffer[Position], Chars, CharCount);
        BufferLength++;
        return CharCount;
    }
    return 0;
}

inline void
os_input::AssembleBuffer(os_key_code Key){
    if(!(InputFlags & OSInputFlag_DoTextInput)) return;
    
    if(Key == KeyCode_NULL){
    }else if(Key < U8_MAX){
        char Char = (char)Key;
        
        if((Char == 'C') && TestModifier(KeyFlag_Control|KeyFlag_Any)){ //- Copy
            if(SelectionMark >= 0){
                s32 Begin = Minimum(CursorPosition, SelectionMark);
                s32 End   = Maximum(CursorPosition, SelectionMark);
                OSCopyChars(&Buffer[Begin], End-Begin);
            }
            
        }else if((Char == 'V') && TestModifier(KeyFlag_Control|KeyFlag_Any)){ //- Paste
            TryDeleteSelection();
            char *ToPaste = OSPasteChars(&TransientStorageArena);
            InsertCharsToBuffer(CursorPosition, ToPaste, CStringLength(ToPaste));
            
        }else if((Char == 'X') && TestModifier(KeyFlag_Control|KeyFlag_Any)){ //- Cut
            if(SelectionMark >= 0){
                s32 Begin = Minimum(CursorPosition, SelectionMark);
                s32 End   = Maximum(CursorPosition, SelectionMark);
                OSCopyChars(&Buffer[Begin], End-Begin);
            }
            TryDeleteSelection();
            
        }else{ //- Normal editing
            TryDeleteSelection();
            Char = CharToLower(Char);
            if(OSInput.KeyFlags & KeyFlag_Shift) Char = KEYBOARD_SHIFT_TABLE[Char]; 
            Assert(Char);
            CursorPosition += InsertCharsToBuffer(CursorPosition, &Char, 1);
            
        }
        
    }else if(Key == KeyCode_BackSpace){
        s32 Begin = CursorPosition-1;
        s32 End = CursorPosition;
        if(!TryDeleteSelection() && TestModifier(KeyFlag_Control|KeyFlag_Any)){
            Begin = SeekBackward(Buffer, CursorPosition);
        }
        DeleteFromBuffer(Begin, End);
        
    }else if(Key == KeyCode_Delete){
        s32 Begin = CursorPosition;
        s32 End = CursorPosition+1;
        if(!TryDeleteSelection() && TestModifier(KeyFlag_Control|KeyFlag_Any)){
            End = SeekForward(Buffer, BufferLength, CursorPosition);
        }
        DeleteFromBuffer(Begin, End);
        
    }else if(Key == KeyCode_Left){
        MaybeSetSelection();
        if(TestModifier(KeyFlag_Control|KeyFlag_Any)){
            CursorPosition = SeekBackward(Buffer, CursorPosition);
        }
        while(0 < CursorPosition) if(!IsNewLine(Buffer[--CursorPosition])) break;
        
    }else if(Key == KeyCode_Right){
        MaybeSetSelection();
        if(TestModifier(KeyFlag_Control|KeyFlag_Any)){
            CursorPosition = SeekForward(Buffer, BufferLength, CursorPosition);
        }
        while(CursorPosition < (s32)BufferLength) if(!IsNewLine(Buffer[++CursorPosition])) break;
        
    }else if(Key == KeyCode_Home){
        MaybeSetSelection();
        while(0 < CursorPosition) if((--CursorPosition > 0) && IsNewLine(Buffer[CursorPosition-1])) break;
        
    }else if(Key == KeyCode_End){
        MaybeSetSelection();
        while(CursorPosition < (s32)BufferLength) if(IsNewLine(Buffer[++CursorPosition])) break;
        
    }else if(Key == KeyCode_Return){
        InputFlags |= OSInputFlag_EndTextInput;
    }else if(Key == KeyCode_Escape){
        SelectionMark = -1;
    }
    
    BufferLength = CStringLength(Buffer);
    CursorPosition = Minimum((u32)CursorPosition, BufferLength);
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

inline void 
os_input::LoadTextInput(const char *From, u32 Length){
    BeginTextInput();
    CopyCString(Buffer, From, DEFAULT_BUFFER_SIZE);
    BufferLength = Length;
}

inline void 
os_input::LoadTextInput(const char *From){
    LoadTextInput(From, CStringLength(From));
}

inline void 
os_input::SaveTextInput(char *To, u32 MaxSize){
    CopyCString(To, Buffer, DEFAULT_BUFFER_SIZE);
    BeginTextInput();
}