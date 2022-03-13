
//~ Helpers
internal inline void
OpenPauseMenu(){
    MenuState.LastGameMode = GameMode;
    MenuState.Page = MenuPage_Pause;
    MenuState.Flags |= MenuFlag_KeyboardMode;
    ChangeState(GameMode_Menu, MakeString(0));
}

internal inline void
MenuChangePage(menu_state *State, menu_page_type NewPage){
    State->Page = NewPage;
    State->SelectedID = 0;
    State->UsingSlider = 0;
    State->Flags &= ~MenuFlag_ConfirmQuit;
    for(u32 I=0; I<MAX_MENU_ITEMS; I++){
        State->ItemStates[I] = {};
    }
}

#if 0
//~ Menu page
internal inline menu_page
MakeMenuPage(font *Font, v2 P, f32 YAdvance){
    menu_page Result = {};
    Result.Font = Font;
    Result.P = P;
    Result.YAdvance = YAdvance;
    Result.R = CenterRect(V2(P.X, P.Y-Font->Descent), V2(500, Font->Size));
    
    return Result;
}

internal inline menu_item_state *
MenuPageGetItem(menu_state *State, menu_page *Page){
    Assert(Page->IDCounter < MAX_MENU_ITEMS);
    menu_item_state *Result = &State->ItemStates[Page->IDCounter];
    Result->ID = Page->IDCounter++;
    return Result;
}

internal inline void
MenuFixSelected(menu_state *State, u32 TotalItems){
    if(State->SelectedID < 0) State->SelectedID += TotalItems;
    State->SelectedID %= TotalItems;
}

internal inline color
MenuItemGetColor(menu_item_state *Item, color BaseColor, b8 Hovered, b8 Active){
    color Color = MixColor(MENU_HOVER_COLOR, BaseColor, Item->HoverT);
    if(Hovered) Item->HoverT += MENU_HOVER_FACTOR*OSInput.dTime;
    else Item->HoverT -= MENU_HOVER_FACTOR*OSInput.dTime;
    Item->HoverT = Clamp(Item->HoverT, 0.0f, 1.0f);
    
    Color = MixColor(MENU_ACTIVE_COLOR, Color, Item->ActiveT);
    if(Active) Item->ActiveT += MENU_ACTIVE_FACTOR*OSInput.dTime; 
    else Item->ActiveT -= MENU_ACTIVE_FACTOR*OSInput.dTime;
    Item->ActiveT = Clamp(Item->ActiveT, 0.0f, 1.0f);
    
    return Color;
}

internal inline b8
MenuShouldHoverItem(menu_state *State, menu_item_state *Item, rect R){
    if(State->UsingSlider) return false;
    b8 KeyboardMode = (State->Flags & MenuFlag_KeyboardMode) != 0;
    if(!KeyboardMode && IsPointInRect(OSInput.MouseP, R)){
        MenuState.SelectedID = Item->ID;
        return true;
    }
    if(KeyboardMode  && (State->SelectedID == (s32)Item->ID)) return true;
    return false;
}

internal inline b8
MenuShouldActivateItem(menu_state *State){
    if(!(State->Flags & MenuFlag_KeyboardMode) && 
       OSInput.MouseJustDown(MouseButton_Left, KeyFlag_Any)) return true;
    b8 Result = OSInput.KeyJustDown(KeyCode_Space, KeyFlag_Any);
    return Result;
}

internal inline b8
MenuPageIsLastSelected(menu_state *State, menu_page *Page){
    b8 Result = ((s32)(Page->IDCounter-1) == State->SelectedID);
    return Result;
}

internal inline b8
MenuPageDoText(game_renderer *Renderer, menu_state *State, menu_page *Page, const char *Name){
    b8 Result = false;
    
    menu_item_state *Item = MenuPageGetItem(State, Page);
    if(MenuShouldHoverItem(State, Item, Page->R)){
        if(MenuShouldActivateItem(State)){
            Result = true;
        }
        
        State->Hovered = Name;
        State->Flags |= MenuFlag_SetHovered;
    }
    
    color Color = MenuItemGetColor(Item, MENU_BASE_COLOR, (State->Hovered==Name), false);
    RenderCenteredString(Renderer, Page->Font, Color, Page->P, 0.0f, Name);
    
    Page->P.Y -= Page->YAdvance;
    Page->R -= V2(0, Page->YAdvance);
    
    return Result;
}

