/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

Main()
{
    1 + 2 + arg;
    1 + arg + 2;
    arg + 1 + 2;
    arg - arg;

    2 == 3;
    2 != 3;
    2 > 3;
    2 >= 3;
    2 < 3;
    2 <= 3;

    arg == arg;
    arg != arg;
    arg > arg;
    arg >= arg;
    arg < arg;
    arg <= arg;

    arg == 2;
    arg != 2;
    arg > 2;
    arg >= 2;
    arg < 2;
    arg <= 2;

    !(arg == 1);

    -1;
    -0;
    !1;
    !0;
    !(1 == 2);

    !(arg == 1);
    !(arg > 1);
    !(arg >= 1);
}
