
//~ Initialization
void
asset_system::InitializeLoader(memory_arena *Arena){
    DirectionTable = PushHashTable<const char *, direction>(Arena, Direction_TOTAL+8);
    InsertIntoHashTable(&DirectionTable, "north",     Direction_North);
    InsertIntoHashTable(&DirectionTable, "northeast", Direction_NorthEast);
    InsertIntoHashTable(&DirectionTable, "east",      Direction_East);
    InsertIntoHashTable(&DirectionTable, "southeast", Direction_SouthEast);
    InsertIntoHashTable(&DirectionTable, "south",     Direction_South);
    InsertIntoHashTable(&DirectionTable, "southwest", Direction_SouthWest);
    InsertIntoHashTable(&DirectionTable, "west",      Direction_West);
    InsertIntoHashTable(&DirectionTable, "northwest", Direction_NorthWest);
    InsertIntoHashTable(&DirectionTable, "up",        Direction_Up);
    InsertIntoHashTable(&DirectionTable, "down",      Direction_Down);
    InsertIntoHashTable(&DirectionTable, "left",      Direction_Left);
    InsertIntoHashTable(&DirectionTable, "right",     Direction_Right);
    
    ASCIITable = PushHashTable<const char *, char>(Arena, 128);
    InsertIntoHashTable(&ASCIITable, "SPACE",                ' ');
    InsertIntoHashTable(&ASCIITable, "EXCLAMATION",          '!');
    InsertIntoHashTable(&ASCIITable, "QUOTATION",            '"');
    InsertIntoHashTable(&ASCIITable, "POUND",                '#');
    InsertIntoHashTable(&ASCIITable, "APOSTROPHE",           '\'');
    InsertIntoHashTable(&ASCIITable, "PARENTHESIS_LEFT",     '(');
    InsertIntoHashTable(&ASCIITable, "PARENTHESIS_RIGHT",    ')');
    InsertIntoHashTable(&ASCIITable, "ASTERISK",             '*');
    InsertIntoHashTable(&ASCIITable, "PLUS",                 '+');
    InsertIntoHashTable(&ASCIITable, "COMMA",                ',');
    InsertIntoHashTable(&ASCIITable, "DASH",                 '-');
    InsertIntoHashTable(&ASCIITable, "PERIOD",               '.');
    InsertIntoHashTable(&ASCIITable, "SLASH",                '/');
    InsertIntoHashTable(&ASCIITable, "ZERO",                 '0');
    InsertIntoHashTable(&ASCIITable, "ONE",                  '1');
    InsertIntoHashTable(&ASCIITable, "TWO",                  '2');
    InsertIntoHashTable(&ASCIITable, "THREE",                '3');
    InsertIntoHashTable(&ASCIITable, "FOUR",                 '4');
    InsertIntoHashTable(&ASCIITable, "FIVE",                 '5');
    InsertIntoHashTable(&ASCIITable, "SIX",                  '6');
    InsertIntoHashTable(&ASCIITable, "SEVEN",                '7');
    InsertIntoHashTable(&ASCIITable, "EIGHT",                '8');
    InsertIntoHashTable(&ASCIITable, "NINE",                 '9');
    InsertIntoHashTable(&ASCIITable, "COLON",                ':');
    InsertIntoHashTable(&ASCIITable, "SEMICOLON",            ';');
    InsertIntoHashTable(&ASCIITable, "ANGLE_BRACKET_LEFT",   '<');
    InsertIntoHashTable(&ASCIITable, "EQUAL",                '=');
    InsertIntoHashTable(&ASCIITable, "ANGLE_BRACKET_RIGHT",  '>');
    InsertIntoHashTable(&ASCIITable, "QUESTION",             '?');
    InsertIntoHashTable(&ASCIITable, "A",                    'A');
    InsertIntoHashTable(&ASCIITable, "B",                    'B');
    InsertIntoHashTable(&ASCIITable, "C",                    'C');
    InsertIntoHashTable(&ASCIITable, "D",                    'D');
    InsertIntoHashTable(&ASCIITable, "E",                    'E');
    InsertIntoHashTable(&ASCIITable, "F",                    'F');
    InsertIntoHashTable(&ASCIITable, "G",                    'G');
    InsertIntoHashTable(&ASCIITable, "H",                    'H');
    InsertIntoHashTable(&ASCIITable, "I",                    'I');
    InsertIntoHashTable(&ASCIITable, "J",                    'J');
    InsertIntoHashTable(&ASCIITable, "K",                    'K');
    InsertIntoHashTable(&ASCIITable, "L",                    'L');
    InsertIntoHashTable(&ASCIITable, "M",                    'M');
    InsertIntoHashTable(&ASCIITable, "N",                    'N');
    InsertIntoHashTable(&ASCIITable, "O",                    'O');
    InsertIntoHashTable(&ASCIITable, "P",                    'P');
    InsertIntoHashTable(&ASCIITable, "Q",                    'Q');
    InsertIntoHashTable(&ASCIITable, "R",                    'R');
    InsertIntoHashTable(&ASCIITable, "S",                    'S');
    InsertIntoHashTable(&ASCIITable, "T",                    'T');
    InsertIntoHashTable(&ASCIITable, "U",                    'U');
    InsertIntoHashTable(&ASCIITable, "V",                    'V');
    InsertIntoHashTable(&ASCIITable, "W",                    'W');
    InsertIntoHashTable(&ASCIITable, "X",                    'X');
    InsertIntoHashTable(&ASCIITable, "Y",                    'Y');
    InsertIntoHashTable(&ASCIITable, "Z",                    'Z');
    InsertIntoHashTable(&ASCIITable, "SQUARE_BRACKET_LEFT",  '[');
    InsertIntoHashTable(&ASCIITable, "BACKSLASH",            '\\');
    InsertIntoHashTable(&ASCIITable, "SQUARE_BRACKET_RIGHT", ']');
    InsertIntoHashTable(&ASCIITable, "CARET",                '^');
    InsertIntoHashTable(&ASCIITable, "BACK_TICK",            '`');
    InsertIntoHashTable(&ASCIITable, "UNDERSCORE",           '_');
    InsertIntoHashTable(&ASCIITable, "CURLY_BRACKET_LEFT",   '{');
    InsertIntoHashTable(&ASCIITable, "PIPE",                  '|');
    InsertIntoHashTable(&ASCIITable, "CURLY_BRACKET_RIGHT",  '}');
    InsertIntoHashTable(&ASCIITable, "TILDE",                '~');
    InsertIntoHashTable(&ASCIITable, "PERCENT",              '%');
    InsertIntoHashTable(&ASCIITable, "DOLLAR_SIGN",          '$');
    InsertIntoHashTable(&ASCIITable, "AMPERSAND",            '&');
    InsertIntoHashTable(&ASCIITable, "AT_SIGN",              '@');
}

