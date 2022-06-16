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
    char Name[32];
    animation_frame* SelectedFrame;
    animation_frame* FirstFrame;
    animation_frame* LastFrame;
};

struct editor
{
    memory_pool BitmapPool;
    memory_pool LayerPool;

    char ProjectName[255];
    u32 Ticks;
    u32 BitmapWidth;
    u32 BitmapHeight;

    bool ColorPickerVisible;
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

