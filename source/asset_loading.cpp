
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
#define ASSET_TAG(S, N) HashTableInsert(&TagTable, S, AssetTag_##N);
    ASSET_TAGS;
#undef ASSET_TAG
    
#define DIRECTION(Name, Direction) HashTableInsert(&DirectionTable, Name, Direction);
    DirectionTable = MakeHashTable<const char *, direction>(Arena, 3*Direction_TOTAL);
    DIRECTIONS;
#undef DIRECTION
    
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

asset_loading_status
asset_system::ChooseStatus(asset_loading_status Status){
    if(Status > LoadingStatus){
        LoadingStatus = Status;
        return Status;
    }
    return LoadingStatus;
}

void 
asset_system::BeginCommand(const char *Name){
    CurrentCommand = Name;
    CurrentAttribute = 0;
    CurrentAsset = 0;
}

void 
asset_system::LogWarning(const char *Format, ...){
    LoadingStatus = AssetLoadingStatus_Warnings;
    
    va_list VarArgs;
    va_start(VarArgs, Format);
    
    
    const char *Attribute = "";
    if(CurrentAttribute) Attribute = ArenaCStringConcatenate(&GlobalTransientMemory, ",", CurrentAttribute);
    const char *Asset = "";
    if(CurrentAsset) Asset = ArenaCStringConcatenateN(&GlobalTransientMemory, 3, ",\"", CurrentAsset, "\"");
    
    char *Message = ArenaPushFormatCString(&GlobalTransientMemory, "(%s%s%s Line: %u) WARNING: %s", CurrentCommand, Asset, Attribute, Reader.Line, Format);
    VLogMessage(Message, VarArgs);
    
    va_end(VarArgs);
}

void
asset_system::LogError(const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    
    const char *Attribute = "";
    if(CurrentAttribute) Attribute = ArenaCStringConcatenate(&GlobalTransientMemory, ",", CurrentAttribute);
    const char *Asset = "";
    if(CurrentAsset) Asset = ArenaCStringConcatenateN(&GlobalTransientMemory, 3, ",\"", CurrentAsset, "\"");
    
    char *Message = ArenaPushFormatCString(&GlobalTransientMemory, "(%s%s%s Line: %u) %s", CurrentCommand, Asset, Attribute, Reader.Line, Format);
    VLogMessage(Message, VarArgs);
    
    va_end(VarArgs);
}

b8
asset_system::SeekNextAttribute(){
    while(true){
        file_token Token = Reader.PeekToken();
        switch(Token.Type){
            case FileTokenType_BeginArguments: {
                u32 ArgumentCount = 1;
                while(ArgumentCount){
                    Token = Reader.PeekToken();
                    if(Token.Type == FileTokenType_EndArguments){
                        ArgumentCount--;
                    }else if((Token.Type == FileTokenType_Invalid) ||
                             (Token.Type == FileTokenType_EndFile)){
                        return false;
                    }
                    Reader.NextToken();
                }
            }break;
            case FileTokenType_Identifier: {
                file_token Token = Reader.PeekToken(2);
                if(Token.Type == FileTokenType_BeginArguments){
                    Reader.NextToken();
                    continue;
                }
                return true;
            }break;
            case FileTokenType_Invalid:
            case FileTokenType_EndFile: {
                return false;
            }break;
            case FileTokenType_BeginCommand: {
                return true;
            }break;
        }
        Reader.NextToken();
    }
}

#define HANDLE_INVALID_ATTRIBUTE(Attribute) \
CurrentAttribute = 0;\
LogWarning("Invalid attribute: %s", Attribute); \
if(!SeekNextAttribute()) return AssetLoadingStatus_Errors;

