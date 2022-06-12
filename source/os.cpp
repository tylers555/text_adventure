
//~ Miscellaneous
inline b8
os_input::WasWindowResized(){
    b8 Result = ((LastWindowSize.Width  != WindowSize.Width) ||
                 (LastWindowSize.Height != WindowSize.Height));
    return Result;
}

//~ Modifiers

inline b8
os_input::TestModifier(os_key_flags Flags){
    b8 Result;
    if(Flags & KeyFlag_Any){
        Flags &= 0b0111;
        Result = ((OSInput.KeyFlags & Flags) == Flags);
    }else{
#if 0
        Result = (((OSInput.KeyFlags & Flags) == Flags) &&
                  ((~OSInput.KeyFlags & ~Flags) == ~Flags));
#endif
        Result = OSInput.KeyFlags == Flags;
    }
    return(Result);
}

//~ Mouse 
inline b8 
os_input::MouseUp(os_mouse_button Button, os_key_flags Flags){
    if(InputFlags & OSInputFlag_CapturedByUI) return false;
    
    key_state ButtonState = MouseState[Button];
    b8 Result = !((ButtonState & KeyState_IsDown) && TestModifier(Flags));
    
    return(Result);
}

inline b8 
os_input::MouseDown(os_mouse_button Button, os_key_flags Flags){
    if(InputFlags & OSInputFlag_CapturedByUI) return false;
    
    key_state ButtonState = MouseState[Button];
    b8 Result = ((ButtonState & KeyState_IsDown) && TestModifier(Flags));
    
    return(Result);
}

inline b8 
os_input::MouseJustDown(os_mouse_button Button, os_key_flags Flags){
    if(InputFlags & OSInputFlag_CapturedByUI) return false;
    
    key_state ButtonState = MouseState[Button];
    b8 Result = ((ButtonState & KeyState_JustDown) && TestModifier(Flags));
    
    return(Result);
}

//~ Keyboard
// TODO(Tyler): Part of me is unsure about having the if in these 
// functions even though, it wouldn't be used in the release version
// and won't be used in the menu at all.

inline b8 
os_input::KeyUp(u32 Key, os_key_flags Flags){
    if(InputFlags & OSInputFlag_CapturedByUI) return false;
    
    key_state KeyState = KeyboardState[Key];
    b8 Result = !((KeyState & KeyState_IsDown) && TestModifier(Flags));
    
    return(Result);
}

inline b8 
os_input::KeyJustUp(u32 Key, os_key_flags Flags){
    if(InputFlags & OSInputFlag_CapturedByUI) return false;
    
    key_state KeyState = KeyboardState[Key];
    b8 Result = ((KeyState & KeyState_JustUp) || !TestModifier(Flags));
    
    return(Result);
}

inline b8 
os_input::KeyJustDown(u32 Key, os_key_flags Flags){
    if(InputFlags & OSInputFlag_CapturedByUI) return false;
    
    key_state KeyState = KeyboardState[Key];
    b8 Result = ((KeyState & KeyState_JustDown) && TestModifier(Flags));
    
    return(Result);
}

inline b8 
os_input::KeyRepeat(u32 Key, os_key_flags Flags){
    if(InputFlags & OSInputFlag_CapturedByUI) return false;
    
    key_state KeyState = KeyboardState[Key];
    b8 Result = ((KeyState & KeyState_RepeatDown) && TestModifier(Flags));
    
    return(Result);
}

inline b8 
os_input::KeyDown(u32 Key, os_key_flags Flags){
    if(InputFlags & OSInputFlag_CapturedByUI) return false;
    
    key_state KeyState = KeyboardState[Key];
    b8 Result = ((KeyState & KeyState_IsDown) && TestModifier(Flags));
    
    return(Result);
}


//~ Text input
inline range_s32
os_input::GetSelectionRange(){
    range_s32 Result = MakeRangeS32(CursorPosition, CursorPosition);
    if(SelectionMark >= 0){
        Result = MakeRangeS32(CursorPosition, SelectionMark);
    }
    return Result;
}

inline void
os_input::DeleteFromBuffer(range_s32 Range){
    if(Range.Start < 0) return;
    MoveMemory(&Buffer[Range.Start],
               &Buffer[Range.End], BufferLength-(Range.End-1));
    s32 Size = RangeSize(Range);
    ZeroMemory(&Buffer[BufferLength-Size], Size);
    CursorPosition = Range.Start;
}

inline b8
os_input::TryDeleteSelection(){
    if(SelectionMark >= 0){
        DeleteFromBuffer(GetSelectionRange());
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
            range_s32 Range = GetSelectionRange();
            OSCopyChars(&Buffer[Range.Start], RangeSize(Range));
        }else if((Char == 'V') && TestModifier(KeyFlag_Control|KeyFlag_Any)){ //- Paste
            TryDeleteSelection();
            char *ToPaste = OSPasteChars(&GlobalTransientMemory);
            InsertCharsToBuffer(CursorPosition, ToPaste, CStringLength(ToPaste));
        }else if((Char == 'X') && TestModifier(KeyFlag_Control|KeyFlag_Any)){ //- Cut
            range_s32 Range = GetSelectionRange();
            OSCopyChars(&Buffer[Range.Start], RangeSize(Range));
            TryDeleteSelection();
        }else if((Char == 'A') && TestModifier(KeyFlag_Control|KeyFlag_Any)){ //- Select all
            CursorPosition = BufferLength;
            SelectionMark = 0;
        }else{ //- Normal editing
            TryDeleteSelection();
            Char = CharToLower(Char);
            if(OSInput.KeyFlags & KeyFlag_Shift) Char = KEYBOARD_SHIFT_TABLE[Char]; 
            Assert(Char);
            CursorPosition += InsertCharsToBuffer(CursorPosition, &Char, 1);
        }
        
    }else if(Key == KeyCode_BackSpace){
        if(!TryDeleteSelection()){
            range_s32 Range = SizeRangeS32(CursorPosition, -1);
            if(TestModifier(KeyFlag_Control|KeyFlag_Any)){
                Range.Start = SeekBackward(Buffer, CursorPosition);
            }
            DeleteFromBuffer(Range);
        }
    }else if(Key == KeyCode_Delete){
        if(!TryDeleteSelection()){
            range_s32 Range = SizeRangeS32(CursorPosition, 1);
            if(TestModifier(KeyFlag_Control|KeyFlag_Any)){
                Range.End = SeekForward(Buffer, BufferLength, CursorPosition);
            }
            DeleteFromBuffer(Range);
        }
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
    Assert(CursorPosition >= 0);
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