internal inline f32
MenuPageDoSlider(game_renderer *Renderer, menu_state *State, menu_page *Page, const char *Name, f32 CurrentValue){
    f32 Result = CurrentValue;
    
    f32 StringAdvance = GetStringAdvance(Page->Font, Name);
    
    f32 Size = 20;
    v2 CursorSize = V2(Size);
    f32 Width = 500;
    f32 EffectiveWidth = Width-CursorSize.X;
    f32 Padding = 20;
    
    
    v2 P = Page->P;
    P.X -= (StringAdvance+Padding);
    RenderString(Renderer, Page->Font, MENU_BASE_COLOR, P, 0.0f, Name);
    P.X += (StringAdvance+Padding);
    P.Y += -Page->Font->Descent;
    
    v2 CursorBaseP = P;
    CursorBaseP.X += 0.5f*CursorSize.Width;
    
    v2 CursorP = CursorBaseP;
    CursorP.X += CurrentValue*EffectiveWidth;
    
    rect CursorRect = CenterRect(CursorP, CursorSize);
    
    rect LineRect = SizeRect(V2(P.X, P.Y-0.5f*Size), V2(Width, Size));
    RenderRoundedRect(Renderer, LineRect, 0.0f, 0.5f*Size, MENU_BASE_COLOR, UIItem(0));
    
    menu_item_state *Item = MenuPageGetItem(State, Page);
    if(State->UsingSlider == Name){
        if(!(State->Flags & MenuFlag_KeyboardMode)){
            Result = (OSInput.MouseP.X - CursorBaseP.X) / EffectiveWidth;
        }else{
            f32 Delta = 0.01f;
            if(OSInput.KeyDown(KeyCode_Shift, KeyFlag_Any)) Delta = 0.05f;
            if(OSInput.KeyRepeat(KeyCode_Left,  KeyFlag_Any)) Result -= Delta;
            if(OSInput.KeyRepeat(KeyCode_Right, KeyFlag_Any)) Result += Delta;
            
            if(OSInput.KeyRepeat(KeyCode_Space, KeyFlag_Any))    State->UsingSlider = 0;
            if(OSInput.KeyRepeat(KeyCode_Up,    KeyFlag_Any))    State->UsingSlider = 0;
            if(OSInput.KeyRepeat(KeyCode_Down,  KeyFlag_Any))    State->UsingSlider = 0;
            if(OSInput.KeyJustDown(KeyCode_Escape, KeyFlag_Any)){
                State->Flags |= MenuFlag_DidADeactivate;
                State->UsingSlider = 0;
            }
        }
        Result = Clamp(Result, 0.0f, 1.0f);
        
        State->Flags |= MenuFlag_SetHovered;
        
    }else if(MenuShouldHoverItem(State, Item, LineRect)){
        State->Hovered = Name;
        State->Flags |= MenuFlag_SetHovered;
        
        if(MenuShouldActivateItem(State)){
            if(OSInput.KeyJustDown(KeyCode_Space, KeyFlag_Any)) State->Flags |= MenuFlag_KeyboardMode;
            State->UsingSlider = Name;
        }
    }
    
    color Color = MenuItemGetColor(Item, MENU_BASE_COLOR2, (State->Hovered==Name), (State->UsingSlider==Name));
    RenderRoundedRect(Renderer, CursorRect, -1.0f, 0.5f*Size, Color, UIItem(0));
    
    Page->P.Y -= Page->YAdvance;
    Page->R -= V2(0, Page->YAdvance);
    
    return Result;
}

internal inline void
MenuPageDoControlChoice(game_renderer *Renderer, menu_state *State, menu_page *Page, const char *Name, os_key_code *Key){
    menu_item_state *Item = MenuPageGetItem(State, Page);
    
    if(State->UsingControlChoice == Name){
        if(OSInput.FirstKeyDown){
            *Key = OSInput.FirstKeyDown;
            State->UsingControlChoice = 0;
        }
    }else if(MenuShouldHoverItem(State, Item, Page->R)){
        if(MenuShouldActivateItem(State)){
            if(OSInput.KeyJustDown(KeyCode_Space, KeyFlag_Any)) State->Flags |= MenuFlag_KeyboardMode;
            State->UsingControlChoice = Name;
        }
        
        State->Hovered = Name;
        State->Flags |= MenuFlag_SetHovered;
    }
    
    color Color = MenuItemGetColor(Item, MENU_BASE_COLOR, (State->Hovered==Name), (State->UsingControlChoice==Name));
    
    f32 StringAdvance = GetStringAdvance(Page->Font, Name);
    
    f32 Size = 20;
    f32 Padding = 40;
    
    v2 P = Page->P;
    P.X -= (StringAdvance+Padding);
    RenderString(Renderer, Page->Font, Color, P, 0.0f, Name);
    P.X += (StringAdvance+Padding);
    RenderString(Renderer, Page->Font, Color, P, 0.0f, OSKeyCodeName(*Key));
    
    Page->P.Y -= Page->YAdvance;
    Page->R -= V2(0, Page->YAdvance);
}

