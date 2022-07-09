
#if !defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)


//~ Initialization
void
asset_loader::Initialize(memory_arena *Arena, audio_mixer *Mixer_, asset_system *Assets, ta_system *TA){
    InProgress.Initialize(Arena);
    
    Mixer = Mixer_;
    MainAssets = Assets;
    TextAdventure = TA;
    
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

#define HandleToken(Token)                   \
if(Token.Type == FileTokenType_BeginCommand) break; \
if(Token.Type == FileTokenType_EndFile)      break; \
if(Token.Type == FileTokenType_Invalid)      break; \

#define SJA_ERROR_FUNCTION(ErrorResult) \
SeekEndOfFunction(); \
return ErrorResult;

#define SJA_ERROR_ATTRIBUTE \
SeekNextAttribute(); \
continue;

#define SJA_ERROR_COMMAND \
SeekNextCommand(); \
return AssetLoadingStatus_Warnings;

#define SJA_HANDLE_ERROR_(Reader, ExpectedType, ...) \
if((Reader)->LastError == FileReaderError_InvalidToken){ \
LogWarning("Expected '%s', instead read: '%s'", TokenTypeName(ExpectedType), TokenToString((Reader)->LastToken)); \
__VA_ARGS__; \
}

#define SJA_EXPECT_TOKEN(Reader, Expected, ErrorResult) { \
file_token Token = (Reader)->ExpectToken(Expected); \
SJA_HANDLE_ERROR_(Reader, Expected, ErrorResult); \
}

#define SJA_EXPECT_UINT(Reader, ...) \
ExpectPositiveInteger_(); \
SJA_HANDLE_ERROR_(Reader, FileTokenType_Integer, __VA_ARGS__);

#define SJA_EXPECT_INT(Reader, ...) \
(Reader)->ExpectToken(FileTokenType_Integer).Integer; \
SJA_HANDLE_ERROR_(Reader, FileTokenType_Integer, __VA_ARGS__);

#define SJA_EXPECT_IDENTIFIER(Reader, ...) \
(Reader)->ExpectToken(FileTokenType_Identifier).Identifier; \
SJA_HANDLE_ERROR_(Reader, FileTokenType_Identifier, __VA_ARGS__);

#define SJA_EXPECT_STRING(Reader, ...) \
(Reader)->ExpectToken(FileTokenType_String).String; \
SJA_HANDLE_ERROR_(Reader, FileTokenType_String, __VA_ARGS__);

#define SJA_EXPECT_FLOAT(Reader, ...) \
(Reader)->ExpectToken(FileTokenType_Float).Float; \
SJA_HANDLE_ERROR_(Reader, FileTokenType_Float, __VA_ARGS__);

#define SJA_BEGIN_FUNCTION(Reader, Name, ErrorResult) \
const char *Identifier = SJA_EXPECT_IDENTIFIER(Reader, return ErrorResult); \
if(!CompareCStrings(Identifier, Name)){ \
LogWarning("Expected \"%s\" instead read: \"%s\"", Name, Identifier); \
return ErrorResult; \
} \
SJA_EXPECT_TOKEN(Reader, FileTokenType_BeginArguments, SJA_ERROR_FUNCTION(ErrorResult));

#define SJA_END_FUNCTION(Reader, ErrorResult) \
SJA_EXPECT_TOKEN(Reader, FileTokenType_EndArguments, SJA_ERROR_FUNCTION(ErrorResult))

asset_loading_status
asset_loader::ChooseStatus(asset_loading_status Status){
    if(Status > LoadingStatus){
        LoadingStatus = Status;
        return Status;
    }
    return LoadingStatus;
}

void
asset_loader::FailCommand(asset_loading_data *Data, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    if(Data) Data->Status = AssetLoadingStatus_Errors;
    VLogWarning(Format, VarArgs);
    SeekNextCommand();
    va_end(VarArgs);
}

void 
asset_loader::BeginCommand(const char *Name){
    CurrentCommand = Name;
    CurrentAttribute = 0;
    CurrentAsset = 0;
}

