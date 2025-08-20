/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

enum platform_memory_block_flags
{
    PlatformMemory_NotRestored = 0x1,
    PlatformMemory_OverflowCheck = 0x2,
    PlatformMemory_UnderflowCheck = 0x4,
};
struct platform_memory_block
{
    u64 Flags;
    u64 Size;
    u8 *Base;
    umm Used;
    platform_memory_block *ArenaPrev;
};

#define PLATFORM_ALLOCATE_MEMORY(name) platform_memory_block *name(umm Size, u64 Flags)
typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory);

#define PLATFORM_DEALLOCATE_MEMORY(name) void name(platform_memory_block *Block)
typedef PLATFORM_DEALLOCATE_MEMORY(platform_deallocate_memory);

struct platform_api
{
    platform_allocate_memory *AllocateMemory;
    platform_deallocate_memory *DeallocateMemory;
};
extern platform_api Platform;
