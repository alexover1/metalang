/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

EmptyIF()
{
    s32 X = 67;
    if(arg)
    {
        // Nothing!
    }
    else
    {
        X = 89;
    }
    X;
}

EmptyELSE()
{
    s32 X = 54;
    if(arg)
    {
        X = 32;
    }
    else
    {
        // Nothing!
    }
    X;
}

MergedIgnoreOriginal()
{
    s32 X = 12; // This value won't ever be used.
    if(arg)
    {
        X = 34;
    }
    else
    {
        X = 56;
    }
    X;
}