void
asset_loader::VLogWarning(const char *Format, va_list VarArgs){
    LoadingStatus = AssetLoadingStatus_Warnings;
    string_builder Builder = BeginResizeableStringBuilder(&GlobalTransientMemory, DEFAULT_BUFFER_SIZE);
    BuilderAdd(&Builder, "(Line: %u)", Reader.Line);
    if(CurrentCommand)   BuilderAdd(&Builder, "[%s", CurrentCommand);
    if(CurrentAsset)     BuilderAdd(&Builder, ",\"%s\"", CurrentAsset);
    if(CurrentAttribute) BuilderAdd(&Builder, ",%s", CurrentAttribute);
    if(CurrentCommand)   BuilderAdd(&Builder, "]");
    BuilderAdd(&Builder, ' ');
    VBuilderAdd(&Builder, Format, VarArgs);
    char *Message = EndStringBuilder(&Builder);
    // NOTE(Tyler): Use the asset loader memory, because it will last until the asset system is reset. 
    DebugInfo.SubmitMessage(DebugMessage_Asset, FinalizeStringBuilder(&InProgress.Memory, &Builder));
}

void 
asset_loader::LogWarning(const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VLogWarning(Format, VarArgs);
    va_end(VarArgs);
}

b8
asset_loader::SeekEndOfFunction(){
    file_token Token = Reader.PeekToken();
    if(Token.Type == FileTokenType_BeginArguments) Reader.NextToken();
    u32 ArgumentCount = 1;
    while(ArgumentCount > 0){
        Token = Reader.PeekToken();
        if(Token.Type == FileTokenType_BeginArguments){
            ArgumentCount++;
        }else if(Token.Type == FileTokenType_EndArguments){
            ArgumentCount--;
        }else if((Token.Type == FileTokenType_Invalid) ||
                 (Token.Type == FileTokenType_EndFile)){
            return false;
        }
        Reader.NextToken();
    }
    return true;
}

b8
asset_loader::SeekNextAttribute(){
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

b8
asset_loader::SeekNextCommand(){
    while(true){
        file_token Token = Reader.PeekToken();
        switch(Token.Type){
            case FileTokenType_BeginCommand: {
                return true;
            }break;
            case FileTokenType_Invalid:
            case FileTokenType_EndFile: {
                return false;
            }break;
        }
        Reader.NextToken();
    }
}

#define HANDLE_INVALID_ATTRIBUTE(Attribute) \
CurrentAttribute = 0;\
LogWarning("Invalid attribute: %s", Attribute); \
if(!SeekNextAttribute()) return AssetLoadingStatus_Warnings;

v2
asset_loader::ExpectTypeV2(){
    v2 Result = V2(0);
    
    SJA_BEGIN_FUNCTION(&Reader, "V2", Result);
    
    Result.X = SJA_EXPECT_FLOAT(&Reader, return Result);
    file_token Token = Reader.PeekToken();
    if(Token.Type != FileTokenType_EndArguments){
        Result.Y = SJA_EXPECT_FLOAT(&Reader, return Result);
    }else{
        Result.Y = Result.X;
    }
    
    SJA_END_FUNCTION(&Reader, Result);
    
    return(Result);
}

array<s32>
asset_loader::ExpectTypeArrayS32(){
    array<s32> Result = MakeArray<s32>(&GlobalTransientMemory, SJA_MAX_ARRAY_ITEM_COUNT);
    
    SJA_BEGIN_FUNCTION(&Reader, "Array", Result);
    
    file_token Token = Reader.PeekToken();
    while(Token.Type != FileTokenType_EndArguments){
        s32 Integer = SJA_EXPECT_INT(&Reader, SJA_ERROR_FUNCTION(Result));
        ArrayAdd(&Result, Integer);
        
        Token = Reader.PeekToken();
    }
    
    SJA_END_FUNCTION(&Reader, Result);
    
    return(Result);
}

array<const char *>
asset_loader::ExpectTypeArrayCString(){
    array<const char *> Result = MakeArray<const char *>(&GlobalTransientMemory, SJA_MAX_ARRAY_ITEM_COUNT);
    
    SJA_BEGIN_FUNCTION(&Reader, "Array", Result);
    
    file_token Token = Reader.PeekToken();
    while(Token.Type != FileTokenType_EndArguments){
        const char *String = SJA_EXPECT_STRING(&Reader, SJA_ERROR_FUNCTION(Result));
        ArrayAdd(&Result, String);
        
        Token = Reader.PeekToken();
    }
    
    SJA_END_FUNCTION(&Reader, Result);
    
    return(Result);
}

color
asset_loader::ExpectTypeColor(){
    color Result = {};
    
    SJA_BEGIN_FUNCTION(&Reader, "Color", ERROR_COLOR);
    
    file_token Token = Reader.PeekToken();
    if(Token.Type == FileTokenType_Float){
        for(u32 I=0; I<4; I++){
            Result.E[I] = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_FUNCTION(ERROR_COLOR));
        }
    }else if(Token.Type == FileTokenType_Integer){
        file_token First = Reader.NextToken();
        Token = Reader.PeekToken();
        if((Token.Type == FileTokenType_Integer) ||
           (Token.Type == FileTokenType_Float)){
            First = MaybeTokenIntegerToFloat(First);
            Assert(First.Type == FileTokenType_Float);
            Result.R = First.Float;
            for(u32 I=1; I<4; I++){
                Result.E[I] = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_FUNCTION(ERROR_COLOR));
            }
        }else if(Token.Type == FileTokenType_EndArguments){
            Result = MakeColor(First.Integer);
        }else{
            LogWarning("Expected ) or a number, and %s is neither!", TokenToString(Token));
            SJA_ERROR_FUNCTION(ERROR_COLOR);
        }
    }
    
    SJA_END_FUNCTION(&Reader, Result);
    
    return(Result);
}

