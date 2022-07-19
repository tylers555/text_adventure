#define SNAIL_JUMPY_ASSET_PROCESSOR_BUILD
#define SNAIL_JUMPY_DEBUG_BUILD
#include "main.h"

asset_processor AssetProcessor;

//~ Stubs
internal render_texture 
MakeTexture(texture_flags Flags){
    render_texture Result = {AssetProcessor.Textures.Count};
    ArrayAlloc(&AssetProcessor.Textures);
    return Result;
}

internal void
TextureUpload(render_texture Texture, u8 *Pixels, u32 Width, u32 Height, u32 Channels){
    AssetProcessor.Textures[Texture.ID].Pixels   = ArenaPushArray(&GlobalPermanentMemory, u8, Width*Height*Channels);
    CopyMemory(AssetProcessor.Textures[Texture.ID].Pixels, Pixels, Width*Height*Channels);
    AssetProcessor.Textures[Texture.ID].Width    = Width;
    AssetProcessor.Textures[Texture.ID].Height   = Height;
    AssetProcessor.Textures[Texture.ID].Channels = Channels;
}

internal b8 RendererBackendInitialize(){ return true; }
internal shader_program  MakeShaderProgramFromFileData(entire_file File){ return {}; }
internal screen_shader MakeScreenShaderFromFileData(entire_file File){ return {}; }
internal s32 ShaderProgramGetUniformLocation(shader_program Program, const char *Name){ return 1; }

internal void InitializeFramebuffer(framebuffer *Framebuffer, screen_shader ScreenShader, v2s Size){}
internal void ResizeFramebuffer(framebuffer *Framebuffer, v2s Size){}
internal void UseFramebuffer(framebuffer *Framebuffer){}

internal void RendererBackendRenderAll(game_renderer *Renderer){}

#include "main.cpp"

//~ 
// TODO(Tyler): I don't like this solution, but I don't have a better way of doing it right now. 
// This is here so that arrays can know what type their arguments are. 
#define DEFINE_TYPE_NAME(Type) internal inline const char *AssetProcessorTypeName(Type *V){ return #Type;}

DEFINE_TYPE_NAME(const char *);
DEFINE_TYPE_NAME(ta_data *);
DEFINE_TYPE_NAME(asset_id);
DEFINE_TYPE_NAME(ta_area);

internal inline void
AssetProcessorEmitDataSizeCheck(){
    BuilderAdd(&AssetProcessor.AssetBuilder, "Assert(%u < DataSize);\n", AssetProcessor.SJAPBuilder.BufferSize); 
}

internal inline void
AssetProcessorMaybeEmitDataSizeCheck(){
    if(!AssetProcessor.DoEmitCheck) return;
    AssetProcessorEmitDataSizeCheck();
    AssetProcessor.DoEmitCheck = false;
}

internal inline const char *
AssetProcessorStringify(u64 Uint){
    return ArenaPushFormatCString(&GlobalTransientMemory, "%llu", Uint);
}
internal inline const char *AssetProcessorStringify(u8  Uint) { return AssetProcessorStringify((u64)Uint); }
internal inline const char *AssetProcessorStringify(u16 Uint) { return AssetProcessorStringify((u64)Uint); }
internal inline const char *AssetProcessorStringify(u32 Uint) { return AssetProcessorStringify((u64)Uint); }

internal inline const char *
AssetProcessorStringify(s64 Int){
    return ArenaPushFormatCString(&GlobalTransientMemory, "%lld", Int);
}
internal inline const char *AssetProcessorStringify(s8  Int) { return AssetProcessorStringify((s64)Int); }
internal inline const char *AssetProcessorStringify(s16 Int) { return AssetProcessorStringify((s64)Int); }
internal inline const char *AssetProcessorStringify(s32 Int) { return AssetProcessorStringify((s64)Int); }

internal inline const char *
AssetProcessorStringify(f32 Float){
    return ArenaPushFormatCString(&GlobalTransientMemory, "%ff", Float);
}

