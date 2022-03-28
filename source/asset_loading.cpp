
//~ Initialization
void
asset_system::InitializeLoader(memory_arena *Arena){
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
    
    TagTable = PushHashTable<const char *, asset_tag_id>(Arena, AssetTag_TOTAL);
    InsertIntoHashTable(&TagTable, "play",       AssetTag_Play);
    InsertIntoHashTable(&TagTable, "examine",    AssetTag_Examine);
    InsertIntoHashTable(&TagTable, "eat",        AssetTag_Eat);
    InsertIntoHashTable(&TagTable, "activate",   AssetTag_Activate);
    InsertIntoHashTable(&TagTable, "take",       AssetTag_Take);
    InsertIntoHashTable(&TagTable, "organ",      AssetTag_Organ);
    InsertIntoHashTable(&TagTable, "bell-tower", AssetTag_BellTower);
    InsertIntoHashTable(&TagTable, "broken",     AssetTag_Broken);
    InsertIntoHashTable(&TagTable, "repaired",   AssetTag_Repaired);
    InsertIntoHashTable(&TagTable, "locked",     AssetTag_Locked);
    InsertIntoHashTable(&TagTable, "open-dawn",  AssetTag_OpenDawn);
    InsertIntoHashTable(&TagTable, "open-noon",  AssetTag_OpenNoon);
    InsertIntoHashTable(&TagTable, "open-dusk",  AssetTag_OpenDusk);
    InsertIntoHashTable(&TagTable, "open-night", AssetTag_OpenNight);
    InsertIntoHashTable(&TagTable, "items",      AssetTag_Items);
    InsertIntoHashTable(&TagTable, "adjacents",  AssetTag_Adjacents);
    InsertIntoHashTable(&TagTable, "static",     AssetTag_Static);
    InsertIntoHashTable(&TagTable, "bread",      AssetTag_Bread);
    InsertIntoHashTable(&TagTable, "key",        AssetTag_Key);
    InsertIntoHashTable(&TagTable, "map",        AssetTag_Map);
    InsertIntoHashTable(&TagTable, "light",      AssetTag_Light);
}

//~ Base

#define ExpectPositiveInteger() \
ExpectPositiveInteger_();   \
HandleError();HandleError();

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

