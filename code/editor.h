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
    bitmap* Bitmap;
    animation_frame* NextFrame;
};

struct layer
{
    animation_frame Frame;
    layer* NextLayer;
};

struct editor
{
    bool ViewportVisible;
    bool ColorPickerVisible;

    memory_pool BitmapPool;
    layer* FirstLayer;
    v2 CameraOffset;
    v2 PixelDim;

    u32 CurrentColor;
    f32 FloatColor[4];
};

