internal void
UpdateAndRenderMap(game_renderer *Renderer, audio_mixer *Mixer, asset_system *Assets, os_input *Input){
    DO_DEBUG_INFO();
    
    if(FrameCounter == 0){
        Input->BeginTextInput();
    }
    
    ta_system *TA = &TextAdventure;
    ta_map *Map = &TA->Map;
    console_theme *Theme = &TA->Theme;
    
    Renderer->NewFrame(&TransientStorageArena, Input->WindowSize, Theme->BackgroundColor);
    
    v2 WindowSize = RoundV2(Renderer->ScreenToWorld(Input->WindowSize));
    asset_font *Font = Assets->GetFont(Theme->BasicFont);
    
    {
        render_texture Texture = Map->Texture;
        rect R = SizeRect(V2(20), 2*Map->Size);
        RenderTexture(Renderer, R, 0.0, Texture);
        
        //FontRenderString(Renderer, Font, V2(20), WHITE, "%f %f", Size.X, Size.Y);
    }
    
    if(Input->KeyJustDown(KeyCode_Escape)){
        Input->BeginTextInput();
#if 0
        Mixer->PlaySound(Assets->GetSoundEffect(String("close_map")));
        ChangeState(GameMode_MainGame);
#endif
    }
    
#if 0    
    f32 ViewWidth = 150;
    f32 ViewTop = WindowSize.Y-10;
    f32 ViewBottom = 10;
    f32 TotalAreaHeight = ViewTop-ViewBottom;
    f32 TotalListHeight = Map->Areas.Count*FontLineAdvance(Font);
    v2 ViewP = V2(WindowSize.X-ViewWidth-10, ViewTop);
    
    s32 CandidateIndex = -1;
    if(Input->Buffer[0]){
        FontRenderString(Renderer, Font, V2(10, 250), WHITE, "%s", Input->Buffer);
        u32 CandidateCount = 0;
        for(u32 I=0; I<Map->Areas.Count; I++){
            ta_area *Area = &Map->Areas[I];
            u32 Count = CountWordMatchCount(Input->Buffer, Area->Name);
            if(Count > CandidateCount){
                CandidateIndex = I;
                CandidateCount = Count;
            }
        }
    }
    
    if(CandidateIndex >= 0){
        ViewP.Y += CandidateIndex*FontLineAdvance(Font) - 0.5f*TotalAreaHeight;
        if(ViewP.Y < ViewTop) ViewP.Y = ViewTop;
        ViewP.Y = ViewBottom+TotalListHeight;
        if((TotalListHeight > TotalAreaHeight) && 
           ((ViewP.Y-TotalListHeight) < ViewBottom)){
        }
    }
    
    for(u32 I=0; I<Map->Areas.Count; I++){
        ta_area *Area = &Map->Areas[I];
        v2 InitialP = ViewP;
        f32 Delta = FontRenderFancyString(Renderer, Font, &Theme->BasicFancy, 1, ViewP, Area->Name);
        ViewP.Y -= Delta;
        
        if(CandidateIndex == (s32)I){
            f32 Advance = FontStringAdvance(Font, Area->Name).Width;
            InitialP.Y -= Font->Descent;
            rect R = SizeRect(InitialP, V2(Advance, Delta));
            RenderRect(Renderer, R, 1.0, RED);
        }
        
    }
#endif
}