v2
asset_system::ExpectTypeV2(){
    v2 Result = V2(0);
    
    const char *Identifier = Expect(Identifier);
    if(CompareStrings(Identifier, "V2")){
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        Result.X = Expect(Float);
        file_token Token = Reader.PeekToken();
        if(Token.Type != FileTokenType_EndArguments){
            Result.Y = Expect(Float);
        }else{
            Result.Y = Result.X;
        }
        
        ExpectToken(FileTokenType_EndArguments);
        HandleError();
        
    }else{
        Reader.LastError = FileReaderError_InvalidToken;
        return(Result);
    }
    
    return(Result);
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

array<const char *>
asset_system::ExpectTypeArrayCString(){
    array<const char *> Result = MakeArray<const char *>(&TransientStorageArena, SJA_MAX_ARRAY_ITEM_COUNT);
    
    const char *Identifier = Expect(Identifier);
    if(CompareStrings(Identifier, "Array")){
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        file_token Token = Reader.PeekToken();
        while(Token.Type != FileTokenType_EndArguments){
            const char *String = Expect(String);
            ArrayAdd(&Result, String);
            
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

color
asset_system::ExpectTypeColor(){
    color Result = {};
    
    file_token Token = Reader.PeekToken();
    if(Token.Type != FileTokenType_Identifier) return Result;
    if(CompareStrings(Token.Identifier, "Color")){
        Expect(Identifier);
        
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        Token = Reader.PeekToken();
        if(Token.Type == FileTokenType_Float){
            for(u32 I=0; I<4; I++){
                Result.E[I] = Expect(Float);
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
                    Result.E[I] = Expect(Float);
                }
            }else if(Token.Type == FileTokenType_EndArguments){
                Result = MakeColor(First.Integer);
            }else{
                LogError("Expected ) or a number, and %s is neither!", 
                         TokenToString(Token));
                Reader.LastError = FileReaderError_InvalidToken;
                return Result;
            }
        }
        
        ExpectToken(FileTokenType_EndArguments);
        HandleError();
    }else{
        Reader.LastError = FileReaderError_InvalidToken;
        return(Result);
    }
    
    return(Result);
}

fancy_font_format
asset_system::ExpectTypeFancy(){
    fancy_font_format Result = {};
    
    file_token Token = Reader.PeekToken();
    if(Token.Type != FileTokenType_Identifier) return Result;
    if(CompareStrings(Token.Identifier, "Fancy")){
        Expect(Identifier);
        
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        Result.Color1 = ExpectTypeColor();
        HandleError();
        
        Token = Reader.PeekToken();
        if(Token.Type == FileTokenType_EndArguments){
        }else if(Token.Type == FileTokenType_Float){
            Result.Amplitude = Expect(Float);
            Result.Speed     = Expect(Float);
            Result.dT        = Expect(Float);
        }else if(Token.Type == FileTokenType_Identifier){
            Result.Color2 = ExpectTypeColor();
            HandleError();
            
            f32 A = Expect(Float);
            f32 B = Expect(Float);
            f32 C = Expect(Float);
            Token = Reader.PeekToken();
            if(Token.Type == FileTokenType_EndArguments){
                Result.ColorSpeed   = A;
                Result.ColordT      = B;
                Result.ColorTOffset = C;
            }else{
                Result.Amplitude    = A;
                Result.Speed        = B;
                Result.dT           = C;
                Result.ColorSpeed   = Expect(Float);
                Result.ColordT      = Expect(Float);
                Result.ColorTOffset = Expect(Float);
            }
        }
        
        ExpectToken(FileTokenType_EndArguments);
        HandleError();
        
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
        Expect(Identifier);
        
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        for(u32 I=0; I<ArrayCount(Result.E); I++){
            Token = Reader.PeekToken();
            if(Token.Type != FileTokenType_String) break;
            const char *S = Expect(String);
            
            enum8(asset_tag_id) ID = (u8)FindInHashTable(&TagTable, S);
            if(!ID){ 
                LogError("WARNING: '%s' is not registered as a tag and it will thus be ignored.!", S);
                //Assert(0);
                continue;
            }
            
            Result.E[I] = ID;
        }
        
        ExpectToken(FileTokenType_EndArguments);
        HandleError();
        
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
        const char *Attribute = Expect(Identifier);
        if(DoAttribute(Attribute, "start_game_mode")){
            const char *S = Expect(String);
            if(GameMode == GameMode_None){
                if(CompareStrings(S, "main game")) GameMode = GameMode_MainGame;
                else if(CompareStrings(S, "menu")) GameMode = GameMode_Menu;
            }
            
        }else if(DoAttribute(Attribute, "start_room")){
            const char *S = Expect(String);
            TA->StartRoom = Strings.GetString(S);
        }else if(DoAttribute(Attribute, "theme")){
            const char *S = Expect(String);
            console_theme *Theme = Strings.FindInHashTablePtr(&TA->ThemeTable, S);
            if(Theme){
                TA->Theme = *Theme;
            }
        }else if(DoAttribute(Attribute, "give_item")){
            const char *S = Expect(String);
            string Item = Strings.GetString(S);
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
    
    const char *Name = Expect(String);
    console_theme *Theme = Strings.GetInHashTablePtr(&TA->ThemeTable, Name);
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = Expect(Identifier);
        if(DoAttribute(Attribute, "basic_font")){
            const char *S = Expect(String);
            Theme->BasicFont = Strings.GetString(S);
        }else if(DoAttribute(Attribute, "title_font")){
            const char *S = Expect(String);
            Theme->TitleFont = Strings.GetString(S);
        }else if(DoAttribute(Attribute, "background_color")){ Theme->BackgroundColor = ExpectTypeColor(); HandleError();
        }else if(DoAttribute(Attribute, "cursor_color")){ Theme->CursorColor = ExpectTypeColor(); HandleError();
        }else if(DoAttribute(Attribute, "selection_color")){ Theme->SelectionColor = ExpectTypeColor(); HandleError();
        }else if(DoAttribute(Attribute, "basic")){      Theme->BasicFancy = ExpectTypeFancy(); HandleError();
        }else if(DoAttribute(Attribute, "room_title")){ Theme->RoomTitleFancy = ExpectTypeFancy(); HandleError();
        }else if(DoAttribute(Attribute, "item")){       Theme->ItemFancy = ExpectTypeFancy(); HandleError();
        }else if(DoAttribute(Attribute, "room")){       Theme->RoomFancy = ExpectTypeFancy(); HandleError();
        }else if(DoAttribute(Attribute, "direction")){  Theme->DirectionFancy = ExpectTypeFancy(); HandleError();
        }else if(DoAttribute(Attribute, "misc")){  Theme->MiscFancy = ExpectTypeFancy(); HandleError();
        }else if(DoAttribute(Attribute, "mood")){  Theme->MoodFancy = ExpectTypeFancy(); HandleError();
        }else if(DoAttribute(Attribute, "response")){   Theme->ResponseFancies[0] = ExpectTypeFancy(); HandleError();
        }else if(DoAttribute(Attribute, "emphasis")){   Theme->ResponseFancies[1] = ExpectTypeFancy(); HandleError();
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
    
    const char *Name = Expect(String);
    asset_sound_effect *Sound = Strings.GetInHashTablePtr(&SoundEffects, Name);
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
        const char *Attribute = Expect(Identifier);
        
        if(DoAttribute(Attribute, "path")){
            const char *Path = Expect(String);
            sound_data Data = LoadWavFile(&OSSoundBuffer, &Memory, Path);
            if(!Data.Samples){
                LogError("'%s' isn't a valid path to a wav file!", Path);
            }
            Sound->Sound = Data;
            
        }else if(DoAttribute(Attribute, "volume")){
            Sound->VolumeMultiplier = Expect(Float);
        }else{ LogInvalidAttribute(Attribute); return false; }
    }
    
    return true;
}

//~ Fonts
b8
asset_system::ProcessFont(){
    b8 Result = false;
    
    const char *Name = Expect(String);
    asset_font *Font = Strings.GetInHashTablePtr(&Fonts, Name);
    *Font = {};
    
    v2s CurrentOffset = V2S(0);
    s32 Height = 0;
    s32 Padding = 0;
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = Expect(Identifier);
        
        if(DoAttribute(Attribute, "path")){ 
            const char *Path = Expect(String);
            
            image *Image = LoadImageFromPath(Path);
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
            const char *S = Expect(String);
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
            
            char C = FindInHashTable(&ASCIITable, Attribute);
            
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
    HandleError();
    
    string_builder Builder = BeginStringBuilder(&TransientStorageArena, DEFAULT_BUFFER_SIZE);
    StringBuilderAddVar(&Builder, Tag);
    while(true){
        file_token Token = Reader.PeekToken();
        if(Token.Type != FileTokenType_String) break;
        const char *S = Expect(String);
        
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
    
    ta_string *Description = (ta_string *)StringBuilderFinalize(&Memory, &Builder);
    ArrayAdd(Descriptions, Description);
    
    return true;
}

b8
asset_system::ProcessTARoom(){
    b8 Result = false;
    
    ta_system *TA = &TextAdventure;
    
    const char *Name = Expect(String);
    ta_room *Room = Strings.GetInHashTablePtr(&TA->RoomTable, Name);
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
        const char *Attribute = Expect(Identifier);
        
        if(DoAttribute(Attribute, "description")){ 
            if(!ProcessTADescription(&Descriptions)) return false;
        }else if(DoAttribute(Attribute, "area")){
            const char *S = Expect(String);
            Room->Area = Strings.GetString(S);
        }else if(DoAttribute(Attribute, "adjacents")){ 
            while(true){
                file_token Token = Reader.PeekToken();
                if(Token.Type != FileTokenType_Identifier) break;
                
                direction Direction = FindInHashTable(&TA->DirectionTable, (const char *)Token.Identifier);
                if(!Direction) break;
                Expect(Identifier);
                
                const char *NextRoomName = Expect(String);
                Room->Adjacents[Direction] = Strings.GetString(NextRoomName);
                asset_tag Tag = MaybeExpectTag();
                if(!Room->Dirty) Room->AdjacentTags[Direction] = Tag;
            }
        }else if(DoAttribute(Attribute, "item_count")){
            MaxItemCount = ExpectPositiveInteger();
        }else if(DoAttribute(Attribute, "items")){
            array<const char *> CStringItems = ExpectTypeArrayCString();
            HandleError();
            u32 Count = Maximum(CStringItems.Count, MaxItemCount);
            Room->Items = MakeArray<string>(&Memory, Count);
            for(u32 I=0; I<CStringItems.Count; I++){
                string S = Strings.GetString(CStringItems[I]);
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
    const char *Name = Expect(String);
    ta_item *Item = Strings.GetInHashTablePtr(&TA->ItemTable, Name);
    
    Item->Tag = MaybeExpectTag();
    
    // Aliases
    array<const char *> Array = ExpectTypeArrayCString();
    HandleError();
    Item->Aliases = MakeArray<const char *>(&Memory, Array.Count);
    for(u32 I=0; I<Array.Count; I++){
        ArrayAdd(&Item->Aliases, Strings.GetPermanentString(Array[I]));
    }
    
    // Attributes
    dynamic_array<ta_string *> Descriptions = MakeDynamicArray<ta_string *>(8, &TransientStorageArena);
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = Expect(Identifier);
        if(DoAttribute(Attribute, "description")){ 
            if(!ProcessTADescription(&Descriptions)) return false;
        }else if(DoAttribute(Attribute, "adjectives")){
            array<const char *> Adjectives = ExpectTypeArrayCString();
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
        const char *Attribute = Expect(Identifier);
        if(DoAttribute(Attribute, "path")){ 
            const char *Path = Expect(String);
            image *Image = LoadImageFromPath(Path);
            Map->Texture = Image->Texture;
            Map->Size = V2(Image->Size);
        }else if(DoAttribute(Attribute, "area")){
            ta_area *Area = ArrayAlloc(&Areas);
            const char *S = Expect(String);
            Area->Name = Strings.GetString(S);
            Area->Offset = ExpectTypeV2();
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