fancy_font_format
asset_loader::ExpectTypeFancy(){
    fancy_font_format Result = {};
    
    SJA_BEGIN_FUNCTION(&Reader, "Fancy", ERROR_FANCY);
    Result.Color1 = ExpectTypeColor();
    
    file_token Token = Reader.PeekToken();
    if(Token.Type == FileTokenType_EndArguments){
    }else if(Token.Type == FileTokenType_Float){
        Result.Amplitude = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_FUNCTION(ERROR_FANCY));
        Result.Speed     = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_FUNCTION(ERROR_FANCY));
        Result.dT        = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_FUNCTION(ERROR_FANCY));
    }else if(Token.Type == FileTokenType_Identifier){
        Result.Color2 = ExpectTypeColor();
        
        f32 A = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_FUNCTION(ERROR_FANCY));
        f32 B = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_FUNCTION(ERROR_FANCY));
        f32 C = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_FUNCTION(ERROR_FANCY));
        Token = Reader.PeekToken();
        if(Token.Type == FileTokenType_EndArguments){
            Result.ColorSpeed   = A;
            Result.ColordT      = B;
            Result.ColorTOffset = C;
        }else{
            Result.Amplitude    = A;
            Result.Speed        = B;
            Result.dT           = C;
            Result.ColorSpeed   = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_FUNCTION(ERROR_FANCY));
            Result.ColordT      = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_FUNCTION(ERROR_FANCY));
            Result.ColorTOffset = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_FUNCTION(ERROR_FANCY));
        }
    }
    
    SJA_END_FUNCTION(&Reader, Result);
    
    Result.Speed        *= 0.5f*PI;
    Result.dT           *= 0.5f*PI;
    Result.ColorSpeed   *= 0.5f*PI;
    Result.ColordT      *= 0.5f*PI;
    Result.ColorTOffset *= 0.5f*PI;
    
    return(Result);
}

asset_tag
asset_loader::MaybeExpectTag(){
    asset_tag Result = {};
    
    file_token Token = Reader.PeekToken();
    if(Token.Type != FileTokenType_Identifier) return Result;
    if(CompareCStrings(Token.Identifier, "Tag")){
        SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_FUNCTION(Result));
        
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_BeginArguments, Result);
        
        for(u32 I=0; I<ArrayCount(Result.E); I++){
            Token = Reader.PeekToken();
            if(Token.Type != FileTokenType_String) break;
            const char *S = SJA_EXPECT_STRING(&Reader, SJA_ERROR_FUNCTION(Result));
            
            u8 ID = (u8)HashTableFind(&TagTable, S);
            if(!ID){ 
                LogWarning("'%s' is not registered as a tag and thus will be ignored!", S);
                //Assert(0);
                continue;
            }
            
            Result.E[I] = ID;
        }
        
        SJA_END_FUNCTION(&Reader, Result);
    }
    
    return(Result);
}

