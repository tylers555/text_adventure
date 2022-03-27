global f32 Counter;
global u32 FrameCounter;

internal u32
CStringLength(const char *String){
    u32 Result = 0;
    for(char C = *String; C; C = *(++String)){
        Result++;
    }
    return(Result);
}

internal void
CopyCString(char *To, const char *From, u32 MaxSize){
    u32 I = 0;
    while(From[I] && (I < MaxSize)){
        To[I] = From[I];
        I++;
    }
    To[I] = '\0';
}

internal direction
InverseDirection(direction Direction){
    local_constant direction Table[Direction_TOTAL] = {
        Direction_None,
        Direction_South,
        Direction_SouthWest,
        Direction_West,
        Direction_NorthWest,
        Direction_North,
        Direction_NorthEast,
        Direction_East,
        Direction_SouthEast,
    };
    direction Result = Table[Direction];
    return(Result);
}

internal inline u32
GetRandomNumber(u32 Seed){
    u32 RandomNumber = RANDOM_NUMBER_TABLE[(u32)(Counter*4132.0f + Seed) % ArrayCount(RANDOM_NUMBER_TABLE)];
    return(RandomNumber);
}

internal inline f32
GetRandomFloat(u32 Seed, u32 Spread=5, f32 Power=0.2f){
    s32 Random = ((s32)GetRandomNumber(Seed)) % Spread;
    f32 Result = Power * (f32)Random;
    return(Result);
}

//~
internal inline b8
IsWhiteSpace(char C){
    b8 Result = ((C == ' ') ||
                 (C == '\t') ||
                 (C == '\n') ||
                 (C == '\r'));
    return(Result);
}

internal inline b8
IsALetter(char C){
    b8 Result = ((('a' <= C) && (C <= 'z')) ||
                 (('A' <= C) && (C <= 'Z')));
    return(Result);
}

internal inline b8
IsANumber(char C){
    b8 Result = (('0' <= C) && (C <= '9'));
    return(Result);
}

internal inline b8 
StopSeeking(char C){
    b8 Result = (!IsALetter(C) &&
                 !IsANumber(C));
    return Result;
}

internal inline u32 
SeekForward(const char *Buffer, u32 BufferLength, u32 Start){
    u32 Result = Start;
    b8 HitAlphabetic = false;
    for(u32 I=Start; I<=BufferLength; I++){
        char C = Buffer[I];
        Result = I;
        if(StopSeeking(C)){
            if(HitAlphabetic) break;
        }else HitAlphabetic = true;
    }
    
    return Result;
}

internal inline u32 
SeekBackward(const char *Buffer, u32 Start){
    if(Start == 0) return 0;
    u32 Result = Start;
    b8 HitAlphabetic = false;
    for(s32 I=Start-1; I>=0; I--){
        char C = Buffer[I];
        if(StopSeeking(C)){
            if(HitAlphabetic) break;
        }else HitAlphabetic = true;
        Result = I;
    }
    
    return Result;
}

internal inline void 
CStringMakeLower(char *S){
    for(u32 I=0; S[I]; I++){
        if((('A' <= S[I]) && (S[I] <= 'Z'))) S[I] -= 'A'-'a';
    }
}

internal inline char 
CharToLower(char C){
    if((('A' <= C) && (C <= 'Z'))) C -= 'A'-'a';
    return C;
}
