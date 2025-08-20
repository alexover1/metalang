/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

// TODO(alex): Add support for TCC? Except that TCC doesn't support C++...

//
// NOTE(alex): Clang OS/Arch cracking
//
#if defined(__clang__)

#define COMPILER_CLANG 1

#if defined(__WIN32)
#define OS_WINDOWS 1
#elif defined(__gnu_linux__) || defined(__linux__)
#define OS_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
#define OS_MAC 1
#else
#error This compiler/OS combo is not supported
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#define ARCH_X64 1
#elif defined(i386) || defined(__i386) || defined(__i386__)
#define ARCH_X86 1
#elif defined(__aarch64__)
#define ARCH_ARM64 1
#elif defined(__arm__)
#define ARCH_ARM32 1
#else
#error This architecture is not supported
#endif

//
// NOTE(alex): MSVC OS/Arch cracking
//
#elif defined(_MSC_VER)

#define COMPILER_MSVC 1

#if defined(_WIN32)
#define OS_WINDOWS 1
#else
#error This compiler/OS combo is not supported
#endif

#if defined(_M_AMD64)
#define ARCH_X64 1
#elif defined(_M_IX86)
#define ARCH_X86 1
#elif defined(_M_ARM64)
#define ARCH_ARM64 1
#elif defined(_M_ARM)
#define ARCH_ARM32 1
#else
#error This architecture is not supported
#endif

//
// NOTE(alex): GCC OS/Arch cracking
//
#elif defined(__GNUC__) || defined(__GNUG__)

#define COMPILER_GCC 1

#if defined(__gnu_linux__) || defined(__linux__)
#define OS_LINUX 1
#else
#error This compiler/OS combo is not supported
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#define ARCH_X64 1
#elif defined(i386) || defined(__i386) || defined(__i386__)
#define ARCH_X86 1
#elif defined(__aarch64__)
#define ARCH_ARM64 1
#elif defined(__arm__)
#define ARCH_ARM32 1
#else
#error This architecture is not supported
#endif

//
// NOTE(alex): Other OS/Arch cracking
//
#else
#error This compiler is not supported
#endif

//
// NOTE(alex): Arch cracking
//
#if defined(ARCH_X64) || defined(ARCH_ARM64)
#define ARCH_64BIT 1
#elif defined(ARCH_X86) || defined(ARCH_ARM32)
#define ARCH_32BIT 1
#endif

//
// NOTE(alex): Language cracking
//
#if defined(__cplusplus)
#define LANG_CPP 1
#else
#define LANG_C 1
#endif

//
// NOTE(alex): Zero all undefined options
//

#if !defined(ARCH_32BIT)
#define ARCH_32BIT 0
#endif
#if !defined(ARCH_64BIT)
#define ARCH_64BIT 0
#endif
#if !defined(ARCH_X64)
#define ARCH_X64 0
#endif
#if !defined(ARCH_X86)
#define ARCH_X86 0
#endif
#if !defined(ARCH_ARM64)
#define ARCH_ARM64 0
#endif
#if !defined(ARCH_ARM32)
#define ARCH_ARM32 0
#endif
#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif
#if !defined(COMPILER_GCC)
#define COMPILER_GCC 0
#endif
#if !defined(COMPILER_CLANG)
#define COMPILER_CLANG 0
#endif
#if !defined(OS_WINDOWS)
#define OS_WINDOWS 0
#endif
#if !defined(OS_LINUX)
#define OS_LINUX 0
#endif
#if !defined(OS_MAC)
#define OS_MAC 0
#endif
#if !defined(LANG_CPP)
#define LANG_CPP 0
#endif
#if !defined(LANG_C)
#define LANG_C 0
#endif
