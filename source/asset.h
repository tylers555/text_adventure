#ifndef SNAIL_JUMPY_ASSET_H
#define SNAIL_JUMPY_ASSET_H

//~ Loading
struct image {
    b8 HasBeenLoadedBefore;
    u64 LastWriteTime;
    b8 IsTranslucent;
    render_texture Texture;
    union{
        struct { s32 Width, Height; };
        v2s Size;
    };
};

//~ Assets

global_constant u32 SJA_MAX_ARRAY_ITEM_COUNT = 256;
global_constant u32 MAX_ASSETS_PER_TYPE = 128;

enum asset_tag_id {
    AssetTag_None = 0,
    AssetTag_Play,
    AssetTag_Take,
    AssetTag_Examine,
    AssetTag_Eat,
    AssetTag_Activate,
    AssetTag_Talk,
    AssetTag_Repair,
    
    AssetTag_Organ,
    AssetTag_BellTower,
    
    AssetTag_Broken,
    AssetTag_Repaired,
    
    AssetTag_Locked,
    AssetTag_OpenDawn,
    AssetTag_OpenNoon,
    AssetTag_OpenDusk,
    AssetTag_OpenNight,
    
    AssetTag_Dark,
    
    AssetTag_Items,
    AssetTag_Adjacents,
    AssetTag_Sound,
    
    AssetTag_Static,
    AssetTag_Key,
    AssetTag_Map,
    AssetTag_Light,
    AssetTag_Fixer,
    
    AssetTag_Bread,
    AssetTag_Exit,
    AssetTag_Enter,
    AssetTag_Use,
    
    AssetTag_TOTAL
};

union asset_tag {
    struct {
        enum8(asset_tag_id) A;
        enum8(asset_tag_id) B;
        enum8(asset_tag_id) C;
        enum8(asset_tag_id) D;
    };
    enum8(asset_tag_id) E[4];
    u32 All;
};

//~ Asset ID
struct asset_id {
    u64 ID;
};

#if defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)
internal inline asset_id
MakeAssetID(u32 ID){
    asset_id Result;
    Result.ID = ID;
    return Result;
}

#define AssetID(Name) MakeAssetID(AssetID_##Name)
#define GetSoundEffect(Assets, ID_) &(Assets)->SoundEffects[ID_.ID]
#define GetFont(Assets, ID_) &(Assets)->Fonts[ID_.ID]
#else // !defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)
internal inline asset_id
MakeAssetID(string ID){
    asset_id Result;
    Result.ID = ID.ID;
    return Result;
}

internal inline asset_id
MakeAssetID(const char *S){
    return MakeAssetID(Strings.GetString(S));
}

#define AssetID(Name) MakeAssetID(#Name)
#define AssetIDName(ID_) Strings.GetString(MakeString((ID_).ID))
#define GetSoundEffect(Assets, ID_) (Assets)->GetSoundEffectByString(MakeString(ID_.ID))
#define GetFont(Assets, ID_) (Assets)->GetFontByString(MakeString(ID_.ID))

global_constant u32 ROOM_TABLE_SIZE = 64;
global_constant u32 ITEM_TABLE_SIZE = 128;

#endif

//~ Sound effects
struct sound_data {
    s16 *Samples;
    u16 ChannelCount;
    u32 SampleCount;
    f32 BaseSpeed;
};

struct asset_sound_effect {
    sound_data Sound;
    f32 VolumeMultiplier;
};

//~ Fonts
global u32 FONT_VERTICAL_SPACE = 3;
global u32 FONT_LETTER_SPACE = 1;

struct fancy_font_format {
    color Color1;
    color Color2;
    f32 Amplitude;
    f32 Speed;
    f32 dT;
    
    f32 ColorSpeed;
    f32 ColordT;
    f32 ColorTOffset;
};

struct asset_font_glyph {
    v2s Offset;
    s32 Width;
};

struct asset_font {
    render_texture Texture;
    v2s Size;
    f32 Height;
    f32 Descent;
    
    asset_font_glyph Table[128];
};

