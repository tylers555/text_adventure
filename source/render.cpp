
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
              rect TextureRect=MakeRect(V2(0,0), V2(1,1)), b8 HasAlpha=false, color Color=WHITE){
    Assert(Texture);
    
    RenderQuad(Renderer, Texture, Z,
               V2(R.Min.X, R.Min.Y), V2(TextureRect.Min.X, TextureRect.Min.Y), Color,
               V2(R.Min.X, R.Max.Y), V2(TextureRect.Min.X, TextureRect.Max.Y), Color,
               V2(R.Max.X, R.Max.Y), V2(TextureRect.Max.X, TextureRect.Max.Y), Color,
               V2(R.Max.X, R.Min.Y), V2(TextureRect.Max.X, TextureRect.Min.Y), Color,
               HasAlpha);
}

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
    InitializeFramebuffer(&GameScreenFramebuffer, GameScreenShader, V2S(OutputSize/CameraScale));
    
    DefaultShader.ID = MakeShaderProgramFromFile("shaders/glsl/game_shader.glsl");
    DefaultShader.ProjectionLocation = ShaderProgramGetUniformLocation(DefaultShader.ID, "InProjection");
    
    InitializeArray(&Vertices, 2000);
    InitializeArray(&Indices,  2000);
}

void
game_renderer::ChangeScale(f32 NewScale){
    CameraScale = NewScale;
    ResizeFramebuffer(&GameScreenFramebuffer, V2S(OutputSize/CameraScale));
}

void
game_renderer::NewFrame(memory_arena *Arena, v2 OutputSize_, color ClearColor_){
    local_persist v2 WindowSize;
    
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
    
    if(OSInput.WasWindowResized()){
        WindowSize = OSInput.WindowSize;
        f32 Aspect = OutputSize.X/OutputSize.Y;
        f32 XFactor = 480.0f;
        f32 YFactor = 270.0f;
        f32 NewScale = Minimum(OutputSize.X/XFactor, OutputSize.Y/YFactor);
        //NewScale = Clamp(NewScale, 1.0f, 0.0f);
        NewScale = Maximum(NewScale, 1.0f);
        ChangeScale(Round(NewScale));
    }
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
