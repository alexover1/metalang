/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

/*
   Example program demonstrating the expression-based printing system
   in this language.

   This language automatically prints expressions that appear as
   standalone statements. This makes simple output extremely concise
   and easy to read, without needing explicit "print" calls.
*/

Main()
{
    // To print something to the console in this language,
    // you write the value as a single statement.
    "Hello, Sailor!";

    // You can combine multiple expressions together.
    "My favorite number is ", 153, ".";

    // You can put any expression here, and its value will be printed.
    // "The square of 5 is ", 5*5;

    // No extra characters are inserted between arguments, which means
    // you can easily concatenate numbers.
    "Concatenated: ", 1, 2, 3;
}