internal inline const char *
AssetProcessorStringify(v2s V){
    return ArenaPushFormatCString(&GlobalTransientMemory, "V2S(%d, %d)", V.X, V.Y);
}

internal inline const char *
AssetProcessorStringify(v2 V){
    return ArenaPushFormatCString(&GlobalTransientMemory, "V2(%ff, %ff)", V.X, V.Y);
}

internal inline const char *
AssetProcessorStringify(const char *S){
    if(!S) return "0";
    string_builder Builder = BeginResizeableStringBuilder(&GlobalTransientMemory, DEFAULT_BUFFER_SIZE);
    BuilderAdd(&Builder, '"');
    
    u32 Length = CStringLength(S);
    for(u32 I=0; S[I]; I++){
        char C = S[I];
        
        if(C == '\n'){
            BuilderAdd(&Builder, "\\n");
            continue;
        }else if(C == '\r'){
            BuilderAdd(&Builder, "\\r");
            continue;
        }else if(C == '\002'){
            I++;
            Assert(I < Length);
            C = S[I];
            BuilderAdd(&Builder, "\\002\\00%d", C);
            continue;
        }
        
        BuilderAdd(&Builder, C);
    }
    
    BuilderAdd(&Builder, '"');
    const char *Result = EndStringBuilder(&Builder);
    return Result;
}

internal inline const char *
AssetProcessorStringify(render_texture TextureID){
    string_builder *AssetBuilder = &AssetProcessor.AssetBuilder;
    string_builder *SJAPBuilder  = &AssetProcessor.SJAPBuilder;
    
    asset_processor_texture *Texture = &AssetProcessor.Textures[TextureID.ID]; 
    const char *Result = ArenaPushFormatCString(&GlobalTransientMemory, 
                                                "MakeAndUploadTexture(((u8 *)Data+%u), %u, %u, %u)", 
                                                SJAPBuilder->BufferSize, Texture->Width, Texture->Height, Texture->Channels); 
    BuilderAddData(SJAPBuilder, Texture->Pixels, Texture->Width*Texture->Height*Texture->Channels); 
    AssetProcessor.DoEmitCheck = true;
    
    return Result;
}

internal inline const char *
AssetProcessorStringify(color Color){
    return ArenaPushFormatCString(&GlobalTransientMemory, "MakeColor(%ff, %ff, %ff, %ff)", 
                                  Color.R, Color.G, Color.B, Color.A);
}

internal inline const char *
AssetProcessorStringify(fancy_font_format Fancy){
    return ArenaPushFormatCString(&GlobalTransientMemory, "MakeFancyFormat(MakeColor(%ff, %ff, %ff, %ff), MakeColor(%ff, %ff, %ff, %ff), %ff, %ff, %ff, %ff, %ff, %ff)", 
                                  Fancy.Color1.R, Fancy.Color1.G, Fancy.Color1.B, Fancy.Color1.A,
                                  Fancy.Color2.R, Fancy.Color2.G, Fancy.Color2.B, Fancy.Color2.A, 
                                  Fancy.Amplitude, Fancy.Speed, Fancy.dT,
                                  Fancy.ColorSpeed, Fancy.ColordT, Fancy.ColorTOffset);
}

internal inline const char *
AssetProcessorStringify(asset_id ID){
    const char *Result = AssetIDName(0, ID);
    if(Result) return ArenaPushFormatCString(&GlobalTransientMemory, "MakeAssetID(%sID_%s)", ID.TableName, Result);
    return "MakeAssetID(0)";
}

internal inline const char *
AssetProcessorStringify(asset_tag Tag){
    return ArenaPushFormatCString(&GlobalTransientMemory,
                                  "MakeAssetTag((asset_tag_id)%u, (asset_tag_id)%u, (asset_tag_id)%u, (asset_tag_id)%u)",
                                  Tag.E[0], Tag.E[1], Tag.E[2], Tag.E[3]);
}

