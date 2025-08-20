/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

internal void DebugToken(token Token)
{
    local_persist char Buffer[Kilobytes(4)];
    FormatString(sizeof(Buffer), Buffer, "%S(%u,%u): \"%S\" - %S", Token.FileName, Token.LineNumber, Token.ColumnNumber, Token.Text, GetTokenTypeName(Token.Type));
    puts(Buffer);
}

internal void DebugTokens(tokenizer *Tokenizer)
{
    while(Parsing(Tokenizer))
    {
        token Token = GetToken(Tokenizer);
        DebugToken(Token);
        if(Token.Type == Token_EndOfStream)
        {
            break;
        }
    }
}

internal b32 SkipBalancedBlock(tokenizer *Tokenizer, token_type OpenType, token_type CloseType)
{
    b32 Result = false;

    u32 Depth = 1;
    while(Parsing(Tokenizer))
    {
        token Token = GetToken(Tokenizer);
        if(Token.Type == Token_EndOfStream)
        {
            break;
        }

        if(Token.Type == OpenType)
        {
            ++Depth;
        }
        else if(Token.Type == CloseType)
        {
            --Depth;
            if(Depth == 0)
            {
                Result = true;
                break;
            }
        }
    }

    return Result;
}

internal type_id TypeIDFromToken(token Token)
{
    type_id Result = {StringHashOf(Token.Text)};

    return(Result);
}

internal parser *ParseTopLevelRoutines(tokenizer Tokenizer_)
{
    tokenizer *Tokenizer = &Tokenizer_;

    parser *Parser = (parser *)calloc(1, sizeof(parser));
    routine_definition *Sentinel = &Parser->RoutineSentinel;
    Sentinel->Prev = Sentinel->Next = Sentinel;

    while(Parsing(Tokenizer))
    {
        token Token = GetToken(Tokenizer);
        if(Token.Type == Token_EndOfStream)
        {
            break;
        }

        if(Token.Type == Token_Identifier)
        {
            token TypeToken = {};
            token NameToken = Token;

            token Token = PeekToken(Tokenizer);
            if(Token.Type == Token_Identifier)
            {
                TypeToken = NameToken;
                NameToken = GetToken(Tokenizer);
            }

            b32 GotParameterList = false;
            if(OptionalToken(Tokenizer, Token_OpenParen))
            {
                GotParameterList = SkipBalancedBlock(Tokenizer, Token_OpenParen, Token_CloseParen);
            }

            if(GotParameterList && OptionalToken(Tokenizer, Token_OpenBrace))
            {
                SkipBalancedBlock(Tokenizer, Token_OpenBrace, Token_CloseBrace);
            }

            routine_definition Routine = {};
            Routine.TypeToken = TypeToken;
            Routine.NameToken = NameToken;

            routine_definition *Result = 0;

            u32 HashValue = StringHashOf(Routine.NameToken.Text);
            u32 HashIndex = HashValue & (ArrayCount(Parser->RoutineHash) - 1);

            routine_definition **HashSlot = Parser->RoutineHash + HashIndex;
            for(routine_definition *Search = *HashSlot; Search; Search = Search->NextInHash)
            {
                if(StringsAreEqual(Search->NameToken.Text, Routine.NameToken.Text))
                {
                    Result = Search;
                    break;
                }
            }

            if(!Result)
            {
                Result = (routine_definition *)malloc(sizeof(routine_definition));
                *Result = Routine;
                Result->NameHash = HashValue;
                Result->Next = Sentinel;
                Result->Prev = Sentinel->Prev;
                Result->Prev->Next = Result;
                Result->Next->Prev = Result;
                Result->NextInHash = *HashSlot;
                *HashSlot = Result;
            }
        }
    }

#if 0
    for(routine_definition *Routine = Sentinel->Next;
        Routine != Sentinel;
        Routine = Routine->Next)
    {
        printf("----------------------------------------\n");
        if(IsValid(Routine->TypeToken))
        {
            DebugToken(Routine->TypeToken);
        }
        DebugToken(Routine->NameToken);
        printf("----------------------------------------\n");
    }
#endif

    return Parser;
}

