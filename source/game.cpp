
//~ 
internal void
GameProcessHotKeys(){
    
}

//~ Update 
internal void
UpdateAndRenderMainGame(game_renderer *Renderer){
    TIMED_FUNCTION();
    
    //RenderTexture(Renderer, MakeRect(V2(0), V2(30)), 10.0, RED);
    asset_font *Font = AssetSystem.GetFont(String("font_one"));
    Assert(Font);
    FontRenderString(Font, V2(100), BLUE, "Hello, World!");
    
    GameProcessHotKeys();
}