internal inline const char *
AssetProcessorStringify(ta_data *Data){
    const char *Extra = 0;
    switch(Data->Type){
        case TADataType_Asset: 
        case TADataType_Room: 
        case TADataType_Item: {
            Extra = AssetProcessorStringify(Data->Asset);
        }break;
        case TADataType_Description:
        case TADataType_Command: {
            Extra = AssetProcessorStringify(Data->Data);
        }break;
    }
    
    return ArenaPushFormatCString(&GlobalTransientMemory, "MakeTAData(Arena, (ta_data_type)%u, %s, %s)",
                                  Data->Type, AssetProcessorStringify(Data->Tag), Extra);
}

template<typename T> internal inline const char *
AssetProcessorStringify(array<T> Array){
    string_builder Builder = BeginResizeableStringBuilder(&GlobalTransientMemory, DEFAULT_BUFFER_SIZE);
    BuilderAdd(&Builder, "MakeFullArrayFromArgs_<%s>(Arena, %u", AssetProcessorTypeName((T *)0), Array.Count);
    FOR_EACH_(Item, Index, &Array) BuilderAdd(&Builder, ", %s", AssetProcessorStringify(Item)); 
    BuilderAdd(&Builder, ")");
    return EndStringBuilder(&Builder);
}

internal inline const char *
AssetProcessorStringify(ta_name Name){
    return ArenaPushFormatCString(&GlobalTransientMemory, "MakeTAName(%s, %s, %s)", 
                                  AssetProcessorStringify(Name.Name), AssetProcessorStringify(Name.Aliases), AssetProcessorStringify(Name.Adjectives));
}

internal inline const char *
AssetProcessorStringify(ta_area Area){
    return ArenaPushFormatCString(&GlobalTransientMemory, "MakeTAArea(%s, %s)", 
                                  AssetProcessorStringify(Area.Name), AssetProcessorStringify(Area.Offset));
}

template<typename KeyType, typename ValueType> internal constexpr inline const char *
AssetProcessorAttributeName(const char *Name, const char *Attribute,
                            hash_table_bucket<KeyType, ValueType> *Bucket){
    return ArenaPushFormatCString(&GlobalTransientMemory, "Assets[%sID_%s].%s", 
                                  Name, AssetIDName(0, Bucket->Key), Attribute);
}

internal inline const char *
AssetProcessorStringify(asset_font_glyph Glyph){
    return ArenaPushFormatCString(&GlobalTransientMemory, "MakeAssetFontGlyph(%s, %s)", 
                                  AssetProcessorStringify(Glyph.Offset), AssetProcessorStringify(Glyph.Width));
}

