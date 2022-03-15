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

//~ Sound effects
struct sound_data {
    s16 *Samples;
    u16 ChannelCount;
    u32 SampleCount;
    f32 BaseSpeed;
};

struct asset_sound_effect {
    sound_data Sound;
};

//~ Fonts
global u32 FONT_VERTICAL_SPACE = 3;
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
    hash_table<const char *, direction> DirectionTable;
    hash_table<const char *, char> ASCIITable;
    
    file_reader Reader;
    file_token ExpectToken(file_token_type Type);
    u32        ExpectPositiveInteger_();
    
    array<s32>         ExpectTypeArrayS32();
    
    void InitializeLoader(memory_arena *Arena);
    
    b8 DoAttribute(const char *String, const char *Attribute);
    
    void LoadAssetFile(const char *Path);
    b8 ProcessCommand();
    b8 ProcessIgnore();
    b8 ProcessFont();
    b8 ProcessSoundEffect();
};

#endif //SNAIL_JUMPY_ASSET_H
