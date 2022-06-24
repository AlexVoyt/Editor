#include "paf.h"

struct buffer
{
    u32 Length;
    u8* Content;
};

buffer ReadEntireFile(char* Filepath, memory_pool* Pool)
{
    buffer Result;

    FILE* File = fopen(Filepath, "rb");
    if(File)
    {
        fseek(File, 0, SEEK_END);
        Result.Length = ftell(File);
        fseek(File, 0, SEEK_SET);

        if(Pool)
        {
            Result.Content = PushArray(Pool, u8, Result.Length);
        }
        else
        {
            Result.Content = (u8* )malloc(Result.Length*sizeof(u8));
        }
        fread(Result.Content, Result.Length, 1, File);

        fclose(File);
    }
    else
    {
        Assert(!"Error during file reading");
    }

    return Result;
}

void EndianSwap(u32* Value)
{
    u32 V = *Value;

    *Value = (V << 24) |
             ((V & 0xFF00) << 8) |
             ((V >> 8) & 0xFF00) |
             (V >> 24);
}

#define ConsumeStruct(Buffer, Type) (Type* )ConsumeSize((Buffer), sizeof(Type))
void* ConsumeSize(buffer* Buffer, u32 Size)
{
    void* Result = Buffer->Content;
    if(Buffer->Length >= Size)
    {
        Buffer->Content += Size;
        Buffer->Length -= Size;
    }
    else
    {
        printf("Buffer underflow");
        Buffer->Content = 0;
        Buffer->Length = 0;
        Result = 0;
    }

    return Result;
}
