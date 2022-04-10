
#if !defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)

//~ Initialization
void
asset_system::InitializeLoader(memory_arena *Arena){
    ASCIITable = MakeHashTable<const char *, char>(Arena, 128);
    HashTableInsert(&ASCIITable, "SPACE",                ' ');
    HashTableInsert(&ASCIITable, "EXCLAMATION",          '!');
    HashTableInsert(&ASCIITable, "QUOTATION",            '"');
    HashTableInsert(&ASCIITable, "POUND",                '#');
    HashTableInsert(&ASCIITable, "APOSTROPHE",           '\'');
    HashTableInsert(&ASCIITable, "PARENTHESIS_LEFT",     '(');
    HashTableInsert(&ASCIITable, "PARENTHESIS_RIGHT",    ')');
    HashTableInsert(&ASCIITable, "ASTERISK",             '*');
    HashTableInsert(&ASCIITable, "PLUS",                 '+');
    HashTableInsert(&ASCIITable, "COMMA",                ',');
    HashTableInsert(&ASCIITable, "DASH",                 '-');
    HashTableInsert(&ASCIITable, "PERIOD",               '.');
    HashTableInsert(&ASCIITable, "SLASH",                '/');
    HashTableInsert(&ASCIITable, "ZERO",                 '0');
    HashTableInsert(&ASCIITable, "ONE",                  '1');
    HashTableInsert(&ASCIITable, "TWO",                  '2');
    HashTableInsert(&ASCIITable, "THREE",                '3');
    HashTableInsert(&ASCIITable, "FOUR",                 '4');
    HashTableInsert(&ASCIITable, "FIVE",                 '5');
    HashTableInsert(&ASCIITable, "SIX",                  '6');
    HashTableInsert(&ASCIITable, "SEVEN",                '7');
    HashTableInsert(&ASCIITable, "EIGHT",                '8');
    HashTableInsert(&ASCIITable, "NINE",                 '9');
    HashTableInsert(&ASCIITable, "COLON",                ':');
    HashTableInsert(&ASCIITable, "SEMICOLON",            ';');
    HashTableInsert(&ASCIITable, "ANGLE_BRACKET_LEFT",   '<');
    HashTableInsert(&ASCIITable, "EQUAL",                '=');
    HashTableInsert(&ASCIITable, "ANGLE_BRACKET_RIGHT",  '>');
    HashTableInsert(&ASCIITable, "QUESTION",             '?');
    HashTableInsert(&ASCIITable, "A",                    'A');
    HashTableInsert(&ASCIITable, "B",                    'B');
    HashTableInsert(&ASCIITable, "C",                    'C');
    HashTableInsert(&ASCIITable, "D",                    'D');
    HashTableInsert(&ASCIITable, "E",                    'E');
    HashTableInsert(&ASCIITable, "F",                    'F');
    HashTableInsert(&ASCIITable, "G",                    'G');
    HashTableInsert(&ASCIITable, "H",                    'H');
    HashTableInsert(&ASCIITable, "I",                    'I');
    HashTableInsert(&ASCIITable, "J",                    'J');
    HashTableInsert(&ASCIITable, "K",                    'K');
    HashTableInsert(&ASCIITable, "L",                    'L');
    HashTableInsert(&ASCIITable, "M",                    'M');
    HashTableInsert(&ASCIITable, "N",                    'N');
    HashTableInsert(&ASCIITable, "O",                    'O');
    HashTableInsert(&ASCIITable, "P",                    'P');
    HashTableInsert(&ASCIITable, "Q",                    'Q');
    HashTableInsert(&ASCIITable, "R",                    'R');
    HashTableInsert(&ASCIITable, "S",                    'S');
    HashTableInsert(&ASCIITable, "T",                    'T');
    HashTableInsert(&ASCIITable, "U",                    'U');
    HashTableInsert(&ASCIITable, "V",                    'V');
    HashTableInsert(&ASCIITable, "W",                    'W');
    HashTableInsert(&ASCIITable, "X",                    'X');
    HashTableInsert(&ASCIITable, "Y",                    'Y');
    HashTableInsert(&ASCIITable, "Z",                    'Z');
    HashTableInsert(&ASCIITable, "SQUARE_BRACKET_LEFT",  '[');
    HashTableInsert(&ASCIITable, "BACKSLASH",            '\\');
    HashTableInsert(&ASCIITable, "SQUARE_BRACKET_RIGHT", ']');
    HashTableInsert(&ASCIITable, "CARET",                '^');
    HashTableInsert(&ASCIITable, "BACK_TICK",            '`');
    HashTableInsert(&ASCIITable, "UNDERSCORE",           '_');
    HashTableInsert(&ASCIITable, "CURLY_BRACKET_LEFT",   '{');
    HashTableInsert(&ASCIITable, "PIPE",                  '|');
    HashTableInsert(&ASCIITable, "CURLY_BRACKET_RIGHT",  '}');
    HashTableInsert(&ASCIITable, "TILDE",                '~');
    HashTableInsert(&ASCIITable, "PERCENT",              '%');
    HashTableInsert(&ASCIITable, "DOLLAR_SIGN",          '$');
    HashTableInsert(&ASCIITable, "AMPERSAND",            '&');
    HashTableInsert(&ASCIITable, "AT_SIGN",              '@');
    
    TagTable = MakeHashTable<const char *, asset_tag_id>(Arena, AssetTag_TOTAL);
    HashTableInsert(&TagTable, "play",       AssetTag_Play);
    HashTableInsert(&TagTable, "examine",    AssetTag_Examine);
    HashTableInsert(&TagTable, "eat",        AssetTag_Eat);
    HashTableInsert(&TagTable, "activate",   AssetTag_Activate);
    HashTableInsert(&TagTable, "take",       AssetTag_Take);
    HashTableInsert(&TagTable, "organ",      AssetTag_Organ);
    HashTableInsert(&TagTable, "bell-tower", AssetTag_BellTower);
    HashTableInsert(&TagTable, "broken",     AssetTag_Broken);
    HashTableInsert(&TagTable, "repaired",   AssetTag_Repaired);
    HashTableInsert(&TagTable, "locked",     AssetTag_Locked);
    HashTableInsert(&TagTable, "open-dawn",  AssetTag_OpenDawn);
    HashTableInsert(&TagTable, "open-noon",  AssetTag_OpenNoon);
    HashTableInsert(&TagTable, "open-dusk",  AssetTag_OpenDusk);
    HashTableInsert(&TagTable, "open-night", AssetTag_OpenNight);
    HashTableInsert(&TagTable, "items",      AssetTag_Items);
    HashTableInsert(&TagTable, "adjacents",  AssetTag_Adjacents);
    HashTableInsert(&TagTable, "static",     AssetTag_Static);
    HashTableInsert(&TagTable, "bread",      AssetTag_Bread);
    HashTableInsert(&TagTable, "key",        AssetTag_Key);
    HashTableInsert(&TagTable, "map",        AssetTag_Map);
    HashTableInsert(&TagTable, "light",      AssetTag_Light);
    
    LoadedImageTable = MakeHashTable<const char *, image>(Arena, 256);
}

