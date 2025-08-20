/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

#include "metalang.h"

#include <windows.h>
#include <stdarg.h>
#include <intrin.h>
#include <assert.h>

#include "metalang_shared.h"
#include "metalang_platform.h"

#include "win32_metalang.h"

global ticket_mutex GlobalMemoryMutex;
global win32_memory_block GlobalMemorySentinel =
{
    {},
    &GlobalMemorySentinel,
    &GlobalMemorySentinel,
};

PLATFORM_ALLOCATE_MEMORY(Win32AllocateMemory)
{
    // NOTE(alex): We require memory block headers not to change the cache
    // line alignment of an allocation
    Assert(sizeof(win32_memory_block) == 64);

    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);

    umm PageSize = SystemInfo.dwPageSize;
    umm TotalSize = Size + sizeof(win32_memory_block);
    umm BaseOffset = sizeof(win32_memory_block);
    umm ProtectOffset = 0;
    if(Flags & PlatformMemory_UnderflowCheck)
    {
        TotalSize = Size + 2*PageSize;
        BaseOffset = 2*PageSize;
        ProtectOffset = PageSize;
    }
    else if(Flags & PlatformMemory_OverflowCheck)
    {
        umm SizeRoundedUp = AlignPow2(Size, PageSize);
        TotalSize = SizeRoundedUp + 2*PageSize;
        BaseOffset = PageSize + SizeRoundedUp - Size;
        ProtectOffset = PageSize + SizeRoundedUp;
    }

    win32_memory_block *Block = (win32_memory_block *)
        VirtualAlloc(0, TotalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Assert(Block);
    Block->Block.Base = (u8 *)Block + BaseOffset;
    Assert(Block->Block.Used == 0);
    Assert(Block->Block.ArenaPrev == 0);

    if(Flags & (PlatformMemory_UnderflowCheck|PlatformMemory_OverflowCheck))
    {
        DWORD OldProtect = 0;
        BOOL Protected = VirtualProtect((u8 *)Block + ProtectOffset, PageSize, PAGE_NOACCESS, &OldProtect);
        Assert(Protected);
    }

    win32_memory_block *Sentinel = &GlobalMemorySentinel;
    Block->Next = Sentinel;
    Block->Block.Size = Size;
    Block->Block.Flags = Flags;
    Block->Flags = 0;

    BeginTicketMutex(&GlobalMemoryMutex);
    Block->Prev = Sentinel->Prev;
    Block->Prev->Next = Block;
    Block->Next->Prev = Block;
    EndTicketMutex(&GlobalMemoryMutex);

    platform_memory_block *PlatBlock = &Block->Block;
    return PlatBlock;
}

internal void Win32FreeMemoryBlock(win32_memory_block *Block)
{
    BeginTicketMutex(&GlobalMemoryMutex);
    Block->Prev->Next = Block->Next;
    Block->Next->Prev = Block->Prev;
    EndTicketMutex(&GlobalMemoryMutex);

    // NOTE(alex): For porting to other platforms that need the size to unmap
    // pages, you can get it from Block->Block.Size!

    BOOL Result = VirtualFree(Block, 0, MEM_RELEASE);
    Assert(Result);
}

PLATFORM_DEALLOCATE_MEMORY(Win32DeallocateMemory)
{
    if(Block)
    {
        win32_memory_block *Win32Block = ((win32_memory_block *)Block);
        Win32FreeMemoryBlock(Win32Block);
    }
}

platform_api Platform =
{
    Win32AllocateMemory,
    Win32DeallocateMemory,
};
