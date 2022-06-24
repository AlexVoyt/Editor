// TODO: check if pragma works on different compilers
#pragma pack(push, 1)

#if 0
enum paf_header_type
{
    PAFHeader_Main     = 'PAF0',
    PAFHeader_Project  =
    PAFHeader_Bitmaps
};
#endif

struct paf_chunk_header
{
    union
    {
        u8 Type[4];
        u32 TypeU32;
    };
    u32 Length;
};

struct paf_chunk_footer
{
    u32 Reserved;
};

#pragma pack(pop)
