#ifndef MATH_H
#define MATH_H

# if 0
#include "typedefs.h"
template<u32 Dim ,class Type>
struct _v
{
    Type V[Dim];

    _v<Dim, Type> operator+(const _v<Dim, Type>& B)
    {
        _v<Dim, Type> Result;
        for(u32 I = 0; I < Dim; I++)
        {
            Result.V[I] = this->V[I] + B.V[I];
        }

        return Result;
    }

    _v<Dim, Type> operator-(const _v<Dim, Type>& B)
    {
        _v<Dim, Type> Result;
        for(u32 I = 0; I < Dim; I++)
        {
            Result.V[I] = this->V[I] - B.V[I];
        }

        return Result;
    }
};

typedef _v<2, f32> v2;
typedef _v<3, f32> v3;

#else
typedef ImVec2 v2;

ImVec2 operator+(ImVec2 A, ImVec2 B)
{
    ImVec2 Result;
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;

    return Result;
}

ImVec2 operator-(ImVec2 A, ImVec2 B)
{
    ImVec2 Result;
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;

    return Result;
}

ImVec2 operator*(f32 A, ImVec2 B)
{
    ImVec2 Result;
    Result.x = A * B.x;
    Result.y = A * B.y;

    return Result;
}

ImVec2 V2(f32 X, f32 Y)
{
    return ImVec2(X, Y);
}

s32 Min(s32 A, s32 B)
{
    if(A < B)
    {
        return A;
    }
    else
    {
        return B;
    }
}

s32 Max(s32 A, s32 B)
{
    if(A < B)
    {
        return B;
    }
    else
    {
        return A;
    }
}

struct rect2
{
    v2 Min;
    v2 Max;
};

rect2 RectMinMax(v2 Min, v2 Max)
{
    rect2 Result;
    Result.Min = Min;
    Result.Max = Max;
    return Result;
}

rect2 RectCenterDim(v2 Center, v2 Dim)
{
    rect2 Result;
    Result.Min = Center - 0.5f*Dim;
    Result.Max = Center + 0.5f*Dim;
    return Result;
}

rect2 RectMinDim(v2 Min, v2 Dim)
{
    v2 Max = Min + Dim;
    rect2 Result = RectMinMax(Min, Max);
    return Result;
}

v2 GetRectDim(rect2 Rect)
{
    v2 Result = Rect.Max - Rect.Min;
    return Result;
}

v2 GetRectCenter(rect2 Rect)
{
    v2 Result = 0.5f*(Rect.Min + Rect.Max);
    return Result;
}

v2 GetMinCorner(rect2 Rect)
{
    v2 Result = Rect.Min;
    return Result;
}

v2 GetMaxCorner(rect2 Rect)
{
    v2 Result = Rect.Max;
    return Result;
}

bool InRect(v2 A, rect2 Rect)
{
    v2 Min = GetMinCorner(Rect);
    v2 Max = GetMaxCorner(Rect);
    bool Result = (A.x > Min.x) && (A.x < Max.x) && (A.y > Min.y) && (A.y < Max.y);

    return Result;
}

#endif

#define Pi           3.14159265358979323846

#endif /* MATH_H */