//~ Base

#define ExpectPositiveInteger() \
ExpectPositiveInteger_();   \
HandleError(&Reader);

#define EnsurePositive(Var) \
if(Var < 0){            \
LogError("'%d' must be positive!", Var); \
return(false);      \
}

#define HandleToken(Token)                   \
if(Token.Type == FileTokenType_BeginCommand) break; \
if(Token.Type == FileTokenType_EndFile)      break; \
if(Token.Type == FileTokenType_Invalid)      break; \

void 
asset_system::BeginCommand(const char *Name){
    CurrentCommand = Name;
    CurrentAttribute = 0;
}

void
asset_system::LogError(const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    
    char Buffer[DEFAULT_BUFFER_SIZE];
    if(CurrentAttribute){
        stbsp_snprintf(Buffer, sizeof(Buffer), "(%s,%s Line: %u) %s", CurrentCommand, CurrentAttribute, Reader.Line, Format);
    }else{
        stbsp_snprintf(Buffer, sizeof(Buffer), "(%s Line: %u) %s", CurrentCommand, Reader.Line, Format);
    }
    VLogMessage(Buffer, VarArgs);
    
    va_end(VarArgs);
}

void
asset_system::LogInvalidAttribute(const char *Attribute){
    LogMessage("(%s Line: %u) Invalid attribute: %s", CurrentCommand, Reader.Line, Attribute);
}

