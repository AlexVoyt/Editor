#include "editor.h"

void UpdateWindowTitle(SDL_Window* Window, editor* Editor)
{
    // TODO: think better
    char Buffer[1024] = {};
    char* Cursor = &Buffer[0];
    strcpy(Cursor, "Editor - ");
    Cursor += sizeof("Editor - ") - 1;
    strcpy(Cursor, Editor->ProjectName);

    SDL_SetWindowTitle(Window, Buffer);
}

void InitializePool(memory_pool* Pool, u8* Base, size_t Size)
{
    Pool->Base = Base;
    Pool->Size = Size;
    Pool->Used = 0;
}

#define PushArray(Pool, Type, Count) (Type* )PushSize_((Pool), sizeof(Type)*Count)
#define PushStruct(Pool, Type)       (Type* )PushSize_((Pool), sizeof(Type))

void* PushSize_(memory_pool* Pool, size_t Size)
{
    Assert(Pool->Used + Size <= Pool->Size);

    void* Result = Pool->Base + Pool->Used;
    Pool->Used += Size;

    return Result;
}

u32 AllocateEmptyBitmap(editor* Editor, bitmap_type Type)
{
    u32 BitmapIndex = Editor->BitmapCount;
    if(BitmapIndex < ArrayCount(Editor->Bitmaps))
    {
        Editor->BitmapCount++;

        bitmap* Bitmap = &Editor->Bitmaps[BitmapIndex];
        Bitmap->Type = Type;

        u32 BitmapSize = Editor->BitmapWidth * Editor->BitmapHeight;
        Bitmap->Data = PushArray(&Editor->BitmapPool, u32, BitmapSize);
    }
    else
    {
        InvalidCodePath;
    }

    return BitmapIndex;
}

layer NewEmptyLayer(layer_type Type)
{
    layer Result = {};
    Result.Type = Type;

    return Result;
}

layer* AddLayer(editor* Editor, layer_type Type)
{
    layer* Result = 0;
    if(Editor->LayerCount < ArrayCount(Editor->Layers))
    {
        u32 LayerIndex = Editor->LayerCount;
        Editor->LayerCount++;

        Result = &Editor->Layers[LayerIndex];
        *Result = NewEmptyLayer(Type);
    }
    else
    {
        InvalidCodePath;
    }

    return Result;
}

bool AddFrameToLayer(layer* Layer, animation_frame Frame)
{
    bool Added = false;
    if(Layer->FrameCount < ArrayCount(Layer->Frames))
    {
        Layer->Frames[Layer->FrameCount++] = Frame;
        Added = true;
    }

    return Added;
}

#if 0
u32 AddBitmap(editor* Editor, bitmap_type Type)
{
    u32 BitmapIndex = Editor->BitmapCount;
    void* Bitmap = PushSize_(&Editor->BitmapPool, sizeof(bitmap_type));

    Editor->Bitmaps[BitmapIndex].Type = Type;
    Editor->Bitmaps[BitmapIndex].Data = Type;
    Bitmap.Type
}
#endif

bitmap* GetBitmapByIndex(editor* Editor, u32 BitmapIndex)
{
    bitmap* Result = 0;
    if(BitmapIndex < Editor->BitmapCount)
    {
        Result = &Editor->Bitmaps[BitmapIndex];
    }
    else
    {
        InvalidCodePath;
    }

    return Result;
}

void FillBitmap(editor* Editor, u32 BitmapIndex, u32 Color = 0xFFFFFFFF)
{
    bitmap* Bitmap = GetBitmapByIndex(Editor, BitmapIndex);
    u32 Stride = Editor->BitmapWidth;
    for(u32 Y = 0; Y < Editor->BitmapHeight; Y++)
    {
        for(u32 X = 0; X < Editor->BitmapWidth; X++)
        {
            Bitmap->Data[X + Y * Stride] = Color;
        }
    }
}

animation_frame GetFrameByIndex(layer* Layer, u32 Index)
{
    animation_frame Result = {};
    if(Index < ArrayCount(Layer->Frames))
    {
        Result = Layer->Frames[Index];
    }
    else
    {
        InvalidCodePath;
    }

    return Result;
}

rect2 GetBitmapRectTranslated(editor* Editor, v2 Offset)
{
    v2 Min = V2(0, 0);
    v2 Max = V2(Editor->BitmapWidth * Editor->PixelDim.x,
                Editor->BitmapHeight * Editor->PixelDim.y);
    Min = Min + Offset;
    Max = Max + Offset;
    rect2 Result = RectMinMax(Min, Max);

    return Result;
}

u32 GetPixelColor(bitmap* Bitmap, u32 Width, u32 Height, u32 X, u32 Y)
{
    u32 Stride = Width;
    Assert(X < Width);
    Assert(Y < Height);

    u32 Result = Bitmap->Data[X + Y * Stride];
    return Result;
}

u32 GetPixelColor(editor* Editor, u32 BitmapIndex, u32 X, u32 Y)
{
    bitmap* Bitmap = GetBitmapByIndex(Editor, BitmapIndex);
    return GetPixelColor(Bitmap, Editor->BitmapWidth, Editor->BitmapHeight, X, Y);
}

void SetPixelColor(bitmap* Bitmap, u32 Width, u32 Height, u32 X, u32 Y, u32 Color)
{
    u32 Stride = Width;
    Assert(X < Width);
    Assert(Y < Height);

    Bitmap->Data[X + Y * Stride] = Color;
}

void SetPixelColor(editor* Editor, u32 BitmapIndex, u32 X, u32 Y, u32 Color)
{
    bitmap* Bitmap = GetBitmapByIndex(Editor, BitmapIndex);
    SetPixelColor(Bitmap, Editor->BitmapWidth, Editor->BitmapHeight, X, Y, Color);
}

#if 0
layer* AllocateEmptyLayer(memory_pool* Pool)
{
    layer* Result = PushStruct(Pool, layer);
    Result->Bitmap = AllocateEmptyBitmap(Pool);
    Result->NextLayer = 0;

    return Result;
}
#endif

u32 ColorFloatToU32(ImVec4 FloatColor)
{
    u32 Result;
    u32 Red = FloatColor.x * 255;
    u32 Blue = FloatColor.y * 255;
    u32 Green = FloatColor.z * 255;
    u32 Alpha = FloatColor.w * 255;

    Result = (Red << 0) |
             (Blue << 8) |
             (Green << 16) |
             (Alpha << 24);

    return Result;
}

u32 ColorFloatToU32(f32* FloatColor)
{
    u32 Result;
    u32 Red = FloatColor[0] * 255;
    u32 Blue = FloatColor[1] * 255;
    u32 Green = FloatColor[2] * 255;
    u32 Alpha = FloatColor[3] * 255;

    Result = (Red << 0) |
             (Blue << 8) |
             (Green << 16) |
             (Alpha << 24);

    return Result;
}

void ColorU32ToFloat(f32* FloatColor, u32 Color)
{
    u8 Red = Color >> 0;
    u8 Blue = Color >> 8;
    u8 Green = Color >> 16;
    u8 Alpha = Color >> 24;

    FloatColor[0] = (f32)Red / 255;
    FloatColor[1] = (f32)Blue / 255;
    FloatColor[2] = (f32)Green / 255;
    FloatColor[3] = (f32)Alpha / 255;
}
