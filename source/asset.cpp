
void
asset_system::Initialize(memory_arena *Arena){
    Memory = MakeArena(Arena, Megabytes(128));
#if !defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)
    SoundEffectTable = MakeHashTable<string, asset_sound_effect>(Arena, MAX_ASSETS_PER_TYPE);
    FontTable        = MakeHashTable<string, asset_font>(Arena, MAX_ASSETS_PER_TYPE);
    InitializeLoader(Arena);
#endif
    
    //~ Dummy assets
    u8 InvalidColor[] = {0xff, 0x00, 0xff, 0xff};
    render_texture InvalidTexture = MakeTexture();
    TextureUpload(InvalidTexture, InvalidColor, 1, 1);
    stbi_set_flip_vertically_on_load(true);
}

//~ Asset tags

internal inline constexpr b8
operator==(asset_tag A, asset_tag B){
    b8 Result = A.All == B.All;
    return Result;
}

internal constexpr inline asset_tag
MakeAssetTag(asset_tag_id A = AssetTag_None, 
             asset_tag_id B = AssetTag_None, 
             asset_tag_id C = AssetTag_None,  
             asset_tag_id D = AssetTag_None){
    asset_tag Result = {};
    Result.A = (u8)A;
    Result.B = (u8)B;
    Result.C = (u8)C;
    Result.D = (u8)D;
    return Result;
}

internal constexpr inline asset_tag
AssetTag(asset_tag_id A = AssetTag_None, 
         asset_tag_id B = AssetTag_None, 
         asset_tag_id C = AssetTag_None,  
         asset_tag_id D = AssetTag_None){
    asset_tag Result = MakeAssetTag(A, B, C, D);
    return Result;
}

internal inline b8 
HasTag(asset_tag Tag, asset_tag_id ID){
    b8 Result = ((Tag.A == ID) ||
                 (Tag.B == ID) ||
                 (Tag.C == ID) ||
                 (Tag.D == ID));
    return Result;
}

#if !defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)
//~ Sound effects

asset_sound_effect *
asset_system::GetSoundEffectByString(string Name){
    asset_sound_effect *Result = 0;
    if(Name.ID){
        asset_sound_effect *Asset = HashTableFindPtr(&SoundEffectTable, Name);
        if(Asset) Result = Asset;
    }
    return(Result);
}

//~ Fonts
asset_font *
asset_system::GetFontByString(string Name){
    asset_font *Result = 0;
    if(Name.ID){
        asset_font *Asset = HashTableFindPtr(&FontTable, Name);
        if(Asset) Result = Asset;
    }
    return(Result);
}
#endif

internal f32
VFontRenderString(game_renderer *Renderer, asset_font *Font, v2 StartP, color Color, const char *Format, va_list VarArgs){
    f32 Height = Font->Height+FONT_VERTICAL_SPACE;
    
    char Buffer[DEFAULT_BUFFER_SIZE];
    stbsp_vsnprintf(Buffer, sizeof(Buffer), Format, VarArgs);
    
    StartP.Y -= Font->Descent;
    v2 P = StartP;
    u32 Length = CStringLength(Buffer);
    for(u32 I=0; I<Length; I++){
        char C = Buffer[I];
        if(C == '\n'){
            P.X = StartP.X;
            P.Y -= Font->Height+FONT_VERTICAL_SPACE;
            Height += Font->Height+FONT_VERTICAL_SPACE;
            continue;
        }else if(C == '\r') continue;
        
        asset_font_glyph Glyph = Font->Table[C];
        rect R = SizeRect(P, V2((f32)Glyph.Width, (f32)Font->Height));
        rect TextureR = SizeRect(V2((f32)Glyph.Offset.X, (f32)Glyph.Offset.Y),
                                 V2((f32)Glyph.Width, Font->Height));
        TextureR.Min.X /= Font->Size.Width;
        TextureR.Max.X /= Font->Size.Width;
        TextureR.Min.Y /= Font->Size.Height;
        TextureR.Max.Y /= Font->Size.Height;
        RenderTexture(Renderer, R, 0.0, Font->Texture, TextureR, false, Color);
        
        P.X += Glyph.Width+FONT_LETTER_SPACE;
    }
    
    return Height;
}

