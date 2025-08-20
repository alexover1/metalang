/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

#include "metalang_defines.h"

typedef char unsigned u8;
typedef short unsigned u16;
typedef int unsigned u32;
typedef long long unsigned u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

typedef s32 b32;

typedef float f32;
typedef double f64;

#if ARCH_64BIT
typedef u64 umm;
typedef s64 smm;
#else
typedef u32 umm;
typedef s32 smm;
#endif

struct ticket_mutex
{
    u64 volatile Ticket;
    u64 volatile Serving;
};

struct buffer
{
    umm Count;
    u8 *Data;
};
typedef buffer string;

#define internal static
#define global static
#define local_persist static

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Stringify_(x) #x
#define Stringify(x) Stringify_(x)

#define Version(Major, Minor, Patch) (u64)((((u64)(Major) & 0xffff) << 32) | ((((u64)(Minor) & 0xffff) << 16)) | ((((u64)(Patch) & 0xffff) << 0)))
#define MajorFromVersion(Version) (((Version) & 0xffff00000000ull) >> 32)
#define MinorFromVersion(Version) (((Version) & 0x0000ffff0000ull) >> 16)
#define PatchFromVersion(Version) (((Version) & 0x00000000ffffull) >> 0)

#define METALANG_VERSION_RELEASE "alpha"
#define METALANG_VERSION_MAJOR   0
#define METALANG_VERSION_MINOR   1
#define METALANG_VERSION_PATCH   001

global u32 const METALANG_VERSION = Version(METALANG_VERSION_MAJOR, METALANG_VERSION_MINOR, METALANG_VERSION_PATCH);
global char const *METALANG_VERSION_STRING = METALANG_VERSION_RELEASE " " Stringify(METALANG_VERSION_MAJOR) "." Stringify(METALANG_VERSION_MINOR) "." Stringify(METALANG_VERSION_PATCH);
