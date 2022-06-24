struct bitmap
{
    u32 Width;
    u32 Height;
    u32 Stride;
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
    bitmap Bitmap;
    animation_frame* NextFrame;
    animation_frame* PrevFrame;
};

struct layer
{
    char Name[MaxLayerNameLength + 1];
    animation_frame* SelectedFrame;
    animation_frame* FirstFrame;
    animation_frame* LastFrame;
};

struct editor
{
    memory_pool BitmapPool;
    memory_pool LayerPool;

    char ProjectName[MaxProjectNameLength + 1];
    u32 Ticks;
    u32 BitmapWidth;
    u32 BitmapHeight;
    u32 BitmapCount;
    bitmap Bitmaps[10000];

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
    layer* FirstLayer;

    bool TimelineVisible;
    bool Animating;
    u32 CurrentFrameTicks;

};

