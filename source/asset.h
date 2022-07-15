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

enum asset_loading_status {
    AssetLoadingStatus_Okay,
    AssetLoadingStatus_Warnings,
    AssetLoadingStatus_Errors,
};

struct asset_loading_data {
#if defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)
#else
    asset_loading_status Status;
#endif
};

internal inline b8 
IsLoadedAssetValid(asset_loading_data *Data){
    return (Data->Status != AssetLoadingStatus_Errors);
}

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
ASSET_TAG("read",       Read) \
ASSET_TAG("feed",       Feed) \
\
ASSET_TAG("organ",          Organ) \
ASSET_TAG("carillon-pages", CarillonPages) \
ASSET_TAG("bell-tower",     BellTower) \
ASSET_TAG("inn",            Inn) \
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
ASSET_TAG("food",       Food) \
\
ASSET_TAG("haunted",    Haunted) \
ASSET_TAG("ghost",      Ghost) \
ASSET_TAG("ritual",     Ritual) \
ASSET_TAG("override",   Override) \

#define ASSET_TAG(S, N) AssetTag_##N,
enum asset_tag_id {
    AssetTag_None = 0,
    
    ASSET_TAGS
        
        AssetTag_TOTAL
};
#undef ASSET_TAG

union asset_tag {
    struct {
        u8 A;
        u8 B;
        u8 C;
        u8 D;
    };
    u8 E[4];
    u32 All;
};

//~ Asset ID
struct asset_id {
    u64 ID;
};

internal inline b8
operator==(asset_id A, asset_id B){
    return (A.ID == B.ID);
}

internal inline b8
operator!=(asset_id A, asset_id B){
    return (A.ID != B.ID);
}

//~
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
#define GetVar(Assets, ID_)      GetVariable(Assets, ID_)->S
#define GetVarTAID(Assets, ID_)  GetVariable(Assets, ID_)->TAID
#define GetVarAsset(Assets, ID_) GetVariable(Assets, ID_)->Asset
#define GetVarName(Assets, ID_)  GetVariable(Assets, ID_)->NameData

//~ 
#else 
internal inline asset_id
MakeAssetID(string ID){
    asset_id Result;
    Result.ID = ID.ID;
    return Result;
}

internal inline asset_id
MakeAssetID(asset_id ID){
    asset_id Result;
    Result.ID = ID.ID;
    return Result;
}

internal inline asset_id
MakeAssetID(const char *S){
    return MakeAssetID(Strings.GetString(S));
}

template<typename ValueType>
internal ValueType *
AssetTableFindByKey_(hash_table<asset_id, ValueType> *Table, asset_id Name){
    if(Name.ID){
        ValueType *Result = HashTableFindPtr(Table, Name);
        if(Result && !IsLoadedAssetValid(&Result->LoadingData)) return 0;
        return Result;
    }
    return 0;
}

internal constexpr u64
HashKey(asset_id Value) {
    u64 Result = Value.ID;
    return(Result);
}

internal constexpr b32
CompareKeys(asset_id A, asset_id B){
    b32 Result = (A.ID == B.ID);
    return(Result);
}

#define asset_table(Name, ValueType) hash_table<asset_id, ValueType> Name##Table
#define AssetTableInit(Name, Arena, MaxCount) HashTableInit(&(Name##Table), Arena, MaxCount)
#define AssetTableGet_(Prefix, Name, Key) HashTableGetPtr(&(Prefix Name##Table), MakeAssetID(Key))
#define AssetTableFind_(Prefix, Name, Key) AssetTableFindByKey_(&(Prefix Name##Table), MakeAssetID(Key))

#define AssetsGet_(System, Name, Key) AssetTableGet_((System)->, Name, Key)
#define AssetsFind_(System, Name, Key) AssetTableFind_((System)->, Name, Key)
#define AssetsFind(System, Name, Key) AssetsFind_(System, Name, #Key)

#define AssetID(Name, ID_) MakeAssetID(#ID_)
#define AssetIDName(Name, ID_) Strings.GetString(MakeString((ID_).ID))

#define GetVar(Assets, ID_)      AssetsFind(Assets, Variable, ID_)->S
#define GetVarTAID(Assets, ID_)  AssetsFind(Assets, Variable, ID_)->TAID
#define GetVarAsset(Assets, ID_) AssetsFind(Assets, Variable, ID_)->Asset
#define GetVarName(Assets, ID_)  AssetsFind(Assets, Variable, ID_)->NameData

#define TAFind_(System, Name, Key) AssetTableFind_((System)->, Name, Key)
#define TAFind(System, Name, Key) TAFind_(System, Name, #Key)

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
    asset_loading_data LoadingData;
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
    asset_loading_data LoadingData;
    
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

internal inline fancy_font_format
MakeFancyFormat(color Color){
    fancy_font_format Result = {};
    Result.Color1 = Color;
    return Result;
}


global_constant u32 FONT_STRING_MAX_LINES = 16;
struct font_string_metrics {
    u32 LineCount;
    f32 LineWidths[FONT_STRING_MAX_LINES];
    v2 StartAdvance; // Used for ranges
    v2 Advance;
};