internal f32
FontRenderString(game_renderer *Renderer, asset_font *Font, v2 P, color Color, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    
    f32 Result = VFontRenderString(Renderer, Font, P, Color, Format, VarArgs);
    
    va_end(VarArgs);
    return Result;
}

#if 0
internal f32 
FontWordAdvance(asset_font *Font, const char *S, u32 WordStart){
    f32 Result = 0;
    u32 Length = CStringLength(S);
    b8 HitAlphabetic = false;
    for(u32 I=WordStart; I<Length; I++){
        char C = S[I];
        if(C == '\n'){ 
            break;
        }else if(IsWhiteSpace(C)){
            if(HitAlphabetic) break;
        }else HitAlphabetic = true;
        
        asset_font_glyph Glyph = Font->Table[C];
        Result += Glyph.Width+FONT_LETTER_SPACE;
    }
    
    return Result;
}

#else 

// NOTE(Tyler): This actually seems to have a fairly significant speedup
internal f32 
FontWordAdvance(asset_font *Font, const char *S, u32 WordStart){
    f32 Result = 0;
    S += WordStart;
    u32 Length = CStringLength(S);
    b8 HitAlphabetic = false;
    for(u32 I=0; I<Length/8; I++){
        u64 C8 = ((u64 *)S)[I];
        
        for(u32 J=0; J<8; J++){
            char C = C8 & 0xff;
            C8 >>= 8;
            
            if(C == '\n'){
                return Result;
            }else if(IsWhiteSpace(C)){
                if(HitAlphabetic) return Result;
            }else HitAlphabetic = true;
            
            Result += Font->Table[C].Width+FONT_LETTER_SPACE;
        }
    }
    
    for(u32 I=8*(Length/8); I<Length; I++){
        char C = S[I];
        if(C == '\n'){ 
            return Result;
        }else if(IsWhiteSpace(C)){
            if(HitAlphabetic) return Result;
        }else HitAlphabetic = true;
        
        Result += Font->Table[C].Width+FONT_LETTER_SPACE;
    }
    
    return Result;
}

#endif 

internal inline constexpr f32
FontLineHeight(asset_font *Font){
    f32 Result = Font->Height+FONT_VERTICAL_SPACE;
    return Result;
}

internal v2
FontStringAdvance(asset_font *Font, u32 N, const char *S, f32 MaxWidth=F32_POSITIVE_INFINITY){
    v2 Result = V2(0);
    u32 Length = Minimum(CStringLength(S), N);
    for(u32 I=0; I<Length; I++){
        char C = S[I];
        asset_font_glyph Glyph = Font->Table[C];
        
        if(C == ' '){
            f32 WordAdvance = FontWordAdvance(Font, S, I);
            if(Result.X+WordAdvance > MaxWidth){
                Result.X = 0;
                Result.Y -= Font->Height+FONT_VERTICAL_SPACE;
                continue;
            }
        }else if(Result.X+Glyph.Width+FONT_LETTER_SPACE >= MaxWidth){
            Result.X = 0;
            Result.Y -= Font->Height+FONT_VERTICAL_SPACE;
        }
        
        Result.X += Glyph.Width+FONT_LETTER_SPACE;
    }
    
    Assert(Result.X <= MaxWidth);
    
    return Result;
}

internal v2
FontStringAdvance(asset_font *Font, const char *S, f32 MaxWidth=F32_POSITIVE_INFINITY){
    v2 Result = FontStringAdvance(Font, CStringLength(S), S, MaxWidth);
    return Result;
}


