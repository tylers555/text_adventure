
void
asset_system::Initialize(memory_arena *Arena){
    Memory = MakeArena(Arena, Megabytes(128));
    SoundEffects = PushHashTable<string, asset_sound_effect>(Arena, MAX_ASSETS_PER_TYPE);
    Fonts        = PushHashTable<string, asset_font>(Arena, MAX_ASSETS_PER_TYPE);
    
    //~ Dummy assets
    u8 InvalidColor[] = {0xff, 0x00, 0xff, 0xff};
    render_texture InvalidTexture = MakeTexture();
    TextureUpload(InvalidTexture, InvalidColor, 1, 1);
    
    InitializeLoader(Arena);
    LoadAssetFile(ASSET_FILE_PATH);
}

//~ Sound effects

asset_sound_effect *
asset_system::GetSoundEffect(string Name){
    asset_sound_effect *Result = 0;
    if(Name.ID){
        asset_sound_effect *Asset = FindInHashTablePtr(&SoundEffects, Name);
        if(Asset) Result = Asset;
    }
    return(Result);
}

//~ Fonts
asset_font *
asset_system::GetFont(string Name){
    asset_font *Result = 0;
    if(Name.ID){
        asset_font *Asset = FindInHashTablePtr(&Fonts, Name);
        if(Asset) Result = Asset;
    }
    return(Result);
}

void
VFontRenderString(asset_font *Font, v2 StartP, color Color, const char *Format, va_list VarArgs){
    char Buffer[DEFAULT_BUFFER_SIZE];
    stbsp_vsnprintf(Buffer, sizeof(Buffer), Format, VarArgs);
    
    v2 P = StartP;
    u32 Length = CStringLength(Buffer);
    for(u32 I=0; I<Length; I++){
        char C = Buffer[I];
        
        asset_font_glyph Glyph = Font->Table[C];
        rect R = SizeRect(P, V2((f32)Glyph.Width, (f32)Font->Height));
        rect TextureR = SizeRect(V2((f32)Glyph.Offset.X, (f32)Glyph.Offset.Y),
                                 V2((f32)Glyph.Width, Font->Height));
        TextureR.Min.X /= Font->Size.Width;
        TextureR.Max.X /= Font->Size.Width;
        TextureR.Min.Y /= Font->Size.Height;
        TextureR.Max.Y /= Font->Size.Height;
        RenderTexture(&GameRenderer, R, 0.0, Font->Texture, TextureR);
        //RenderRect(&GameRenderer, R, 0.0, MakeColor(1.0, 1.0, 0.0, 0.0));
        
        P.X += Glyph.Width + 1;
    }
}

void
FontRenderString(asset_font *Font, v2 P, color Color, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    
    VFontRenderString(Font, P, Color, Format, VarArgs);
    
    va_end(VarArgs);
}