internal inline b8
AssetProcessorMain(){
    AssetProcessor.Textures = MakeDynamicArray<asset_processor_texture>(512);
    main_state State = {};
    MainStateInitialize(&State, 0, 0);
    if(State.AssetLoader.LoadAssetFile(ASSET_FILE_PATH) != AssetLoadingStatus_Okay){
        return false;
    }
    
    asset_system *Assets = &State.Assets;
    ta_system *TA = &State.TextAdventure;
    
    ArenaClear(&GlobalTransientMemory);
    AssetProcessor.SJAPBuilder  = BeginResizeableStringBuilder(&GlobalTransientMemory, Megabytes(200));
    AssetProcessor.IDBuilder    = BeginResizeableStringBuilder(&GlobalTransientMemory, Megabytes(2));
    AssetProcessor.NameBuilder  = BeginResizeableStringBuilder(&GlobalTransientMemory, Megabytes(2));
    AssetProcessor.AssetBuilder = BeginResizeableStringBuilder(&GlobalTransientMemory, Megabytes(2));
    
    string_builder *SJAPBuilder  = &AssetProcessor.SJAPBuilder;
    string_builder *IDBuilder    = &AssetProcessor.IDBuilder;
    string_builder *NameBuilder  = &AssetProcessor.NameBuilder;
    string_builder *AssetBuilder = &AssetProcessor.AssetBuilder;
    
    sjap_header Header = {};
    Header.SJAP[0] = 'S';
    Header.SJAP[1] = 'J';
    Header.SJAP[2] = 'A';
    Header.SJAP[3] = 'P';
    BuilderAddVar(SJAPBuilder, Header);
    
    BuilderAdd(IDBuilder, 
               "#if !defined(GENERATED_ASSET_ID_H) && defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)\n"
               "#define GENERATED_ASSET_ID_H\n"
               "enum {\n");
    
    BuilderAdd(AssetBuilder, 
               "#if !defined(GENERATED_ASSET_DATA_H) && defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)\n"
               "#define GENERATED_ASSET_DATA_H\n");
    
    dynamic_array<asset_id> IDArray = MakeDynamicArray<asset_id>(&GlobalTransientMemory, 128);
    
    //~ Core 
    //- Emit table 
#define PROCESSOR_BEGIN_EMIT_TABLE(System, Name, Type) \
BuilderAdd(AssetBuilder, \
"internal inline void\n" \
"ProcessedAssetsInitialize" #Name "(memory_arena *Arena, "#Type" Assets[" #Name "ID_TOTAL], void *Data, u32 DataSize){\n"); \
BuilderAdd(NameBuilder, "global const char *"#Name"NameTable[] = {"); \
PROCESSOR_BEGIN_EMIT_ASSET(System, Name)
    
#define PROCESSOR_END_EMIT_TABLE(System, Name) \
PROCESSOR_END_EMIT_ASSET(System, Name); \
BuilderAdd(AssetBuilder, "\n}\n\n"); \
BuilderAdd(NameBuilder,  "\n};\n");
    
#define PROCESSOR_BEGIN_EMIT_ASSET(System, Name) \
HASH_TABLE_FOR_EACH_BUCKET_(Bucket, I, Index, &(System)->Name##Table){ \
ASSET_TABLE_EMIT_ID(Name); \
auto &Asset = Bucket.Value; \
BuilderAdd(AssetBuilder, "{\n"); \
BuilderAdd(NameBuilder, "\n\"%s\",", AssetIDName(0, Bucket.Key)); \
AssetProcessorEmitDataSizeCheck()
    
#define PROCESSOR_END_EMIT_ASSET(System, Name) \
BuilderAdd(AssetBuilder, "\n}"); \
} \
ASSET_TABLE_EMIT_ID_TOTAL(System, Name);
    
    //- Emit attributes
#define ASSET_TABLE_EMIT_ID_(Name, ID) \
BuilderAdd(IDBuilder, #Name"ID_%s = %u,\n", AssetIDName(0, ID), Index); \
ArrayAdd(&IDArray, ID);
    
#define ASSET_TABLE_EMIT_ID(Name) ASSET_TABLE_EMIT_ID_(Name, Bucket.Key)
    
#define ASSET_TABLE_EMIT_ID_TOTAL_(Name, Count) \
BuilderAdd(IDBuilder, #Name"ID_TOTAL = %u,\n\n", Count)
    
#define ASSET_TABLE_EMIT_ID_TOTAL(System, Name) ASSET_TABLE_EMIT_ID_TOTAL_(Name, (System)->Name##Table.Count);
    
#define ASSET_TABLE_EMIT_ATTRIBUTE(Name, Attribute) \
AssetProcessor.CurrentAttribute = AssetProcessorAttributeName(#Name, #Attribute, &Bucket); \
BuilderAdd(AssetBuilder, "%s = %s;\n", \
AssetProcessor.CurrentAttribute, AssetProcessorStringify(Asset.Attribute)); \
AssetProcessorMaybeEmitDataSizeCheck();
    
#define ASSET_TABLE_EMIT_DATA_ARRAY(Name, Attribute, Count) \
AssetProcessor.CurrentAttribute = AssetProcessorAttributeName(#Name, #Attribute, &Bucket); \
AssetProcessorEmitDataSizeCheck(); \
BuilderAdd(AssetBuilder, "AssetProcessorAssign(&%s, (u8 *)Data+%u);", \
AssetProcessor.CurrentAttribute, SJAPBuilder->BufferSize); \
BuilderAddData(SJAPBuilder, Asset.Attribute, sizeof(Asset.Attribute)*Count); \
    
#define ASSET_TABLE_EMIT_C_ARRAY(Name, Attribute) \
AssetProcessor.CurrentAttribute = AssetProcessorAttributeName(#Name, #Attribute, &Bucket); \
FOR_RANGE(I, 0, ArrayCount(Asset.Attribute)){ \
BuilderAdd(AssetBuilder, "%s[%u] = %s;\n", \
AssetProcessor.CurrentAttribute, I, AssetProcessorStringify(Asset.Attribute[I])); \
}
    
    //~ Assets
    //- Sound effects
    {
        PROCESSOR_BEGIN_EMIT_TABLE(Assets, SoundEffect, asset_sound_effect);
        
        ASSET_TABLE_EMIT_ATTRIBUTE(SoundEffect, Sound.ChannelCount);
        ASSET_TABLE_EMIT_ATTRIBUTE(SoundEffect, Sound.SampleCount);
        ASSET_TABLE_EMIT_ATTRIBUTE(SoundEffect, Sound.BaseSpeed);
        ASSET_TABLE_EMIT_ATTRIBUTE(SoundEffect, VolumeMultiplier);
        ASSET_TABLE_EMIT_DATA_ARRAY(SoundEffect, Sound.Samples, Asset.Sound.ChannelCount*Asset.Sound.SampleCount);
        
        PROCESSOR_END_EMIT_TABLE(Assets, SoundEffect);
    }
    
    //- Fonts
    {
        PROCESSOR_BEGIN_EMIT_TABLE(Assets, Font, asset_font);
        
        ASSET_TABLE_EMIT_ATTRIBUTE(Font, Size);
        ASSET_TABLE_EMIT_ATTRIBUTE(Font, Height);
        ASSET_TABLE_EMIT_ATTRIBUTE(Font, Descent);
        ASSET_TABLE_EMIT_C_ARRAY(Font, Table);
        ASSET_TABLE_EMIT_ATTRIBUTE(Font, Texture);
        
        PROCESSOR_END_EMIT_TABLE(Assets, Font);
    }
    
    //- Variables
    {
        PROCESSOR_BEGIN_EMIT_TABLE(Assets, Variable, asset_variable);
        
        ASSET_TABLE_EMIT_ATTRIBUTE(Variable, S);
        ASSET_TABLE_EMIT_ATTRIBUTE(Variable, TAID);
        ASSET_TABLE_EMIT_ATTRIBUTE(Variable, Asset);
        
        PROCESSOR_END_EMIT_TABLE(Assets, Variable);
    }
    
    //- Themes 
    {
        PROCESSOR_BEGIN_EMIT_TABLE(Assets, Theme, console_theme);
        
        ASSET_TABLE_EMIT_ATTRIBUTE(Theme, BasicFont); 
        ASSET_TABLE_EMIT_ATTRIBUTE(Theme, TitleFont); 
        ASSET_TABLE_EMIT_ATTRIBUTE(Theme, BackgroundColor); 
        ASSET_TABLE_EMIT_ATTRIBUTE(Theme, CursorColor); 
        ASSET_TABLE_EMIT_ATTRIBUTE(Theme, SelectionColor); 
        ASSET_TABLE_EMIT_ATTRIBUTE(Theme, RoomTitleFancy);
        ASSET_TABLE_EMIT_ATTRIBUTE(Theme, BasicFancy);
        ASSET_TABLE_EMIT_ATTRIBUTE(Theme, DirectionFancy);
        ASSET_TABLE_EMIT_ATTRIBUTE(Theme, RoomFancy);
        ASSET_TABLE_EMIT_ATTRIBUTE(Theme, ItemFancy);
        ASSET_TABLE_EMIT_ATTRIBUTE(Theme, MiscFancy);
        ASSET_TABLE_EMIT_ATTRIBUTE(Theme, MoodFancy);
        ASSET_TABLE_EMIT_ATTRIBUTE(Theme, ResponseFancies[0]);
        ASSET_TABLE_EMIT_ATTRIBUTE(Theme, ResponseFancies[1]);
        
        PROCESSOR_END_EMIT_TABLE(Assets, Theme);
    }
    
    //- Rooms
    {
        PROCESSOR_BEGIN_EMIT_TABLE(TA, Room, ta_room);
        
        ASSET_TABLE_EMIT_ATTRIBUTE(Room, ID);
        ASSET_TABLE_EMIT_ATTRIBUTE(Room, Flags);
        ASSET_TABLE_EMIT_ATTRIBUTE(Room, NameData);
        ASSET_TABLE_EMIT_ATTRIBUTE(Room, Area);
        ASSET_TABLE_EMIT_ATTRIBUTE(Room, Tag);
        ASSET_TABLE_EMIT_ATTRIBUTE(Room, Datas);
        ASSET_TABLE_EMIT_ATTRIBUTE(Room, Items);
        ASSET_TABLE_EMIT_C_ARRAY(Room, Adjacents);
        ASSET_TABLE_EMIT_C_ARRAY(Room, AdjacentTags);
        
        PROCESSOR_END_EMIT_TABLE(TA, Room);
    }
    
    //- Rooms
    {
        PROCESSOR_BEGIN_EMIT_TABLE(TA, Item, ta_item);
        
        ASSET_TABLE_EMIT_ATTRIBUTE(Item, ID);
        ASSET_TABLE_EMIT_ATTRIBUTE(Item, NameData);
        ASSET_TABLE_EMIT_ATTRIBUTE(Item, Tag);
        ASSET_TABLE_EMIT_ATTRIBUTE(Item, Cost);
        ASSET_TABLE_EMIT_ATTRIBUTE(Item, Datas);
        
        PROCESSOR_END_EMIT_TABLE(TA, Item);
    }
    
    //- Map
    {
        ta_map *Map = &TA->Map;
        
        BuilderAdd(AssetBuilder, "internal inline void\n" 
                   "ProcessedAssetsInitializeMap(void *Data, u32 DataSize, ta_map *Map, memory_arena *Arena){\n"); 
        BuilderAdd(AssetBuilder, "Map->Texture = %s;\n", AssetProcessorStringify(Map->Texture));
        BuilderAdd(AssetBuilder, "Map->Size    = %s;\n", AssetProcessorStringify(Map->Size));
        BuilderAdd(AssetBuilder, "Map->Areas   = %s;\n", AssetProcessorStringify(Map->Areas));
        BuilderAdd(AssetBuilder, "\n}\n\n");
        
        FOR_EACH_(Area, Index, &Map->Areas){
            ASSET_TABLE_EMIT_ID_(Area, Area.Name);
        }
        ASSET_TABLE_EMIT_ID_TOTAL_(Area, Map->Areas.Count);
    }
    
    //~ Output to file
    BuilderAdd(IDBuilder, 
               "};\n"
               "%s"
               "\n#endif // GENERATED_ASSET_ID_H\n",
               EndStringBuilder(NameBuilder));
    
    BuilderAdd(AssetBuilder, 
               "#endif // GENERATED_ASSET_DATA_H\n");
    
    {
        os_file *OutputFile = OSOpenFile("./processed_assets.sjap", OpenFile_Write|OpenFile_Clear);
        Assert(OutputFile);
        BuilderToFile(SJAPBuilder, OutputFile);
        OSCloseFile(OutputFile);
    }
    
    {
        os_file *OutputFile = OSOpenFile("../source/generated_asset_id.h", OpenFile_Write|OpenFile_Clear);
        Assert(OutputFile);
        BuilderToFile(IDBuilder, OutputFile);
        OSCloseFile(OutputFile);
    }
    
    {
        os_file *OutputFile = OSOpenFile("../source/generated_asset_data.h", OpenFile_Write|OpenFile_Clear);
        Assert(OutputFile);
        BuilderToFile(AssetBuilder, OutputFile);
        OSCloseFile(OutputFile);
    }
    
    return true;
}