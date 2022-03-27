internal void
UpdateAndRenderMap(game_renderer *Renderer, audio_mixer *Mixer, asset_system *Assets, os_input *Input){
    DO_DEBUG_INFO();
    
    ta_system *TA = &TextAdventure;
    asset_font *Font = Assets->GetFont(String("basic"));
    
    Renderer->NewFrame(&TransientStorageArena, Input->WindowSize, BLACK);
    
    render_texture Texture = TA->MapImage->Texture;
    v2 Size = V2(TA->MapImage->Size);
    rect R = SizeRect(V2(100), Size);
    RenderTexture(Renderer, R, 0.0, Texture);
    
    FontRenderString(Renderer, Font, V2(20), WHITE, "%f %f", Size.X, Size.Y);
}