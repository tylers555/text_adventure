
internal inline render_texture
MakeAndUploadTexture(u8 *Pixels, u32 Width, u32 Height, u32 Channels){
    render_texture Result = MakeTexture();
    TextureUpload(Result, Pixels, Width, Height, Channels);
    return Result;
}

template <typename T> internal inline void 
AssetProcessorAssign(T **Attribute, u8 *Data){
    *Attribute = (T *)Data;
}

internal inline ta_data *
MakeTAData(memory_arena *Arena, ta_data_type Type, asset_tag Tag, asset_id ID){
    ta_data *Result = ArenaPushType(Arena, ta_data);
    Result->Type  = Type;
    Result->Tag   = Tag;
    Result->Asset = ID;
    return Result;
}

internal inline ta_data *
MakeTAData(memory_arena *Arena, ta_data_type Type, asset_tag Tag, const char *Data){
    string_builder Builder = BeginResizeableStringBuilder(&GlobalTransientMemory, DEFAULT_BUFFER_SIZE);
    BuilderAddVar(&Builder, Type);
    BuilderAddVar(&Builder, Tag);
    Assert(Builder.BufferSize == offsetof(ta_data, Data));
    BuilderAdd(&Builder, "%s", Data);
    ta_data *Result = (ta_data *)FinalizeStringBuilder(Arena, &Builder);
    
    return Result;
}

internal inline ta_name
MakeTAName(const char *Name, array<const char *> Aliases, array<const char *> Adjectives){
    ta_name Result = {};
    Result.Name       = Name; 
    Result.Aliases    = Aliases;
    Result.Adjectives = Adjectives;
    return Result;
}

internal inline asset_font_glyph
MakeAssetFontGlyph(v2s Offset, s32 Width){
    asset_font_glyph Result = {};
    Result.Offset = Offset;
    Result.Width = Width;
    return Result;
}

inline ta_area
MakeTAArea(asset_id Name, v2 Offset){
    ta_area Result = {};;
    Result.Name = Name;
    Result.Offset = Offset;
    return Result;
}