//~ Base

#define ExpectPositiveInteger() \
ExpectPositiveInteger_();   \
HandleError();

#define EnsurePositive(Var) \
if(Var < 0){            \
LogError("'%d' must be positive!", Var); \
return(false);      \
}

#define Expect(Name) \
ExpectToken(FileTokenType_##Name).Name; \
HandleError();

#define HandleError() \
if(Reader.LastError == FileReaderError_InvalidToken) return(Result) \

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

file_token
asset_system::ExpectToken(file_token_type Type){
    Reader.LastError = FileReaderError_None;
    file_token Token = Reader.NextToken();
    if(Type == FileTokenType_Float){
        Token = MaybeTokenIntegerToFloat(Token);
    }
    
    if(Token.Type == Type){
        return(Token);
    }else {
        LogError("Expected %s, instead read: %s", TokenTypeName(Type), TokenToString(Token));
    }
    
    Reader.LastError = FileReaderError_InvalidToken;
    return(Token);
}

u32
asset_system::ExpectPositiveInteger_(){
    u32 Result = 0;
    s32 Integer = Expect(Integer);
    if(Integer < 0){
        LogError("Expected a positive integer, instead read '%d', which is negative", Integer);
        return(0);
    }
    
    return(Integer);
}

array<s32>
asset_system::ExpectTypeArrayS32(){
    array<s32> Result = MakeArray<s32>(&TransientStorageArena, SJA_MAX_ARRAY_ITEM_COUNT);
    
    const char *Identifier = Expect(Identifier);
    if(CompareStrings(Identifier, "Array")){
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        file_token Token = Reader.PeekToken();
        while(Token.Type != FileTokenType_EndArguments){
            s32 Integer = Expect(Integer);
            ArrayAdd(&Result, Integer);
            
            Token = Reader.PeekToken();
        }
        
        ExpectToken(FileTokenType_EndArguments);
        HandleError();
        
    }else{
        Reader.LastError = FileReaderError_InvalidToken;
        return(Result);
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
        ArenaClear(&Memory);
        memory_arena_marker Marker = ArenaBeginMarker(&TransientStorageArena);
        
        os_file *File = OpenFile(Path, OpenFile_Read);
        u64 NewFileWriteTime = GetLastFileWriteTime(File);
        CloseFile(File);
        
        if(LastFileWriteTime < NewFileWriteTime){
            HitError = false;
            
            Reader = MakeFileReader(Path);
            
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
    
    const char *String = Expect(Identifier);
    
    IfCommand(Ignore);
    IfCommand(Font);
    IfCommand(SoundEffect);
    
    LogMessage("(Line: %u) '%s' isn't a valid command!", Reader.Line, String);
    return(false);
}
#undef IfCommand

//~ Sound effects

b8
asset_system::ProcessSoundEffect(){
    b8 Result = false;
    
    const char *Name = Expect(Identifier);
    asset_sound_effect *Sound = Strings.GetInHashTablePtr(&SoundEffects, Name);
    if(Sound->Sound.Samples){
        ProcessIgnore();
        LogError("Cannot change a sound after game has started");
        return true;
    }
    
    *Sound = {};
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *String = Expect(Identifier);
        
        if(DoAttribute(String, "path")){
            const char *Path = Expect(String);
            sound_data Data = LoadWavFile(&OSSoundBuffer, &Memory, Path);
            if(!Data.Samples){
                LogError("'%s' isn't a valid path to a wav file!", Path);
            }
            Sound->Sound = Data;
            
        }else{ LogInvalidAttribute(String); return false; }
    }
    
    return true;
}

//~ Fonts
b8
asset_system::ProcessFont(){
    b8 Result = false;
    
    const char *Name = Expect(Identifier);
    asset_font *Font = Strings.GetInHashTablePtr(&Fonts, Name);
    *Font = {};
    
    v2s CurrentOffset = V2S(0);
    s32 Height = 0;
    s32 Padding = 0;
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *String = Expect(Identifier);
        
        if(DoAttribute(String, "path")){ 
            const char *Path = Expect(String);
            
            image *Image = LoadImageFromPath(Path);
            if(!Image){
                LogError("'%s' isn't a valid path to an image!", Path);
                return(false);
            }
            
            Font->Texture = Image->Texture;
            Font->Size = Image->Size;
        }else if(DoAttribute(String, "height")){
            Height = ExpectPositiveInteger();
            
            Font->Height = (f32)Height;
        }else if(DoAttribute(String, "padding")){
            Padding = ExpectPositiveInteger();
            
            CurrentOffset.Y += Padding;
        }else if(DoAttribute(String, "descent")){
            Font->Descent = (f32)ExpectPositiveInteger();
        }else{
            if(!Font->Texture){
                LogError("The font image must be defined before any characters!");
                return false;
            }
            if(!Height){
                LogError("The font height must be defined before any characters!");
                return false;
            }
            
            char C = FindInHashTable(&ASCIITable, String);
            
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
            }else{ LogInvalidAttribute(String); return false; }
        }
    }
    
    return true;
}