//~ 
internal void 
DoMainMenu(game_renderer *Renderer, menu_state *State, font *ItemFont, v2 P, f32 YAdvance){
    menu_page Page = MakeMenuPage(ItemFont, P, YAdvance);
    MenuFixSelected(State, 4);
    
    if(MenuPageDoText(Renderer, State, &Page, "Start")){
        ChangeState(GameMode_MainGame, String(STARTUP_LEVEL));
    }
    if(MenuPageDoText(Renderer, State, &Page, "Settings")){
        State->LastPage = State->Page;
        MenuChangePage(State, MenuPage_Settings);
    }
    if(MenuPageDoText(Renderer, State, &Page, "Controls")){
        State->LastPage = State->Page;
        MenuChangePage(State, MenuPage_Controls);
    }
    if(!(State->Flags & MenuFlag_ConfirmQuit) && MenuPageDoText(Renderer, State, &Page, "Quit")){
        State->Flags |= MenuFlag_ConfirmQuit;
    }else if((State->Flags & MenuFlag_ConfirmQuit) && MenuPageDoText(Renderer, State, &Page, "Are you sure?")){
        OSEndGame();
    }
    if(!MenuPageIsLastSelected(State, &Page)) State->Flags &= ~MenuFlag_ConfirmQuit;
}

internal void 
DoPauseMenu(game_renderer *Renderer, menu_state *State, font *ItemFont, v2 P, f32 YAdvance){
    menu_page Page = MakeMenuPage(ItemFont, P, YAdvance);
    MenuFixSelected(State, 4);
    
    if(MenuPageDoText(Renderer, State, &Page, "Resume") ||
       OSInput.KeyJustDown(PAUSE_KEY, KeyFlag_Any)){
        ChangeState(State->LastGameMode, String(0));
    }
    if(MenuPageDoText(Renderer, State, &Page, "Settings")){
        State->LastPage = State->Page;
        MenuChangePage(State, MenuPage_Settings);
    }
    if(MenuPageDoText(Renderer, State, &Page, "Controls")){
        State->LastPage = State->Page;
        MenuChangePage(State, MenuPage_Controls);
    }
    if(!(State->Flags & MenuFlag_ConfirmQuit) && MenuPageDoText(Renderer, State, &Page, "Quit")){
        State->Flags |= MenuFlag_ConfirmQuit;
    }else if((State->Flags & MenuFlag_ConfirmQuit) && MenuPageDoText(Renderer, State, &Page, "Are you sure?")){
        OSEndGame();
    }
}

internal void 
DoSettingsMenu(game_renderer *Renderer, menu_state *State, font *ItemFont, v2 P, f32 YAdvance){
    menu_page Page = MakeMenuPage(ItemFont, P, YAdvance);
    MenuFixSelected(State, 3);
    
    AudioMixer.MusicMasterVolume = V2(MenuPageDoSlider(Renderer, State, &Page, "Music volume", AudioMixer.MusicMasterVolume.E[0]));
    AudioMixer.SoundEffectMasterVolume = V2(MenuPageDoSlider(Renderer, State, &Page, "Sound effect volume", AudioMixer.SoundEffectMasterVolume.E[0]));
    if(MenuPageDoText(Renderer, State, &Page, "Back") || 
       (OSInput.KeyJustDown(PAUSE_KEY, KeyFlag_Any) && !(State->Flags & MenuFlag_DidADeactivate))){
        MenuChangePage(State, State->LastPage);
    }
}

