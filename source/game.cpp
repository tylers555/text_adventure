
internal void
UpdateAndRenderMainGame(game_renderer *Renderer){
    TIMED_FUNCTION();
    
    //RenderTexture(Renderer, MakeRect(V2(0), V2(30)), 10.0, RED);
    asset_font *Font = AssetSystem.GetFont(String("font_one"));
    Assert(Font);
    FontRenderString(Font, V2(100), BLUE, "Hello, world!");
    
    v2 P = V2(100, 93);
    char *Text = OSInput.Buffer;
    FontRenderString(Font, P, WHITE, Text);
    
    if(FrameCounter == 0){
        OSInput.BeginTextInput();
    }else if(OSInput.MouseDown(MouseButton_Right)){
        OSInput.EndTextInput();
    }
    
    v2 CursorP = P;
    CursorP.X += FontStringAdvance(Font, OSInput.CursorPosition, Text);
    if(((FrameCounter / 50) % 2) == 0){
        RenderLine(&GameRenderer, CursorP, CursorP+V2(0, 5), 0.0, 1, WHITE);
    }
    
    if(OSInput.SelectionMark >= 0){
        f32 SelectionX = P.X+FontStringAdvance(Font, OSInput.SelectionMark, Text);
        f32 Width = SelectionX-CursorP.X;
        RenderRect(&GameRenderer, RectFix(SizeRect(CursorP, V2(Width, Font->Height))), 1.0, BLUE);
    }
}