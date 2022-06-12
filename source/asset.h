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
global_constant u32 MAX_VARIABLES = 256;

// @asset_tags
#define ASSET_TAGS \
ASSET_TAG("play",       Play) \
ASSET_TAG("take",       Take) \
ASSET_TAG("examine",    Examine) \
ASSET_TAG("eat",        Eat) \
ASSET_TAG("activate",   Activate) \
ASSET_TAG("talk",       Talk) \
ASSET_TAG("repair",     Repair) \
ASSET_TAG("use",        Use) \
ASSET_TAG("exit",       Exit) \
ASSET_TAG("enter",      Enter) \
\
ASSET_TAG("organ",      Organ) \
ASSET_TAG("bell-tower", BellTower) \
ASSET_TAG("inn",        Inn) \
ASSET_TAG("altar",      Altar) \
\
ASSET_TAG("broken",     Broken) \
ASSET_TAG("repaired",   Repaired) \
\
ASSET_TAG("locked",     Locked) \
ASSET_TAG("open-dawn",  OpenDawn) \
ASSET_TAG("open-noon",  OpenNoon) \
ASSET_TAG("open-dusk",  OpenDusk) \
ASSET_TAG("open-night", OpenNight) \
\
ASSET_TAG("items",      Items) \
ASSET_TAG("adjacents",  Adjacents) \
ASSET_TAG("sound",      Sound) \
\
ASSET_TAG("static",     Static) \
ASSET_TAG("key",        Key) \
ASSET_TAG("map",        Map) \
ASSET_TAG("light",      Light) \
ASSET_TAG("fixer",      Fixer) \
ASSET_TAG("bread",      Bread) \
\
ASSET_TAG("haunted",    Haunted) \
ASSET_TAG("ghost",      Ghost) \

#define ASSET_TAG(S, N) AssetTag_##N,
enum asset_tag_id {
    AssetTag_None = 0,
    
    ASSET_TAGS
        
        AssetTag_TOTAL
};
#undef ASSET_TAG

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
#define GetVariable(Assets, ID_) (&(Assets)->Variables[AssetVariable_##ID_])
#define GetVar(Assets, ID_)     GetVariable(Assets, ID_)->S
#define GetVarTAID(Assets, ID_) GetVariable(Assets, ID_)->TAID

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
#define GetVariable(Assets, ID_) (Assets)->GetVariableByString(MakeString(ID_.ID))
#define GetVar(Assets, ID_)     GetVariable(Assets, AssetID(ID_))->S
#define GetVarTAID(Assets, ID_) GetVariable(Assets, AssetID(ID_))->TAID

global_constant u32 ROOM_TABLE_SIZE = 64;
global_constant u32 ITEM_TABLE_SIZE = 128;
global_constant u32 THEME_TABLE_SIZE = 8;

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

global_constant u32 FONT_STRING_MAX_LINES = 16;
struct font_string_metrics {
    u32 LineCount;
    f32 LineWidths[FONT_STRING_MAX_LINES];
    v2 StartAdvance; // Used for ranges
    v2 Advance;
};

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

//~ Variables
struct asset_variable {
    const char *S;
    ta_id TAID;
};

//~ Asset system
struct asset_system {
    //~ Asset stuff
    memory_arena Memory;
    
    void Initialize(memory_arena *Arena);
    
#if defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)
    asset_sound_effect SoundEffects[AssetSoundEffect_TOTAL];
    asset_font         Fonts[AssetFont_TOTAL];
    asset_variable     Variables[AssetVariable_TOTAL];
#else
    hash_table<string, asset_sound_effect> SoundEffectTable;
    hash_table<string, asset_font> FontTable;
    hash_table<string, asset_variable> VariableTable;
    
    asset_sound_effect *GetSoundEffectByString(string Name);
    asset_font *GetFontByString(string Name);
    asset_variable *GetVariableByString(string Name);
    
    //~ Logging 
    const char *CurrentCommand;
    const char *CurrentAttribute;
    
    void BeginCommand(const char *Name);
    void LogError(const char *Format, ...);
    void LogInvalidAttribute(const char *Attribute);
    b8 SeekNextAttribute();
    
    //~ SJA reading and parsing
    u64 LastFileWriteTime;
    hash_table<const char *, char>         ASCIITable;
    hash_table<const char *, asset_tag_id> TagTable;
    hash_table<const char *, direction>    DirectionTable;
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
    
    b8 ProcessSpecialCommands();
    b8 ProcessVariables();
    b8 ProcessTheme();
    
    b8 ProcessSoundEffect();
    b8 ProcessFont();
    
    b8 ExpectDescriptionStrings(string_builder *Builder);
    b8 ProcessTADescription(dynamic_array<ta_data *> *Descriptions, ta_data_type Type=TADataType_Description);
    b8 ProcessTARoom();
    b8 ProcessTAItem();
    b8 ProcessTAMap();
#endif
    
    void LoadAssetFile(const char *Path);
    
    //~ Processed assets loading
    void LoadProcessedAssets(void *Data, u32 DataSize);
};

//~ Asset processing
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

//~ Miscellaneous
struct ta_system;
struct ta_item;
internal void GameProcessItem(ta_system *TA, ta_id ItemID, ta_item *Item);

#endif //SNAIL_JUMPY_ASSET_H