internal variable_definition *AddVariable(parser *Parser, string Name, u32 Size)
{
    Assert(Parser->VariableCount < ArrayCount(Parser->Variables));

    variable_definition *Variable = Parser->Variables + Parser->VariableCount++;
    Variable->Name = Name;
    Variable->NameHash = StringHashOf(Name);
    Variable->Size = Size;
    Variable->Offset = Parser->StackSize;
    Parser->StackSize += Align16(Size);

    return Variable;
}

internal variable_definition *GetVariable(parser *Parser, string Name)
{
    u32 NameHash = StringHashOf(Name);

    variable_definition *Result = 0;
    for(u32 VariableIndex = 0;
        VariableIndex < Parser->VariableCount;
        ++VariableIndex)
    {
        variable_definition *TestVariable = Parser->Variables + (Parser->VariableCount - VariableIndex - 1);
        if((TestVariable->NameHash == NameHash) &&
           StringsAreEqual(TestVariable->Name, Name))
        {
            Result = TestVariable;
            break;
        }
    }

    return Result;
}

internal void GenerateExpression(parser *Parser, tokenizer *Tokenizer, token Token)
{
    while(Parsing(Tokenizer))
    {
        switch(Token.Type)
        {
            case Token_Identifier:
            {
                variable_definition *Variable = GetVariable(Parser, Token.Text);
                if(Variable)
                {
                    if(Variable->Size == 8)
                    {
                        fprintf(Parser->Stream, "mov rcx, qword [rbp-%u]\n", Variable->Offset);
                    }
                    else if(Variable->Size == 4)
                    {
                        fprintf(Parser->Stream, "mov ecx, dword [rbp-%u]\n", Variable->Offset);
                    }
                    else if(Variable->Size == 2)
                    {
                        fprintf(Parser->Stream, "movzx rcx, word [rbp-%u]\n", Variable->Offset);
                    }
                    else if(Variable->Size == 1)
                    {
                        fprintf(Parser->Stream, "movzx rcx, byte [rbp-%u]\n", Variable->Offset);
                    }
                    else
                    {
                        Assert(!"Invalid variable size");
                    }
                    fprintf(Parser->Stream, "call _PrintU64\n");
                }
                else
                {
                    Error(Tokenizer, Token, "Undeclared variable");
                }
            } break;

            case Token_Number:
            {
                fprintf(Parser->Stream, "mov rcx, %d\n", Token.S32);
                fprintf(Parser->Stream, "call _PrintU64\n");
            } break;

            case Token_String:
            {
                u8 *Data = Token.Text.Data;
                u32 DataSize = Token.Text.Count;

                // NOTE(alex): Technically, we only need BytesWritten number of bytes,
                // but we can't assert after the loop because then we might write off
                // the end before triggering this assert.
                Assert((Parser->DataSize + DataSize) <= ArrayCount(Parser->DataSegment));

                u8 *At = Parser->DataSegment + Parser->DataSize;
                for(u32 DataIndex = 0; DataIndex < DataSize; ++DataIndex)
                {
                    if(Data[DataIndex] == '\\')
                    {
                        ++DataIndex;
                        switch(Data[DataIndex])
                        {
                            case '\\': {*At++ = '\\';}; break;

                            case '0': {*At++ = '\0';}; break;
                            case 'n': {*At++ = '\n';}; break;
                            case 'r': {*At++ = '\r';}; break;
                            case 'v': {*At++ = '\v';}; break;

                            default: {*At++ = Data[DataIndex];}; break;
                        }
                    }
                    else
                    {
                        *At++ = Data[DataIndex];
                    }
                }

                u32 BytesWritten = At - (Parser->DataSegment + Parser->DataSize);

                u32 DataOffset = Parser->DataSize;
                Parser->DataSize += BytesWritten;
                *(Parser->DataSegment + Parser->DataSize++) = 0;

                fprintf(Parser->Stream, "lea rcx, [program_data+%u]\n", DataOffset);
                fprintf(Parser->Stream, "mov edx, %u\n", BytesWritten);
                fprintf(Parser->Stream, "call _PrintString\n");
            } break;

            default:
            {
                Error(Tokenizer, Token, "Illegal expression");
            } break;
        }

        if(Parsing(Tokenizer))
        {
            token NextToken = GetToken(Tokenizer);
            if(NextToken.Type == Token_Comma)
            {
                Token = GetToken(Tokenizer);
            }
            else if(NextToken.Type == Token_Semicolon)
            {
                fprintf(Parser->Stream, "mov rcx, %d\n", '\n');
                fprintf(Parser->Stream, "call _PrintChar\n");
                break;
            }
            else
            {
                Error(Tokenizer, NextToken, "Unexpected token type (expected %.*s or %.*s)",
                      ExpandString(GetTokenTypeName(Token_Comma)), ExpandString(GetTokenTypeName(Token_Semicolon)));
                break;
            }
        }
    }
}

