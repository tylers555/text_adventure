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
    AssetTag_Examine,
    AssetTag_Eat,
    AssetTag_Activate,
    
    AssetTag_Organ,
    AssetTag_Broken,
    AssetTag_Repaired,
    
    AssetTag_Locked,
    AssetTag_OpenDawn,
    AssetTag_OpenNoon,
    AssetTag_OpenDusk,
    AssetTag_OpenNight,
    
    AssetTag_Items,
    AssetTag_Adjacents,
    
    AssetTag_Static,
    AssetTag_Bread,
    AssetTag_Key,
    AssetTag_Map,
    AssetTag_Light,
    
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
    color Color;
    f32 Amplitude;
    f32 Speed;
    f32 dT;
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
    Result.Color = Color;
    Result.Amplitude = Amplitude;
    Result.Speed = Speed;
    Result.dT = dT;
    return Result;
}

//~
struct ta_string;

//~ Asset system
struct asset_system {
    //~ Asset stuff
    memory_arena Memory;
    
    hash_table<string, asset_sound_effect> SoundEffects;
    hash_table<string, asset_font> Fonts;
    
    void Initialize(memory_arena *Arena);
    
    asset_sound_effect *GetSoundEffect(string Name);
    asset_font *GetFont(string Name);
    
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
    
    file_reader Reader;
    file_token ExpectToken(file_token_type Type);
    u32        ExpectPositiveInteger_();
    
    array<s32>          ExpectTypeArrayS32();
    array<const char *> ExpectTypeArrayCString();
    asset_tag           MaybeExpectTag();
    
    void InitializeLoader(memory_arena *Arena);
    
    b8 DoAttribute(const char *String, const char *Attribute);
    
    void LoadAssetFile(const char *Path);
    b8 ProcessCommand();
    b8 ProcessIgnore();
    b8 ProcessSoundEffect();
    b8 ProcessFont();
    
    b8 ProcessTADescription(dynamic_array<ta_string *> *Descriptions);
    b8 ProcessTARoom();
    b8 ProcessTAItem();
};

#endif //SNAIL_JUMPY_ASSET_H
