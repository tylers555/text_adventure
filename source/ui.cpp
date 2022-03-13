
#if 0
void
ui_manager::ConstructTextInput(os_key_code Key){
    if(BuildTextInputBuffer){
        if(Key < U8_MAX){
            char Char = (char)Key;
            if(('A' <= Char) && (Char <= 'Z')){
                Char += 'a'-'A';
            }
            if(OSInput.KeyFlags & KeyFlag_Shift){
                Char = KEYBOARD_SHIFT_TABLE[Char];
            }
            Buffer[BufferIndex++] = Char;
        }else if(Key == KeyCode_BackSpace){
            BackSpaceCount++;
        }else if(Key == KeyCode_Left){
            CursorMove--;
        }else if(Key == KeyCode_Right){
            CursorMove++;
        }else if(Key == KeyCode_Escape){
            BuildTextInputBuffer = false;
            ResetActiveElement();
        }
    }
}
ui_behavior
ui_manager::DoTextInputElement(u64 ID, rect ActionRect, s32 Priority){
    ui_element Element = MakeElement(UIElementFlags_TextInput, ID, Priority);
    
    b8 DoDeactivate = ((!IsPointInRect(OSInput.MouseP, ActionRect) &&
                        MouseButtonIsDown(MouseButton_Left)));
    ui_behavior Result = DoElement(&Element, 
                                   IsPointInRect(OSInput.MouseP, ActionRect), 
                                   MouseButtonJustDown(MouseButton_Left),
                                   DoDeactivate);
    if(Result == UIBehavior_JustActivate){
        BuildTextInputBuffer = true;
        Result = UIBehavior_Activate;
    }else if(Result == UIBehavior_Deactivate){
        BuildTextInputBuffer = false;
    }
    
    
    return(Result);
}

void 
ui_window::TextInput(char *Buffer, u32 BufferSize, u64 ID){
    if(DontUpdateOrRender()) return;;
    
    ui_text_input_theme *Theme = &WindowTheme->TextInputTheme;
    font *Font = Manager->GetFont(Theme->Font);
    
    ui_text_input_state *State = FindOrCreateInHashTablePtr(&Manager->TextInputStates, ID);
    State->CursorP = CStringLength(Buffer);
    
    f32 Width = GetItemWidth();
    f32 Height = EmToPixels(Theme->Font, Theme->HeightEm);
    v2 P;
    if(!AdvanceForItem(Width, Height, &P)) return;
    rect TextBoxRect = SizeRect(P, V2(Width, Height));
    
    u32 BufferIndex = CStringLength(Buffer);
    
    b8 IsActive = false;
    switch(Manager->DoTextInputElement(ID, TextBoxRect)){
        case UIBehavior_None: {
            State->T += Theme->TDecrease*OSInput.dTime;
            State->ActiveT += Theme->ActiveTDecrease*OSInput.dTime;
        }break;
        case UIBehavior_Hovered: {
            State->T += Theme->TIncrease*OSInput.dTime;
            State->ActiveT += Theme->ActiveTDecrease*OSInput.dTime;
        }break;
        case UIBehavior_Activate: {
            for(u32 I = 0; 
                (I < Manager->BufferIndex) && (BufferIndex < BufferSize);
                I++){
                Buffer[BufferIndex++] = Manager->Buffer[I];
            }
            if(BufferIndex < Manager->BackSpaceCount){
                BufferIndex = 0;
            }else{
                BufferIndex -= Manager->BackSpaceCount;
            }
            
            //State->CursorP += Manager->CursorMove;
            //State->CursorP = Clamp(State->CursorP, 0, BufferSize);
            
            Manager->CursorMove = 0;
            Manager->BackSpaceCount = 0;
            Manager->BufferIndex = 0;
            Buffer[BufferIndex] = '\0';
            
            State->T += Theme->TIncrease*OSInput.dTime;
            State->ActiveT += Theme->ActiveTIncrease*OSInput.dTime;
            
            IsActive = true;
        }break;
    }
    
    State->T = Clamp(State->T, 0.0f, 1.0f);
    f32 T = EaseOutSquared(State->T);
    State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
    f32 ActiveT = EaseOutSquared(State->ActiveT);
    
    color TextColor       = MixColor(Theme->ActiveTextColor, Theme->TextColor, ActiveT);
    color BackgroundColor = MixColor(Theme->ActiveColor, Theme->HoverColor, ActiveT);
    color Color           = MixColor(BackgroundColor, Theme->BaseColor, T);
    TextBoxRect           = RectGrow(TextBoxRect, EmToPixels(Theme->Font, Theme->BoxGrowEm)*T);
    
    DrawRect(TextBoxRect, Z-0.1f, Theme->Roundness, Color);
    v2 StringP = VCenterStringP(Font, P, Height);
    StringP = PadPRight(Font, StringP, Theme->PaddingEm);
    DrawString(Font, TextColor, StringP, Z-0.2f, "%s", Buffer);
    
    if(IsActive){
        color CursorColor = MixColor(Theme->TextColor, Theme->ActiveTextColor, T);
        
        f32 Advance = GetStringAdvanceByCount(Font, Buffer, State->CursorP, true);
        f32 CursorWidth = EmToPixels(Theme->Font, 0.1f);
        f32 TextHeight = Font->Ascent;
        rect CursorRect = MakeRect(V2(0, Font->Descent), V2(CursorWidth, TextHeight));
        CursorRect += StringP+V2(Advance, 0.0f);
        DrawRect(CursorRect, Z-0.2f, 0.0f, CursorColor);
    }
}
#endif

