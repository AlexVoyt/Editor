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

bitmap AllocateEmptyBitmap(memory_pool* Pool, u32 Width, u32 Height)
{
    bitmap Result;
    Result.Width = Width;
    Result.Height = Height;
    Result.Stride = Result.Width;

    u32 BitmapSize = Result.Width * Result.Height;
    Result.Data = PushArray(Pool, u32, BitmapSize);

    return Result;
}

animation_frame* AllocateEmptyAnimationFrame(memory_pool* Pool, u32 BitmapWidth, u32 BitmapHeight)
{
    animation_frame* Result = PushStruct(Pool, animation_frame);
    Result->Bitmap = AllocateEmptyBitmap(Pool, BitmapWidth, BitmapHeight);
    Result->NextFrame = 0;
    Result->PrevFrame = 0;

    return Result;
}

void FillBitmap(bitmap* Bitmap, u32 Color = 0xFFFFFFFF)
{
    for(u32 Y = 0; Y < Bitmap->Height; Y++)
    {
        for(u32 X = 0; X < Bitmap->Width; X++)
        {
            Bitmap->Data[X + Y * Bitmap->Stride] = Color;
        }
    }
}

rect2 GetBitmapRectTranslated(bitmap* Bitmap, v2 PixelDim, v2 Offset)
{
    v2 Min = V2(0, 0);
    v2 Max = V2(Bitmap->Width * PixelDim.x, Bitmap->Height * PixelDim.y);
    Min = Min + Offset;
    Max = Max + Offset;
    rect2 Result = RectMinMax(Min, Max);

    return Result;
}

u32 GetPixelColor(bitmap* Bitmap, u32 X, u32 Y)
{
    Assert(X < Bitmap->Width);
    Assert(Y < Bitmap->Height);

    u32 Result = Bitmap->Data[X + Y * Bitmap->Stride];
    return Result;
}

u32 GetPixelColor(bitmap* Bitmap, v2 Index)
{
    return GetPixelColor(Bitmap, Index.x, Index.y);
}

void SetPixelColor(bitmap* Bitmap, u32 X, u32 Y, u32 Color)
{
    Assert(X < Bitmap->Width);
    Assert(Y < Bitmap->Height);

    Bitmap->Data[X + Y * Bitmap->Stride] = Color;
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
