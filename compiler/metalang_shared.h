/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

#define U8Max 255
#define U16Max 65535
#define S32Min ((s32)0x80000000)
#define S32Max ((s32)0x7fffffff)
#define U32Min 0
#define U32Max ((u32)-1)
#define U64Max ((u64)-1)
#define F32Max FLT_MAX
#define F32Min -FLT_MAX

#define UMMFromPointer(Pointer) ((umm)(Pointer))
#define PointerFromUMM(type, Value) (type *)(Value)

#define U32FromPointer(Pointer) ((u32)(memory_index)(Pointer))
#define PointerFromU32(type, Value) (type *)((memory_index)Value)

#define OffsetOf(type, Member) (umm)&(((type *)0)->Member)

#define FILE_AND_LINE__(A, B) A "|" #B
#define FILE_AND_LINE_(A, B) FILE_AND_LINE__(A, B)
#define FILE_AND_LINE FILE_AND_LINE_(__FILE__, __LINE__)

#define Kilobytes(Value) ((u64)(Value) << 10)
#define Megabytes(Value) ((u64)(Value) << 20)
#define Gigabytes(Value) ((u64)(Value) << 30)
#define Terabytes(Value) ((u64)(Value) << 40)

#define Assert(Expression) assert(Expression) // if(!(Expression)) {*(int volatile *)0 = 0;}

#define AlignPow2(Value, Alignment) ((Value + ((Alignment) - 1)) & ~((Alignment) - 1))
#define Align4(Value) ((Value + 3) & ~3)
#define Align8(Value) ((Value + 7) & ~7)
#define Align16(Value) ((Value + 15) & ~15)

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

internal s32 RoundReal32ToInt32(f32 Real32)
{
    s32 Result = _mm_cvtss_si32(_mm_set_ss(Real32));
    return Result;
}

#define ConstZ(Z) {sizeof(Z) - 1, (u8 *)(Z)}
#define BundleZ(Z) BundleString(sizeof(Z) - 1, (Z))

#define CopyArray(Count, Source, Dest) Copy((Count)*sizeof(*(Source)), (Source), (Dest))
internal void *Copy(umm Size, void *SourceInit, void *DestInit)
{
    u8 *Source = (u8 *)SourceInit;
    u8 *Dest = (u8 *)DestInit;
    while(Size--) {*Dest++ = *Source++;}

    return(DestInit);
}

internal u32 StringLength(char *String)
{
    u32 Count = 0;
    if(String)
    {
        while(*String++)
        {
            ++Count;
        }
    }

    return Count;
}

internal string WrapZ(char *Z)
{
    string Result = {StringLength(Z), (u8 *)Z};

    return Result;
}

internal string BundleString(umm Count, char *Data)
{
    string Result = {Count, (u8 *)Data};

    return Result;
}

internal b32 StringsAreEqual(char *A, char *B)
{
    b32 Result = (A == B);

    if(A && B)
    {
        while(*A && *B && (*A == *B))
        {
            ++A;
            ++B;
        }

        Result = ((*A == 0) && (*B == 0));
    }

    return Result;
}

internal b32 StringsAreEqual(umm ALength, char *A, char *B)
{
    b32 Result = (ALength == 0);

    if(B)
    {
        char *At = B;
        for(umm Index = 0; Index < ALength; ++Index, ++At)
        {
            if ((*At == 0) ||
                (A[Index] != *At))
            {
                return Result;
            }
        }

        Result = (*At == 0);
    }

    return Result;
}

internal b32 StringsAreEqual(umm ALength, char *A, umm BLength, char *B)
{
    b32 Result = (ALength == BLength);

    if(Result)
    {
        for(umm Index = 0; Index < ALength; ++Index)
        {
            if(A[Index] != B[Index])
            {
                Result = false;
                break;
            }
        }
    }

    return Result;
}

internal b32 StringsAreEqual(string A, char *B)
{
    b32 Result = StringsAreEqual(A.Count, (char *)A.Data, B);

    return Result;
}

