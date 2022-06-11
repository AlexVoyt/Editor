#include "editor.h"

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
    Result.Stride = Result.Width * sizeof(Result.Width);

    u32 BitmapSize = Result.Width * Result.Height;
    Result.Data = PushArray(Pool, u32, BitmapSize);

    return Result;
}

void FillBitmap(bitmap* Bitmap, u32 Color = 0xFFFFFFFF)
{
    for(u32 Y = 0; Y < Bitmap->Height; Y++)
    {
        for(u32 X = 0; X < Bitmap->Width; X++)
        {
            Bitmap->Data[X + Y * Bitmap->Stride] = 0xFFFFFFFF;
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