internal inline fancy_font_format
MakeFancyFormat(color Color, f32 Amplitude, f32 Speed, f32 dT){
    fancy_font_format Result = {};
    Result.Color1 = Color;
    Result.Amplitude = Amplitude;
    Result.Speed = 0.5f*PI*Speed;
    Result.dT = 0.5f*PI*dT;
    return Result;
}

internal inline fancy_font_format
MakeFancyFormat(color Color1, color Color2, 
                f32 Amplitude, f32 Speed, f32 dT, 
                f32 ColorSpeed, f32 ColordT, f32 ColorTOffset){
    fancy_font_format Result = {};
    Result.Color1 = Color1;
    Result.Color2 = Color2;
    Result.Amplitude = Amplitude;
    Result.Speed = Speed;
    Result.dT = 0.5f*PI*dT;
    Result.ColorSpeed = 0.5f*PI*ColorSpeed;
    Result.ColordT = 0.5f*PI*ColordT;
    Result.ColorTOffset = 0.5f*PI*ColorTOffset;
    return Result;
}

//~ Text adventure stuff

struct ta_id {
    u64 ID;
};

enum ta_data_type {
    TADataType_None,
    TADataType_Asset,
    TADataType_Room,
    TADataType_Item,
    TADataType_Description,
    TADataType_Command,
};

struct ta_data {
    ta_data_type Type;
    asset_tag Tag;
    union{
        asset_id Asset;
        ta_id TAID;
        const char Data[];
    };
};

struct ta_name {
    const char *Name;
    array<const char *> Aliases;
    array<const char *> Adjectives;
};

//~ Asset system
struct asset_system {
    //~ Asset stuff
    memory_arena Memory;
    
    void Initialize(memory_arena *Arena);
    
#if defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)
    asset_sound_effect SoundEffects[AssetSoundEffect_TOTAL];
    asset_font         Fonts[AssetFont_TOTAL];
#else
    hash_table<string, asset_sound_effect> SoundEffectTable;
    hash_table<string, asset_font> FontTable;
    
    asset_sound_effect *GetSoundEffectByString(string Name);
    asset_font *GetFontByString(string Name);
    
    
    //~ Logging 
    const char *CurrentCommand;
    const char *CurrentAttribute;
    
    void BeginCommand(const char *Name);
    void LogError(const char *Format, ...);
    void LogInvalidAttribute(const char *Attribute);
    
    //~ SJA reading and parsing
    u64 LastFileWriteTime;
    hash_table<const char *, char>         ASCIITable;
    hash_table<const char *, asset_tag_id> TagTable;
    hash_table<const char *, image> LoadedImageTable;
    
    file_reader Reader;
    
    u32 ExpectPositiveInteger_();
    image *LoadImage(const char *Path);
    
    fancy_font_format   ExpectTypeFancy();
    asset_tag           MaybeExpectTag();
    ta_name             ExpectTypeName();
    
    void InitializeLoader(memory_arena *Arena);
    
    b8 DoAttribute(const char *String, const char *Attribute);
    
    b8 ProcessCommand();
    b8 ProcessIgnore();
    
    b8 ProcessVariables();
    b8 ProcessTheme();
    
    b8 ProcessSoundEffect();
    b8 ProcessFont();
    
    b8 ProcessTADescription(dynamic_array<ta_data *> *Descriptions, ta_data_type Type=TADataType_Description);
    b8 ProcessTARoom();
    b8 ProcessTAItem();
    b8 ProcessTAMap();
#endif
    
    void LoadAssetFile(const char *Path);
    
    //~ Processed assets loading
    void LoadProcessedAssets(void *Data, u32 DataSize);
};

struct asset_processor_texture {
    u8 *Pixels;
    u32 Width;
    u32 Height;
    u32 Channels;
};

struct asset_processor {
    array<asset_processor_texture> Textures;
};

#pragma pack(push, 1)
struct sjap_header {
    char SJAP[4];
};
#pragma pack(pop)

#endif //SNAIL_JUMPY_ASSET_H
