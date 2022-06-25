enum bitmap_type
{
    BitmapType_Plain,
    BitmapType_LUT,
    BitmapType_LightProfile,
};

struct bitmap
{
    bitmap_type Type;
    u32* Data;
};

struct memory_pool
{
    u8* Base;
    size_t Size;
    size_t Used;
};

struct animation_frame
{
    u32 BitmapIndex;
};

enum layer_type
{
    LayerType_Plain,
    LayerType_LUT,
    LayerType_LightProfile,
};

struct layer
{
    layer_type Type;
    char Name[MaxLayerNameLength + 1];

    u32 FrameCount;
    animation_frame Frames[16];
};

struct editor
{
    memory_pool BitmapPool;
    char ProjectName[MaxProjectNameLength + 1];
    u32 Ticks;
    u32 BitmapWidth;
    u32 BitmapHeight;

    u32 BitmapCount;
    bitmap Bitmaps[4096];

    bool ToolsVisible;
    u32 SelectedPaletteSlot;
    u32 Palette[5*5];
    u32 BitmapBgColor;
    f32 FloatBitmapBgColor[4];
    u32 CurrentColor;
    f32 FloatColor[4];

    bool ViewportVisible;
    v2 CameraOffset;
    v2 PixelDim;

    // TODO: multiple layers
    u32 SelectedFrameIndex;

    u32 LayerCount;
    layer Layers[16];

    bool TimelineVisible;
    bool Animating;
    u32 CurrentFrameTicks;

};