fancy_font_format
asset_system::ExpectTypeFancy(){
    fancy_font_format Result = {};
    
    file_token Token = Reader.PeekToken();
    if(Token.Type != FileTokenType_Identifier) return Result;
    if(CompareStrings(Token.Identifier, "Fancy")){
        Expect(&Reader, Identifier);
        
        Reader.ExpectToken(FileTokenType_BeginArguments);
        HandleError(&Reader);
        
        Result.Color1 = Reader.ExpectTypeColor();
        HandleError(&Reader);
        
        Token = Reader.PeekToken();
        if(Token.Type == FileTokenType_EndArguments){
        }else if(Token.Type == FileTokenType_Float){
            Result.Amplitude = Expect(&Reader, Float);
            Result.Speed     = Expect(&Reader, Float);
            Result.dT        = Expect(&Reader, Float);
        }else if(Token.Type == FileTokenType_Identifier){
            Result.Color2 = Reader.ExpectTypeColor();
            HandleError(&Reader);
            
            f32 A = Expect(&Reader, Float);
            f32 B = Expect(&Reader, Float);
            f32 C = Expect(&Reader, Float);
            Token = Reader.PeekToken();
            if(Token.Type == FileTokenType_EndArguments){
                Result.ColorSpeed   = A;
                Result.ColordT      = B;
                Result.ColorTOffset = C;
            }else{
                Result.Amplitude    = A;
                Result.Speed        = B;
                Result.dT           = C;
                Result.ColorSpeed   = Expect(&Reader, Float);
                Result.ColordT      = Expect(&Reader, Float);
                Result.ColorTOffset = Expect(&Reader, Float);
            }
        }
        
        Reader.ExpectToken(FileTokenType_EndArguments);
        HandleError(&Reader);
        
        Result.Speed        *= 0.5f*PI;
        Result.dT           *= 0.5f*PI;
        Result.ColorSpeed   *= 0.5f*PI;
        Result.ColordT      *= 0.5f*PI;
        Result.ColorTOffset *= 0.5f*PI;
    }else{
        Reader.LastError = FileReaderError_InvalidToken;
        return(Result);
    }
    
    return(Result);
}

asset_tag
asset_system::MaybeExpectTag(){
    asset_tag Result = {};
    
    file_token Token = Reader.PeekToken();
    if(Token.Type != FileTokenType_Identifier) return Result;
    if(CompareStrings(Token.Identifier, "Tag")){
        Expect(&Reader, Identifier);
        
        Reader.ExpectToken(FileTokenType_BeginArguments);
        HandleError(&Reader);
        
        for(u32 I=0; I<ArrayCount(Result.E); I++){
            Token = Reader.PeekToken();
            if(Token.Type != FileTokenType_String) break;
            const char *S = Expect(&Reader, String);
            
            enum8(asset_tag_id) ID = (u8)HashTableFind(&TagTable, S);
            if(!ID){ 
                LogError("WARNING: '%s' is not registered as a tag and it will thus be ignored.!", S);
                //Assert(0);
                continue;
            }
            
            Result.E[I] = ID;
        }
        
        Reader.ExpectToken(FileTokenType_EndArguments);
        HandleError(&Reader);
        
    }
    
    return(Result);
}

u32
asset_system::ExpectPositiveInteger_(){
    u32 Result = 0;
    s32 Integer = Expect(&Reader, Integer);
    if(Integer < 0){
        LogError("Expected a positive integer, instead read '%d', which is negative", Integer);
        return(0);
    }
    
    return(Integer);
}

image *
asset_system::LoadImage(const char *Path){
    image *Result = 0;
    
    os_file *File = 0;
    File = OpenFile(Path, OpenFile_Read);
    if(!File) return(Result);
    u64 LastImageWriteTime;
    LastImageWriteTime = GetLastFileWriteTime(File);
    CloseFile(File);
    u8 *ImageData;
    s32 Components;
    
    Result = HashTableGetPtr(&LoadedImageTable, Path);
    if(Result->HasBeenLoadedBefore){
        if(Result->LastWriteTime < LastImageWriteTime){
            entire_file File = ReadEntireFile(&TransientStorageArena, Path);
            
            ImageData = (u8 *)stbi_load_from_memory((u8 *)File.Data,
                                                    (int)File.Size,
                                                    &Result->Width, &Result->Height,
                                                    &Components, 4);
            TextureUpload(Result->Texture, ImageData, Result->Width, Result->Height);
            stbi_image_free(ImageData);
        }
    }else{
        entire_file File;
        File = ReadEntireFile(&TransientStorageArena, Path);
        s32 Components = 0;
        stbi_info_from_memory((u8 *)File.Data, (int)File.Size, 
                              &Result->Width, &Result->Height, &Components);
        ImageData = (u8 *)stbi_load_from_memory((u8 *)File.Data,
                                                (int)File.Size,
                                                &Result->Width, &Result->Height,
                                                &Components, 4);
        Result->HasBeenLoadedBefore = true;
        Result->LastWriteTime = LastImageWriteTime,
        Result->IsTranslucent = true;
        Result->Texture = MakeTexture();
        TextureUpload(Result->Texture, ImageData, Result->Width, Result->Height);
        
        stbi_image_free(ImageData);
    }
    
    return(Result);
}

