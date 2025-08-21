/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

#include "metalang.h"

/* NOTE(alex): _CRT_SECURE_NO_WARNINGS is here because otherwise we cannot
   call fopen(). If we replace fopen() with fopen_s() to avoid the warning,
   then the code doesn't compile on Linux anymore, since fopen_s() does not
   exist there.

   What exactly the CRT maintainers were thinking when they made this choice,
   I have no idea.
*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <intrin.h>

#include "metalang_platform.h"
#include "metalang_shared.h"
#include "metalang_memory.h"
#include "metalang_tokenizer.h"
#include "metalang_node.h"
#include "metalang_parser.h"

#include "metalang_tokenizer.cpp"
#include "metalang_node.cpp"
#include "metalang_parser.cpp"

struct entire_file
{
    u32 ContentsSize;
    void *Contents;
};
internal entire_file ReadEntireFile(char *FileName)
{
    entire_file Result = {};

    FILE *In = fopen(FileName, "rb");
    if(In)
    {
        fseek(In, 0, SEEK_END);
        Result.ContentsSize = ftell(In);
        fseek(In, 0, SEEK_SET);

        Result.Contents = malloc(Result.ContentsSize);
        fread(Result.Contents, Result.ContentsSize, 1, In);
        fclose(In);
    }
    else
    {
        fprintf(stderr, "Error: Cannot open file \"%s\"\n", FileName);
    }

    return Result;
}

internal void ShowAvailableArguments(void)
{
    fprintf(stderr, "Available arguments:\n\n");
    fprintf(stderr, "-exec            Executes the program immediately after compiling.\n");
    fprintf(stderr, "-version         Print the version of the compiler.\n");
}

int main(int ArgCount, char **Args)
{
    SetDefaultFPBehavior();

    if(ArgCount > 1)
    {
        for(int ArgIndex = 1; ArgIndex < ArgCount; ++ArgIndex)
        {
            char *FileName = Args[ArgIndex];

            if(StringsAreEqual(FileName, "-exec"))
            {
            }
            else if(StringsAreEqual(FileName, "-help"))
            {
                ShowAvailableArguments();
            }
            else if(StringsAreEqual(FileName, "-version"))
            {
                fprintf(stderr, "Version: %s, built on %s\n", METALANG_VERSION_STRING, __DATE__);
            }
            else
            {
                entire_file ReadResult = ReadEntireFile(FileName);
                if(ReadResult.ContentsSize)
                {
                    tokenizer Tokenizer = Tokenize(BundleString(ReadResult.ContentsSize, (char *)ReadResult.Contents),
                                                   WrapZ(FileName));
                    parser *Parser = ParseTopLevelRoutines(Tokenizer);
                    Parser->Stream = fopen("test.asm", "wb");
                    ParseAndGenerateProgram(Parser, Tokenizer);
                    fclose(Parser->Stream);
                }
            }
        }
    }
    else
    {
        fprintf(stderr, "Usage: %s [input file] ...\n", Args[0]);
    }

    return 0;
}
