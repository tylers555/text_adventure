#include "main.h"

global ta_system TextAdventure;

global game_mode GameMode = GameMode_None;
global asset_processor AssetProcessor;

//~ Helpers
internal inline string
String(const char *S){
    return Strings.GetString(S);
}

//~ Stubs
internal render_texture 
MakeTexture(texture_flags Flags){
    render_texture Result = AssetProcessor.Textures.Count;
    ArrayAlloc(&AssetProcessor.Textures);
    return Result;
}

internal void
TextureUpload(render_texture Texture, u8 *Pixels, u32 Width, u32 Height, u32 Channels){
    AssetProcessor.Textures[Texture].Pixels   = PushArray(&PermanentStorageArena, u8, Width*Height*Channels);
    CopyMemory(AssetProcessor.Textures[Texture].Pixels, Pixels, Width*Height*Channels);
    AssetProcessor.Textures[Texture].Width    = Width;
    AssetProcessor.Textures[Texture].Height   = Height;
    AssetProcessor.Textures[Texture].Channels = Channels;
}

internal void
RenderTexture(game_renderer *Renderer, rect R, f32 Z, render_texture Texture, 
              rect TextureRect=MakeRect(V2(0,0), V2(1,1)), b8 HasAlpha=false, color Color=WHITE){}

internal void
RenderRect(game_renderer *Renderer, rect R, f32 Z, color Color){}

internal void
RenderLine(game_renderer *Renderer, v2 A, v2 B, f32 Z, f32 Thickness, color Color){}

//~ Includes 
#include "logging.cpp"
#include "stream.cpp"
#include "file_processing.cpp"
#include "wav.cpp"
#include "asset.cpp"
#include "text_adventure.cpp"
#include "asset_loading.cpp"
#include "audio_mixer.cpp"
#include "commands.cpp"

#define DO_DEBUG_INFO() 
#include "game.cpp"
#include "map.cpp"

template <typename ValueType>
internal inline hash_table<ta_id, ValueType>
ProcessTAIDTable(hash_table<ta_id, ValueType> *InTable){
    hash_table<ta_id, ValueType> Result = MakeHashTable<ta_id, ValueType>(&TransientStorageArena, InTable->BucketsUsed);
    HashTableCopy(&Result, InTable);
    return Result;
}

internal inline void 
StringBuilderAddFancyFormat(string_builder *Builder, fancy_font_format *Fancy){
    StringBuilderAdd(Builder, "MakeFancyFormat(MakeColor(%.2ff, %.2ff, %.2ff, %.2ff), MakeColor(%.2ff, %.2ff, %.2ff, %.2ff), %.2ff, %.2ff, %.2ff, %.2ff, %.2ff, %.2ff)", 
                     Fancy->Color1.R, Fancy->Color1.G, Fancy->Color1.B, Fancy->Color1.A,
                     Fancy->Color2.R, Fancy->Color2.G, Fancy->Color2.B, Fancy->Color2.A, 
                     Fancy->Amplitude, Fancy->Speed, Fancy->dT,
                     Fancy->ColorSpeed, Fancy->ColordT, Fancy->ColorTOffset);
}


