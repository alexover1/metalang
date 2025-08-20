/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

#include "metalang.h"
#include "metalang_tokenizer.cpp"
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

int main(int ArgCount, char **Args)
{
    if(ArgCount > 1)
    {
        for(int ArgIndex = 1; ArgIndex < ArgCount; ++ArgIndex)
        {
            char *FileName = Args[ArgIndex];

            if(StringsAreEqual(FileName, "-exec"))
            {
            }
            else if(StringsAreEqual(FileName, "-version"))
            {
                printf("Version: %s, built on %s\n", METALANG_VERSION, __DATE__);
            }
            else
            {
                entire_file ReadResult = ReadEntireFile(FileName);
                if(ReadResult.ContentsSize)
                {
                    tokenizer Tokenizer = Tokenize(BundleString(ReadResult.ContentsSize, (char *)ReadResult.Contents),
                                                   WrapZ(FileName));
                    parser *Parser = ParseTopLevelRoutines(Tokenizer);
                    Parser->Stream = stdout;
                    ParseAndGenerateProgram(Parser, Tokenizer);
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