#if 1
internal f32
FontRenderFancyString(game_renderer *Renderer, asset_font *Font, const fancy_font_format *Fancies, u32 FancyCount, v2 StartP, const char *S, f32 MaxWidth=F32_POSITIVE_INFINITY){
    if(!S) return 0;
    if(!S[0]) return 0;
    f32 Height = Font->Height+FONT_VERTICAL_SPACE;
    
    
    Assert(FancyCount > 0);
    const u32 MAX_FANCY_COUNT = 10;
    
    u32 Length = CStringLength(S);
    u32 CurrentFancyIndex = 0;
    const fancy_font_format *Fancy = &Fancies[CurrentFancyIndex];
    
    StartP.Y -= Font->Descent;
    v2 P = StartP;
    f32 Ts[MAX_FANCY_COUNT];
    f32 ColorTs[MAX_FANCY_COUNT];
    for(u32 I=0; I<FancyCount; I++){
        Ts[I]      = Fancies[I].Speed*Counter;
        ColorTs[I] = Fancies[I].ColorTOffset+Fancies[I].ColorSpeed*Counter;
    }
    
    render_item *RenderItem = Renderer->NewRenderItem(Font->Texture, false, 0.0);
    Assert(RenderItem);
    
    item_vertex *Vertices = Renderer->AddVertices(RenderItem, Length*4);
    u32 *Indices = Renderer->AddIndices(RenderItem, Length*6);
    f32 Z = 0.0f;
    
    u32 J = 0;
    for(u32 I=0; I<Length; I++){
        char C = S[I];
        asset_font_glyph Glyph = Font->Table[C];
        
        if(C == ' '){
            f32 WordAdvance = FontWordAdvance(Font, S, I);
            if(P.X-StartP.X+WordAdvance >= MaxWidth){
                P.X = StartP.X;
                P.Y -= Font->Height+FONT_VERTICAL_SPACE;
                Height += Font->Height+FONT_VERTICAL_SPACE;
                continue;
            }else{
                P.X += Glyph.Width+FONT_LETTER_SPACE;
                continue;
            }
        }else if(C == '\n'){
            P.X = StartP.X;
            P.Y -= Font->Height+FONT_VERTICAL_SPACE;
            Height += Font->Height+FONT_VERTICAL_SPACE;
            continue;
        }else if(C == '\r'){
            continue;
        }else if(C == '\x02'){
            I++;
            Assert(I < Length);
            u32 NewIndex = S[I];
            if(NewIndex > FancyCount) continue;
            CurrentFancyIndex = NewIndex;
            CurrentFancyIndex--;
            Assert(CurrentFancyIndex < FancyCount);
            Fancy = &Fancies[CurrentFancyIndex];
            continue;
        }else if(P.X-StartP.X+Glyph.Width+FONT_LETTER_SPACE >= MaxWidth){
            P.X = StartP.X;
            P.Y -= Font->Height+FONT_VERTICAL_SPACE;
            Height += Font->Height+FONT_VERTICAL_SPACE;
        }
        
        {
            v2 CharP = P;
            color Color = Fancy->Color1;
            if(Ts[CurrentFancyIndex] != 0.0f)
                CharP.Y += Fancy->Amplitude*Sin(Ts[CurrentFancyIndex]);
            if(ColorTs[CurrentFancyIndex] != 0.0f){
                f32 Alpha = 0.5f*(Sin(ColorTs[CurrentFancyIndex])+1.0f);
                Color = MixColor(Fancy->Color2, Fancy->Color1, Alpha);
            }
            
            f32 X0 = CharP.X;
            f32 Y0 = CharP.Y;
            f32 X1 = CharP.X + (f32)Glyph.Width;
            f32 Y1 = CharP.Y + Font->Height;
            
            f32 TX0 = (f32)(Glyph.Offset.X)              / Font->Size.Width;
            f32 TY0 = (f32)(Glyph.Offset.Y)              / Font->Size.Height;
            f32 TX1 = (f32)(Glyph.Offset.X+Glyph.Width)  / Font->Size.Width;
            f32 TY1 = (f32)(Glyph.Offset.Y+Font->Height) / Font->Size.Height;
            
            Vertices[4*J+0] = {V2(X0, Y0), Z, V2(TX0, TY0), Color};
            Vertices[4*J+1] = {V2(X0, Y1), Z, V2(TX0, TY1), Color};
            Vertices[4*J+2] = {V2(X1, Y1), Z, V2(TX1, TY1), Color};
            Vertices[4*J+3] = {V2(X1, Y0), Z, V2(TX1, TY0), Color};
            
            Indices[6*J+0] = 4*J+0;
            Indices[6*J+1] = 4*J+1;
            Indices[6*J+2] = 4*J+2;
            Indices[6*J+3] = 4*J+0;
            Indices[6*J+4] = 4*J+2;
            Indices[6*J+5] = 4*J+3;
            
            P.X += (f32)Glyph.Width+FONT_LETTER_SPACE;
        }
        
        J++;
        Ts[CurrentFancyIndex] += Fancy->dT;
        ColorTs[CurrentFancyIndex] += Fancy->ColordT;
    }
    RenderItem->IndexCount = 6*J;
    
    return Height;
}

#else

