/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

struct win32_memory_block
{
    platform_memory_block Block;
    win32_memory_block *Prev;
    win32_memory_block *Next;
    u64 Flags;
};
