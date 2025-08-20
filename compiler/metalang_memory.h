/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

struct memory_arena
{
    platform_memory_block *CurrentBlock;
    umm MinimumBlockSize;

    u64 AllocationFlags;
    s32 TempCount;
};

struct temporary_memory
{
    memory_arena *Arena;
    platform_memory_block *Block;
    umm Used;
};

inline void SetMinimumBlockSize(memory_arena *Arena, umm MinimumBlockSize)
{
    Arena->MinimumBlockSize = MinimumBlockSize;
}

inline umm GetAlignmentOffset(memory_arena *Arena, umm Alignment)
{
    umm AlignmentOffset = 0;

    umm ResultPointer = UMMFromPointer(Arena->CurrentBlock->Base) + Arena->CurrentBlock->Used;
    umm AlignmentMask = Alignment - 1;
    if(ResultPointer & AlignmentMask)
    {
        AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);
    }

    return AlignmentOffset;
}

enum arena_push_flag
{
    ArenaFlag_ClearToZero = 0x1,
};
struct arena_push_params
{
    u32 Flags;
    u32 Alignment;
};

inline arena_push_params DefaultArenaParams(void)
{
    arena_push_params Params;
    Params.Flags = ArenaFlag_ClearToZero;
    Params.Alignment = 4;
    return Params;
}

inline arena_push_params AlignNoClear(u32 Alignment)
{
    arena_push_params Params = DefaultArenaParams();
    Params.Flags &= ~ArenaFlag_ClearToZero;
    Params.Alignment = Alignment;
    return Params;
}

inline arena_push_params Align(u32 Alignment, b32 Clear)
{
    arena_push_params Params = DefaultArenaParams();
    if(Clear)
    {
        Params.Flags |= ArenaFlag_ClearToZero;
    }
    else
    {
        Params.Flags &= ~ArenaFlag_ClearToZero;
    }
    Params.Alignment = Alignment;
    return Params;
}

inline arena_push_params NoClear(void)
{
    arena_push_params Params = DefaultArenaParams();
    Params.Flags &= ~ArenaFlag_ClearToZero;
    return Params;
}

struct arena_bootstrap_params
{
    u64 AllocationFlags;
    umm MinimumBlockSize;
};

inline arena_bootstrap_params DefaultBootstrapParams(void)
{
    arena_bootstrap_params Params = {};
    return Params;
}

inline arena_bootstrap_params NonRestoredArena(void)
{
    arena_bootstrap_params Params = DefaultBootstrapParams();
    Params.AllocationFlags = PlatformMemory_NotRestored;
    return(Params);
}

#define PushStruct(Arena, type, ...) (type *)PushSize_(Arena, sizeof(type), ## __VA_ARGS__)
#define PushArray(Arena, Count, type, ...) (type *)PushSize_(Arena, (Count)*sizeof(type), ## __VA_ARGS__)
#define PushSize(Arena, Size, ...) (type *)PushSize_(Arena, Size, ## __VA_ARGS__)
#define PushCopy(Arena, Size, Source, ...) Copy(Size, Source, PushSize_(Arena, Size, ## __VA_ARGS__))
#define BootstrapPushStruct(type, Member, ...) (type *)BootstrapPushSize_(sizeof(type), OffsetOf(type, Member), ## __VA_ARGS__)

inline umm GetEffectiveSizeFor(memory_arena *Arena, umm SizeInit, arena_push_params Params = DefaultArenaParams())
{
    umm Size = SizeInit;

    umm AlignmentOffset = GetAlignmentOffset(Arena, Params.Alignment);
    Size += AlignmentOffset;

    return Size;
}