ta_name
asset_loader::ExpectTypeName(){
    ta_name Result = {};
    
    SJA_BEGIN_FUNCTION(&Reader, "Name", Result);
    
    file_token Token = Reader.PeekToken();
    if(Token.Type == FileTokenType_String){
        const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_FUNCTION(Result));
        Result.Name = Strings.GetPermanentString(Name);
    }
    
    Token = Reader.PeekToken();
    if(Token.Type == FileTokenType_Identifier){
        array<const char *> Aliases = ExpectTypeArrayCString();
        Result.Aliases = MakeArray<const char *>(&InProgress.Memory, Aliases.Count);
        for(u32 I=0; I<Aliases.Count; I++){
            char *Alias = ArenaPushLowerCString(&GlobalTransientMemory, Aliases[I]);
            ArrayAdd(&Result.Aliases, Strings.GetPermanentString(Alias));
        }
    }
    
    Token = Reader.PeekToken();
    if(Token.Type == FileTokenType_Identifier){
        array<const char *> Adjectives = ExpectTypeArrayCString();
        Result.Adjectives = MakeArray<const char *>(&InProgress.Memory, Adjectives.Count);
        for(u32 I=0; I<Adjectives.Count; I++){
            char *Adjective = ArenaPushLowerCString(&GlobalTransientMemory, Adjectives[I]);
            ArrayAdd(&Result.Adjectives, Strings.GetPermanentString(Adjective));
        }
    }
    
    SJA_END_FUNCTION(&Reader, Result);
    
    return Result;
}

u32
asset_loader::ExpectPositiveInteger_(){
    u32 Result = 0;
    s32 Integer = SJA_EXPECT_INT(&Reader, return Result);
    if(Integer < 0){
        LogWarning("Expected a positive integer, instead read '%d', which is negative", Integer);
        return(0);
    }
    
    return(Integer);
}

image *
asset_loader::LoadImage(const char *Path){
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
asset_loader::DoAttribute(const char *String, const char *Attribute){
    b8 Result = CompareCStrings(String, Attribute);
    if(Result) CurrentAttribute = Attribute;
    return(Result);
}

// TODO(Tyler): This should be made into an actual comment type such as /* */ or #if 0 #endif
asset_loading_status
asset_loader::ProcessIgnore(){
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        Reader.NextToken();
    }
    return AssetLoadingStatus_Okay;
}

asset_loading_status
asset_loader::LoadAssetFile(const char *Path){
    ARENA_FUNCTION_MARKER(&GlobalTransientMemory);
    
    CurrentCommand = 0;
    CurrentAttribute = 0;
    
    os_file *File = OSOpenFile(Path, OpenFile_Read);
    u64 NewFileWriteTime = OSGetLastFileWriteTime(File);
    OSCloseFile(File);
    
    if(LastFileWriteTime < NewFileWriteTime){
        LoadCounter++;
        ArenaClear(&InProgress.Memory);
        LoadingStatus = AssetLoadingStatus_Okay;
        
        Reader = MakeFileReader(Path);
        
        while(LoadingStatus != AssetLoadingStatus_Errors){
            file_token Token = Reader.NextToken();
            
            switch(Token.Type){
                case FileTokenType_BeginCommand: {
                    ChooseStatus(ProcessCommand());
                }break;
                case FileTokenType_EndFile: {
                    goto end_loop;
                }break;
                default: {
                    LogWarning("Token: '%s' was not expected!", TokenToString(Token));
                }break;
            }
        }
        end_loop:;
    }
    
    LastFileWriteTime = NewFileWriteTime;
    
    if(LoadingStatus == AssetLoadingStatus_Errors) return LoadingStatus;
    
    { //- Post processing
        ta_system *TA = TextAdventure;
        if(SpecialCommands & SpecialCommand_StartCarillonPages) MurkwellStartCarillonPages(TA, &InProgress);
        
        SpecialCommands = 0;
        
        // Checking items
        FOR_EACH(ItemID, &TA->Inventory){
            TA->CheckAndLogItemID(ItemID);
        }
    }
    
    Swap(*MainAssets, InProgress); 
    return LoadingStatus;
}