internal void GenerateBlock(parser *Parser, tokenizer *Tokenizer)
{
    u32 SavedVariableCount = Parser->VariableCount;

    while(Parsing(Tokenizer))
    {
        token Token = GetToken(Tokenizer);
        if((Token.Type == Token_EndOfStream) ||
           (Token.Type == Token_CloseBrace))
        {
            break;
        }

        if(Token.Type == Token_Identifier)
        {
            token NameToken = Token;
            token Token = PeekToken(Tokenizer);

            switch(Token.Type)
            {
                case Token_Identifier:
                {
                    token TypeToken = NameToken;
                    NameToken = GetToken(Tokenizer);

                    Assert(StringsAreEqual(TypeToken.Text, "s32"));

                    variable_definition *Variable = AddVariable(Parser, NameToken.Text, 4);
                    fprintf(Parser->Stream, "sub rsp, %u ; %.*s %.*s\n", Align16(Variable->Size),
                            ExpandString(TypeToken.Text), ExpandString(NameToken.Text));

                    for(u32 VariableIndex = SavedVariableCount;
                        VariableIndex < (Parser->VariableCount - 1);
                        ++VariableIndex)
                    {
                        variable_definition *TestVariable = Parser->Variables + VariableIndex;
                        if((Variable->NameHash == TestVariable->NameHash) &&
                           StringsAreEqual(Variable->Name, TestVariable->Name))
                        {
                            Error(Tokenizer, NameToken, "Redeclaration of variable");
//                            Error(Tokenizer, NameToken, "Original variable was declared here");
                        }
                    }

                    if(OptionalToken(Tokenizer, Token_Semicolon))
                    {
                        if(Variable->Size == 8)
                        {
                            fprintf(Parser->Stream, "mov qword [rbp-%u], 0\n", Variable->Offset);
                        }
                        else if(Variable->Size == 4)
                        {
                            fprintf(Parser->Stream, "mov dword [rbp-%u], 0\n", Variable->Offset);
                        }
                        else if(Variable->Size == 2)
                        {
                            fprintf(Parser->Stream, "mov word [rbp-%u], 0\n", Variable->Offset);
                        }
                        else if(Variable->Size == 1)
                        {
                            fprintf(Parser->Stream, "mov byte [rbp-%u], 0\n", Variable->Offset);
                        }
                        else
                        {
                            // TODO(alex): Memset intrinsic
                            Assert(!"Invalid variable size");
                        }
                    }
                    else
                    {
                        token Token = RequireToken(Tokenizer, Token_Equals);
                        if(Token.Type == Token_Equals)
                        {
                            token Token = RequireToken(Tokenizer, Token_Number);
                            if(Token.Type == Token_Number)
                            {
                                fprintf(Parser->Stream, "mov dword [rbp-%u], %d ; %.*s = %d\n", Variable->Offset, Token.S32,
                                        ExpandString(Variable->Name), Token.S32);
                            }

                            RequireToken(Tokenizer, Token_Semicolon);
                        }
                    }
                } break;

                case Token_Equals:
                {
                    GetToken(Tokenizer);

                    string Name = NameToken.Text;
                    u32 NameHash = StringHashOf(Name);

                    variable_definition *Variable = 0;
                    for(u32 VariableIndex = 0;
                        VariableIndex < Parser->VariableCount;
                        ++VariableIndex)
                    {
                        variable_definition *TestVariable = Parser->Variables + (Parser->VariableCount - VariableIndex - 1);
                        if((TestVariable->NameHash == NameHash) &&
                           StringsAreEqual(TestVariable->Name, Name))
                        {
                            Variable = TestVariable;
                            break;
                        }
                    }

                    token Token = RequireToken(Tokenizer, Token_Number);
                    if(Token.Type == Token_Number)
                    {
                        if(Variable)
                        {
                            fprintf(Parser->Stream, "mov dword [rbp-%u], %d ; %.*s = %d\n", Variable->Offset, Token.S32,
                                    ExpandString(Variable->Name), Token.S32);
                        }

                        RequireToken(Tokenizer, Token_Semicolon);
                    }

                    if(!Variable)
                    {
                        Error(Tokenizer, NameToken, "Undeclared variable");
                    }
                } break;

                default:
                {
                    GenerateExpression(Parser, Tokenizer, NameToken);
                } break;
            }
        }
        else
        {
            GenerateExpression(Parser, Tokenizer, Token);
        }
    }

    Parser->VariableCount = SavedVariableCount;
}