inline void *PushSize_(memory_arena *Arena, umm SizeInit, arena_push_params Params = DefaultArenaParams())
{
    void *Result = 0;

    Assert(Params.Alignment <= 128);
    Assert(IsPow2(Params.Alignment));

    umm Size = 0;
    if(Arena->CurrentBlock)
    {
        Size =  GetEffectiveSizeFor(Arena, SizeInit, Params);
    }

    if(!Arena->CurrentBlock ||
       ((Arena->CurrentBlock->Used + Size) > Arena->CurrentBlock->Size))
    {
        Size = SizeInit; // NOTE(alex): The base will automatically be aligned now!
        if(Arena->AllocationFlags & (PlatformMemory_OverflowCheck|
                                     PlatformMemory_UnderflowCheck))
        {
            Arena->MinimumBlockSize = 0;
            Size = AlignPow2(Size, Params.Alignment);
        }
        else if(!Arena->MinimumBlockSize)
        {
            // TODO(alex): Tune default block size eventually?
            Arena->MinimumBlockSize = Megabytes(1);
        }

        umm BlockSize = Maximum(Size, Arena->MinimumBlockSize);

        platform_memory_block *NewBlock =
            Platform.AllocateMemory(BlockSize, Arena->AllocationFlags);
        NewBlock->ArenaPrev = Arena->CurrentBlock;
        Arena->CurrentBlock = NewBlock;
    }

    Assert((Arena->CurrentBlock->Used + Size) <= Arena->CurrentBlock->Size);

    umm AlignmentOffset = GetAlignmentOffset(Arena, Params.Alignment);
    umm OffsetInBlock = Arena->CurrentBlock->Used + AlignmentOffset;
    Result = Arena->CurrentBlock->Base + OffsetInBlock;
    Arena->CurrentBlock->Used += Size;

    Assert(Size >= SizeInit);

    // NOTE(alex): This is just to guarantee that nobody passed in an alignment
    // on their first allocation that was _greater_ that than the page alignment
    Assert(Arena->CurrentBlock->Used <= Arena->CurrentBlock->Size);

    if(Params.Flags & ArenaFlag_ClearToZero)
    {
        ZeroSize(SizeInit, Result);
    }

    return Result;
}

inline void FreeLastBlock(memory_arena *Arena)
{
    platform_memory_block *Free = Arena->CurrentBlock;
    Arena->CurrentBlock = Free->ArenaPrev;
    Platform.DeallocateMemory(Free);
}

inline temporary_memory BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;

    Result.Arena = Arena;
    Result.Block = Arena->CurrentBlock;
    Result.Used = Arena->CurrentBlock ? Arena->CurrentBlock->Used : 0;

    ++Arena->TempCount;

    return(Result);
}

inline void EndTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;
    while(Arena->CurrentBlock != TempMem.Block)
    {
        FreeLastBlock(Arena);
    }

    if(Arena->CurrentBlock)
    {
        Assert(Arena->CurrentBlock->Used >= TempMem.Used);
        Arena->CurrentBlock->Used = TempMem.Used;
    }

    Assert(Arena->TempCount > 0);
    --Arena->TempCount;
}

inline void KeepTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;

    Assert(Arena->TempCount > 0);
    --Arena->TempCount;
}

inline void Clear(memory_arena *Arena)
{
    while(Arena->CurrentBlock)
    {
        // NOTE(alex): Because the arena itself may be stored in the last block,
        // we must ensure that we don't look at it after freeing.
        b32 ThisIsLastBlock = (Arena->CurrentBlock->ArenaPrev == 0);
        FreeLastBlock(Arena);
        if(ThisIsLastBlock)
        {
            break;
        }
    }
}

inline void *BootstrapPushSize_(umm StructSize, umm OffsetToArena,
                                arena_bootstrap_params BootstrapParams = DefaultBootstrapParams(),
                                arena_push_params Params = DefaultArenaParams())
{
    memory_arena Bootstrap = {};
    Bootstrap.AllocationFlags = BootstrapParams.AllocationFlags;
    Bootstrap.MinimumBlockSize = BootstrapParams.MinimumBlockSize;
    void *Struct = PushSize_(&Bootstrap, StructSize, Params);
    *(memory_arena *)((u8 *)Struct + OffsetToArena) = Bootstrap;

    return(Struct);
}