//~ 

b8 
asset_system::DoAttribute(const char *String, const char *Attribute){
    b8 Result = CompareStrings(String, Attribute);
    if(Result) CurrentAttribute = Attribute;
    return(Result);
}

// TODO(Tyler): This should be made into an actual comment type such as /* */ or #if 0 #endif
b8
asset_system::ProcessIgnore(){
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        Reader.NextToken();
    }
    return(true);
}

void
asset_system::LoadAssetFile(const char *Path){
    CurrentCommand = 0;
    CurrentAttribute = 0;
    
    b8 HitError = false;
    do{
        memory_arena_marker Marker = ArenaBeginMarker(&TransientStorageArena);
        
        os_file *File = OpenFile(Path, OpenFile_Read);
        u64 NewFileWriteTime = GetLastFileWriteTime(File);
        CloseFile(File);
        
        if(LastFileWriteTime < NewFileWriteTime){
            u64 Start = OSGetMicroseconds();
            ArenaClear(&Memory);
            
            HitError = false;
            
            Reader = MakeFileReader(Path, this);
            
            while(!HitError){
                file_token Token = Reader.NextToken();
                
                switch(Token.Type){
                    case FileTokenType_BeginCommand: {
                        if(!ProcessCommand()){
                            HitError = true;
                            break;
                        }
                    }break;
                    case FileTokenType_EndFile: {
                        goto end_loop;
                    }break;
                    default: {
                        LogMessage("(Line: %u) Token: %s was not expected!", Reader.Line, TokenToString(Token));
                        HitError = true;
                    }break;
                }
            }
            end_loop:;
            
            u64 Time = OSGetMicroseconds()-Start;
            LogMessage("AssetLoadingTime = %f", (f32)Time/1000000.0f);
        }
        
        if(HitError){
            ArenaEndMarker(&TransientStorageArena, &Marker);
            OSSleep(10); // To prevent consuming the CPU
        }
        LastFileWriteTime = NewFileWriteTime;
    }while(HitError); 
    // This loop does result in a missed FPS but for right now it works just fine.
}