//~ Theme
global_constant color BASE_BACKGROUND_COLOR = MakeColor(0x0a0d4aff);
global_constant color BASIC_COLOR      = MakeColor(0xf2f2f2ff);
global_constant color ROOM_TITLE_COLOR = MakeColor(0xff6969ff);
global_constant color ROOM_COLOR       = MakeColor(0xffe369ff);
global_constant color DIRECTION_COLOR  = MakeColor(0x84d197ff);
global_constant color ITEM_COLOR       = MakeColor(0x24e3e3ff);

global_constant color RESPONSE_COLOR = MakeColor(0x9063ffff);
global_constant color EMPHASIS_COLOR = MakeColor(0xe06ecdff);

struct console_theme {
    asset_loading_data LoadingData;
    
    asset_id BasicFont;
    asset_id TitleFont;
    
    color BackgroundColor;
    color CursorColor;
    color SelectionColor;
    
    fancy_font_format RoomTitleFancy;
    union{
        struct {
            fancy_font_format BasicFancy;     // '\0'
            fancy_font_format DirectionFancy; // '\1'
            fancy_font_format RoomFancy;      // '\2'
            fancy_font_format ItemFancy;      // '\3'
            fancy_font_format MiscFancy;      // '\'
            fancy_font_format MoodFancy;      // '\5'
        };
        fancy_font_format DescriptionFancies[6];
    };
    fancy_font_format ResponseFancies[2];
};
internal inline console_theme MakeDefaultConsoleTheme();

//~ Text adventure stuff

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
        asset_id RoomID;
        asset_id ItemID;
        asset_id Asset;
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
    asset_loading_data LoadingData;
    
    const char *S;
    asset_id TAID;
    asset_id Asset;
    ta_name NameData;
};

//~ Special commands
typedef u32 special_commands;
enum special_commands_ {
    SpecialCommand_None               = (0 << 0),
    SpecialCommand_StartCarillonPages = (1 << 0),
};


//~ Asset system
global_constant color             ERROR_COLOR = MakeColor(1.0f, 0.0f, 1.0f);
global_constant fancy_font_format ERROR_FANCY = MakeFancyFormat(ERROR_COLOR);

struct ta_system;
struct audio_mixer;
struct asset_system {
    //~ Asset stuff
    memory_arena Memory;
    
    void Initialize(memory_arena *Arena);
    
    //~ Processed assets loading
    void LoadProcessedAssets(void *Data, u32 DataSize);
    
    asset_table(SoundEffect, asset_sound_effect);
    asset_table(Font,        asset_font);
    asset_table(Variable,    asset_variable);
    asset_table(Theme,       console_theme);
};

//~ Asset loading
struct asset_loader {
    ta_system *TextAdventure;
    audio_mixer *Mixer;
    asset_system *MainAssets;
    asset_system InProgress;
    
    //~ Logging 
    const char *CurrentCommand;
    const char *CurrentAsset;
    const char *CurrentAttribute;
    asset_loading_status LoadingStatus;
    u32 LoadCounter;
    
    asset_loading_status ChooseStatus(asset_loading_status Status);
    void BeginCommand(const char *Name);
    void LogWarning(const char *Format, ...);
    void VLogWarning(const char *Format, va_list VarArgs);
    b8 SeekNextAttribute();
    b8 SeekEndOfFunction();
    b8 SeekNextCommand();
    void FailCommand(asset_loading_data *Data, const char *Format, ...);
    
    //~ SJA reading and parsing
    u64 LastFileWriteTime;
    hash_table<const char *, char>         ASCIITable;
    hash_table<const char *, asset_tag_id> TagTable;
    hash_table<const char *, direction>    DirectionTable;
    hash_table<const char *, image> LoadedImageTable;
    
    file_reader Reader;
    
    u32 ExpectPositiveInteger_();
    image *LoadImage(const char *Path);
    
    v2                  ExpectTypeV2();
    array<s32>          ExpectTypeArrayS32();
    array<const char *> ExpectTypeArrayCString();
    color               ExpectTypeColor();
    fancy_font_format   ExpectTypeFancy();
    asset_tag           MaybeExpectTag();
    ta_name             ExpectTypeName();
    
    void Initialize(memory_arena *Arena, audio_mixer *Mixer_, asset_system *Assets, ta_system *TA);
    
    b8 DoAttribute(const char *String, const char *Attribute);
    
    asset_loading_status ProcessCommand();
    asset_loading_status ProcessIgnore();
    
    special_commands SpecialCommands;
    asset_loading_status ProcessSpecialCommands();
    asset_loading_status ProcessVariables();
    asset_loading_status ProcessTheme();
    
    asset_loading_status ProcessSoundEffect();
    asset_loading_status ProcessFont();
    
    asset_loading_status ExpectDescriptionStrings(string_builder *Builder);
    asset_loading_status ProcessTADescription(dynamic_array<ta_data *> *Descriptions, ta_data_type Type=TADataType_Description);
    asset_loading_status ProcessTARoom();
    asset_loading_status ProcessTAItem();
    asset_loading_status ProcessTAMap();
    
    asset_loading_status LoadAssetFile(const char *Path);
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
struct ta_item;
internal void MurkwellProcessItem(ta_system *TA, asset_id ItemID, ta_item *Item);

#endif //SNAIL_JUMPY_ASSET_H