internal b32 StringsAreEqual(string A, string B)
{
    b32 Result = StringsAreEqual(A.Count, (char *)A.Data, B.Count, (char *)B.Data);

    return(Result);
}

internal u8 *Advance(string *Buffer, umm Count)
{
    u8 *Result = 0;

    if(Buffer->Count >= Count)
    {
        Result = Buffer->Data;
        Buffer->Data += Count;
        Buffer->Count -= Count;
    }
    else
    {
        Buffer->Data += Buffer->Count;
        Buffer->Count = 0;
    }

    return Result;
}

internal char ToLowercase(char C)
{
    char Result = C;

    if((Result >= 'A') && (Result <= 'Z'))
    {
        Result += 'a' - 'A';
    }

    return Result;
}

internal void UpdateStringHash(u32 *HashValue, char Value)
{
    // TODO(alex): Better hash function
    *HashValue = 65599*(*HashValue) + Value;
}

internal u32 StringHashOf(char *Z)
{
    u32 HashValue = 0;

    while(*Z)
    {
        UpdateStringHash(&HashValue, *Z++);
    }

    return HashValue;
}

internal u32 StringHashOf(string S)
{
    u32 HashValue = 0;

    for(umm Index = 0; Index < S.Count; ++Index)
    {
        UpdateStringHash(&HashValue, S.Data[Index]);
    }

    return HashValue;
}

internal b32 IsEndOfLine(char C)
{
    b32 Result = ((C == '\n') ||
                  (C == '\r'));

    return Result;
}

internal b32 IsSpacing(char C)
{
    b32 Result = ((C == ' ') ||
                  (C == '\t') ||
                  (C == '\v') ||
                  (C == '\f'));

    return Result;
}

internal b32 IsWhitespace(char C)
{
    b32 Result = (IsSpacing(C) || IsEndOfLine(C));

    return Result;
}

internal b32 IsAlpha(char C)
{
    b32 Result = (((C >= 'a') && (C <= 'z')) ||
                   ((C >= 'A') && (C <= 'Z')));

    return Result;
}

internal b32 IsNumber(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));

    return Result;
}

internal b32 IsHex(char Char)
{
    b32 Result = (((Char >= '0') && (Char <= '9')) ||
                  ((Char >= 'A') && (Char <= 'F')) ||
                  ((Char >= 'a') && (Char <= 'f')));

    return Result;
}

internal u32 GetHex(char Char)
{
    u32 Result = 0;

    if((Char >= '0') && (Char <= '9'))
    {
        Result = Char - '0';
    }
    else if((Char >= 'A') && (Char <= 'F'))
    {
        Result = 0xA + (Char - 'A');
    }
    else if((Char >= 'a') && (Char <= 'f'))
    {
        Result = 0xA + (Char - 'a');
    }

    return Result;
}

internal s32 S32FromZInternal(char **AtInit)
{
    s32 Result = 0;

    char *At = *AtInit;
    while((*At >= '0') &&
          (*At <= '9'))
    {
        Result *= 10;
        Result += (*At - '0');
        ++At;
    }

    *AtInit = At;

    return Result;
}

internal s32 S32FromZ(char *At)
{
    char *Ignored = At;
    s32 Result = S32FromZInternal(&Ignored);
    return Result;
}

struct format_dest
{
    umm Size;
    char *At;
};

internal void OutChar(format_dest *Dest, char Value)
{
    if(Dest->Size)
    {
        --Dest->Size;
        *Dest->At++ = Value;
    }
}

internal void OutChars(format_dest *Dest, char *Value)
{
    // NOTE(alex): Not particularly speedy, are we?
    while(*Value)
    {
        OutChar(Dest, *Value++);
    }
}

#define ReadVarArgUnsignedInteger(Length, ArgList) ((Length) == 8) ? va_arg(ArgList, u64) : (u64)va_arg(ArgList, u32)
#define ReadVarArgSignedInteger(Length, ArgList) ((Length) == 8) ? va_arg(ArgList, s64) : (s64)va_arg(ArgList, s32)
#define ReadVarArgFloat(Length, ArgList) va_arg(ArgList, f64)