internal inline void 
AssetProcessorMain(){
    //~ Initialize core stuff
    {
        umw Size = Megabytes(256);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&PermanentStorageArena, Memory, Size);
    }{
        umw Size = Megabytes(512);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&TransientStorageArena, Memory, Size);
    }
    
    //~ Initialize asset processor
    AssetProcessor.Textures = MakeArray<asset_processor_texture>(&PermanentStorageArena, 512);
    
    //~ Other initialization
    asset_system Assets = {};
    ta_system *TA = &TextAdventure;
    
    Strings.Initialize(&PermanentStorageArena);
    TextAdventure.Initialize(&PermanentStorageArena);
    Assets.Initialize(&PermanentStorageArena);
    Assets.LoadAssetFile(ASSET_FILE_PATH);
    
    ArenaClear(&TransientStorageArena);
    string_builder SJAPBuilder  = BeginStringBuilder(&TransientStorageArena, Megabytes(200));
    
    sjap_header Header = {};
    Header.SJAP[0] = 'S';
    Header.SJAP[1] = 'J';
    Header.SJAP[2] = 'A';
    Header.SJAP[3] = 'P';
    
    StringBuilderAddVar(&SJAPBuilder, Header);
    
    string_builder EnumBuilder = BeginStringBuilder(&TransientStorageArena, Megabytes(2));
    StringBuilderAdd(&EnumBuilder, 
                     "#if !defined(GENERATED_ASSET_ID_H) && defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)\n"
                     "#define GENERATED_ASSET_ID_H\n"
                     "enum {\n", TA->StartRoomID);
    
    string_builder AssetBuilder = BeginStringBuilder(&TransientStorageArena, Megabytes(2));
    StringBuilderAdd(&AssetBuilder, 
                     "#if !defined(GENERATED_ASSET_DATA_H) && defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)\n"
                     "#define GENERATED_ASSET_DATA_H\n"
                     "internal inline void\n"
                     "InitializeProcessedAssets(asset_system *Assets, void *Data, u32 DataSize){\n"
                     "ta_system *TA = &TextAdventure;\n"
                     "memory_arena *Memory = &Assets->Memory;\n");
    