// NOTE(Tyler): This does not seem to have too significant of a speedup
internal f32
FontRenderFancyString(game_renderer *Renderer, asset_font *Font, const fancy_font_format *Fancies, u32 FancyCount, v2 StartP, const char *S, f32 MaxWidth=F32_POSITIVE_INFINITY){
    if(!S) return 0;
    if(!S[0]) return 0;
    f32 Height = Font->Height+FONT_VERTICAL_SPACE;
    
    Assert(FancyCount > 0);
    const u32 MAX_FANCY_COUNT = 10;
    
    u32 Length = CStringLength(S);
    u32 CurrentFancyIndex = 0;
    const fancy_font_format *Fancy = &Fancies[CurrentFancyIndex];
    
    StartP.Y -= Font->Descent;
    v2 P = StartP;
    f32 Ts[MAX_FANCY_COUNT];
    for(u32 I=0; I<FancyCount; I++){
        Ts[I] = Fancies[I].Speed*Counter;
    }
    
    render_item *RenderItem = Renderer->NewRenderItem(Font->Texture, false, 0.0);
    Assert(RenderItem);
    
    item_vertex *Vertices = Renderer->AddVertices(RenderItem, Length*4);
    u32 *Indices = Renderer->AddIndices(RenderItem, Length*6);
    f32 Z = 0.0f;
    
    b8 NextCharIsSpecial = false;;
    u32 K=0;
    for(u32 I=0; I<Length/16; I++){
        u64 C8 = ((u64 *)S)[I];
        
        for(u32 J=I*8; J<I*8+8; J++){
            char C = C8 & 0xff;
            C8 >>= 8;
            asset_font_glyph Glyph = Font->Table[C];
            
            if(NextCharIsSpecial){
                CurrentFancyIndex = C;
                Assert(CurrentFancyIndex <= FancyCount);
                CurrentFancyIndex--;
                Fancy = &Fancies[CurrentFancyIndex];
                NextCharIsSpecial = false;
                continue;
            }else if(C == ' '){
                f32 WordAdvance = FontWordAdvance(Font, S, J);
                if(P.X-StartP.X+WordAdvance >= MaxWidth){
                    P.X = StartP.X;
                    P.Y -= Font->Height+FONT_VERTICAL_SPACE;
                    Height += Font->Height+FONT_VERTICAL_SPACE;
                    continue;
                }else{
                    P.X += Glyph.Width+FONT_LETTER_SPACE;
                    continue;
                }
            }else if(C == '\n'){
                P.X = StartP.X;
                P.Y -= Font->Height+FONT_VERTICAL_SPACE;
                Height += Font->Height+FONT_VERTICAL_SPACE;
                continue;
            }else if(C == '\r'){
                continue;
            }else if(C == '\x02'){
                NextCharIsSpecial = true;
                continue;
            }else if(P.X-StartP.X+Glyph.Width+FONT_LETTER_SPACE >= MaxWidth){
                P.X = StartP.X;
                P.Y -= Font->Height+FONT_VERTICAL_SPACE;
                Height += Font->Height+FONT_VERTICAL_SPACE;
            }
            
            {
                v2 CharP = P;
                if(Ts[CurrentFancyIndex] != 0.0f)
                    CharP.Y += Fancy->Amplitude*Sin(Ts[CurrentFancyIndex]);
                
                f32 X0 = CharP.X;
                f32 Y0 = CharP.Y;
                f32 X1 = CharP.X + (f32)Glyph.Width;
                f32 Y1 = CharP.Y + Font->Height;
                
                f32 TX0 = (f32)(Glyph.Offset.X)              / Font->Size.Width;
                f32 TY0 = (f32)(Glyph.Offset.Y)              / Font->Size.Height;
                f32 TX1 = (f32)(Glyph.Offset.X+Glyph.Width)  / Font->Size.Width;
                f32 TY1 = (f32)(Glyph.Offset.Y+Font->Height) / Font->Size.Height;
                
                Vertices[4*K+0] = {V2(X0, Y0), Z, V2(TX0, TY0), Fancy->Color};
                Vertices[4*K+1] = {V2(X0, Y1), Z, V2(TX0, TY1), Fancy->Color};
                Vertices[4*K+2] = {V2(X1, Y1), Z, V2(TX1, TY1), Fancy->Color};
                Vertices[4*K+3] = {V2(X1, Y0), Z, V2(TX1, TY0), Fancy->Color};
                
                Indices[6*K+0] = 4*K+0;
                Indices[6*K+1] = 4*K+1;
                Indices[6*K+2] = 4*K+2;
                Indices[6*K+3] = 4*K+0;
                Indices[6*K+4] = 4*K+2;
                Indices[6*K+5] = 4*K+3;
                
                
                K++;
                P.X += (f32)Glyph.Width+FONT_LETTER_SPACE;
                Ts[CurrentFancyIndex] += Fancy->dT;
            }
        }
        
    }
    
    for(u32 I=8*(Length/16); I<Length; I++){
        char C = S[I];
        asset_font_glyph &Glyph = Font->Table[C];
        
        if(C == '\x02'){
            I++;
            Assert(I < Length);
            CurrentFancyIndex = S[I];
            Assert(CurrentFancyIndex <= FancyCount);
            CurrentFancyIndex--;
            Fancy = &Fancies[CurrentFancyIndex];
            continue;
        }else if(C == '\n'){
            P.X = StartP.X;
            P.Y -= Font->Height+FONT_VERTICAL_SPACE;
            Height += Font->Height+FONT_VERTICAL_SPACE;
            continue;
        }else if(C == '\r'){
            continue;
        }else if(C == ' '){
            f32 WordAdvance = FontWordAdvance(Font, S, I);
            if(P.X-StartP.X+WordAdvance >= MaxWidth){
                P.X = StartP.X;
                P.Y -= Font->Height+FONT_VERTICAL_SPACE;
                Height += Font->Height+FONT_VERTICAL_SPACE;
                continue;
            }else{
                P.X += Glyph.Width+FONT_LETTER_SPACE;
                continue;
            }
        }else if(P.X-StartP.X+Glyph.Width+FONT_LETTER_SPACE >= MaxWidth){
            P.X = StartP.X;
            P.Y -= Font->Height+FONT_VERTICAL_SPACE;
            Height += Font->Height+FONT_VERTICAL_SPACE;
        }
        
        
        {
            v2 CharP = P;
            if(Ts[CurrentFancyIndex] != 0.0f)
                CharP.Y += Fancy->Amplitude*Sin(Ts[CurrentFancyIndex]);
            
            f32 X0 = CharP.X;
            f32 Y0 = CharP.Y;
            f32 X1 = CharP.X + (f32)Glyph.Width;
            f32 Y1 = CharP.Y + Font->Height;
            
            f32 TX0 = (f32)(Glyph.Offset.X)              / Font->Size.Width;
            f32 TY0 = (f32)(Glyph.Offset.Y)              / Font->Size.Height;
            f32 TX1 = (f32)(Glyph.Offset.X+Glyph.Width)  / Font->Size.Width;
            f32 TY1 = (f32)(Glyph.Offset.Y+Font->Height) / Font->Size.Height;
            
            Vertices[4*K+0] = {V2(X0, Y0), Z, V2(TX0, TY0), Fancy->Color};
            Vertices[4*K+1] = {V2(X0, Y1), Z, V2(TX0, TY1), Fancy->Color};
            Vertices[4*K+2] = {V2(X1, Y1), Z, V2(TX1, TY1), Fancy->Color};
            Vertices[4*K+3] = {V2(X1, Y0), Z, V2(TX1, TY0), Fancy->Color};
            
            Indices[6*K+0] = 4*K+0;
            Indices[6*K+1] = 4*K+1;
            Indices[6*K+2] = 4*K+2;
            Indices[6*K+3] = 4*K+0;
            Indices[6*K+4] = 4*K+2;
            Indices[6*K+5] = 4*K+3;
            
            K++;
            P.X += (f32)Glyph.Width+FONT_LETTER_SPACE;
            Ts[CurrentFancyIndex] += Fancy->dT;
        }
    }
    RenderItem->IndexCount = 6*K;
    
    return Height;
}

#endif

//~ 
internal inline f32
FontRenderFancyString(game_renderer *Renderer, asset_font *Font, const fancy_font_format *Fancies, u32 FancyCount,
                      const char *S, rect R){
    v2 Size = RectSize(R);
    f32 Result = FontRenderFancyString(Renderer, Font, Fancies, FancyCount, V2(R.X0,R.Y1), S, Size.X);
    return Result;
}
