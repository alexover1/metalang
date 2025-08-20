/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

Main(string Args[])
{
    if(Args.Count)
    {
        u32 Value = Fibonacci(ParseU32(Args[0]));
        PrintLine(ToString(Value));
    }
}

u32 Fibonacci(u32 N)
{
    Result = N;
    if(N > 1)
    {
        s32 First = 1;
        s32 Second = 0;
        for(u32 Index = 0; Index <= N; ++Index)
        {
            Result = First + Second;
            Second = First;
            First = Result;
        }
    }
}