global char DecChars[] = "0123456789";
global char LowerHexChars[] = "0123456789abcdef";
global char UpperHexChars[] = "0123456789ABCDEF";
internal void U64ToASCII(format_dest *Dest, u64 Value, u32 Base, char *Digits)
{
    Assert(Base != 0);

    char *Start = Dest->At;
    do
    {
        u64 DigitIndex = (Value % Base);
        char Digit = Digits[DigitIndex];
        OutChar(Dest, Digit);

        Value /= Base;
    } while(Value != 0);
    char *End = Dest->At;

    while(Start < End)
    {
        --End;
        char Temp = *End;
        *End = *Start;
        *Start = Temp;
        ++Start;
    }
}

internal void F64ToASCII(format_dest *Dest, f64 Value, u32 Precision)
{
    if(Value < 0)
    {
        OutChar(Dest, '-');
        Value = -Value;
    }

    u64 IntegerPart = (u64)Value;
    Value -= (f64)IntegerPart;
    U64ToASCII(Dest, IntegerPart, 10, DecChars);

    OutChar(Dest, '.');

    // TODO(alex): Note that this is NOT an accurate way to do this!
    for(u32 PrecisionIndex = 0; PrecisionIndex < Precision; ++PrecisionIndex)
    {
        Value *= 10.0f;
        u32 Integer = (u32)Value;
        Value -= (f32)Integer;
        OutChar(Dest, DecChars[Integer]);
    }
}

