#ifndef SNAIL_JUMPY_RENDER_H
#define SNAIL_JUMPY_RENDER_H

//~ Basic colors

global_constant color BLACK      = MakeColor(0.0f,  0.0f,  0.0f, 1.0f);
global_constant color WHITE      = MakeColor(1.0f,  1.0f,  1.0f, 1.0f);
global_constant color RED        = MakeColor(1.0f,  0.0f,  0.0f, 1.0f);
global_constant color YELLOW     = MakeColor(1.0f,  1.0f,  0.0f, 1.0f);
global_constant color BLUE       = MakeColor(0.0f,  0.0f,  1.0f, 1.0f);
global_constant color GREEN      = MakeColor(0.0f,  1.0f,  0.0f, 1.0f);
global_constant color DARK_GREEN = MakeColor(0.0f,  0.5f,  0.0f, 1.0f);
global_constant color BROWN      = MakeColor(0.41f, 0.20f, 0.0f, 1.0f);
global_constant color PINK       = MakeColor(1.0f,  0.0f,  1.0f, 1.0f);
global_constant color PURPLE     = MakeColor(0.42f, 0.05f, 0.68f,1.0f);
global_constant color ORANGE     = MakeColor(1.0f,  0.5f,  0.0f, 1.0f);


//~ Primitive types
typedef u32 render_texture;
typedef u32 shader_program;

typedef u32 texture_flags;
enum texture_flags_ {
    TextureFlag_None  = (0 << 0),
    TextureFlag_Blend = (1 << 0),
};

struct item_shader {
    shader_program ID;
    s32 ProjectionLocation;
    s32 CameraPLocation;
};

struct item_vertex {
    v2 P;
    f32 Z;
    v2 PixelUV;
    color Color;
};

struct screen_shader {
    u32 ID;
    s32 ScaleLocation;
};

struct framebuffer {
    screen_shader ScreenShader;
    u32 ID;
    u32 RenderbufferID;
    render_texture Texture;
};

//~ Renderer

struct render_item {
    rect ClipRect;
    u32 VertexOffset;
    u32 IndexOffset;
    u32 IndexCount;
    
    render_texture Texture;
};

struct render_item_z {
    render_item *Item;
    f32 Z;
};

global_constant u32 RENDER_NODE_ITEMS = 256;
struct render_node {
    render_node *Next;
    u32 Count;
    render_item Items[RENDER_NODE_ITEMS];
    f32 ItemZs[RENDER_NODE_ITEMS];
};

global_constant u32 MAX_LIGHT_COUNT = 128;
struct render_light {
    v2 P;
    f32 Z;
    f32 Radius;
    f32 R, G, B;
};

global u32 MAX_CLIP_RECTS = 128;

struct game_renderer {
    //~
    render_texture WhiteTexture;
    
    item_shader  GameShader;
    framebuffer   GameScreenFramebuffer;
    v2    OutputSize;
    color ClearColor;
    stack<rect> ClipRects;
    
    //~ Rendering variables
    u32 RenderItemCount;
    dynamic_array<item_vertex> Vertices;
    dynamic_array<u32>         Indices;
    render_node *RenderNode;
    
    //~ Render functions
    void Initialize(memory_arena *Arena, v2 OutputSize_);
    void NewFrame(memory_arena *Arena, v2 OutputSize_, color ClearColor_);
    
    render_item *NewRenderItem(render_texture Texture, b8 HasAlpha, f32 Z);
    item_vertex *AddVertices(render_item *Item, u32 VertexCount);
    u32         *AddIndices(render_item *Item, u32 IndexCount);
    
    void BeginClipRect(rect ClipRect);
    void EndClipRect();
    
    //~ Camera stuff
    f32 CameraScale;
    void ChangeScale(f32 NewScale);
    
    v2   WorldToScreen(v2 P);
    v2   ScreenToWorld(v2 P);
    rect WorldToScreen(rect R);
    rect ScreenToWorld(rect R);
};

struct render_group {
    game_renderer *Renderer;
    render_node *FirstNode;
};

//~ Backend functions
internal b8 InitializeRendererBackend();
internal void RendererRenderAll(game_renderer *Renderer);

internal render_texture MakeTexture(texture_flags Flags=TextureFlag_None);
internal void TextureUpload(render_texture Texture, u8 *Pixels, 
                            u32 Width, u32 Height, u32 Channels=4);
internal void DeleteTexture(render_texture Texture);

// TODO(Tyler): Maybe move these into renderer backend initialization
internal shader_program  MakeShaderProgramFromFileData(entire_file File);
internal screen_shader MakeScreenShaderFromFileData(entire_file File);
internal s32 ShaderProgramGetUniformLocation(shader_program Program, const char *Name);

internal void InitializeFramebuffer(framebuffer *Framebuffer, screen_shader ScreenShader, v2s Size);
internal void ResizeFramebuffer(framebuffer *Framebuffer, v2s Size);
internal void UseFramebuffer(framebuffer *Framebuffer);

#endif //SNAIL_JUMPY_RENDER_H