#define IfCommand(Command)                 \
if(CompareStrings(String, #Command)) { \
BeginCommand(#Command);            \
if(!Process ## Command()){ return(false); } \
return(true);                      \
}       

b8
asset_system::ProcessCommand(){
    b8 Result = false;
    
    const char *String = Expect(&Reader, Identifier);
    
    IfCommand(Ignore);
    IfCommand(Variables);
    IfCommand(Theme);
    IfCommand(Font);
    IfCommand(SoundEffect);
    IfCommand(TARoom);
    IfCommand(TAItem);
    IfCommand(TAMap);
    
    LogMessage("(Line: %u) '%s' isn't a valid command!", Reader.Line, String);
    return(false);
}
#undef IfCommand

//~ Variables

b8
asset_system::ProcessVariables(){
    b8 Result = false;
    ta_system *TA = &TextAdventure;
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = Expect(&Reader, Identifier);
        if(DoAttribute(Attribute, "start_game_mode")){
            const char *S = Expect(&Reader, String);
            if(GameMode == GameMode_None){
                if(CompareStrings(S, "main game")) GameMode = GameMode_MainGame;
                else if(CompareStrings(S, "menu")) GameMode = GameMode_Menu;
            }
            
        }else if(DoAttribute(Attribute, "start_room")){
            const char *S = Expect(&Reader, String);
            TA->StartRoomName = Strings.GetPermanentString(S);
            TA->StartRoomID = TAIDByName(TA, S);
        }else if(DoAttribute(Attribute, "theme")){
            const char *S = Expect(&Reader, String);
            console_theme *Theme = HashTableFindPtr(&TA->ThemeTable, TAIDByName(TA, S));
            if(Theme) TA->Theme = *Theme; 
        }else if(DoAttribute(Attribute, "give_item")){
            const char *S = Expect(&Reader, String);
            ta_id Item = TAIDByName(TA, S);
            b8 FoundIt = false;
            for(u32 I=0; I<TA->Inventory.Count; I++){
                if(TA->Inventory[I] == Item) { FoundIt = true; break; }
            }
            if(!FoundIt) TA->AddItem(Item);
        }else{ LogInvalidAttribute(Attribute); return false; }
    }
    
    return true;
}

//~ Theme
b8
asset_system::ProcessTheme(){
    b8 Result = false;
    ta_system *TA = &TextAdventure;
    
    const char *Name = Expect(&Reader, String);
    console_theme *Theme = HashTableGetPtr(&TA->ThemeTable, TAIDByName(TA, Name));
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = Expect(&Reader, Identifier);
        if(DoAttribute(Attribute, "basic_font")){
            const char *S = Expect(&Reader, String);
            Theme->BasicFont = MakeAssetID(Strings.GetString(S));
        }else if(DoAttribute(Attribute, "title_font")){
            const char *S = Expect(&Reader, String);
            Theme->TitleFont = MakeAssetID(Strings.GetString(S));
        }else if(DoAttribute(Attribute, "background_color")){ Theme->BackgroundColor = Reader.ExpectTypeColor(); HandleError(&Reader);
        }else if(DoAttribute(Attribute, "cursor_color")){ Theme->CursorColor = Reader.ExpectTypeColor(); HandleError(&Reader);
        }else if(DoAttribute(Attribute, "selection_color")){ Theme->SelectionColor = Reader.ExpectTypeColor(); HandleError(&Reader);
        }else if(DoAttribute(Attribute, "basic")){      Theme->BasicFancy = ExpectTypeFancy(); HandleError(&Reader);
        }else if(DoAttribute(Attribute, "room_title")){ Theme->RoomTitleFancy = ExpectTypeFancy(); HandleError(&Reader);
        }else if(DoAttribute(Attribute, "item")){       Theme->ItemFancy = ExpectTypeFancy(); HandleError(&Reader);
        }else if(DoAttribute(Attribute, "room")){       Theme->RoomFancy = ExpectTypeFancy(); HandleError(&Reader);
        }else if(DoAttribute(Attribute, "direction")){  Theme->DirectionFancy = ExpectTypeFancy(); HandleError(&Reader);
        }else if(DoAttribute(Attribute, "misc")){  Theme->MiscFancy = ExpectTypeFancy(); HandleError(&Reader);
        }else if(DoAttribute(Attribute, "mood")){  Theme->MoodFancy = ExpectTypeFancy(); HandleError(&Reader);
        }else if(DoAttribute(Attribute, "response")){   Theme->ResponseFancies[0] = ExpectTypeFancy(); HandleError(&Reader);
        }else if(DoAttribute(Attribute, "emphasis")){   Theme->ResponseFancies[1] = ExpectTypeFancy(); HandleError(&Reader);
        }else{ LogInvalidAttribute(Attribute); return false; }
    }
    
    Theme->DescriptionFancies[0] = Theme->BasicFancy;
    Theme->DescriptionFancies[1] = Theme->DirectionFancy;
    Theme->DescriptionFancies[2] = Theme->RoomFancy; 
    Theme->DescriptionFancies[3] = Theme->ItemFancy;
    Theme->DescriptionFancies[4] = Theme->MiscFancy;
    Theme->DescriptionFancies[5] = Theme->MoodFancy;
    
    return true;
}

//~ Sound effects

b8
asset_system::ProcessSoundEffect(){
    b8 Result = false;
    
    const char *Name = Expect(&Reader, String);
    asset_sound_effect *Sound = Strings.GetInHashTablePtr(&SoundEffectTable, Name);
    if(Sound->Sound.Samples){
        ProcessIgnore();
        //LogError("Cannot change a sound after game has started");
        return true;
    }
    
    *Sound = {};
    Sound->VolumeMultiplier = 1.0f;
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = Expect(&Reader, Identifier);
        
        if(DoAttribute(Attribute, "path")){
            const char *Path = Expect(&Reader, String);
            sound_data Data = LoadWavFile(&Memory, Path);
            if(!Data.Samples){
                LogError("'%s' isn't a valid path to a wav file!", Path);
            }
            Sound->Sound = Data;
        }else if(DoAttribute(Attribute, "volume")){
            Sound->VolumeMultiplier = Expect(&Reader, Float);
        }else{ LogInvalidAttribute(Attribute); return false; }
    }
    
    return true;
}