// NOTE(alex): Size returned __DOES NOT__ include the null terminator.
internal umm FormatStringList(umm DestSize, char *DestInit, char *Format, va_list ArgList)
{
    format_dest Dest = {DestSize, DestInit};
    if(Dest.Size)
    {
        char *At = Format;
        while(At[0])
        {
            if(*At == '%')
            {
                ++At;

                b32 ForceSign = false;
                b32 PadWithZeros = false;
                b32 LeftJustify = false;
                b32 PostiveSignIsBlank = false;
                b32 AnnotateIfNotZero = false;

                b32 Parsing = true;

                //
                // NOTE(alex): Handle the flags
                //
                while(Parsing)
                {
                    switch(*At)
                    {
                        case '+': {ForceSign = true;} break;
                        case '0': {PadWithZeros = true;} break;
                        case '-': {LeftJustify = true;} break;
                        case ' ': {PostiveSignIsBlank = true;} break;
                        case '#': {AnnotateIfNotZero = true;} break;
                        default: {Parsing = false;} break;
                    }

                    if(Parsing)
                    {
                        ++At;
                    }
                }

                //
                // NOTE(alex): Handle the width
                //
                b32 WidthSpecified = false;
                s32 Width = 0;
                if(*At == '*')
                {
                    Width = va_arg(ArgList, int);
                    WidthSpecified = true;
                    ++At;
                }
                else if((*At >= '0') && (*At <= '9'))
                {
                    Width = S32FromZInternal(&At);
                    WidthSpecified = true;
                }

                //
                // NOTE(alex): Handle the precision
                //
                b32 PrecisionSpecified = false;
                s32 Precision = 0;
                if(*At == '.')
                {
                    ++At;

                    if(*At == '*')
                    {
                        Precision = va_arg(ArgList, int);
                        PrecisionSpecified = true;
                        ++At;
                    }
                    else if((*At >= '0') && (*At <= '9'))
                    {
                        Precision = S32FromZInternal(&At);
                        PrecisionSpecified = true;
                    }
                    else
                    {
                        Assert(!"Malformed precision specifier!");
                    }
                }

                // TODO(alex): Right now our routine doesn't allow non-specified
                // precisions, so we just set non-specified precisions to a specified value
                if(!PrecisionSpecified)
                {
                    Precision = 6;
                }

                 //
                // NOTE(alex): Handle the length
                //
                u32 IntegerLength = 4;
                u32 FloatLength = 8;
                // TODO(alex): Actually set different values here!
                if((At[0] == 'h') && (At[1] == 'h'))
                {
                    At += 2;
                }
                else if((At[0] == 'l') && (At[1] == 'l'))
                {
                    At += 2;
                }
                else if(*At == 'h')
                {
                    ++At;
                }
                else if(*At == 'l')
                {
                    IntegerLength = 8;
                    ++At;
                }
                else if(*At == 'j')
                {
                    ++At;
                }
                else if(*At == 'z')
                {
                    ++At;
                }
                else if(*At == 't')
                {
                    ++At;
                }
                else if(*At == 'L')
                {
                    ++At;
                }

                char TempBuffer[64];
                char *Temp = TempBuffer;
                format_dest TempDest = {ArrayCount(TempBuffer), Temp};
                char *Prefix = "";
                b32 IsFloat = false;

                switch(*At)
                {
                    case 'd':
                    case 'i':
                    {
                        s64 Value = ReadVarArgSignedInteger(IntegerLength, ArgList);
                        b32 WasNegative = (Value < 0);
                        if(WasNegative)
                        {
                            Value = -Value;
                        }
                        U64ToASCII(&TempDest, (u64)Value, 10, DecChars);

                        // TODO(alex): Make this a common routine once floating
                        // point is available.
                        if(WasNegative)
                        {
                            Prefix = "-";
                        }
                        else if(ForceSign)
                        {
                            Assert(!PostiveSignIsBlank); // NOTE(alex): Not a problem here, but probably shouldn't be specified?
                            Prefix = "+";
                        }
                        else if(PostiveSignIsBlank)
                        {
                            Prefix = " ";
                        }
                    } break;

                    case 'u':
                    {
                        u64 Value = ReadVarArgUnsignedInteger(IntegerLength, ArgList);
                        U64ToASCII(&TempDest, Value, 10, DecChars);
                    } break;

                    case 'm':
                    {
                        // TODO(alex): Put in a fractional thing here...
                        umm Value = va_arg(ArgList, umm);
                        char *Suffix = "b ";
                        if(Value >= Gigabytes(1))
                        {
                            Suffix = "gb";
                            Value = (Value + Gigabytes(1) - 1) / Gigabytes(1);
                        }
                        else if(Value >= Megabytes(1))
                        {
                            Suffix = "mb";
                            Value = (Value + Megabytes(1) - 1) / Megabytes(1);
                        }
                        else if(Value >= Kilobytes(1))
                        {
                            Suffix = "kb";
                            Value = (Value + Kilobytes(1) - 1) / Kilobytes(1);
                        }
                        U64ToASCII(&TempDest, Value, 10, DecChars);
                        OutChars(&TempDest, Suffix);
                    } break;

                    case 'o':
                    {
                        u64 Value = ReadVarArgUnsignedInteger(IntegerLength, ArgList);
                        U64ToASCII(&TempDest, Value, 8, DecChars);
                        if(AnnotateIfNotZero && (Value != 0))
                        {
                            Prefix = "0";
                        }
                    } break;

                    case 'x':
                    {
                        u64 Value = ReadVarArgUnsignedInteger(IntegerLength, ArgList);
                        U64ToASCII(&TempDest, Value, 16, LowerHexChars);
                        if(AnnotateIfNotZero && (Value != 0))
                        {
                            Prefix = "0x";
                        }
                    } break;

                    case 'X':
                    {
                        u64 Value = ReadVarArgUnsignedInteger(IntegerLength, ArgList);
                        U64ToASCII(&TempDest, Value, 16, UpperHexChars);
                        if(AnnotateIfNotZero && (Value != 0))
                        {
                            Prefix = "0X";
                        }
                    } break;

                    // TODO(alex): Support other kinds of floating point prints
                    // (right now we only do basic decimal output)
                    case 'f':
                    case 'F':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'a':
                    case 'A':
                    {
                        f64 Value = ReadVarArgFloat(FloatLength, ArgList);
                        F64ToASCII(&TempDest, Value, Precision);
                        IsFloat = true;
                    } break;

                    case 'c':
                    {
                        int Value = va_arg(ArgList, int);
                        OutChar(&TempDest, (char)Value);
                    } break;

                    case 's':
                    {
                        char *String = va_arg(ArgList, char *);

                        // TODO(alex): Obey precision, width, etc.

                        Temp = String;
                        if(PrecisionSpecified)
                        {
                            TempDest.Size = 0;
                            for(char *Scan = String;
                                *Scan && (TempDest.Size < Precision);
                                ++Scan)
                            {
                                ++TempDest.Size;
                            }
                        }
                        else
                        {
                            TempDest.Size = StringLength(String);
                        }
                        TempDest.At = String + TempDest.Size;
                    } break;

                    case 'S':
                    {
                        string String = va_arg(ArgList, string);

                        // TODO(alex): Obey precision, width, etc.

                        Temp = (char *)String.Data;
                        TempDest.Size = String.Count;
                        if(PrecisionSpecified && (TempDest.Size > Precision))
                        {
                            TempDest.Size = Precision;
                        }
                        TempDest.At = Temp + TempDest.Size;
                    } break;

                    case 'p':
                    {
                        void *Value = va_arg(ArgList, void *);
                        U64ToASCII(&TempDest, *(umm *)&Value, 16, LowerHexChars);
                    } break;

                    case 'n':
                    {
                        int *TabDest = va_arg(ArgList, int *);
                        *TabDest = (int)(Dest.At - DestInit);
                    } break;

                    case '%':
                    {
                        OutChar(&Dest, '%');
                    } break;

                    default:
                    {
                        Assert(!"Unrecognized format specifier");
                    } break;
                }

                if(TempDest.At - Temp)
                {
                    smm UsePrecision = Precision;
                    if(IsFloat || !PrecisionSpecified)
                    {
                        UsePrecision = (TempDest.At - Temp);
                    }

                    smm PrefixLength = StringLength(Prefix);
                    smm UseWidth = Width;
                    smm ComputedWidth = UsePrecision + PrefixLength;
                    if(UseWidth < ComputedWidth)
                    {
                        UseWidth = ComputedWidth;
                    }

                    if(PadWithZeros)
                    {
                        Assert(!LeftJustify); // NOTE(alex): Not a problem, but no way to do it?
                        LeftJustify = false;
                    }

                    if(!LeftJustify)
                    {
                        while(UseWidth > (UsePrecision + PrefixLength))
                        {
                            OutChar(&Dest, PadWithZeros ? '0' : ' ');
                            --UseWidth;
                        }
                    }

                    for(char *Pre = Prefix; *Pre && UseWidth; ++Pre)
                    {
                        OutChar(&Dest, *Pre);
                        --UseWidth;
                    }

                    if(UsePrecision > UseWidth)
                    {
                        UsePrecision = UseWidth;
                    }
                    while(UsePrecision > (TempDest.At - Temp))
                    {
                        OutChar(&Dest, '0');
                        --UsePrecision;
                        --UseWidth;
                    }
                    while(UsePrecision && (TempDest.At != Temp))
                    {
                        OutChar(&Dest, *Temp++);
                        --UsePrecision;
                        --UseWidth;
                    }

                    if(LeftJustify)
                    {
                        while(UseWidth)
                        {
                            OutChar(&Dest, ' ');
                            --UseWidth;
                        }
                    }
                }

                if(*At)
                {
                    ++At;
                }
            }
            else
            {
                OutChar(&Dest, *At++);
            }
        }

        if(Dest.Size)
        {
            Dest.At[0] = 0;
        }
        else
        {
            Dest.At[-1] = 0;
        }
    }

    umm Result = Dest.At - DestInit;
    return Result;
}

// TODO(alex): Eventually, make this return a string struct
internal umm FormatString(umm DestSize, char *Dest, char *Format, ...)
{
    va_list ArgList;
    va_start(ArgList, Format);

    umm Result = FormatStringList(DestSize, Dest, Format, ArgList);

    va_end(ArgList);

    return(Result);
}