internal void ParseAndGenerateProgram(parser *Parser, tokenizer Tokenizer_)
{
    tokenizer *Tokenizer = &Tokenizer_;

    string EntryName = ConstZ("Main");
    u32 EntryHash = StringHashOf(EntryName);

    routine_definition *Sentinel = &Parser->RoutineSentinel;

    routine_definition *EntryPoint = 0;
    for(routine_definition *Routine = Sentinel->Next;
        Routine != Sentinel;
        Routine = Routine->Next)
    {
        if((Routine->NameHash == EntryHash) &&
           StringsAreEqual(Routine->NameToken.Text, EntryName))
        {
            EntryPoint = Routine;
            break;
        }
    }

#if 0
    if(!EntryPoint)
    {
        // TODO(alex): Make the error printing routine more flexible so we don't have to hardcode this!
        fprintf(stderr, "\x1b[1;31m%.*s\x1b[0m: Missing program entry point (entry point name is \"%.*s\")\n",
                ExpandString(Tokenizer->FileName), ExpandString(EntryName));
    }
#endif

    fprintf(Parser->Stream, "; %.*s disassembly:\n", ExpandString(Tokenizer->FileName));
    fprintf(Parser->Stream, "format PE64 console\n");
    fprintf(Parser->Stream, "entry _ProgramMain\n");
    fprintf(Parser->Stream, "section '.text' code readable executable\n");

    while(Parsing(Tokenizer))
    {
        if(OptionalToken(Tokenizer, Token_EndOfStream))
        {
            break;
        }

        token Token = RequireToken(Tokenizer, Token_Identifier);
        if(Token.Type == Token_Identifier)
        {
            token TypeToken = {};
            token NameToken = Token;

            token Token = PeekToken(Tokenizer);
            if(Token.Type == Token_Identifier)
            {
                TypeToken = NameToken;
                NameToken = GetToken(Tokenizer);
            }

            b32 GotParameterList = false;
            if(OptionalToken(Tokenizer, Token_OpenParen))
            {
                GotParameterList = SkipBalancedBlock(Tokenizer, Token_OpenParen, Token_CloseParen);
            }

            if(GotParameterList && OptionalToken(Tokenizer, Token_OpenBrace))
            {
                fprintf(Parser->Stream, "_%.*s:\n", ExpandString(NameToken.Text));
                fprintf(Parser->Stream, "push rbp\n");
                fprintf(Parser->Stream, "mov rbp, rsp\n");

                GenerateBlock(Parser, Tokenizer);

                fprintf(Parser->Stream, "mov rsp, rbp\n");
                fprintf(Parser->Stream, "pop rbp\n");
                fprintf(Parser->Stream, "ret\n");
            }
        }
    }

    fprintf(Parser->Stream, "; npad <n>\n");
    fprintf(Parser->Stream, "macro npad n {\n");
    fprintf(Parser->Stream, "repeat n\n");
    fprintf(Parser->Stream, "nop\n");
    fprintf(Parser->Stream, "end repeat\n");
    fprintf(Parser->Stream, "}\n");

    fprintf(Parser->Stream, "; void _PrintString(char *Data, u32 Size)\n");
    fprintf(Parser->Stream, "_PrintString:\n");
    fprintf(Parser->Stream, "mov     [rsp+8], rbx\n");
    fprintf(Parser->Stream, "push    rdi\n");
    fprintf(Parser->Stream, "sub     rsp, 48\n");
    fprintf(Parser->Stream, "mov     rdi, rcx\n");
    fprintf(Parser->Stream, "mov     rbx, rdx\n");
    fprintf(Parser->Stream, "mov     ecx, -11\n");
    fprintf(Parser->Stream, "call    [GetStdHandle]\n");
    fprintf(Parser->Stream, "xor     r9d, r9d\n");
    fprintf(Parser->Stream, "mov     qword [rsp+32], 0\n");
    fprintf(Parser->Stream, "mov     rcx, rax\n");
    fprintf(Parser->Stream, "mov     r8d, ebx\n");
    fprintf(Parser->Stream, "mov     rdx, rdi\n");
    fprintf(Parser->Stream, "call    [WriteFile]\n");
    fprintf(Parser->Stream, "mov     rbx, [rsp+64]\n");
    fprintf(Parser->Stream, "add     rsp, 48\n");
    fprintf(Parser->Stream, "pop     rdi\n");
    fprintf(Parser->Stream, "ret     0\n");

    fprintf(Parser->Stream, "; void _PrintU64(u64 Number)\n");
    fprintf(Parser->Stream, "_PrintU64:\n");
    fprintf(Parser->Stream, "mov     qword [rsp+8], rbx\n");
    fprintf(Parser->Stream, "push    rdi\n");
    fprintf(Parser->Stream, "sub     rsp, 80\n");
    fprintf(Parser->Stream, "xor     edi, edi\n");
    fprintf(Parser->Stream, "lea     r9, qword [rsp+79]\n");
    fprintf(Parser->Stream, "mov     r10, -3689348814741910323\n");
    fprintf(Parser->Stream, "npad    5\n");
    fprintf(Parser->Stream, ".L0:\n");
    fprintf(Parser->Stream, "mov     rax, r10\n");
    fprintf(Parser->Stream, "lea     r9, qword [r9-1]\n");
    fprintf(Parser->Stream, "mul     rcx\n");
    fprintf(Parser->Stream, "inc     rdi\n");
    fprintf(Parser->Stream, "shr     rdx, 3\n");
    fprintf(Parser->Stream, "movzx   eax, dl\n");
    fprintf(Parser->Stream, "shl     al, 2\n");
    fprintf(Parser->Stream, "lea     r8d, dword [rax+rdx]\n");
    fprintf(Parser->Stream, "add     r8b, r8b\n");
    fprintf(Parser->Stream, "sub     cl, r8b\n");
    fprintf(Parser->Stream, "add     cl, 48\n");
    fprintf(Parser->Stream, "mov     byte [r9+1], cl\n");
    fprintf(Parser->Stream, "mov     rcx, rdx\n");
    fprintf(Parser->Stream, "test    rdx, rdx\n");
    fprintf(Parser->Stream, "jne     .L0\n");
    fprintf(Parser->Stream, "lea     rbx, qword [rsp+80]\n");
    fprintf(Parser->Stream, "mov     ecx, -11\n");
    fprintf(Parser->Stream, "sub     rbx, rdi\n");
    fprintf(Parser->Stream, "call    [GetStdHandle]\n");
    fprintf(Parser->Stream, "xor     r9d, r9d\n");
    fprintf(Parser->Stream, "mov     qword [rsp+32], 0\n");
    fprintf(Parser->Stream, "mov     rcx, rax\n");
    fprintf(Parser->Stream, "mov     r8d, edi\n");
    fprintf(Parser->Stream, "mov     rdx, rbx\n");
    fprintf(Parser->Stream, "call    [WriteFile]\n");
    fprintf(Parser->Stream, "mov     rbx, qword [rsp+96]\n");
    fprintf(Parser->Stream, "add     rsp, 80\n");
    fprintf(Parser->Stream, "pop     rdi\n");
    fprintf(Parser->Stream, "ret     0\n");

    fprintf(Parser->Stream, "; void _PrintChar(char C)\n");
    fprintf(Parser->Stream, "_PrintChar:\n");
    fprintf(Parser->Stream, "mov     [rsp+8], rbx\n");
    fprintf(Parser->Stream, "push    rdi\n");
    fprintf(Parser->Stream, "sub     rsp, 48\n");
    fprintf(Parser->Stream, "mov     byte [rsp+47], cl\n");
    fprintf(Parser->Stream, "lea     rdx, [rsp+47]\n");
    fprintf(Parser->Stream, "mov     r8d, 1\n");
    fprintf(Parser->Stream, "mov     ecx, -11\n");
    fprintf(Parser->Stream, "call    [GetStdHandle]\n");
    fprintf(Parser->Stream, "xor     r9d, r9d\n");
    fprintf(Parser->Stream, "mov     qword [rsp+32], 0\n");
    fprintf(Parser->Stream, "mov     rcx, rax\n");
    fprintf(Parser->Stream, "call    [WriteFile]\n");
    fprintf(Parser->Stream, "mov     rbx, [rsp+56]\n");
    fprintf(Parser->Stream, "add     rsp, 48\n");
    fprintf(Parser->Stream, "pop     rdi\n");
    fprintf(Parser->Stream, "ret     0\n");

    fprintf(Parser->Stream, "; [[noreturn]] void _ProgramMain(void)\n");
    fprintf(Parser->Stream, "_ProgramMain:\n");
    fprintf(Parser->Stream, "sub rsp, 0x28\n");
    if(EntryPoint)
    {
        fprintf(Parser->Stream, "call _%.*s\n", ExpandString(EntryName));
    }
    fprintf(Parser->Stream, "xor ecx, ecx\n");
    fprintf(Parser->Stream, "call [ExitProcess]\n");

    if(Parser->DataSize > 0)
    {
        fprintf(Parser->Stream, "section '.data' data readable writeable\n");
        fprintf(Parser->Stream, "program_data: db ");

        for(u32 DataIndex = 0; DataIndex < Parser->DataSize; ++DataIndex)
        {
            if(DataIndex > 0)
            {
                fprintf(Parser->Stream, ",");
            }
            fprintf(Parser->Stream, "%u", Parser->DataSegment[DataIndex]);
        }

        fprintf(Parser->Stream, "\n");
    }

    fprintf(Parser->Stream, "section '.idata' import data readable writeable\n");
    fprintf(Parser->Stream, "dd 0,0,0,RVA kernel_name,RVA kernel_table\n");
    fprintf(Parser->Stream, "dd 0,0,0,0,0\n");

    fprintf(Parser->Stream, "kernel_name db 'KERNEL32.DLL',0\n");
    fprintf(Parser->Stream, "kernel_table:\n");
    fprintf(Parser->Stream, "ExitProcess dq RVA _ExitProcess\n");
    fprintf(Parser->Stream, "GetStdHandle dq RVA _GetStdHandle\n");
    fprintf(Parser->Stream, "WriteFile dq RVA _WriteFile\n");
    fprintf(Parser->Stream, "dq 0\n");

    fprintf(Parser->Stream, "_ExitProcess db 0,0,'ExitProcess',0\n");
    fprintf(Parser->Stream, "_GetStdHandle db 0,0,'GetStdHandle',0\n");
    fprintf(Parser->Stream, "_WriteFile db 0,0,'WriteFile',0\n");
}
