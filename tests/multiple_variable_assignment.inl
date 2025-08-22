/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

Main()
{
    s32 A = 1;
    s32 B = 2;
    s32 C;
    {
        s32 B = 3;
        C = A + B;
    }
    s32 D = A + B;
}