#define ADD_FANCY(Name) \
StringBuilderAdd(&AssetBuilder, "TA->Theme." #Name " = "); \
StringBuilderAddFancyFormat(&AssetBuilder, &Theme->Name); \
StringBuilderAdd(&AssetBuilder, ";\n"); 
    {
        StringBuilderAdd(&AssetBuilder, "{\n");
        //~ Theme
        console_theme *Theme = &TA->Theme;
        StringBuilderAdd(&AssetBuilder, "TA->Theme.BasicFont = AssetID(%s); \n", AssetIDName(Theme->BasicFont));
        StringBuilderAdd(&AssetBuilder, "TA->Theme.TitleFont = AssetID(%s); \n", AssetIDName(Theme->TitleFont));
        StringBuilderAdd(&AssetBuilder, "TA->Theme.BackgroundColor = MakeColor(%ff, %ff, %ff, %ff); \n", 
                         Theme->BackgroundColor.R, Theme->BackgroundColor.G, Theme->BackgroundColor.B, Theme->BackgroundColor.A);
        StringBuilderAdd(&AssetBuilder, "TA->Theme.CursorColor = MakeColor(%ff, %ff, %ff, %ff); \n", 
                         Theme->CursorColor.R, Theme->CursorColor.G, Theme->CursorColor.B, Theme->CursorColor.A);
        StringBuilderAdd(&AssetBuilder, "TA->Theme.SelectionColor = MakeColor(%ff, %ff, %ff, %ff); \n", 
                         Theme->SelectionColor.R, Theme->SelectionColor.G, Theme->SelectionColor.B, Theme->SelectionColor.A);
        ADD_FANCY(BasicFancy);
        ADD_FANCY(RoomTitleFancy);
        ADD_FANCY(ItemFancy);
        ADD_FANCY(RoomFancy);
        ADD_FANCY(DirectionFancy);
        ADD_FANCY(MiscFancy);
        ADD_FANCY(MoodFancy);
        ADD_FANCY(ResponseFancies[0]);
        ADD_FANCY(ResponseFancies[1]);
        StringBuilderAdd(&AssetBuilder, "TA->Theme.DescriptionFancies[0] = TA->Theme.BasicFancy;\n");
        StringBuilderAdd(&AssetBuilder, "TA->Theme.DescriptionFancies[1] = TA->Theme.DirectionFancy;\n");
        StringBuilderAdd(&AssetBuilder, "TA->Theme.DescriptionFancies[2] = TA->Theme.RoomFancy;\n");
        StringBuilderAdd(&AssetBuilder, "TA->Theme.DescriptionFancies[3] = TA->Theme.ItemFancy;\n");
        StringBuilderAdd(&AssetBuilder, "TA->Theme.DescriptionFancies[4] = TA->Theme.MiscFancy;\n");
        StringBuilderAdd(&AssetBuilder, "TA->Theme.DescriptionFancies[5] = TA->Theme.MoodFancy;\n");
        StringBuilderAdd(&AssetBuilder, "}\n");
    }
#undef ADD_FANCY
    
    //~ Sound effects
    {
        u32 Index=1;
        for(u32 I=0; I<Assets.SoundEffectTable.MaxBuckets; I++){
            string Name = Assets.SoundEffectTable.Keys[I];
            if(Name.ID){
                asset_sound_effect *Effect = &Assets.SoundEffectTable.Values[I];
                
                StringBuilderAdd(&EnumBuilder, "AssetID_%s = %u,\n", Strings.GetString(Name), Index);
                StringBuilderAdd(&AssetBuilder, "Assets->SoundEffects[%u].Sound.ChannelCount = %u;\n", Index, Effect->Sound.ChannelCount);
                StringBuilderAdd(&AssetBuilder, "Assets->SoundEffects[%u].Sound.SampleCount  = %u;\n", Index, Effect->Sound.SampleCount);
                StringBuilderAdd(&AssetBuilder, "Assets->SoundEffects[%u].Sound.BaseSpeed    = %ff;\n", Index, Effect->Sound.BaseSpeed);
                StringBuilderAdd(&AssetBuilder, "Assets->SoundEffects[%u].VolumeMultiplier   = %ff;\n", Index, Effect->VolumeMultiplier);
                StringBuilderAdd(&AssetBuilder, "Assert(%u < DataSize);\n", SJAPBuilder.BufferSize);
                StringBuilderAdd(&AssetBuilder, "Assets->SoundEffects[%u].Sound.Samples = (s16 *)((u8 *)Data+%u);\n", Index, SJAPBuilder.BufferSize);
                
                StringBuilderAddData(&SJAPBuilder, Effect->Sound.Samples, Effect->Sound.ChannelCount*Effect->Sound.SampleCount*sizeof(s16));
                
                Index++;
            }
        }
        StringBuilderAdd(&EnumBuilder, (const char *)"AssetSoundEffect_TOTAL = %u,\n", Index++);
    }
    
    //~ Fonts
    {
        u32 Index = 1;
        for(u32 I=0; I<Assets.FontTable.MaxBuckets; I++){
            string Name = Assets.FontTable.Keys[I];
            if(Name.ID){
                asset_font *Font = &Assets.FontTable.Values[I];
                
                StringBuilderAdd(&EnumBuilder, "AssetID_%s = %u,\n", Strings.GetString(Name), Index);
                StringBuilderAdd(&AssetBuilder, "{\n");
                StringBuilderAdd(&AssetBuilder, "Assets->Fonts[%u].Size    = V2S(%d, %d);\n", Index, Font->Size.X, Font->Size.Y);
                StringBuilderAdd(&AssetBuilder, "Assets->Fonts[%u].Height  = %f;\n", Index, Font->Height);
                StringBuilderAdd(&AssetBuilder, "Assets->Fonts[%u].Descent = %f;\n", Index, Font->Descent);
                StringBuilderAdd(&AssetBuilder, "Assert(%u < DataSize);\n", SJAPBuilder.BufferSize);
                StringBuilderAdd(&AssetBuilder, "CopyMemory(Assets->Fonts[%u].Table, ((u8 *)Data+%u), sizeof(Assets->Fonts[%u].Table));\n", Index, SJAPBuilder.BufferSize, Index);
                StringBuilderAddVar(&SJAPBuilder, Font->Table);
                
                asset_processor_texture *Texture = &AssetProcessor.Textures[Font->Texture];
                StringBuilderAdd(&AssetBuilder, "Assert(%u < DataSize);\n", SJAPBuilder.BufferSize);
                StringBuilderAdd(&AssetBuilder, "u8 *Pixels = ((u8 *)Data+%u);\n", SJAPBuilder.BufferSize);
                StringBuilderAdd(&AssetBuilder, "Assets->Fonts[%u].Texture = MakeTexture();\n", Index);
                StringBuilderAdd(&AssetBuilder, "TextureUpload(Assets->Fonts[%u].Texture, Pixels, %u, %u, %u);\n", 
                                 Index, Texture->Width, Texture->Height, Texture->Channels);
                StringBuilderAdd(&AssetBuilder, "}\n", Index, SJAPBuilder.BufferSize);
                // TODO(Tyler): This currently does no compression, this will likely be wanted in the future
                StringBuilderAddData(&SJAPBuilder, Texture->Pixels, Texture->Width*Texture->Height*Texture->Channels);
                
                Index++;
            }
        }
        StringBuilderAdd(&EnumBuilder, (const char *)"AssetFont_TOTAL = %u,\n", Index++);
    }
    
    StringBuilderAdd(&EnumBuilder, "};\n");
    
    //~ Text adventure stuff
    StringBuilderAdd(&EnumBuilder, "global_constant u32 ROOM_TABLE_SIZE = %u;\n", TA->RoomTable.BucketsUsed);
    StringBuilderAdd(&EnumBuilder, "global_constant u32 ITEM_TABLE_SIZE = %u;\n", TA->ItemTable.BucketsUsed);
    
    hash_table<ta_id, ta_room> RoomTable = ProcessTAIDTable(&TA->RoomTable);
    hash_table<ta_id, ta_item> ItemTable = ProcessTAIDTable(&TA->ItemTable);
    
    StringBuilderAdd(&AssetBuilder,
                     "TA->StartRoomID = MakeTAID(%llu);\n"
                     "TA->StartRoomName = \"%s\";\n",
                     TA->StartRoomID, TA->StartRoomName);
    
    //~ TA Rooms
    StringBuilderAdd(&AssetBuilder, "TA->RoomTable.BucketsUsed = %u;\n", RoomTable.BucketsUsed);
    for(u32 I=0; I<RoomTable.MaxBuckets; I++){
        ta_id Name = RoomTable.Keys[I];
        if(Name.ID){
            ta_room *Room = &RoomTable.Values[I];
            
            StringBuilderAdd(&AssetBuilder, "{\n");
            StringBuilderAdd(&AssetBuilder, "TA->RoomTable.Keys[%u] = MakeTAID(%llu);\n", I, Name);
            StringBuilderAdd(&AssetBuilder, "TA->RoomTable.Hashes[%u] = %llu;\n", I, RoomTable.Hashes[I]);
            StringBuilderAdd(&AssetBuilder, "ta_room *Room = &TA->RoomTable.Values[%u];\n", I);
            StringBuilderAdd(&AssetBuilder, "Room->Name = \"%s\";\n", Room->Name);
            StringBuilderAdd(&AssetBuilder, "Room->Area = MakeTAID(%llu);\n", Room->Area);
            StringBuilderAdd(&AssetBuilder, "Room->Tag = MakeAssetTag((asset_tag_id)%u, (asset_tag_id)%u, (asset_tag_id)%u, (asset_tag_id)%u);\n", 
                             Room->Tag.A, Room->Tag.B, Room->Tag.C, Room->Tag.D);
            StringBuilderAdd(&AssetBuilder, "Room->Descriptions = MakeFullArray<ta_string *>(Memory, %u);\n", 
                             Room->Descriptions.Count);
            for(u32 J=0; J<Room->Descriptions.Count; J++){
                ta_string *Description = Room->Descriptions[J];
                StringBuilderAdd(&AssetBuilder, "Assert(%u < DataSize);\n", SJAPBuilder.BufferSize);
                StringBuilderAdd(&AssetBuilder, "Room->Descriptions[%u] = (ta_string *)((u8 *)Data+%u);\n", J, SJAPBuilder.BufferSize);
                StringBuilderAddVar(&SJAPBuilder, Room->Descriptions[J]->Tag);
                StringBuilderAdd(&SJAPBuilder, Room->Descriptions[J]->Data);
                StringBuilderAdd(&SJAPBuilder, '\0');
            }
            StringBuilderAdd(&AssetBuilder, "Room->Items = MakeFullArray<ta_id>(Memory, %u);\n", 
                             Room->Items.Count);
            for(u32 J=0; J<Room->Items.Count; J++){
                StringBuilderAdd(&AssetBuilder, "Room->Items[%u] = MakeTAID(%llu);\n", J, Room->Items[J]);
            }
            for(u32 J=0; J<ArrayCount(Room->Adjacents); J++){
                asset_tag Tag = Room->AdjacentTags[J];
                StringBuilderAdd(&AssetBuilder, "Room->Adjacents[%u] = MakeTAID((u64)%llu);\n", J, Room->Adjacents[J]);
                StringBuilderAdd(&AssetBuilder, "Room->AdjacentTags[%u] = MakeAssetTag((asset_tag_id)%u, (asset_tag_id)%u, (asset_tag_id)%u, (asset_tag_id)%u);\n",
                                 J, Tag.A, Tag.B, Tag.C, Tag.D);
            }
            
            StringBuilderAdd(&AssetBuilder, "}\n");
        }
    }
    
    //~ TA Items
    StringBuilderAdd(&AssetBuilder, 
                     "TA->ItemNameTable = MakeHashTable<const char *, ta_id>(Memory, ITEM_TABLE_SIZE);\n");
    StringBuilderAdd(&AssetBuilder, "TA->ItemTable.BucketsUsed = %u;\n", ItemTable.BucketsUsed);
    
    for(u32 I=0; I<ItemTable.MaxBuckets; I++){
        ta_id Name = ItemTable.Keys[I];
        if(Name.ID){
            ta_item *Item = &ItemTable.Values[I];
            StringBuilderAdd(&AssetBuilder, "{\n");
            StringBuilderAdd(&AssetBuilder, "HashTableInsert(&TA->ItemNameTable, \"%s\", MakeTAID(%llu));\n", Item->Name, Name);
            StringBuilderAdd(&AssetBuilder, "TA->ItemTable.Keys[%u] = MakeTAID(%llu);\n", I, Name);
            StringBuilderAdd(&AssetBuilder, "TA->ItemTable.Hashes[%u] = %llu;\n", I, ItemTable.Hashes[I]);
            StringBuilderAdd(&AssetBuilder, "ta_item *Item = &TA->ItemTable.Values[%u];\n", I);
            StringBuilderAdd(&AssetBuilder, "Item->Name = \"%s\";\n", Item->Name);
            StringBuilderAdd(&AssetBuilder, "Item->Tag  = MakeAssetTag((asset_tag_id)%u, (asset_tag_id)%u, (asset_tag_id)%u, (asset_tag_id)%u);\n", 
                             Item->Tag.A, Item->Tag.B, Item->Tag.C, Item->Tag.D);
            StringBuilderAdd(&AssetBuilder, "Item->Cost = %u;\n", Item->Cost);
            StringBuilderAdd(&AssetBuilder, "Item->Aliases = MakeFullArray<const char *>(Memory, %u);\n", 
                             Item->Aliases.Count);
            for(u32 J=0; J<Item->Aliases.Count; J++){
                StringBuilderAdd(&AssetBuilder, "Item->Aliases[%u] = \"%s\";\n", J, Item->Aliases[J]);
            }
            StringBuilderAdd(&AssetBuilder, "Item->Adjectives = MakeFullArray<const char *>(Memory, %u);\n", 
                             Item->Adjectives.Count);
            for(u32 J=0; J<Item->Adjectives.Count; J++){
                StringBuilderAdd(&AssetBuilder, "Item->Adjectives[%u] = \"%s\";\n", J, Item->Adjectives[J]);
            }
            
            StringBuilderAdd(&AssetBuilder, "Item->Descriptions = MakeFullArray<ta_string *>(Memory, %u);\n", 
                             Item->Descriptions.Count);
            for(u32 J=0; J<Item->Descriptions.Count; J++){
                StringBuilderAdd(&AssetBuilder, "Assert(%u < DataSize);\n", SJAPBuilder.BufferSize);
                StringBuilderAdd(&AssetBuilder, "Item->Descriptions[%u] = (ta_string *)((u8 *)Data+%u);\n", J, SJAPBuilder.BufferSize);
                StringBuilderAddVar(&SJAPBuilder, Item->Descriptions[J]->Tag);
                StringBuilderAdd(&SJAPBuilder, Item->Descriptions[J]->Data);
                StringBuilderAdd(&SJAPBuilder, '\0');
            }
            
            StringBuilderAdd(&AssetBuilder, "}\n");
        }
    }
    
    //~ TA Map
    {
        ta_map *Map = &TA->Map;
        StringBuilderAdd(&AssetBuilder, "{\n");
        
        asset_processor_texture *Texture = &AssetProcessor.Textures[Map->Texture];
        StringBuilderAdd(&AssetBuilder, "Assert(%u < DataSize);\n", SJAPBuilder.BufferSize);
        StringBuilderAdd(&AssetBuilder, "u8 *Pixels = ((u8 *)Data+%u);\n", SJAPBuilder.BufferSize);
        StringBuilderAdd(&AssetBuilder, "TA->Map.Texture = MakeTexture();\n");
        StringBuilderAdd(&AssetBuilder, "TextureUpload(TA->Map.Texture, Pixels, %u, %u, %u);\n", 
                         Texture->Width, Texture->Height, Texture->Channels);
        StringBuilderAddData(&SJAPBuilder, Texture->Pixels, Texture->Width*Texture->Height*Texture->Channels);
        StringBuilderAdd(&AssetBuilder, "TA->Map.Size = V2(%f, %f);\n", Map->Size.X, Map->Size.Y);
        StringBuilderAdd(&AssetBuilder, "TA->Map.Areas = MakeFullArray<ta_area>(Memory, %u);\n", TA->Map.Areas.Count);
        for(u32 J=0; J<TA->Map.Areas.Count; J++){
            StringBuilderAdd(&AssetBuilder, "TA->Map.Areas[%u] = MakeTAArea(MakeTAID((u64)%llu), V2(%f, %f));\n", 
                             J, TA->Map.Areas[J].Name, TA->Map.Areas[J].Offset.X, TA->Map.Areas[J].Offset.Y);
        }
        
        StringBuilderAdd(&AssetBuilder, "}\n");
    }
    
    //~ Output to file
    
    
    StringBuilderAdd(&EnumBuilder, 
                     "#endif // GENERATED_ASSET_ID_H\n");
    StringBuilderAdd(&AssetBuilder, 
                     "}\n"
                     "#endif // GENERATED_ASSET_DATA_H\n");
    
    {
        os_file *OutputFile = OpenFile("./processed_assets.sjap", OpenFile_Write|OpenFile_Clear);
        Assert(OutputFile);
        StringBuilderToFile(&SJAPBuilder, OutputFile);
        CloseFile(OutputFile);
    }
    
    {
        os_file *OutputFile = OpenFile("../source/generated_asset_id.h", OpenFile_Write|OpenFile_Clear);
        Assert(OutputFile);
        StringBuilderToFile(&EnumBuilder, OutputFile);
        CloseFile(OutputFile);
    }
    
    {
        os_file *OutputFile = OpenFile("../source/generated_asset_data.h", OpenFile_Write|OpenFile_Clear);
        Assert(OutputFile);
        StringBuilderToFile(&AssetBuilder, OutputFile);
        CloseFile(OutputFile);
    }
}