internal void 
DoControlsMenu(game_renderer *Renderer, menu_state *State, font *ItemFont, v2 P, f32 YAdvance){
    menu_page Page = MakeMenuPage(ItemFont, P, YAdvance);
    MenuFixSelected(State, 10);
    // TODO(Tyler): HACK This is a HACK!
    Page.P.Y += 100;
    Page.R   += V2(0, 100);
    
    MenuPageDoControlChoice(Renderer, State, &Page, "Board up",     &GameSettings.BoardUp);
    MenuPageDoControlChoice(Renderer, State, &Page, "Board down",   &GameSettings.BoardDown);
    MenuPageDoControlChoice(Renderer, State, &Page, "Board left",   &GameSettings.BoardLeft);
    MenuPageDoControlChoice(Renderer, State, &Page, "Board right",  &GameSettings.BoardRight);
    MenuPageDoControlChoice(Renderer, State, &Page, "Board place",  &GameSettings.BoardPlace);
    MenuPageDoControlChoice(Renderer, State, &Page, "Jump",         &GameSettings.Jump);
    MenuPageDoControlChoice(Renderer, State, &Page, "Select",       &GameSettings.Select);
    MenuPageDoControlChoice(Renderer, State, &Page, "Player left",  &GameSettings.PlayerLeft);
    MenuPageDoControlChoice(Renderer, State, &Page, "Player right", &GameSettings.PlayerRight);
    if(MenuPageDoText(Renderer, State, &Page, "Back") || 
       (OSInput.KeyJustDown(PAUSE_KEY, KeyFlag_Any) && !(State->Flags & MenuFlag_DidADeactivate))){
        MenuChangePage(State, State->LastPage);
    }
}
#endif

internal void
UpdateAndRenderMenu(game_renderer *Renderer){
#if 0
    Renderer->SetLightingConditions(WHITE, 1.0f);
    RenderRect(Renderer, SizeRect(V2(0), Renderer->ScreenToWorld(OSInput.WindowSize, ScaledItem(0))), 
               5.5f, MENU_BACKGROUND_COLOR, GameItem(0));
    
    font *MenuTitleFont = FontSystem.GetFont(String("menu_title_font"), "asset_fonts/Merriweather/Merriweather-Black.ttf", 200);
    font *ItemFont = FontSystem.GetFont(String("menu_item_font"), "asset_fonts/Merriweather/Merriweather-Black.ttf", 75);
    
    v2 P = V2(OSInput.WindowSize.Width/2, OSInput.WindowSize.Height-150);
    f32 Padding = 10.0f;
    f32 FontSize = ItemFont->Ascent - ItemFont->Descent;
    f32 YAdvance = FontSize+Padding;
    
    RenderCenteredString(Renderer, MenuTitleFont, WHITE, P, 0.0f, "Toe Tac Tic");
    P.Y -= MenuTitleFont->Size;
    
    if(!MenuState.UsingControlChoice){
        if(OSInput.KeyJustDown(GameSettings.BoardUp, KeyFlag_Any)){
            MenuState.SelectedID--;
            MenuState.Flags |= MenuFlag_KeyboardMode;
        }else if(OSInput.KeyJustDown(GameSettings.BoardDown, KeyFlag_Any)){
            MenuState.SelectedID++;
            MenuState.Flags |= MenuFlag_KeyboardMode;
        }
    }
    
    switch(MenuState.Page){
        case MenuPage_Main: {
            DoMainMenu(Renderer, &MenuState, ItemFont, P, YAdvance);
        }break;
        case MenuPage_Pause: {
            DoPauseMenu(Renderer, &MenuState, ItemFont, P, YAdvance);
        }break;
        case MenuPage_Settings: {
            DoSettingsMenu(Renderer, &MenuState, ItemFont, P, YAdvance);
        }break;
        case MenuPage_Controls: {
            DoControlsMenu(Renderer, &MenuState, ItemFont, P, YAdvance);
        }break;
    }
    
    
    if(!(MenuState.Flags & MenuFlag_SetHovered)) MenuState.Hovered = 0;
    MenuState.Flags &= ~(MenuFlag_SetHovered|MenuFlag_DidADeactivate);
    
    if(OSInput.InputFlags & OSInputFlag_MouseMoved) MenuState.Flags &= ~MenuFlag_KeyboardMode;
    
    if(!(MenuState.Flags & MenuFlag_KeyboardMode) &&
       OSInput.MouseUp(MouseButton_Left, KeyFlag_Any)){
        MenuState.UsingSlider = 0;
    }
#endif
}