#define SJA_COMMMAND(Command)                 \
if(CompareCStrings(String, #Command)) { \
BeginCommand(#Command);            \
return Process##Command(); \
}       

asset_loading_status
asset_loader::ProcessCommand(){
    const char *String = SJA_EXPECT_IDENTIFIER(&Reader, SeekNextCommand(); return AssetLoadingStatus_Warnings);
    
    SJA_COMMMAND(Ignore);
    SJA_COMMMAND(SpecialCommands);
    SJA_COMMMAND(Variables);
    SJA_COMMMAND(Theme);
    SJA_COMMMAND(Font);
    SJA_COMMMAND(SoundEffect);
    SJA_COMMMAND(TARoom);
    SJA_COMMMAND(TAItem);
    SJA_COMMMAND(TAMap);
    
    char *Message = ArenaPushFormatCString(&InProgress.Memory, "(Line: %u) '%s' isn't a valid command!", Reader.Line, String);
    LogMessage(Message);
    DebugInfo.SubmitMessage(DebugMessage_Asset, Message);
    ProcessIgnore();
    return AssetLoadingStatus_Warnings;
}
#undef SJA_COMMMAND

//~ Variables

asset_loading_status 
asset_loader::ProcessSpecialCommands(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    ta_system *TA = TextAdventure;
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_ATTRIBUTE);
        if(DoAttribute(Attribute, "give_item")){
            const char *S = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
            ta_id Item = TAIDByName(TA, S);
            b8 FoundIt = false;
            for(u32 I=0; I<TA->Inventory.Count; I++){
                if(TA->Inventory[I] == Item) { FoundIt = true; break; }
            }
            if(!FoundIt) TA->InventoryAddItem(Item);
        }else if(DoAttribute(Attribute, "start_carillon_pages")){
            SpecialCommands |= SpecialCommand_StartCarillonPages;
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    return ChooseStatus(Result);
}

asset_loading_status
asset_loader::ProcessVariables(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    ta_system *TA = TextAdventure;
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_ATTRIBUTE);
        if(DoAttribute(Attribute, "var")){
            const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
            string_builder Builder = BeginResizeableStringBuilder(&GlobalTransientMemory, DEFAULT_BUFFER_SIZE);
            ExpectDescriptionStrings(&Builder);
            const char *Data = FinalizeStringBuilder(&InProgress.Memory, &Builder);
            asset_variable *Variable = Strings.HashTableGetPtr(&InProgress.VariableTable, Name);
            Variable->S = Data;
        }else if(DoAttribute(Attribute, "ta_id")){
            const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
            const char *Data = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
            asset_variable *Variable = Strings.HashTableGetPtr(&InProgress.VariableTable, Name);
            Variable->S = Strings.GetPermanentString(Data);
            Variable->TAID = TAIDByName(TA, Data);
        }else if(DoAttribute(Attribute, "name")){
            const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
            ta_name NameData = ExpectTypeName();
            asset_variable *Variable = Strings.HashTableGetPtr(&InProgress.VariableTable, Name);
            Variable->NameData = NameData;
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    return ChooseStatus(Result);
}

//~ Theme
asset_loading_status
asset_loader::ProcessTheme(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    ta_system *TA = TextAdventure;
    
    const char *Name = SJA_EXPECT_STRING(&Reader, return AssetLoadingStatus_Warnings);
    CurrentAsset = Name;
    console_theme *Theme = HashTableGetPtr(&TA->ThemeTable, TAIDByName(TA, Name));
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = SJA_EXPECT_IDENTIFIER(&Reader, return AssetLoadingStatus_Warnings);
        asset_loading_status Status;
        if(DoAttribute(Attribute, "basic_font")){
            const char *S = SJA_EXPECT_STRING(&Reader, return AssetLoadingStatus_Warnings);
            Theme->BasicFont = MakeAssetID(Strings.GetString(S));
        }else if(DoAttribute(Attribute, "title_font")){
            const char *S = SJA_EXPECT_STRING(&Reader, return AssetLoadingStatus_Warnings);
            Theme->TitleFont = MakeAssetID(Strings.GetString(S));
        }else if(DoAttribute(Attribute, "background_color")){ Theme->BackgroundColor = ExpectTypeColor(); 
        }else if(DoAttribute(Attribute, "cursor_color")){     Theme->CursorColor     = ExpectTypeColor(); 
        }else if(DoAttribute(Attribute, "selection_color")){  Theme->SelectionColor  = ExpectTypeColor(); 
        }else if(DoAttribute(Attribute, "basic")){            Theme->BasicFancy     = ExpectTypeFancy(); 
        }else if(DoAttribute(Attribute, "room_title")){       Theme->RoomTitleFancy = ExpectTypeFancy(); 
        }else if(DoAttribute(Attribute, "item")){             Theme->ItemFancy      = ExpectTypeFancy(); 
        }else if(DoAttribute(Attribute, "room")){             Theme->RoomFancy      = ExpectTypeFancy(); 
        }else if(DoAttribute(Attribute, "direction")){        Theme->DirectionFancy = ExpectTypeFancy(); 
        }else if(DoAttribute(Attribute, "misc")){             Theme->MiscFancy      = ExpectTypeFancy(); 
        }else if(DoAttribute(Attribute, "mood")){             Theme->MoodFancy      = ExpectTypeFancy(); 
        }else if(DoAttribute(Attribute, "response")){         Theme->ResponseFancies[0] = ExpectTypeFancy(); 
        }else if(DoAttribute(Attribute, "emphasis")){         Theme->ResponseFancies[1] = ExpectTypeFancy();
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    return ChooseStatus(Result);
}

//~ Sound effects

asset_loading_status
asset_loader::ProcessSoundEffect(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    const char *Name = SJA_EXPECT_STRING(&Reader, return AssetLoadingStatus_Warnings);
    CurrentAsset = Name;
    asset_sound_effect *Sound = Strings.HashTableGetPtr(&InProgress.SoundEffectTable, Name);
    TicketMutexBegin(&Mixer->SoundMutex);
    
    *Sound = {};
    Sound->VolumeMultiplier = 1.0f;
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = SJA_EXPECT_IDENTIFIER(&Reader, return AssetLoadingStatus_Warnings);
        
        if(DoAttribute(Attribute, "path")){
            const char *Path = SJA_EXPECT_STRING(&Reader, return AssetLoadingStatus_Warnings);
            sound_data Data = LoadWavFile(&InProgress.Memory, Path);
            if(!Data.Samples){
                FailCommand(&Sound->LoadingData, "'%s' isn't a valid path to a wav file!", Path);
            }
            Sound->Sound = Data;
        }else if(DoAttribute(Attribute, "volume")){
            Sound->VolumeMultiplier = SJA_EXPECT_FLOAT(&Reader, return AssetLoadingStatus_Warnings);
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    TicketMutexEnd(&Mixer->SoundMutex);
    return ChooseStatus(Result);
}

//~ Fonts
asset_loading_status
asset_loader::ProcessFont(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_COMMAND);
    CurrentAsset = Name;
    asset_font *Font = Strings.HashTableGetPtr(&InProgress.FontTable, Name);
    *Font = {};
    
    v2s CurrentOffset = V2S(0);
    s32 Height = 0;
    s32 Padding = 0;
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_ATTRIBUTE);
        
        if(DoAttribute(Attribute, "path")){ 
            const char *Path = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
            
            image *Image = LoadImage(Path);
            if(!Image){
                FailCommand(&Font->LoadingData, "'%s' isn't a valid path to an image!", Path);
                return AssetLoadingStatus_Warnings;
            }
            
            Font->Texture = Image->Texture;
            Font->Size = Image->Size;
        }else if(DoAttribute(Attribute, "height")){
            Height = SJA_EXPECT_UINT(&Reader, SJA_ERROR_ATTRIBUTE);
            
            Font->Height = (f32)Height;
        }else if(DoAttribute(Attribute, "padding")){
            Padding = SJA_EXPECT_UINT(&Reader, SJA_ERROR_ATTRIBUTE);
            
            CurrentOffset.Y += Padding;
        }else if(DoAttribute(Attribute, "descent")){
            Font->Descent = (f32)SJA_EXPECT_UINT(&Reader, SJA_ERROR_ATTRIBUTE);
        }else if(DoAttribute(Attribute, "char")){
            if(!Font->Texture){
                FailCommand(&Font->LoadingData, "The font image must be defined before any characters!");
                return AssetLoadingStatus_Warnings;
            }
            if(!Height){
                FailCommand(&Font->LoadingData, "The font height must be defined before any characters!");
                return AssetLoadingStatus_Warnings;
            }
            
            const char *S = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
            if(S[1] || !S[0]){
                LogWarning("'%s' is not a single character and will thus be ignored!", S);
                Result = AssetLoadingStatus_Warnings;
                SeekNextAttribute();
                continue;
            }
            char C = S[0];
            
            s32 Width = SJA_EXPECT_UINT(&Reader, SJA_ERROR_ATTRIBUTE);
            
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
                FailCommand(&Font->LoadingData, "The font image must be defined before any characters!");
                return AssetLoadingStatus_Warnings;
            }
            if(!Height){
                FailCommand(&Font->LoadingData, "The font height must be defined before any characters!");
                return AssetLoadingStatus_Warnings;
            }
            
            char C = HashTableFind(&ASCIITable, Attribute);
            
            if(C){
                s32 Width = SJA_EXPECT_UINT(&Reader, SJA_ERROR_ATTRIBUTE);
                
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
asset_loader::ExpectDescriptionStrings(string_builder *Builder){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    while(true){
        file_token Token = Reader.PeekToken();
        if(Token.Type != FileTokenType_String) break;
        const char *S = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
        
        u32 Length = CStringLength(S);
        for(u32 I=0; S[I]; I++){
            char C = S[I];
            
            if(C == '\\'){
                char Next = S[I+1];
                if(IsANumber(Next)){
                    Next -= '0';
                    BuilderAdd(Builder, '\002');
                    BuilderAdd(Builder, Next+1);
                    I++;
                    continue;
                }
            }
            BuilderAdd(Builder, C);
            
        }
    }
    
    return ChooseStatus(Result);
}

asset_loading_status
asset_loader::ProcessTADescription(dynamic_array<ta_data *> *Descriptions, ta_data_type Type){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    asset_tag Tag = MaybeExpectTag();
    
    string_builder Builder = BeginResizeableStringBuilder(&GlobalTransientMemory, DEFAULT_BUFFER_SIZE);
    BuilderAddVar(&Builder, Type);
    BuilderAddVar(&Builder, Tag);
    Assert(Builder.BufferSize == offsetof(ta_data, Data));
    ExpectDescriptionStrings(&Builder);
    ta_data *Description = (ta_data *)FinalizeStringBuilder(&InProgress.Memory, &Builder);
    ArrayAdd(Descriptions, Description);
    
    return ChooseStatus(Result);
}

asset_loading_status
asset_loader::ProcessTARoom(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    ta_system *TA = TextAdventure;
    
    const char *Identifier = SJA_EXPECT_STRING(&Reader, SJA_ERROR_COMMAND);
    CurrentAsset = Identifier;
    ta_id ID = TAIDByName(TA, Identifier);
    ta_room *Room = HashTableGetPtr(&TA->RoomTable, ID);
    Room->ID = ID;
    Room->NameData = ExpectTypeName();
    Room->Tag = MaybeExpectTag();
    
    dynamic_array<ta_data *> Descriptions = MakeDynamicArray<ta_data *>(8, &GlobalTransientMemory);
    
    u32 MaxItemCount = TA_ROOM_DEFAULT_ITEM_COUNT;
    v2s CurrentOffset = V2S(0);
    s32 Height = 0;
    s32 Padding = 0;
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_ATTRIBUTE);
        
        if(DoAttribute(Attribute, "description")){ 
            Result = ProcessTADescription(&Descriptions);
            if(Result == AssetLoadingStatus_Errors) return ChooseStatus(Result);
        }else if(DoAttribute(Attribute, "data_command")){
            Result = ProcessTADescription(&Descriptions, TADataType_Command);
            if(Result == AssetLoadingStatus_Errors) return ChooseStatus(Result);
        }else if(DoAttribute(Attribute, "data_room")){ 
            ta_data *Data = ArenaPushType(&InProgress.Memory, ta_data);
            Data->Type = TADataType_Room;
            Data->Tag = MaybeExpectTag();
            const char *S = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
            Data->TAID = TAIDByName(TA, S);
            ArrayAdd(&Descriptions, Data);
        }else if(DoAttribute(Attribute, "data_asset")){
            ta_data *Data = ArenaPushType(&InProgress.Memory, ta_data);
            Data->Type = TADataType_Asset;
            Data->Tag = MaybeExpectTag();
            const char *S = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
            Data->Asset = MakeAssetID(Strings.GetString(S));
            ArrayAdd(&Descriptions, Data);
        }else if(DoAttribute(Attribute, "area")){
            const char *S = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
            Room->Area = MakeTAID(Strings.GetString(S));
        }else if(DoAttribute(Attribute, "adjacents")){ 
            while(true){
                file_token Token = Reader.PeekToken();
                if(Token.Type != FileTokenType_Identifier) break;
                
                direction Direction = HashTableFind(&DirectionTable, (const char *)Token.Identifier);
                if(!Direction) break;
                SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_ATTRIBUTE);
                
                const char *NextRoomName = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
                Room->Adjacents[Direction] = TAIDByName(TA, NextRoomName);
                asset_tag Tag = MaybeExpectTag();
                if(!(Room->Flags & RoomFlag_Dirty)) Room->AdjacentTags[Direction] = Tag;
            }
        }else if(DoAttribute(Attribute, "item_count")){
            MaxItemCount = SJA_EXPECT_UINT(&Reader, SJA_ERROR_ATTRIBUTE);
        }else if(DoAttribute(Attribute, "items")){
            array<const char *> CStringItems = ExpectTypeArrayCString();
            u32 Count = Maximum(CStringItems.Count, MaxItemCount);
            Room->Items = MakeArray<ta_id>(&InProgress.Memory, Count);
            for(u32 I=0; I<CStringItems.Count; I++){
                ta_id S = TAIDByName(TA, CStringItems[I]);
                ta_item *Item = TA->FindItem(S);
                if(!Item || !Item->IsDirty) ArrayAdd(&Room->Items, S);
            }
            
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    Room->Datas = MakeArray<ta_data *>(&InProgress.Memory, Descriptions.Count);
    for(u32 I=0; I<Descriptions.Count; I++){
        ArrayAdd(&Room->Datas, Descriptions[I]);
    }
    
    return ChooseStatus(Result);
}

//~
asset_loading_status
asset_loader::ProcessTAItem(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    ta_system *TA = TextAdventure;
    
    const char *Identifier = SJA_EXPECT_STRING(&Reader, SJA_ERROR_COMMAND);
    CurrentAsset = Identifier;
    ta_id ID = TAIDByName(TA, Identifier);
    ta_item *Item = HashTableGetPtr(&TA->ItemTable, ID);
    Item->ID = ID;
    Item->NameData = ExpectTypeName();
    Item->Tag = MaybeExpectTag();
    
    // Attributes
    dynamic_array<ta_data *> Descriptions = MakeDynamicArray<ta_data *>(8, &GlobalTransientMemory);
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_ATTRIBUTE);
        if(DoAttribute(Attribute, "description")){ 
            Result = ProcessTADescription(&Descriptions);
            if(Result == AssetLoadingStatus_Errors) return ChooseStatus(Result);
        }else if(DoAttribute(Attribute, "data_command")){
            Result = ProcessTADescription(&Descriptions, TADataType_Command);
            if(Result == AssetLoadingStatus_Errors) return ChooseStatus(Result);
        }else if(DoAttribute(Attribute, "data_item")){ 
            ta_data *Data = ArenaPushType(&InProgress.Memory, ta_data);
            Data->Type = TADataType_Item;
            Data->Tag = MaybeExpectTag();
            const char *S = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
            Data->TAID = TAIDByName(TA, S);
            ArrayAdd(&Descriptions, Data);
        }else if(DoAttribute(Attribute, "data_asset")){
            ta_data *Data = ArenaPushType(&InProgress.Memory, ta_data);
            Data->Type = TADataType_Asset;
            Data->Tag = MaybeExpectTag();
            const char *S = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
            Data->Asset = MakeAssetID(Strings.GetString(S));
            ArrayAdd(&Descriptions, Data);
        }else if(DoAttribute(Attribute, "cost")){
            u32 Cost = SJA_EXPECT_UINT(&Reader, SJA_ERROR_ATTRIBUTE);
            if(!Item->IsDirty) Item->Cost = Cost;
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    Item->Datas = MakeArray<ta_data *>(&InProgress.Memory, Descriptions.Count);
    for(u32 I=0; I<Descriptions.Count; I++){
        ArrayAdd(&Item->Datas, Descriptions[I]);
    }
    
    MurkwellProcessItem(TA, ID, Item);
    
    return ChooseStatus(Result);
}

//~
asset_loading_status
asset_loader::ProcessTAMap(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    ta_system *TA = TextAdventure;
    ta_map *Map = &TA->Map;
    
    // Attributes
    dynamic_array<ta_area> Areas = MakeDynamicArray<ta_area>(8, &GlobalTransientMemory);
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_ATTRIBUTE);
        if(DoAttribute(Attribute, "path")){ 
            const char *Path = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
            image *Image = LoadImage(Path);
            if(!Image){
                FailCommand(0, "'%s' isn't a valid path to an image!", Path);
                return AssetLoadingStatus_Warnings;
            }
            Map->Texture = Image->Texture;
            Map->Size = V2(Image->Size);
        }else if(DoAttribute(Attribute, "area")){
            ta_area *Area = ArrayAlloc(&Areas);
            const char *S = SJA_EXPECT_STRING(&Reader, SJA_ERROR_ATTRIBUTE);
            Area->Name = TAIDByName(TA, S);
            Area->Offset = ExpectTypeV2();
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    Map->Areas = MakeArray<ta_area>(&InProgress.Memory, Areas.Count);
    for(u32 I=0; I<Areas.Count; I++){
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
asset_loader::LoadProcessedAssets(void *Data, u32 DataSize){
    ta_system *TA = TextAdventure;
    
    stream Stream = MakeReadStream(Data, DataSize);
    sjap_header Header; StreamReadVar(&Stream, Header);
    Assert((Header.SJAP[0] == 'S') &&
           (Header.SJAP[1] == 'J') &&
           (Header.SJAP[2] == 'A') &&
           (Header.SJAP[3] == 'P'));
    
    InitializeProcessedAssets(this, Data, DataSize);
}


#endif // SNAIL_JUMPY_USE_PROCESSED_ASSETS