//~ Fonts
b8
asset_system::ProcessFont(){
    b8 Result = false;
    
    const char *Name = Expect(&Reader, String);
    asset_font *Font = Strings.GetInHashTablePtr(&FontTable, Name);
    *Font = {};
    
    v2s CurrentOffset = V2S(0);
    s32 Height = 0;
    s32 Padding = 0;
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = Expect(&Reader, Identifier);
        
        if(DoAttribute(Attribute, "path")){ 
            const char *Path = Expect(&Reader, String);
            
            image *Image = LoadImage(Path);
            if(!Image){
                LogError("'%s' isn't a valid path to an image!", Path);
                return(false);
            }
            
            Font->Texture = Image->Texture;
            Font->Size = Image->Size;
        }else if(DoAttribute(Attribute, "height")){
            Height = ExpectPositiveInteger();
            
            Font->Height = (f32)Height;
        }else if(DoAttribute(Attribute, "padding")){
            Padding = ExpectPositiveInteger();
            
            CurrentOffset.Y += Padding;
        }else if(DoAttribute(Attribute, "descent")){
            Font->Descent = (f32)ExpectPositiveInteger();
        }else if(DoAttribute(Attribute, "char")){
            const char *S = Expect(&Reader, String);
            if(S[1] || !S[0]){
                LogError("'%s' is not a single character!", S);
                return false;
            }
            char C = S[0];
            
            s32 Width = ExpectPositiveInteger();
            
            if(CurrentOffset.X+Width+Padding >= Font->Size.X){
                CurrentOffset.X = 0;
                CurrentOffset.Y += Height+2*Padding;
                Assert(CurrentOffset.Y >= 0);
            }
            CurrentOffset.X += Padding;
            
            Font->Table[C].Width = Width;
            Font->Table[C].Offset.X = CurrentOffset.X;
            Font->Table[C].Offset.Y = Font->Size.Height-CurrentOffset.Y-Height;
            
            CurrentOffset.X += Width+Padding;
        }else{
            if(!Font->Texture){
                LogError("The font image must be defined before any characters!");
                return false;
            }
            if(!Height){
                LogError("The font height must be defined before any characters!");
                return false;
            }
            
            char C = HashTableFind(&ASCIITable, Attribute);
            
            if(C){
                s32 Width = ExpectPositiveInteger();
                
                if(CurrentOffset.X+Width+Padding >= Font->Size.X){
                    CurrentOffset.X = 0;
                    CurrentOffset.Y += Height+2*Padding;
                    Assert(CurrentOffset.Y >= 0);
                }
                CurrentOffset.X += Padding;
                
                Font->Table[C].Width = Width;
                Font->Table[C].Offset.X = CurrentOffset.X;
                Font->Table[C].Offset.Y = Font->Size.Height-CurrentOffset.Y-Height;
                if(IsALetter(C)){
                    C = C - 'A' + 'a';
                    Font->Table[C].Width = Width;
                    Font->Table[C].Offset.X = CurrentOffset.X;
                    Font->Table[C].Offset.Y = Font->Size.Height-CurrentOffset.Y-Height;
                }
                
                CurrentOffset.X += Width+Padding;
            }else{ LogInvalidAttribute(Attribute); return false; }
        }
    }
    
    return true;
}