fancy_font_format
asset_system::ExpectTypeFancy(){
    fancy_font_format Result = {};
    
    file_token Token = Reader.PeekToken();
    if(Token.Type != FileTokenType_Identifier) return Result;
    if(CompareCStrings(Token.Identifier, "Fancy")){
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
    if(CompareCStrings(Token.Identifier, "Tag")){
        Expect(&Reader, Identifier);
        
        Reader.ExpectToken(FileTokenType_BeginArguments);
        HandleError(&Reader);
        
        for(u32 I=0; I<ArrayCount(Result.E); I++){
            Token = Reader.PeekToken();
            if(Token.Type != FileTokenType_String) break;
            const char *S = Expect(&Reader, String);
            
            u8 ID = (u8)HashTableFind(&TagTable, S);
            if(!ID){ 
                LogWarning("'%s' is not registered as a tag and thus will be ignored!", S);
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

ta_name
asset_system::ExpectTypeName(){
    ta_name Result = {};
    
    const char *Identifier = Expect(&Reader, Identifier);
    if(!CompareCStrings(Identifier, "Name")){
        Reader.LastError = FileReaderError_InvalidToken;
        return Result;
    }
    
    Reader.ExpectToken(FileTokenType_BeginArguments);
    HandleError(&Reader);
    
    file_token Token = Reader.PeekToken();
    if(Token.Type == FileTokenType_String){
        const char *Name = Expect(&Reader, String);
        Result.Name = Strings.GetPermanentString(Name);
    }
    
    Token = Reader.PeekToken();
    if(Token.Type == FileTokenType_Identifier){
        array<const char *> Aliases = Reader.ExpectTypeArrayCString();
        Result.Aliases = MakeArray<const char *>(&Memory, Aliases.Count);
        for(u32 I=0; I<Aliases.Count; I++){
            char *Alias = ArenaPushLowerCString(&GlobalTransientMemory, Aliases[I]);
            ArrayAdd(&Result.Aliases, Strings.GetPermanentString(Alias));
        }
    }
    
    Token = Reader.PeekToken();
    if(Token.Type == FileTokenType_Identifier){
        array<const char *> Adjectives = Reader.ExpectTypeArrayCString();
        Result.Adjectives = MakeArray<const char *>(&Memory, Adjectives.Count);
        for(u32 I=0; I<Adjectives.Count; I++){
            char *Adjective = ArenaPushLowerCString(&GlobalTransientMemory, Adjectives[I]);
            ArrayAdd(&Result.Adjectives, Strings.GetPermanentString(Adjective));
        }
    }
    
    Reader.ExpectToken(FileTokenType_EndArguments);
    HandleError(&Reader);
    
    return Result;
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
    File = OSOpenFile(Path, OpenFile_Read);
    if(!File) return(Result);
    u64 LastImageWriteTime;
    LastImageWriteTime = OSGetLastFileWriteTime(File);
    OSCloseFile(File);
    u8 *ImageData;
    s32 Components;
    
    Result = HashTableGetPtr(&LoadedImageTable, Path);
    if(Result->HasBeenLoadedBefore){
        if(Result->LastWriteTime < LastImageWriteTime){
            entire_file File = ReadEntireFile(&GlobalTransientMemory, Path);
            
            ImageData = (u8 *)stbi_load_from_memory((u8 *)File.Data,
                                                    (int)File.Size,
                                                    &Result->Width, &Result->Height,
                                                    &Components, 4);
            TextureUpload(Result->Texture, ImageData, Result->Width, Result->Height);
            stbi_image_free(ImageData);
        }
    }else{
        entire_file File;
        File = ReadEntireFile(&GlobalTransientMemory, Path);
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
    b8 Result = CompareCStrings(String, Attribute);
    if(Result) CurrentAttribute = Attribute;
    return(Result);
}

// TODO(Tyler): This should be made into an actual comment type such as /* */ or #if 0 #endif
asset_loading_status
asset_system::ProcessIgnore(){
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        Reader.NextToken();
    }
    return AssetLoadingStatus_Okay;
}

asset_loading_status
asset_system::LoadAssetFile(const char *Path){
    ARENA_FUNCTION_MARKER(&GlobalTransientMemory);
    
    CurrentCommand = 0;
    CurrentAttribute = 0;
    
    asset_loading_status Status = LoadingStatus;
    
    os_file *File = OSOpenFile(Path, OpenFile_Read);
    u64 NewFileWriteTime = OSGetLastFileWriteTime(File);
    OSCloseFile(File);
    
    if(LastFileWriteTime < NewFileWriteTime){
        ArenaClear(&Memory);
        LoadingStatus = AssetLoadingStatus_Okay;
        Status = LoadingStatus;
        
        Reader = MakeFileReader(Path, this);
        
        while(Status != AssetLoadingStatus_Errors){
            file_token Token = Reader.NextToken();
            
            switch(Token.Type){
                case FileTokenType_BeginCommand: {
                    Status = ProcessCommand();
                }break;
                case FileTokenType_EndFile: {
                    goto end_loop;
                }break;
                default: {
                    LogMessage("(Line: %u) Token: %s was not expected!", Reader.Line, TokenToString(Token));
                    Status = AssetLoadingStatus_Errors;
                }break;
            }
        }
        end_loop:;
    }
    
    LastFileWriteTime = NewFileWriteTime;
    
    Assert(Status >= LoadingStatus);
    if(Status == AssetLoadingStatus_Errors) return ChooseStatus(Status);
    
    { //- Post processing
        ta_system *TA = TextAdventure;
        if(SpecialCommands & SpecialCommand_StartCarillonPages) MurkwellStartCarillonPages(TA, this);
        
        SpecialCommands = 0;
        
        // Checking items
        FOR_EACH(ItemID, &TA->Inventory){
            TA->CheckAndLogItemID(ItemID);
        }
    }
    
    return ChooseStatus(Status);
}

#define ASSET_LOADER_COMMMAND(Command)                 \
if(CompareCStrings(String, #Command)) { \
BeginCommand(#Command);            \
return Process##Command(); \
}       

asset_loading_status
asset_system::ProcessCommand(){
    asset_loading_status Result = AssetLoadingStatus_Errors;
    
    const char *String = Expect(&Reader, Identifier);
    
    ASSET_LOADER_COMMMAND(Ignore);
    ASSET_LOADER_COMMMAND(SpecialCommands);
    ASSET_LOADER_COMMMAND(Variables);
    ASSET_LOADER_COMMMAND(Theme);
    ASSET_LOADER_COMMMAND(Font);
    ASSET_LOADER_COMMMAND(SoundEffect);
    ASSET_LOADER_COMMMAND(TARoom);
    ASSET_LOADER_COMMMAND(TAItem);
    ASSET_LOADER_COMMMAND(TAMap);
    
    LogMessage("(Line: %u) '%s' isn't a valid command!", Reader.Line, String);
    ProcessIgnore();
    return AssetLoadingStatus_Warnings;
}
#undef ASSET_LOADER_COMMMAND

//~ Variables

asset_loading_status 
asset_system::ProcessSpecialCommands(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    ta_system *TA = TextAdventure;
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = Expect(&Reader, Identifier);
        if(DoAttribute(Attribute, "give_item")){
            const char *S = Expect(&Reader, String);
            ta_id Item = TAIDByName(TA, S);
            b8 FoundIt = false;
            for(u32 I=0; I<TA->Inventory.Count; I++){
                if(TA->Inventory[I] == Item) { FoundIt = true; break; }
            }
            if(!FoundIt) TA->InventoryAddItem(Item);
        }else if(DoAttribute(Attribute, "start_carillon_pages")){
            SpecialCommands |= SpecialCommand_StartCarillonPages;
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute);  }
    }
    
    return ChooseStatus(Result);
}

asset_loading_status
asset_system::ProcessVariables(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    ta_system *TA = TextAdventure;
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = Expect(&Reader, Identifier);
        if(DoAttribute(Attribute, "var")){
            const char *Name = Expect(&Reader, String);
            string_builder Builder = BeginStringBuilder(&GlobalTransientMemory, DEFAULT_BUFFER_SIZE);
            ExpectDescriptionStrings(&Builder);
            const char *Data = FinalizeStringBuilder(&Memory, &Builder);
            asset_variable *Variable = Strings.HashTableGetPtr(&VariableTable, Name);
            Variable->S = Data;
        }else if(DoAttribute(Attribute, "ta_id")){
            const char *Name = Expect(&Reader, String);
            const char *Data = Expect(&Reader, String);
            asset_variable *Variable = Strings.HashTableGetPtr(&VariableTable, Name);
            Variable->S = Strings.GetPermanentString(Data);
            Variable->TAID = TAIDByName(TA, Data);
        }else if(DoAttribute(Attribute, "name")){
            const char *Name = Expect(&Reader, String);
            ta_name NameData = ExpectTypeName();
            HandleError(&Reader);
            asset_variable *Variable = Strings.HashTableGetPtr(&VariableTable, Name);
            Variable->NameData = NameData;
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    return ChooseStatus(Result);
}

//~ Theme
asset_loading_status
asset_system::ProcessTheme(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    ta_system *TA = TextAdventure;
    
    const char *Name = Expect(&Reader, String);
    CurrentAsset = Name;
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
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    return ChooseStatus(Result);
}

//~ Sound effects

asset_loading_status
asset_system::ProcessSoundEffect(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    const char *Name = Expect(&Reader, String);
    CurrentAsset = Name;
    asset_sound_effect *Sound = Strings.HashTableGetPtr(&SoundEffectTable, Name);
    TicketMutexBegin(&Mixer->SoundMutex);
    
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
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    TicketMutexEnd(&Mixer->SoundMutex);
    return ChooseStatus(Result);
}

//~ Fonts
asset_loading_status
asset_system::ProcessFont(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    const char *Name = Expect(&Reader, String);
    CurrentAsset = Name;
    asset_font *Font = Strings.HashTableGetPtr(&FontTable, Name);
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
                return AssetLoadingStatus_Errors;
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
                LogWarning("'%s' is not a single character and will thus be ignored!", S);
                Result = AssetLoadingStatus_Warnings;
                SeekNextAttribute();
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
                return AssetLoadingStatus_Errors;
            }
            if(!Height){
                LogError("The font height must be defined before any characters!");
                return AssetLoadingStatus_Errors;
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
            }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
        }
    }
    
    return ChooseStatus(Result);
}

//~ Text adventure rooms
asset_loading_status
asset_system::ExpectDescriptionStrings(string_builder *Builder){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
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
                    StringBuilderAdd(Builder, '\002');
                    StringBuilderAdd(Builder, Next+1);
                    I++;
                    continue;
                }
            }
            StringBuilderAdd(Builder, C);
            
        }
    }
    
    return ChooseStatus(Result);
}

