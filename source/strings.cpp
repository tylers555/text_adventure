
//~ Strings
struct string {
    u64 ID;
};

global_constant string String0 = {0};

internal inline constexpr b8
operator==(string A, string B){
    b8 Result = (A.ID == B.ID);
    return(Result);
}

internal inline string
MakeString(u64 ID){
    string Result = {ID};
    return(Result);
}

internal constexpr u64
HashKey(string Value) {
    u64 Result = Value.ID;
    return(Result);
}

internal constexpr b32
CompareKeys(string A, string B){
    b32 Result = (A == B);
    return(Result);
}

union string_buffer_node {
    string_buffer_node *Next;
    char Buffer[DEFAULT_BUFFER_SIZE];
};

// String manager
struct string_manager {
    memory_arena StringMemory;
    hash_table<const char *, const char *> Table;
    
    memory_arena BufferMemory;
    string_buffer_node *NextBuffer;
    
    void Initialize(memory_arena *Arena);
    
    string GetString(const char *String);
    const char *GetString(string String);
    const char *GetPermanentString(const char *String);
    char *MakeBuffer();
    void  RemoveBuffer(char *Buffer);
    template<typename T> T *GetInHashTablePtr(hash_table<string, T> *Table, const char *Key);
    template<typename T> T *FindInHashTablePtr(hash_table<string, T> *Table, const char *Key);
};

void
string_manager::Initialize(memory_arena *Arena){
    StringMemory = MakeArena(Arena, Kilobytes(32));
    Table = PushHashTable<const char *, const char *>(Arena, 512);
    
    u32 BufferCount = 128;
    //u32 BufferCount = 2;
    BufferMemory = MakeArena(Arena, sizeof(string_buffer_node)*BufferCount);
    
    NextBuffer = 0;
    for(u32 I=0; I<BufferCount; I++){
        string_buffer_node *Node = PushStruct(&BufferMemory, string_buffer_node);
        Node->Next = NextBuffer;
        NextBuffer = Node;
    }
}

string
string_manager::GetString(const char *String){
    string Result = {};
    if(!String) return Result;
    
    const char *ResultString = FindInHashTable(&Table, String);
    if(!ResultString){
        ResultString = ArenaPushCString(&StringMemory, String);
        InsertIntoHashTable(&Table, ResultString, ResultString);
    }
    
    Result.ID = (u64)ResultString;
    return(Result);
}

const char *
string_manager::GetString(string String){
    const char *Result = (const char *)String.ID;
    
    return(Result);
}

const char *
string_manager::GetPermanentString(const char *String){
    const char *Result = GetString(GetString(String));
    return Result;
}

char *
string_manager::MakeBuffer(){
    Assert(NextBuffer);
    char *Result = NextBuffer->Buffer;
    NextBuffer = NextBuffer->Next;
    ZeroMemory(Result, DEFAULT_BUFFER_SIZE);
    return(Result);
}

void
string_manager::RemoveBuffer(char *Buffer){
    u8 *MemoryMin = BufferMemory.Memory;
    u8 *MemoryMax = BufferMemory.Memory+BufferMemory.Used;
    Assert((MemoryMin <= (u8 *)Buffer) && ((u8 *)Buffer < MemoryMax));
    string_buffer_node *Node = (string_buffer_node *)Buffer;
    Node->Next = NextBuffer;
    NextBuffer = Node;
}

template<typename T> T *
string_manager::GetInHashTablePtr(hash_table<string, T> *Table, const char *Key){
    string String = GetString(Key);
    T *Result = ::FindInHashTablePtr(Table, String);
    if(!Result){
        Result = CreateInHashTablePtr(Table, String);
    }
    return(Result);
}

template<typename T> T *
string_manager::FindInHashTablePtr(hash_table<string, T> *Table, const char *Key){
    string String = GetString(Key);
    T *Result = ::FindInHashTablePtr(Table, String);
    return(Result);
}

//~ String builder
struct string_builder {
    char *Buffer;
    u32 BufferSize;
    u32 BufferCapacity;
};

internal inline string_builder 
BeginStringBuilder(memory_arena *Arena, u32 Capacity){
    string_builder Result = {};
    Result.Buffer = PushArray(Arena, char, Capacity);
    Result.BufferCapacity = Capacity;
    return Result;
}

internal inline char *
StringBuilderFinalize(memory_arena *Arena, string_builder *Builder){
    u32 Size = Builder->BufferSize+1;
    char *Result = PushArray(Arena, char, Size);
    CopyMemory(Result, Builder->Buffer, Size);
    Result[Size] = 0;
    return Result;
}

internal inline char *
EndStringBuilder(memory_arena *Arena, string_builder *Builder){
    char *Result = Builder->Buffer;
    return Result;
}

internal inline void
StringBuilderAdd(string_builder *Builder, const char *S){
    u32 Length = CStringLength(S);
    Assert(Builder->BufferSize+Length < Builder->BufferCapacity-1);
    CopyCString(&Builder->Buffer[Builder->BufferSize], S, Length);
    Builder->BufferSize += Length;
    Builder->Buffer[Builder->BufferSize] = 0;
}

internal inline void
StringBuilderAdd(string_builder *Builder, char C){
    Assert(Builder->BufferSize+1 < Builder->BufferCapacity-1);
    Builder->Buffer[Builder->BufferSize++] = C;
    Builder->Buffer[Builder->BufferSize] = 0;
}

internal inline void
StringBuilderAdd(string_builder *Builder, void *Data, u32 DataSize){
    Assert(Builder->BufferSize+DataSize < Builder->BufferCapacity-1);
    CopyMemory(&Builder->Buffer[Builder->BufferSize], Data, DataSize);
    Builder->BufferSize += DataSize;
    Builder->Buffer[Builder->BufferSize] = 0;
}

#define StringBuilderAddVar(Builder, Data) StringBuilderAdd(Builder, &Data, sizeof(Data))

internal inline void
StringBuilderVAdd(string_builder *Builder, const char *Format, va_list VarArgs){
    char Buffer[DEFAULT_BUFFER_SIZE];
    stbsp_vsnprintf(Buffer, DEFAULT_BUFFER_SIZE, Format, VarArgs);
    StringBuilderAdd(Builder, Buffer);
}

internal inline void
StringBuilderAdd(string_builder *Builder, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    StringBuilderVAdd(Builder, Format, VarArgs);
    va_end(VarArgs);
}