//~ Text adventure rooms
b8
asset_system::ProcessTADescription(dynamic_array<ta_string *> *Descriptions){
    b8 Result = false;
    
    asset_tag Tag = MaybeExpectTag();
    HandleError(&Reader);
    
    string_builder Builder = BeginStringBuilder(&TransientStorageArena, DEFAULT_BUFFER_SIZE);
    StringBuilderAddVar(&Builder, Tag);
    while(true){
        file_token Token = Reader.PeekToken();
        if(Token.Type != FileTokenType_String) break;
        const char *S = Expect(&Reader, String);
        
        u32 Length = CStringLength(S);
        for(u32 I=0; S[I]; I++){
            char C = S[I];
            
            if(C == '\\'){
                char Next = S[I+1];
                if(IsANumber(Next)){
                    Next -= '0';
                    StringBuilderAdd(&Builder, '\002');
                    StringBuilderAdd(&Builder, Next+1);
                    I++;
                    continue;
                }else if(Next == '\\') I++;
                else if(Next == 'n'){
                    I++;
                    StringBuilderAdd(&Builder, '\n');
                    continue;
                }
            }
            StringBuilderAdd(&Builder, C);
            
        }
    }
    
    ta_string *Description = (ta_string *)FinalizeStringBuilder(&Memory, &Builder);
    ArrayAdd(Descriptions, Description);
    
    return true;
}

b8
asset_system::ProcessTARoom(){
    b8 Result = false;
    ta_system *TA = &TextAdventure;
    
    const char *Name = Expect(&Reader, String);
    ta_room *Room = HashTableGetPtr(&TA->RoomTable, TAIDByName(TA, Name));
    Room->Name = Strings.GetPermanentString(Name);
    Room->Tag = MaybeExpectTag();
    
    dynamic_array<ta_string *> Descriptions = MakeDynamicArray<ta_string *>(8, &TransientStorageArena);
    
    u32 MaxItemCount = TA_ROOM_DEFAULT_ITEM_COUNT;
    v2s CurrentOffset = V2S(0);
    s32 Height = 0;
    s32 Padding = 0;
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = Expect(&Reader, Identifier);
        
        if(DoAttribute(Attribute, "description")){ 
            if(!ProcessTADescription(&Descriptions)) return false;
        }else if(DoAttribute(Attribute, "area")){
            const char *S = Expect(&Reader, String);
            Room->Area = MakeTAID(Strings.GetString(S));
        }else if(DoAttribute(Attribute, "adjacents")){ 
            while(true){
                file_token Token = Reader.PeekToken();
                if(Token.Type != FileTokenType_Identifier) break;
                
                direction Direction = HashTableFind(&TA->DirectionTable, (const char *)Token.Identifier);
                if(!Direction) break;
                Expect(&Reader, Identifier);
                
                const char *NextRoomName = Expect(&Reader, String);
                Room->Adjacents[Direction] = TAIDByName(TA, NextRoomName);
                asset_tag Tag = MaybeExpectTag();
                if(!Room->Dirty) Room->AdjacentTags[Direction] = Tag;
            }
        }else if(DoAttribute(Attribute, "item_count")){
            MaxItemCount = ExpectPositiveInteger();
        }else if(DoAttribute(Attribute, "items")){
            array<const char *> CStringItems = Reader.ExpectTypeArrayCString();
            HandleError(&Reader);
            u32 Count = Maximum(CStringItems.Count, MaxItemCount);
            Room->Items = MakeArray<ta_id>(&Memory, Count);
            for(u32 I=0; I<CStringItems.Count; I++){
                ta_id S = TAIDByName(TA, CStringItems[I]);
                // TODO(Tyler): I'm not sure how this should be done. 
                // This will not work if an item is present in multiple locations
                if(Room->Dirty){
                    for(u32 J=0; J<TA->Inventory.Count; J++){
                        if(TA->Inventory[J] == S) goto repeat_loop;
                    }
                }
                ArrayAdd(&Room->Items, S);
                
                repeat_loop:;
            }
            
        }else{ LogInvalidAttribute(Attribute); return false; }
    }
    
    Room->Descriptions = MakeArray<ta_string *>(&Memory, Descriptions.Count);
    for(u32 I=0; I<Descriptions.Count; I++){
        ArrayAdd(&Room->Descriptions, Descriptions[I]);
    }
    
    return true;
}