asset_loading_status
asset_system::ProcessTADescription(dynamic_array<ta_data *> *Descriptions, ta_data_type Type){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    asset_tag Tag = MaybeExpectTag();
    HandleError(&Reader);
    
    string_builder Builder = BeginStringBuilder(&GlobalTransientMemory, DEFAULT_BUFFER_SIZE);
    StringBuilderAddVar(&Builder, Type);
    StringBuilderAddVar(&Builder, Tag);
    Assert(Builder.BufferSize == offsetof(ta_data, Data));
    ExpectDescriptionStrings(&Builder);
    ta_data *Description = (ta_data *)FinalizeStringBuilder(&Memory, &Builder);
    ArrayAdd(Descriptions, Description);
    
    return ChooseStatus(Result);
}

asset_loading_status
asset_system::ProcessTARoom(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    ta_system *TA = TextAdventure;
    
    const char *Identifier = Expect(&Reader, String);
    CurrentAsset = Identifier;
    ta_id ID = TAIDByName(TA, Identifier);
    ta_room *Room = HashTableGetPtr(&TA->RoomTable, ID);
    Room->ID = ID;
    Room->NameData = ExpectTypeName();
    HandleError(&Reader);
    Room->Tag = MaybeExpectTag();
    HandleError(&Reader);
    
    dynamic_array<ta_data *> Descriptions = MakeDynamicArray<ta_data *>(8, &GlobalTransientMemory);
    
    u32 MaxItemCount = TA_ROOM_DEFAULT_ITEM_COUNT;
    v2s CurrentOffset = V2S(0);
    s32 Height = 0;
    s32 Padding = 0;
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = Expect(&Reader, Identifier);
        
        if(DoAttribute(Attribute, "description")){ 
            Result = ProcessTADescription(&Descriptions);
            if(Result == AssetLoadingStatus_Errors) return ChooseStatus(Result);
        }else if(DoAttribute(Attribute, "data_command")){
            Result = ProcessTADescription(&Descriptions, TADataType_Command);
            if(Result == AssetLoadingStatus_Errors) return ChooseStatus(Result);
        }else if(DoAttribute(Attribute, "data_room")){ 
            ta_data *Data = ArenaPushType(&Memory, ta_data);
            Data->Type = TADataType_Room;
            Data->Tag = MaybeExpectTag();
            HandleError(&Reader);
            const char *S = Expect(&Reader, String);
            Data->TAID = TAIDByName(TA, S);
            ArrayAdd(&Descriptions, Data);
        }else if(DoAttribute(Attribute, "data_asset")){
            ta_data *Data = ArenaPushType(&Memory, ta_data);
            Data->Type = TADataType_Asset;
            Data->Tag = MaybeExpectTag();
            HandleError(&Reader);
            const char *S = Expect(&Reader, String);
            Data->Asset = MakeAssetID(Strings.GetString(S));
            ArrayAdd(&Descriptions, Data);
        }else if(DoAttribute(Attribute, "area")){
            const char *S = Expect(&Reader, String);
            Room->Area = MakeTAID(Strings.GetString(S));
        }else if(DoAttribute(Attribute, "adjacents")){ 
            while(true){
                file_token Token = Reader.PeekToken();
                if(Token.Type != FileTokenType_Identifier) break;
                
                direction Direction = HashTableFind(&DirectionTable, (const char *)Token.Identifier);
                if(!Direction) break;
                Expect(&Reader, Identifier);
                
                const char *NextRoomName = Expect(&Reader, String);
                Room->Adjacents[Direction] = TAIDByName(TA, NextRoomName);
                asset_tag Tag = MaybeExpectTag();
                if(!(Room->Flags & RoomFlag_Dirty)) Room->AdjacentTags[Direction] = Tag;
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
                if(Room->Flags & RoomFlag_Dirty){
                    for(u32 J=0; J<TA->Inventory.Count; J++){
                        if(TA->Inventory[J] == S) goto repeat_loop;
                    }
                }
                ArrayAdd(&Room->Items, S);
                
                repeat_loop:;
            }
            
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    Room->Datas = MakeArray<ta_data *>(&Memory, Descriptions.Count);
    for(u32 I=0; I<Descriptions.Count; I++){
        ArrayAdd(&Room->Datas, Descriptions[I]);
    }
    
    return ChooseStatus(Result);
}

//~
asset_loading_status
asset_system::ProcessTAItem(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    ta_system *TA = TextAdventure;
    
    const char *Identifier = Expect(&Reader, String);
    CurrentAsset = Identifier;
    ta_id ID = TAIDByName(TA, Identifier);
    ta_item *Item = HashTableGetPtr(&TA->ItemTable, ID);
    Item->ID = ID;
    Item->NameData = ExpectTypeName();
    HandleError(&Reader);
    Item->Tag = MaybeExpectTag();
    HandleError(&Reader);
    
    // Attributes
    dynamic_array<ta_data *> Descriptions = MakeDynamicArray<ta_data *>(8, &GlobalTransientMemory);
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = Expect(&Reader, Identifier);
        if(DoAttribute(Attribute, "description")){ 
            Result = ProcessTADescription(&Descriptions);
            if(Result == AssetLoadingStatus_Errors) return ChooseStatus(Result);
        }else if(DoAttribute(Attribute, "data_command")){
            Result = ProcessTADescription(&Descriptions, TADataType_Command);
            if(Result == AssetLoadingStatus_Errors) return ChooseStatus(Result);
        }else if(DoAttribute(Attribute, "data_item")){ 
            ta_data *Data = ArenaPushType(&Memory, ta_data);
            Data->Type = TADataType_Item;
            Data->Tag = MaybeExpectTag();
            HandleError(&Reader);
            const char *S = Expect(&Reader, String);
            Data->TAID = TAIDByName(TA, S);
            ArrayAdd(&Descriptions, Data);
        }else if(DoAttribute(Attribute, "data_asset")){
            ta_data *Data = ArenaPushType(&Memory, ta_data);
            Data->Type = TADataType_Asset;
            Data->Tag = MaybeExpectTag();
            HandleError(&Reader);
            const char *S = Expect(&Reader, String);
            Data->Asset = MakeAssetID(Strings.GetString(S));
            ArrayAdd(&Descriptions, Data);
        }else if(DoAttribute(Attribute, "cost")){
            if(!Item->Dirty) Item->Cost = ExpectPositiveInteger();
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    Item->Datas = MakeArray<ta_data *>(&Memory, Descriptions.Count);
    for(u32 I=0; I<Descriptions.Count; I++){
        ArrayAdd(&Item->Datas, Descriptions[I]);
    }
    
    MurkwellProcessItem(TA, ID, Item);
    
    return ChooseStatus(Result);
}

//~
asset_loading_status
asset_system::ProcessTAMap(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    ta_system *TA = TextAdventure;
    ta_map *Map = &TA->Map;
    
    // Attributes
    dynamic_array<ta_area> Areas = MakeDynamicArray<ta_area>(8, &GlobalTransientMemory);
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
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
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
    
    return ChooseStatus(Result);
}

#else // SNAIL_JUMPY_USE_PROCESSED_ASSETS

//~ Processed assets

internal inline ta_data *
MakeTAString(memory_arena *Memory, asset_tag Tag, const char *S, u32 L){
    ta_data *Result = (ta_data *)ArenaPush(Memory, sizeof(Tag)+L+1);
    Result->Tag = Tag;
    CopyCString((char *)Result->Data, S, L);
    return Result;
}

internal inline ta_data *
MakeTAString(memory_arena *Memory, asset_tag Tag, const char *S){
    return MakeTAString(Memory, Tag, S, CStringLength(S));
}

internal inline ta_data *
ReadTAString(stream *Stream){
    ta_data *Result = (ta_data *)Stream->BufferPos;
    Assert(StreamConsumeType(Stream, asset_tag));
    Assert(StreamConsumeString(Stream));
    
    return Result;
}

#include "generated_asset_data.h"

void
asset_system::LoadProcessedAssets(void *Data, u32 DataSize){
    ta_system *TA = TextAdventure;
    
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