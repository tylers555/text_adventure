
//~ Basic rendering primitives

internal inline b8
ColorHasAlpha(color C){
    b8 Result = (C.A != 0.0f) && (C.A < 1.0f);
    return(Result);
}

internal void
RenderQuad(game_renderer *Renderer, render_texture Texture, f32 Z,
           v2 P0, v2 T0, color C0, 
           v2 P1, v2 T1, color C1, 
           v2 P2, v2 T2, color C2, 
           v2 P3, v2 T3, color C3,
           b8 HasAlpha_=false){
    b8 HasAlpha = (HasAlpha_         ||
                   ColorHasAlpha(C0) ||
                   ColorHasAlpha(C1) ||
                   ColorHasAlpha(C2) ||
                   ColorHasAlpha(C3));
    
    render_item *RenderItem = Renderer->NewRenderItem(Texture, HasAlpha, Z);
    if(!RenderItem) return;
    
    item_vertex *Vertices = Renderer->AddVertices(RenderItem, 4);
    Vertices[0] = {P0, Z, T0, C0};
    Vertices[1] = {P1, Z, T1, C1};
    Vertices[2] = {P2, Z, T2, C2};
    Vertices[3] = {P3, Z, T3, C3};
    
    u32 *Indices = Renderer->AddIndices(RenderItem, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
}

internal void
RenderLine(game_renderer *Renderer, v2 A, v2 B, f32 Z, f32 Thickness, color Color){
    v2 AB = B - A;
    v2 AB90 = Normalize(Clockwise90(AB));
    v2 P0 = A - 0.5f*Thickness*AB90;
    v2 P1 = A + 0.5f*Thickness*AB90;
    v2 P3 = B - 0.5f*Thickness*AB90;
    v2 P2 = B + 0.5f*Thickness*AB90;
    
    RenderQuad(Renderer, Renderer->WhiteTexture, Z,
               P0, V2(0, 0), Color,
               P1, V2(0, 1), Color,
               P2, V2(1, 1), Color,
               P3, V2(1, 0), Color);
}

internal void
RenderLineFrom(game_renderer *Renderer, v2 A, v2 Delta, f32 Z, f32 Thickness, color Color){
    RenderLine(Renderer, A, A+Delta, Z, Thickness, Color);
}

internal void
RenderCircle(game_renderer *Renderer, v2 P, f32 Radius, f32 Z, color Color, u32 Sides=30){
    
    f32 T = 0.0f;
    f32 Step = 1.0f/(f32)Sides;
    
    render_item *RenderItem = Renderer->NewRenderItem(Renderer->WhiteTexture, ColorHasAlpha(Color), Z);
    if(!RenderItem) return;
    
    item_vertex *Vertices = Renderer->AddVertices(RenderItem, Sides+2);
    Vertices[0] = {P, Z, 0.0f, 0.0f, Color};
    for(u32 I = 0; I <= Sides; I++){
        v2 Offset = V2(Cos(T*TAU), Sin(T*TAU));
        Vertices[I+1] = {P+Radius*Offset, Z, V2(0.0f, 0.0f), Color};
        T += Step;
    }
    
    u32 *Indices = Renderer->AddIndices(RenderItem, Sides*3);
    u16 CurrentIndex = 1;
    for(u32 I = 0; I < Sides*3; I += 3){
        Indices[I] = 0;
        Indices[I+1] = CurrentIndex;
        CurrentIndex++;
        if(CurrentIndex == Sides+1){
            CurrentIndex = 1;
        }
        Indices[I+2] = CurrentIndex;
    }
    
}

internal void
RenderRect(game_renderer *Renderer, rect R, f32 Z, color Color){
    RenderQuad(Renderer, Renderer->WhiteTexture, Z,
               V2(R.Min.X, R.Min.Y), V2(0, 0), Color,
               V2(R.Min.X, R.Max.Y), V2(0, 1), Color,
               V2(R.Max.X, R.Max.Y), V2(1, 1), Color,
               V2(R.Max.X, R.Min.Y), V2(1, 0), Color);
}

internal inline void
RenderRectOutline(game_renderer *Renderer, rect R, f32 Z, color Color, f32 Thickness=1){
    rect B = RectGrow(R, Thickness);
    RenderRect(Renderer, MakeRect(V2(B.Min.X, B.Min.Y), V2(R.Max.X, R.Min.Y)), Z, Color);
    RenderRect(Renderer, MakeRect(V2(R.Max.X, B.Min.Y), V2(B.Max.X, R.Max.Y)), Z, Color);
    RenderRect(Renderer, MakeRect(V2(R.Min.X, R.Max.Y), V2(B.Max.X, B.Max.Y)), Z, Color);
    RenderRect(Renderer, MakeRect(V2(B.Min.X, R.Min.Y), V2(R.Min.X, B.Max.Y)), Z, Color);
    //RenderRect(Rect(V2(R.Min.X, R.Min.Y),           V2(R.Max.X, R.Min.Y+Thickness)), Z, Color);
    //RenderRect(Rect(V2(R.Max.X-Thickness, R.Min.Y), V2(R.Max.X, R.Max.Y)),           Z, Color);
    //RenderRect(Rect(V2(R.Min.X, R.Max.Y),           V2(R.Max.X, R.Max.Y-Thickness)), Z, Color);
    //RenderRect(Rect(V2(R.Min.X, R.Min.Y),           V2(R.Min.X+Thickness, R.Max.Y)), Z, Color);
}

internal void
RenderTexture(game_renderer *Renderer, rect R, f32 Z, render_texture Texture, 
              rect TextureRect=MakeRect(V2(0,0), V2(1,1)), b8 HasAlpha=false){
    Assert(Texture);
    
    color Color = WHITE;
    RenderQuad(Renderer, Texture, Z,
               V2(R.Min.X, R.Min.Y), V2(TextureRect.Min.X, TextureRect.Min.Y), Color,
               V2(R.Min.X, R.Max.Y), V2(TextureRect.Min.X, TextureRect.Max.Y), Color,
               V2(R.Max.X, R.Max.Y), V2(TextureRect.Max.X, TextureRect.Max.Y), Color,
               V2(R.Max.X, R.Min.Y), V2(TextureRect.Max.X, TextureRect.Min.Y), Color,
               HasAlpha);
}

internal void
RenderRoundedRect(game_renderer *Renderer, rect R, f32 Z, f32 Roundness, color C){
    R = RectFix(R);
    v2 Size = RectSize(R);
    
    f32 Radius;
    if(Roundness < 0){
        Radius = 0.5f*Minimum(Size.Width, Size.Height) * -Roundness;
    }else{
        Radius = Roundness;
    }
    
    b8 HasAlpha = ColorHasAlpha(C);
    
    u32 Segments = (u32)Round(TAU*Radius / 10.0f);
    
    v2 R0 = V2(R.Min.X, R.Max.Y);
    v2 R1 = V2(R.Max.X, R.Max.Y);
    v2 R2 = V2(R.Max.X, R.Min.Y);
    v2 R3 = V2(R.Min.X, R.Min.Y);
    
    v2 I0 = R0 + V2( Radius, -Radius);
    v2 I1 = R1 + V2(-Radius, -Radius);
    v2 I2 = R2 + V2(-Radius,  Radius);
    v2 I3 = R3 + V2( Radius,  Radius);
    
    v2 O0 = R0 + V2( Radius,       0);
    v2 O1 = R1 + V2(-Radius,       0);
    v2 O2 = R1 + V2(      0, -Radius);
    v2 O3 = R2 + V2(      0,  Radius);
    v2 O4 = R2 + V2(-Radius,       0);
    v2 O5 = R3 + V2( Radius,       0);
    v2 O6 = R3 + V2(      0,  Radius);
    v2 O7 = R0 + V2(      0, -Radius);
    render_item *Item = Renderer->NewRenderItem(Renderer->WhiteTexture, HasAlpha, Z);
    if(!Item) return;
    
    u32 VertexCount = 4*5 + 4*2*Segments;
    u32 IndexCount  = 6*5 + 4*3*Segments;
    item_vertex *Vertices = Renderer->AddVertices(Item, VertexCount);
    u32 *Indices = Renderer->AddIndices(Item, IndexCount);
    
    Vertices[ 0] = {I0, Z, V2(0, 1), C};
    Vertices[ 1] = {I1, Z, V2(1, 1), C};
    Vertices[ 2] = {I2, Z, V2(1, 0), C};
    Vertices[ 3] = {I3, Z, V2(0, 0), C};
    Indices[ 0] = 0;
    Indices[ 1] = 1;
    Indices[ 2] = 2;
    Indices[ 3] = 0;
    Indices[ 4] = 2;
    Indices[ 5] = 3;
    
    Vertices[ 4] = {O0, Z, V2(0, 1), C};
    Vertices[ 5] = {O1, Z, V2(1, 1), C};
    Vertices[ 6] = {I1, Z, V2(1, 0), C};
    Vertices[ 7] = {I0, Z, V2(0, 0), C};
    Indices[ 6] = 4;
    Indices[ 7] = 5;
    Indices[ 8] = 6;
    Indices[ 9] = 4;
    Indices[10] = 6;
    Indices[11] = 7;
    
    Vertices[ 8] = {I1, Z, V2(0, 1), C};
    Vertices[ 9] = {O2, Z, V2(1, 1), C};
    Vertices[10] = {O3, Z, V2(1, 0), C};
    Vertices[11] = {I2, Z, V2(0, 0), C};
    Indices[12] =  8;
    Indices[13] =  9;
    Indices[14] = 10;
    Indices[15] =  8;
    Indices[16] = 10;
    Indices[17] = 11;
    
    Vertices[12] = {I3, Z, V2(0, 1), C};
    Vertices[13] = {I2, Z, V2(1, 1), C};
    Vertices[14] = {O4, Z, V2(1, 0), C};
    Vertices[15] = {O5, Z, V2(0, 0), C};
    Indices[18] = 12;
    Indices[19] = 13;
    Indices[20] = 14;
    Indices[21] = 12;
    Indices[22] = 14;
    Indices[23] = 15;
    
    Vertices[16] = {O7, Z, V2(0, 1), C};
    Vertices[17] = {I0, Z, V2(1, 1), C};
    Vertices[18] = {I3, Z, V2(1, 0), C};
    Vertices[19] = {O6, Z, V2(0, 0), C};
    Indices[24] = 16;
    Indices[25] = 17;
    Indices[26] = 18;
    Indices[27] = 16;
    Indices[28] = 18;
    Indices[29] = 19;
    
    f32 T = 0.0f;
    f32 Step = 0.25f * 1.0f/(f32)Segments;
    
    u32 VO0 = 20+0*2*Segments;
    u32 VO1 = 20+1*2*Segments;
    u32 VO2 = 20+2*2*Segments;
    u32 VO3 = 20+3*2*Segments;
    
    u32 IO0 = 30+0*3*Segments;
    u32 IO1 = 30+1*3*Segments;
    u32 IO2 = 30+2*3*Segments;
    u32 IO3 = 30+3*3*Segments;
    
    for(u32 I=0; I<Segments; I++){
        
        f32 T0 = T;
        T += Step;
        f32 T1 = T;
        
        f32 C0 = Cos(T0*TAU);
        f32 S0 = Sin(T0*TAU);
        f32 C1 = Cos(T1*TAU);
        f32 S1 = Sin(T1*TAU);
        
        Indices[IO0++] = 0;
        Vertices[VO0] = {I0+Radius*V2(-C0, S0), Z, V2(0.0f, 0.0f), C};
        Indices[IO0++] = VO0++;
        Vertices[VO0] = {I0+Radius*V2(-C1, S1), Z, V2(0.0f, 0.0f), C};
        Indices[IO0++] = VO0++;
        
        Indices[IO1++] = 1;
        Vertices[VO1] = {I1+Radius*V2(C0, S0), Z, V2(0.0f, 0.0f), C};
        Indices[IO1++] = VO1++;
        Vertices[VO1] = {I1+Radius*V2(C1, S1), Z, V2(0.0f, 0.0f), C};
        Indices[IO1++] = VO1++;
        
        Indices[IO2++] = 2;
        Vertices[VO2] = {I2+Radius*V2(C0, -S0), Z, V2(0.0f, 0.0f), C};
        Indices[IO2++] = VO2++;
        Vertices[VO2] = {I2+Radius*V2(C1, -S1), Z, V2(0.0f, 0.0f), C};
        Indices[IO2++] = VO2++;
        
        Indices[IO3++] = 3;
        Vertices[VO3] = {I3+Radius*V2(-C0, -S0), Z, V2(0.0f, 0.0f), C};
        Indices[IO3++] = VO3++;
        Vertices[VO3] = {I3+Radius*V2(-C1, -S1), Z, V2(0.0f, 0.0f), C};
        Indices[IO3++] = VO3++;
    }
}

//~ String rendering

#if 0
internal void
RenderString(game_renderer *Renderer, font *Font, color Color, v2 P, f32 Z, const char *String){
    v2 OutputSize = Renderer->OutputSize;
    
    P.Y = OutputSize.Y - P.Y;
    
    u32 Length = CStringLength(String);
    
    render_item *RenderItem = Renderer->NewRenderItem(Font->Texture, true, Z);
    if(!RenderItem) return;
    
    item_vertex *Vertices = Renderer->AddVertices(RenderItem, 4*Length);
    u32 VertexOffset = 0;
    for(char C = *String; C; C = *(++String)){
        stbtt_aligned_quad Q;
        stbtt_GetBakedQuad(Font->CharData,
                           Font->TextureWidth, Font->TextureHeight,
                           C-32, &P.X, &P.Y, &Q, 1);
        Q.y0 = OutputSize.Y - Q.y0;
        Q.y1 = OutputSize.Y - Q.y1;
        Vertices[VertexOffset]   = {V2(Q.x0, Q.y0), Z, V2(Q.s0, Q.t0), Color};
        Vertices[VertexOffset+1] = {V2(Q.x0, Q.y1), Z, V2(Q.s0, Q.t1), Color};
        Vertices[VertexOffset+2] = {V2(Q.x1, Q.y1), Z, V2(Q.s1, Q.t1), Color};
        Vertices[VertexOffset+3] = {V2(Q.x1, Q.y0), Z, V2(Q.s1, Q.t0), Color};
        
        VertexOffset += 4;
    }
    
    u32 *Indices = Renderer->AddIndices(RenderItem, 6*Length);
    u16 FaceOffset = 0;
    for(u32 IndexOffset = 0; IndexOffset < 6*Length; IndexOffset += 6){
        Indices[IndexOffset]   = FaceOffset;
        Indices[IndexOffset+1] = FaceOffset+1;
        Indices[IndexOffset+2] = FaceOffset+2;
        Indices[IndexOffset+3] = FaceOffset;
        Indices[IndexOffset+4] = FaceOffset+2;
        Indices[IndexOffset+5] = FaceOffset+3;
        FaceOffset += 4;
    }
}

internal inline void
VRenderFormatString(game_renderer *Renderer, font *Font, color Color, 
                    v2 P, f32 Z, const char *Format, va_list VarArgs){
    char Buffer[1024];
    stbsp_vsnprintf(Buffer, 1024, Format, VarArgs);
    RenderString(Renderer, Font, Color, P, Z, Buffer);
}

internal inline void
RenderFormatString(game_renderer *Renderer, font *Font, color Color, v2 P, f32 Z, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VRenderFormatString(Renderer, Font, Color, P, Z, Format, VarArgs);
    va_end(VarArgs);
}

internal f32
GetStringAdvance(font *Font, const char *String, b8 CountEndingSpace=false){
    f32 Result = 0;
    f32 X = 0.0f;
    f32 Y = 0.0f;
    char LastC = '\0';
    for(char C = *String; C; C = *(++String)){
        stbtt_aligned_quad Q;
        stbtt_GetBakedQuad(Font->CharData,
                           Font->TextureWidth, Font->TextureHeight,
                           C-32, &X, &Y, &Q, 1);
        Result = Q.x1;
        LastC = C;
    }
    
    // NOTE(Tyler): This is so that the ending space is actually factored in, usually it is
    // not.
    if(CountEndingSpace){
        if(LastC == ' '){
            stbtt_aligned_quad Q;
            stbtt_GetBakedQuad(Font->CharData,
                               Font->TextureWidth, Font->TextureHeight,
                               LastC-32, &X, &Y, &Q, 1);
            Result = Q.x1;
        }
    }
    
    return(Result);
}

internal f32
GetStringAdvanceByCount(font *Font, const char *String, u32 Count, b8 CountEndingSpace=false){
    f32 Result = 0;
    f32 X = 0.0f;
    f32 Y = 0.0f;
    char LastC = '\0';
    u32 I = 0;
    for(char C = *String; C; C = *(++String)){
        if(I >= Count) break;
        stbtt_aligned_quad Q;
        stbtt_GetBakedQuad(Font->CharData,
                           Font->TextureWidth, Font->TextureHeight,
                           C-32, &X, &Y, &Q, 1);
        Result = Q.x1;
        LastC = C;
        I++;
    }
    
    // NOTE(Tyler): This is so that the ending space is actually factored in, usually it is
    // not.
    if(CountEndingSpace){
        if(LastC == ' '){
            stbtt_aligned_quad Q;
            stbtt_GetBakedQuad(Font->CharData,
                               Font->TextureWidth, Font->TextureHeight,
                               LastC-32, &X, &Y, &Q, 1);
            Result = Q.x1;
        }
    }
    
    return(Result);
}

internal inline f32
VGetFormatStringAdvance(font *Font, const char *Format, va_list VarArgs){
    char Buffer[1024];
    stbsp_vsnprintf(Buffer, 1024, Format, VarArgs);
    f32 Result = GetStringAdvance(Font, Buffer);
    return(Result);
}

internal inline f32
GetFormatStringAdvance(font *Font, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    f32 Result = VGetFormatStringAdvance(Font, Format, VarArgs);
    va_end(VarArgs);
    return(Result);
}

internal inline void 
RenderCenteredString(game_renderer *Renderer, font *Font, color Color, v2 Center,
                     f32 Z, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    f32 Advance = VGetFormatStringAdvance(Font, Format, VarArgs);
    Center.X -= Advance/2.0f;
    VRenderFormatString(Renderer, Font, Color, Center, Z, Format, VarArgs);
    va_end(VarArgs);
}
#endif

//~ Renderer
internal inline item_shader
MakeItemShader(const char *Path){
    item_shader Result = {};
    Result.ID = MakeShaderProgramFromFile(Path);
    if(Result.ID == -1) return Result;
    
    Result.ProjectionLocation = ShaderProgramGetUniformLocation(Result.ID, "InProjection");
    Result.CameraPLocation = ShaderProgramGetUniformLocation(Result.ID, "InCameraP");
    
    return Result;
}

void
game_renderer::Initialize(memory_arena *Arena, v2 OutputSize_){
    OutputSize = OutputSize_;
    ClipRects = MakeStack<rect>(Arena, MAX_CLIP_RECTS);
    
    //~ Camera
    CameraScale = 5;
    
    //~ Other
    u8 TemplateColor[] = {0xff, 0xff, 0xff, 0xff};
    WhiteTexture = MakeTexture();
    TextureUpload(WhiteTexture, TemplateColor, 1, 1);
    
    GameShader = MakeItemShader("shaders/glsl/game_shader.glsl");
    
    //Assert(0);
    screen_shader GameScreenShader = MakeScreenShaderFromFile("shaders/glsl/game_screen_shader.glsl");
    InitializeFramebuffer(&GameScreenFramebuffer, GameScreenShader, OutputSize/CameraScale);
    
    DefaultShader.ID = MakeShaderProgramFromFile("shaders/glsl/game_shader.glsl");
    DefaultShader.ProjectionLocation = ShaderProgramGetUniformLocation(DefaultShader.ID, "InProjection");
    
    InitializeArray(&Vertices, 2000);
    InitializeArray(&Indices,  2000);
}

void
game_renderer::ChangeScale(f32 NewScale){
    f32 Epsilon = 0.0001f;
    if((NewScale <= (CameraScale+Epsilon)) &&((CameraScale-Epsilon) <= NewScale)){
    }else{
        CameraScale = NewScale;
        ResizeFramebuffer(&GameScreenFramebuffer, OutputSize/CameraScale);
    }
}

void
game_renderer::NewFrame(memory_arena *Arena, v2 OutputSize_, color ClearColor_){
    v2 OldOutputSize = OutputSize;
    OutputSize = OutputSize_;
    ClearColor = ClearColor_;
    StackClear(&ClipRects);
    StackPush(&ClipRects, SizeRect(V2(0), OutputSize));
    
    //~ Render items
    RenderItemCount = 0;
    ArrayClear(&Vertices);
    ArrayClear(&Indices);
    
    RenderNode = PushStruct(Arena, render_node);
    
    //~ Camera
#if 0 
    CameraTargetP.X = Clamp(CameraTargetP.X, CameraBounds.Min.X, CameraBounds.Max.X);
    CameraTargetP.Y = Clamp(CameraTargetP.Y, CameraBounds.Min.Y, CameraBounds.Max.Y);
    
    f32 dTime = OSInput.dTime;
    v2 Delta = dTime*CameraSpeed*(CameraTargetP-CameraFinalP);
    CameraFinalP += Delta;
#endif
    
    f32 Factor = 210.0f;
    f32 NewScale = Minimum(OutputSize.X/Factor, OutputSize.Y/Factor);
    NewScale = Maximum(NewScale, 1.0f);
    ChangeScale(Round(NewScale));
}

//~ Render stuff
render_item *
game_renderer::NewRenderItem(render_texture Texture, b8 HasAlpha, f32 Z){
    render_node *Node = RenderNode;
    if(Node->Count >= RENDER_NODE_ITEMS){
        render_node *OldNode = Node;
        Node = PushStruct(&TransientStorageArena, render_node);
        Node->Next = OldNode;
        RenderNode = Node;
    }
    
    u32 Index = Node->Count++;
    render_item *Result = &Node->Items[Index];
    Result->ClipRect = StackPeek(&ClipRects);
    Result->Texture = Texture;
    if(HasAlpha){
        Node->ItemZs[Index] = Z;
    }else{
        Node->ItemZs[Index] = F32_NEGATIVE_INFINITY;
    }
    RenderItemCount++;
    
    return(Result);
}

item_vertex *
game_renderer::AddVertices(render_item *Item, u32 VertexCount){
    Item->VertexOffset = Vertices.Count;
    item_vertex *Result = ArrayAlloc(&Vertices, VertexCount);
    return(Result);
}

u32 *
game_renderer::AddIndices(render_item *Item, u32 IndexCount){
    Item->IndexOffset  = Indices.Count;
    Item->IndexCount   = IndexCount;
    u32 *Result = ArrayAlloc(&Indices, IndexCount);
    return(Result);
}

void
game_renderer::BeginClipRect(rect ClipRect){
    StackPush(&ClipRects, RectFix(ClipRect));
}

void
game_renderer::EndClipRect(){
    StackPop(&ClipRects);
}

//~ Camera stuff

v2
game_renderer::WorldToScreen(v2 P){
    v2 Result = P * CameraScale;
    return(Result);
}

rect
game_renderer::WorldToScreen(rect R){
    rect Result;
    Result.Min = WorldToScreen(R.Min);
    Result.Max = WorldToScreen(R.Max);
    return(Result);
}

v2
game_renderer::ScreenToWorld(v2 P){
    v2 Result = P / CameraScale;
    return(Result);
}

rect
game_renderer::ScreenToWorld(rect R){
    rect Result;
    Result.Min = ScreenToWorld(R.Min);
    Result.Max = ScreenToWorld(R.Max);
    return(Result);
}