//~
b8
asset_system::ProcessTAItem(){
    b8 Result = false;
    
    ta_system *TA = &TextAdventure;
    const char *Name = Expect(&Reader, String);
    ta_item *Item = HashTableGetPtr(&TA->ItemTable, TAIDByName(TA, Name));
    Item->Name = Strings.GetPermanentString(Name);
    Item->Tag = MaybeExpectTag();
    
    // Aliases
    array<const char *> Array = Reader.ExpectTypeArrayCString();
    HandleError(&Reader);
    Item->Aliases = MakeArray<const char *>(&Memory, Array.Count);
    for(u32 I=0; I<Array.Count; I++){
        ArrayAdd(&Item->Aliases, Strings.GetPermanentString(Array[I]));
    }
    
    // Attributes
    dynamic_array<ta_string *> Descriptions = MakeDynamicArray<ta_string *>(8, &TransientStorageArena);
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = Expect(&Reader, Identifier);
        if(DoAttribute(Attribute, "description")){ 
            if(!ProcessTADescription(&Descriptions)) return false;
        }else if(DoAttribute(Attribute, "adjectives")){
            array<const char *> Adjectives = Reader.ExpectTypeArrayCString();
            Item->Adjectives = MakeArray<const char *>(&Memory, Adjectives.Count);
            for(u32 I=0; I<Adjectives.Count; I++){
                ArrayAdd(&Item->Adjectives, Strings.GetPermanentString(Adjectives[I]));
            }
        }else if(DoAttribute(Attribute, "cost")){
            if(!Item->Dirty) Item->Cost = ExpectPositiveInteger();
        }else{ LogInvalidAttribute(Attribute); return false; }
    }
    
    Item->Descriptions = MakeArray<ta_string *>(&Memory, Descriptions.Count);
    for(u32 I=0; I<Descriptions.Count; I++){
        ArrayAdd(&Item->Descriptions, Descriptions[I]);
    }
    
    return true;
}

//~
b8
asset_system::ProcessTAMap(){
    b8 Result = false;
    
    ta_system *TA = &TextAdventure;
    ta_map *Map = &TA->Map;
    
    // Attributes
    dynamic_array<ta_area> Areas = MakeDynamicArray<ta_area>(8, &TransientStorageArena);
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = Expect(&Reader, Identifier);
        if(DoAttribute(Attribute, "path")){ 
            const char *Path = Expect(&Reader, String);
            image *Image = LoadImage(Path);
            Map->Texture = Image->Texture;
            Map->Size = V2(Image->Size);
        }else if(DoAttribute(Attribute, "area")){
            ta_area *Area = ArrayAlloc(&Areas);
            const char *S = Expect(&Reader, String);
            Area->Name = TAIDByName(TA, S);
            Area->Offset = Reader.ExpectTypeV2();
        }else{ LogInvalidAttribute(Attribute); return false; }
    }
    
    Map->Areas = MakeArray<ta_area>(&Memory, Areas.Count);
    for(u32 I=0; I<Areas.Count; I++){
#if 0
        u32 J;
        for(J=0; J<Map->Areas.Count; J++){
            if(IsFirstStringFirst(Strings.GetString(Areas[I].Name), Strings.GetString(Map->Areas[J].Name))){
                break;
            }
        }
        ArrayInsert(&Map->Areas, J, Areas[I]);
#endif
        ArrayAdd(&Map->Areas, Areas[I]);
    }
    
    return true;
}

#else // SNAIL_JUMPY_USE_PROCESSED_ASSETS

//~ Processed assets

internal inline ta_string *
MakeTAString(memory_arena *Memory, asset_tag Tag, const char *S, u32 L){
    ta_string *Result = (ta_string *)ArenaPush(Memory, sizeof(Tag)+L+1);
    Result->Tag = Tag;
    CopyCString((char *)Result->Data, S, L);
    return Result;
}

internal inline ta_string *
MakeTAString(memory_arena *Memory, asset_tag Tag, const char *S){
    return MakeTAString(Memory, Tag, S, CStringLength(S));
}

internal inline ta_string *
ReadTAString(stream *Stream){
    ta_string *Result = (ta_string *)Stream->BufferPos;
    Assert(StreamConsumeType(Stream, asset_tag));
    Assert(StreamConsumeString(Stream));
    
    return Result;
}

#include "generated_asset_data.h"

void
asset_system::LoadProcessedAssets(void *Data, u32 DataSize){
    ta_system *TA = &TextAdventure;
    
    stream Stream = MakeReadStream(Data, DataSize);
    sjap_header Header; StreamReadVar(&Stream, Header);
    Assert((Header.SJAP[0] == 'S') &&
           (Header.SJAP[1] == 'J') &&
           (Header.SJAP[2] == 'A') &&
           (Header.SJAP[3] == 'P'));
    
    InitializeProcessedAssets(this, Data, DataSize);
}


void
asset_system::LoadAssetFile(const char *Path){}

#endif // SNAIL_JUMPY_USE_PROCESSED_ASSETS