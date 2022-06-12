
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
        Result = ((KeyFlags & Flags) == Flags);
    }else{
        Result = KeyFlags == Flags;
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

inline b8
os_input::TryDeleteSelection(){
    if(SelectionMark >= 0){
        HistoryChangeEvent(TextInputEvent_RemoveRange);
        BufferDeleteRange(GetSelectionRange());
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

//- Buffer stuff
inline void
os_input::BufferDeleteRange(range_s32 Range){
    Range = RangeCrop(MakeRangeS32(0, BufferLength), Range);
    MoveMemory(&Buffer[Range.Start],&Buffer[Range.End], BufferLength-(Range.End-1));
    s32 Size = RangeSize(Range);
    ZeroMemory(&Buffer[BufferLength-Size], Size);
    CursorPosition = Range.Start;
    
    TextInputFlags |= TextInputFlag_IsDirty;
}

inline u32
os_input::BufferInsertChars(u32 Position, char *Chars, u32 CharCount){
    if(BufferLength+CharCount < DEFAULT_BUFFER_SIZE){
        MoveMemory(&Buffer[Position+CharCount],
                   &Buffer[Position],
                   BufferLength-Position);
        CopyMemory(&Buffer[Position], Chars, CharCount);
        BufferLength++;
        TextInputFlags |= TextInputFlag_IsDirty;
        return CharCount;
    }
    return 0;
}

inline void
os_input::TextInputProcessKey(os_key_code Key){
    if(!(InputFlags & OSInputFlag_DoTextInput)) return;
    
    if(Key == KeyCode_NULL){
    }else if(Key < U8_MAX){
        char Char = (char)Key;
        
        if((Char == 'C') && TestModifier(KeyFlag_Control|KeyFlag_Any)){ //- Copy
            range_s32 Range = GetSelectionRange();
            OSCopyChars(&Buffer[Range.Start], RangeSize(Range));
        }else if((Char == 'V') && TestModifier(KeyFlag_Control|KeyFlag_Any)){ //- Paste
            HistoryChangeEvent(TextInputEvent_AddRange);
            TryDeleteSelection();
            char *ToPaste = OSPasteChars(&GlobalTransientMemory);
            BufferInsertChars(CursorPosition, ToPaste, CStringLength(ToPaste));
            HistoryAddNode();
        }else if((Char == 'X') && TestModifier(KeyFlag_Control|KeyFlag_Any)){ //- Cut
            HistoryChangeEvent(TextInputEvent_RemoveRange);
            range_s32 Range = GetSelectionRange();
            OSCopyChars(&Buffer[Range.Start], RangeSize(Range));
            TryDeleteSelection();
        }else if((Char == 'A') && TestModifier(KeyFlag_Control|KeyFlag_Any)){ //- Select all
            HistoryChangeEvent(TextInputEvent_MoveCursor);
            CursorPosition = BufferLength;
            SelectionMark = 0;
        }else if((Char == 'Z') && TestModifier(KeyFlag_Control|KeyFlag_Any)){ HistoryUndo();
        }else if((Char == 'Y') && TestModifier(KeyFlag_Control|KeyFlag_Any)){ HistoryRedo();
        }else{ //- Normal editing
            HistoryChangeEvent(TextInputEvent_AddChar);
            TryDeleteSelection();
            
            Char = CharToLower(Char);
            if(KeyFlags & KeyFlag_Shift) Char = KEYBOARD_SHIFT_TABLE[Char]; 
            Assert(Char);
            CursorPosition += BufferInsertChars(CursorPosition, &Char, 1);
            
            if(Char == ' ') HistoryAddNode();
        }
        
    }else if(Key == KeyCode_BackSpace){
        if(!TryDeleteSelection()){
            HistoryChangeEvent(TextInputEvent_BackSpace);
            if(TestModifier(KeyFlag_Control|KeyFlag_Any)){
                BufferDeleteRange(SeekBackward(Buffer, CursorPosition));
                HistoryAddNode();
            }else{
                BufferDeleteRange(SizeRangeS32(CursorPosition, -1));
            }
        }
    }else if(Key == KeyCode_Delete){
        if(!TryDeleteSelection()){
            HistoryChangeEvent(TextInputEvent_Delete);
            if(TestModifier(KeyFlag_Control|KeyFlag_Any)){
                BufferDeleteRange(SeekForward(Buffer, BufferLength, CursorPosition));
                HistoryAddNode();
            }else{
                BufferDeleteRange(SizeRangeS32(CursorPosition, 1));
            }
        }
    }else if(Key == KeyCode_Left){
        MaybeSetSelection();
        if(TestModifier(KeyFlag_Control|KeyFlag_Any)){
            CursorPosition = SeekBackward(Buffer, CursorPosition).Start;
        }
        while(0 < CursorPosition) if(!IsNewLine(Buffer[--CursorPosition])) break;
        HistoryChangeEvent(TextInputEvent_MoveCursor);
        
    }else if(Key == KeyCode_Right){
        MaybeSetSelection();
        if(TestModifier(KeyFlag_Control|KeyFlag_Any)){
            CursorPosition = SeekForward(Buffer, BufferLength, CursorPosition).End;
        }
        while(CursorPosition < (s32)BufferLength) if(!IsNewLine(Buffer[++CursorPosition])) break;
        HistoryChangeEvent(TextInputEvent_MoveCursor);
        
    }else if(Key == KeyCode_Home){
        MaybeSetSelection();
        while(0 < CursorPosition) if((--CursorPosition > 0) && IsNewLine(Buffer[CursorPosition-1])) break;
        HistoryChangeEvent(TextInputEvent_MoveCursor);
        
    }else if(Key == KeyCode_End){
        MaybeSetSelection();
        while(CursorPosition < (s32)BufferLength) if(IsNewLine(Buffer[++CursorPosition])) break;
        HistoryChangeEvent(TextInputEvent_MoveCursor);
        
    }else if(Key == KeyCode_Return){
        InputFlags |= OSInputFlag_EndTextInput;
    }else if(Key == KeyCode_Escape){
        SelectionMark = -1;
    }
    
    BufferLength = CStringLength(Buffer);
    CursorPosition = Minimum((u32)CursorPosition, BufferLength);
    Assert(CursorPosition >= 0);
}

//- Management
inline void
os_input::BeginTextInput(){
    ZeroMemory(Buffer, DEFAULT_BUFFER_SIZE);
    BufferLength = 0;
    SelectionMark = -1;
    CursorPosition = 0;
    InputFlags |= OSInputFlag_DoTextInput;
    
    DLIST_INIT(&HistorySentinel);
    
    CurrentHistoryNode = &HistorySentinel;
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

//- Undo/redo
inline text_input_history_node *
os_input::HistoryAddNode(){
    text_input_history_node *Node = CurrentHistoryNode->Next;
    while(Node != &HistorySentinel){
        text_input_history_node *Next = Node->Next;
        DLIST_REMOVE(Node);
        FREELIST_FREE(FreeHistoryNode, Node);
        Node = Next;
    }
    
    text_input_history_node *Result = FREELIST_ALLOC(FreeHistoryNode, 
                                                     PushStruct(&GlobalTickMemory, text_input_history_node));
    
    Result->Buffer = PushArray(&GlobalTickMemory, char, BufferLength);
    CopyMemory(Result->Buffer, Buffer, BufferLength);
    Result->BufferLength = BufferLength;
    Result->CursorPosition = CursorPosition;
    
    DLIST_ADD_LAST(&HistorySentinel, Result);
    CurrentHistoryNode = Result;
    
    return Result;
}

inline void
os_input::HistoryUndo(){
    HistoryChangeEvent(TextInputEvent_Other);
    if(CurrentHistoryNode == &HistorySentinel) return; 
    
    text_input_history_node *Node = CurrentHistoryNode->Prev;
    
    CopyMemory(Buffer, Node->Buffer, Node->BufferLength);
    if(BufferLength > Node->BufferLength) ZeroMemory(&Buffer[Node->BufferLength], BufferLength-Node->BufferLength);
    
    BufferLength = Node->BufferLength;
    CursorPosition = Node->CursorPosition;
    
    CurrentHistoryNode = Node;
}

inline void
os_input::HistoryRedo(){
    HistoryChangeEvent(TextInputEvent_Other);
    if(CurrentHistoryNode->Next == &HistorySentinel) return; 
    
    text_input_history_node *Node = CurrentHistoryNode->Next;
    
    CopyMemory(Buffer, Node->Buffer, Node->BufferLength);
    if(BufferLength > Node->BufferLength) ZeroMemory(&Buffer[Node->BufferLength], BufferLength-Node->BufferLength);
    
    BufferLength = Node->BufferLength;
    CursorPosition = Node->CursorPosition;
    
    CurrentHistoryNode = Node;
}

inline void
os_input::HistoryChangeEvent(text_input_event_type Type){
    if((LastEvent != Type) && 
       (TextInputFlags & TextInputFlag_IsDirty)){
        HistoryAddNode();
        TextInputFlags &= ~TextInputFlag_IsDirty;
    }
    LastEvent = Type;
}
