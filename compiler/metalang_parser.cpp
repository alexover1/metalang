/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

#if 1

#define DEBUG_RECORD_ALLOCATION(Node)
#define DEBUG_RECORD_FREE(Node)
#define DEBUG_RECORD_REFERENCE(Node)
#define DEBUG_RECORD_UNREFERENCE(Node)

#else

#define DEBUG_LOG(Type, Node) do {printf("%s: node=%p, id=%d\n", (Type), (Node), (Node)->ID); } while(0)

#define DEBUG_RECORD_ALLOCATION(Node) DEBUG_LOG("ALLOCATE", (Node))
#define DEBUG_RECORD_FREE(Node) DEBUG_LOG("FREE", (Node))
#define DEBUG_RECORD_REFERENCE(Node) DEBUG_LOG("REFERENCE", (Node))
#define DEBUG_RECORD_UNREFERENCE(Node) DEBUG_LOG("UNREFERENCE", (Node))

#endif

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

internal void DebugType(data_type Type)
{
    printf("%.*s", ExpandString(GetDataTypeName(Type)));

    if(IsConstantInteger(Type))
    {
        printf("(%d)", Type.Value);
    }
}

internal void DebugNode(node *Node)
{
    if(IsValid(Node->DebugLabel))
    {
        printf("%.*s", ExpandString(Node->DebugLabel));
    }
    else
    {
        printf("%.*s(", ExpandString(GetNodeTypeName(Node->Type)));

        if(IsConstant(Node) && IsConstantInteger(Node->DataType))
        {
            printf("%d", Node->DataType.Value);
        }
        else
        {
            u32 StartIndex = IsControl(Node) ? 1 : 0;
            b32 First = true;
            for(u32 OperandIndex = StartIndex; OperandIndex < MAX_NODE_OPERAND_COUNT; ++OperandIndex)
            {
                node *Operand = GetOperand(Node, OperandIndex);
                if(Operand)
                {
                    if(!First)
                    {
                        printf(", ");
                    }
                    DebugNode(Operand);
                    First = false;
                }
            }

            if(Node->Type == Node_Proj)
            {
                if(!First)
                {
                    printf(", ");
                }
                printf("%u", Node->Index);
            }
        }

        printf(")");
    }
}

internal variable_definition *AllocateVariable(parser *Parser)
{
    Assert(Parser->VariableCount < ArrayCount(Parser->Variables));

    variable_definition *Result = Parser->Variables + Parser->VariableCount++;
    return Result;
}

internal variable_definition *AddVariable(parser *Parser, string Name, node *Value)
{
    Assert(Parser->VariableCount < ArrayCount(Parser->Variables));

    variable_definition *Variable = Parser->Variables + Parser->VariableCount++;
    Variable->Name = Name;
    Variable->NameHash = StringHashOf(Name);
    Variable->Value = Value;
    AddReference(Parser, Value);

    Variable->Prev = Parser->MostRecentVariable;
    Parser->MostRecentVariable = Variable;

    return Variable;
}

internal variable_definition *GetVariable(variable_definition *Start, string Name, u32 NameHash)
{
    variable_definition *Result = 0;
    for(variable_definition *TestVariable = Start;
        TestVariable;
        TestVariable = TestVariable->Prev)
    {
        if((TestVariable->NameHash == NameHash) &&
           StringsAreEqual(TestVariable->Name, Name))
        {
            Result = TestVariable;
            break;
        }
    }

    return Result;
}

internal variable_definition *GetVariable(parser *Parser, string Name)
{
    variable_definition *Result = GetVariable(Parser->MostRecentVariable, Name, StringHashOf(Name));
    return Result;
}

internal node *GetOrCreateNodeInternal(parser *Parser, node_type Type, u32 OperandCount, node **Operands)
{
    if(!Parser->FirstFreeNode)
    {
        Parser->FirstFreeNode = PushStruct(&Parser->Arena, node, NoClear());
        Parser->FirstFreeNode->NextFree = 0;
    }

    // TODO(alex): Hash table for looking up already created nodes!
    node *Result = Parser->FirstFreeNode;
    Parser->FirstFreeNode = Result->NextFree;

    ZeroStruct(*Result);
    Result->Type = Type;
    Result->ID = Parser->NextNodeID++;

    Assert(OperandCount <= MAX_NODE_OPERAND_COUNT);

    for(u32 OperandIndex = 0;
        OperandIndex < OperandCount;
        ++OperandIndex)
    {
        node *Operand = Operands[OperandIndex];
        if(Operand)
        {
            (&Result->Array)[OperandIndex] = Operand;
            AddReference(Parser, Operand);
        }
    }

    DEBUG_RECORD_ALLOCATION(Result);

    return Result;
}

internal node *GetOrCreateNode(parser *Parser, node_type Type, node *Operand)
{
    node *Result = GetOrCreateNodeInternal(Parser, Type, 1, &Operand);

    return Result;
}

internal node *GetOrCreateNode(parser *Parser, node_type Type, node *LHS, node *RHS)
{
    node *Operands[] = {LHS, RHS};
    node *Result = GetOrCreateNodeInternal(Parser, Type, ArrayCount(Operands), Operands);

    return Result;
}

internal node *GetOrCreateNode(parser *Parser, node_type Type)
{
    node *Result = GetOrCreateNodeInternal(Parser, Type, 0, 0);

    return Result;
}


internal node *GetOrCreateProj(parser *Parser, node *Operand, u32 Index, string DebugLabel = {})
{
    node *Result = GetOrCreateNode(Parser, Node_Proj, Operand);
    Result->Index = Index;
    Result->DebugLabel = DebugLabel;

    return Result;
}

internal node *GetOrCreateRegion(parser *Parser, node *Prev, node *True, node *False)
{
    node *Operands[] = {Prev, True, False};
    node *Result = GetOrCreateNodeInternal(Parser, Node_Region, ArrayCount(Operands), Operands);

    return Result;
}

internal node *GetOrCreatePhi(parser *Parser, node *Region, node *True, node *False)
{
    node *Operands[] = {True, False, Region};
    node *Result = GetOrCreateNodeInternal(Parser, Node_Phi, ArrayCount(Operands), Operands);

    return Result;
}

internal node *GetOrCreateConstant(parser *Parser, data_type DataType)
{
    node *Result = GetOrCreateNode(Parser, Node_Constant);
    Result->DataType = DataType;

    return Result;
}

internal node *GetOrCreateInteger(parser *Parser, s32 Value)
{
    node *Result = GetOrCreateConstant(Parser, GetIntegerType(Value));

    return Result;
}

internal void AddReference(parser *Parser, node *Node)
{
    DEBUG_RECORD_REFERENCE(Node);

    ++Node->RefCount;
}

internal void FreeNode(parser *Parser, node *Node)
{
    DEBUG_RECORD_FREE(Node);

    Assert(Node->RefCount == 0);
    Node->NextFree = Parser->FirstFreeNode;
    Parser->FirstFreeNode = Node;
}

internal void RemoveChildReferences(parser *Parser, node *Parent)
{
    for(u32 OperandIndex = 0; OperandIndex < ArrayCount(Parent->Operands); ++OperandIndex)
    {
        node *Operand = Parent->Operands[OperandIndex];
        if(Operand)
        {
            RemoveReference(Parser, Operand);
        }
    }
}

internal void RemoveReference(parser *Parser, node *Node)
{
    DEBUG_RECORD_UNREFERENCE(Node);

    Assert(Node->RefCount > 0);
    --Node->RefCount;

    if(Node->RefCount == 0)
    {
        RemoveChildReferences(Parser, Node);
        FreeNode(Parser, Node);
    }
}

internal variable_definition *MergeScopes(parser *Parser, node *Region,
                                          variable_definition *True,
                                          variable_definition *False)
{
    printf("--- True branch ---\n");
    for(variable_definition *Variable = True;
        Variable;
        Variable = Variable->Prev)
    {
        printf("%.*s = ", ExpandString(Variable->Name));
        DebugNode(Variable->Value);
        printf("\n");
    }
    printf("--- False branch ---\n");
    for(variable_definition *Variable = False;
        Variable;
        Variable = Variable->Prev)
    {
        printf("%.*s = ", ExpandString(Variable->Name));
        DebugNode(Variable->Value);
        printf("\n");
    }
    printf("--- End ---\n");

    variable_definition *Result = Parser->MostRecentVariable;

    for(variable_definition *Variable = Result;
        Variable;
        Variable = Variable->Prev)
    {
        variable_definition *TrueVar = GetVariable(True, Variable->Name, Variable->NameHash);
        variable_definition *FalseVar = GetVariable(False, Variable->Name, Variable->NameHash);

        if(TrueVar && FalseVar)
        {
            if(TrueVar->Value != FalseVar->Value)
            {
                node *Phi = GetOrCreatePhi(Parser, Region, TrueVar->Value, FalseVar->Value);
                variable_definition *Merged = AllocateVariable(Parser);
                Merged->Name = Variable->Name;
                Merged->NameHash = Variable->NameHash;
                Merged->Value = Phi;
                Merged->Prev = Result;
                Result = Merged;
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

internal parser *ParseTopLevelRoutines(tokenizer Tokenizer_)
{
    tokenizer *Tokenizer = &Tokenizer_;

    parser *Parser = BootstrapPushStruct(parser, Arena, DefaultBootstrapParams(), NoClear());
    routine_definition *Sentinel = &Parser->RoutineSentinel;
    Sentinel->Prev = Sentinel->Next = Sentinel;

    Parser->FirstFreeNode = 0;
    Parser->NextNodeID = 0;
    Parser->VariableCount = 0;

    Parser->StartNode = Parser->ControlNode = GetOrCreateNode(Parser, Node_Start);
    Parser->EndNode = GetOrCreateNode(Parser, Node_End);

    string ARG0 = ConstZ("arg");
    node *Value = GetOrCreateProj(Parser, Parser->StartNode, 1, ARG0);
#if 1
    Value->DataType = GetIntegerBottomType();
#else
    Value->DataType = GetIntegerType(2);
#endif
    AddVariable(Parser, ARG0, Value);

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
                Result = PushStruct(&Parser->Arena, routine_definition, NoClear());
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

    return Parser;
}

internal node *DeadCodeEliminate(parser *Parser, node *Old, node *New)
{
    if((Old != New) &&
       (Old->RefCount == 0))
    {
        RemoveChildReferences(Parser, Old);
        FreeNode(Parser, Old);
    }

    return New;
}

internal b32 PhiHasSameOp(node *Phi)
{
    b32 Result = true;


    return Result;
}

internal node *Idealize(parser *Parser, node *Node)
{
    node *Result = 0;

    node *LHS = Node->Operands[0];
    node *RHS = Node->Operands[1];

    switch(Node->Type)
    {
        case Node_Add:
        {
            Assert(!(IsConstantType(LHS->DataType) && IsConstantType(RHS->DataType)));

            if(IsConstantInteger(RHS->DataType) && (RHS->DataType.Value == 0))
            {
                Result = LHS;
            }
            else if(LHS == RHS)
            {
                node *Two = GetOrCreateInteger(Parser, 2);
                Result = GetOrCreateNode(Parser, Node_Mul, LHS, Two);
            }
            else if((LHS->Type != Node_Add) &&
                    (RHS->Type == Node_Add))
            {
                Result = SwapOperands(Node);
            }
            else if(RHS->Type == Node_Add)
            {
                node *X = LHS;
                node *Y = RHS->Operands[0];
                node *Z = RHS->Operands[1];

                node *XY = Peephole(Parser, GetOrCreateNode(Parser, Node_Add, X, Y));
                Result = GetOrCreateNode(Parser, Node_Add, XY, Z);
            }
            else if(LHS->Type != Node_Add)
            {
                if(SplineCompare(LHS, RHS))
                {
                    Result = SwapOperands(Node);
                }
            }
            else if(IsConstantType(LHS->Operands[1]->DataType) &&
                    IsConstantType(RHS->DataType))
            {
                node *X = LHS->Operands[0];
                node *Y = LHS->Operands[1];
                node *Z = RHS;

                node *YZ = Peephole(Parser, GetOrCreateNode(Parser, Node_Add, Y, Z));
                Result = GetOrCreateNode(Parser, Node_Add, X, YZ);
            }
            else
            {
                if(SplineCompare(LHS->Operands[1], RHS))
                {
                    node *X = LHS->Operands[0];
                    node *Y = RHS;
                    node *Z = LHS->Operands[1];

                    node *XY = Peephole(Parser, GetOrCreateNode(Parser, Node_Add, X, Y));
                    Result = GetOrCreateNode(Parser, Node_Add, XY, Z);
                }
            }
        } break;

        case Node_Mul:
        {
            Assert(!(IsConstantType(LHS->DataType) && IsConstantType(RHS->DataType)));

            if(IsConstantInteger(RHS->DataType) && (RHS->DataType.Value == 1))
            {
                Result = LHS;
            }
            else if(IsConstantType(LHS->DataType) &&
                    !IsConstantType(RHS->DataType))
            {
                Result = SwapOperands(Node);
            }
        } break;

        case Node_Div:
        {
            Assert(!(IsConstantType(LHS->DataType) && IsConstantType(RHS->DataType)));

            if(IsConstantInteger(RHS->DataType) && (RHS->DataType.Value == 1))
            {
                Result = LHS;
            }
        } break;

        case Node_Neg: {} break;

        case Node_Not:
        {
            Assert(!IsConstantType(LHS->DataType));

            switch(LHS->Type)
            {
                case Node_EQ:
                {
                    Result = GetOrCreateNode(Parser, Node_NE, LHS->Operands[0], LHS->Operands[1]);
                } break;

                case Node_LT:
                {
                    Result = GetOrCreateNode(Parser, Node_LE, LHS->Operands[1], LHS->Operands[0]);
                } break;

                case Node_LE:
                {
                    Result = GetOrCreateNode(Parser, Node_LT, LHS->Operands[1], LHS->Operands[0]);
                } break;
            }
        } break;

        case Node_Phi:
        {
            if(LHS == RHS)
            {
                Result = LHS;
            }
            else if(LHS->Type == RHS->Type)
            {
                node *PhiLHS = Peephole(Parser, GetOrCreateNode(Parser, Node_Phi, LHS->Operands[0], RHS->Operands[0]));
                node *PhiRHS = Peephole(Parser, GetOrCreateNode(Parser, Node_Phi, LHS->Operands[1], RHS->Operands[1]));
                Result = GetOrCreateNode(Parser, LHS->Type, PhiLHS, PhiRHS);
            }
        } break;
    }

    return Result;
}

internal node *Peephole(parser *Parser, node *Node)
{
    node *Result = Node;

    data_type Type = Node->DataType = ComputeType(Node);

    if(!IsConstant(Node) && IsConstantType(Type))
    {
        node *Constant = GetOrCreateConstant(Parser, Type);
        Result = DeadCodeEliminate(Parser, Node, Constant);
    }
    else
    {
        node *TestNode = Idealize(Parser, Node);
        if(TestNode)
        {
            node *Replacement = DeadCodeEliminate(Parser, Node, TestNode);
            Result = Peephole(Parser, Replacement);
            // Result = DeadCodeEliminate(Parser, Node, Peephole(Parser, TestNode));
        }
    }

    return Result;
}

internal node *ParsePrimaryExpression(parser *Parser, tokenizer *Tokenizer)
{
    node *Result = 0;
    token Token = GetToken(Tokenizer);

    switch(Token.Type)
    {
        case Token_OpenParen:
        {
            Result = ParseExpression(Parser, Tokenizer);
            RequireToken(Tokenizer, Token_CloseParen);
        } break;

        case Token_Identifier:
        {
            variable_definition *Variable = GetVariable(Parser, Token.Text);
            if(Variable)
            {
                Result = Variable->Value;
            }
            else
            {
                Error(Tokenizer, Token, "Undeclared variable");
            }
        } break;

        case Token_Number:
        {
            Result = GetOrCreateInteger(Parser, Token.S32);
        } break;

        case Token_String:
        {
            Assert(!"Strings are not implement yet");
        } break;

        default:
        {
            Error(Tokenizer, Token, "Invalid expression");
        } break;
    }

    return Result;
}

internal node *ParseUnaryOp(parser *Parser, tokenizer *Tokenizer)
{
    node *Result = 0;

    node_type OpType = Node_Invalid;
    if(OptionalToken(Tokenizer, Token_Minus))
    {
        OpType = Node_Neg;
    }
    else if(OptionalToken(Tokenizer, Token_Not))
    {
        OpType = Node_Not;
    }
    else
    {
        Result = ParsePrimaryExpression(Parser, Tokenizer);
    }

    if(OpType)
    {
        node *RHS = ParseUnaryOp(Parser, Tokenizer);
        node *UnaryOp = GetOrCreateNode(Parser, OpType, RHS);
        Result = Peephole(Parser, UnaryOp);
    }

    return Result;
}

internal node *ParseMultiplication(parser *Parser, tokenizer *Tokenizer)
{
    node *LHS = ParseUnaryOp(Parser, Tokenizer);
    if(LHS && OptionalToken(Tokenizer, Token_Asterisk))
    {
        node *RHS = ParseMultiplication(Parser, Tokenizer);
        node *BinaryOp = GetOrCreateNode(Parser, Node_Mul, LHS, RHS);
        LHS = Peephole(Parser, BinaryOp);
    }

    return LHS;
}

// TODO(alex): Right now this parses `1 + 2 + 3` as `1 + (2 + 3)` which might be wrong...
internal node *ParseAddition(parser *Parser, tokenizer *Tokenizer)
{
    node *LHS = ParseMultiplication(Parser, Tokenizer);

    node_type OpType = Node_Invalid;
    if(OptionalToken(Tokenizer, Token_Plus))
    {
        OpType = Node_Add;
    }
    else if(OptionalToken(Tokenizer, Token_Minus))
    {
        OpType = Node_Sub;
    }

    if(LHS && OpType)
    {
        node *RHS = ParseAddition(Parser, Tokenizer);
        node *BinaryOp = GetOrCreateNode(Parser, OpType, LHS, RHS);
        LHS = Peephole(Parser, BinaryOp);
    }

    return LHS;
}

internal node *ParseComparison(parser *Parser, tokenizer *Tokenizer)
{
    node *Result = ParseAddition(Parser, Tokenizer);

    node_type OpType = Node_Invalid;
    b32 Flip = false;

    tokenizer Temp = *Tokenizer;

    token Token1 = GetToken(&Temp);
    token Token2 = GetTokenRaw(&Temp);
    b32 HasEquals = (Token2.Type == Token_Equals);

    switch(Token1.Type)
    {
        case Token_Equals:
        {
            if(HasEquals)
            {
                OpType = Node_EQ;
            }
        } break;

        case Token_Not:
        {
            if(HasEquals)
            {
                OpType = Node_NE;
            }
        } break;

        case Token_OpenAngleBracket:
        {
            OpType = HasEquals ? Node_LE : Node_LT;
        } break;

        case Token_CloseAngleBracket:
        {
            OpType = HasEquals ? Node_LE : Node_LT;
            Flip = true;
        } break;
    }

    if(Result && OpType)
    {
        GetToken(Tokenizer);
        if(HasEquals)
        {
            GetTokenRaw(Tokenizer);
        }

        node *LHS = Result;
        node *RHS = ParseComparison(Parser, Tokenizer);
        if(Flip)
        {
            LHS = RHS;
            RHS = Result;
        }

        node *BinaryOp = GetOrCreateNode(Parser, OpType, LHS, RHS);
        Result = Peephole(Parser, BinaryOp);
    }

    return Result;
}

internal node *ParseExpression(parser *Parser, tokenizer *Tokenizer)
{
    node *Result = ParseComparison(Parser, Tokenizer);
    return Result;
}

internal variable_definition *ParseBlock(parser *Parser, tokenizer *Tokenizer)
{
    u32 SavedVariableCount = Parser->VariableCount;
    variable_definition *SavedVariable = Parser->MostRecentVariable;

    while(Parsing(Tokenizer))
    {
        token Token = PeekToken(Tokenizer);
        if((Token.Type == Token_EndOfStream) ||
           (Token.Type == Token_CloseBrace))
        {
            RequireToken(Tokenizer, Token_CloseBrace);
            break;
        }

        // TODO(alex): We could make tokenizer have a "save point" so
        // that we don't have to clone the entire structure in order
        // to do this kind of speculative parsing, especially since
        // we are doing this cloning for every statement.
        // TODO(alex): By this point, we should know which identifiers
        // refer to types, in which case we can drive the parsing by
        // whether or not we see a type at statement level and avoid
        // doing this save/restore stuff!!!
        tokenizer Temp = *Tokenizer;

        token Token1 = GetToken(&Temp);
        token Token2 = GetToken(&Temp);
        if((Token1.Type == Token_Identifier) &&
           (Token2.Type == Token_Identifier) &&
           TokenEquals(Token1, "s32"))
        {
            token TypeToken = GetToken(Tokenizer);
            token NameToken = GetToken(Tokenizer);

            node *Value = 0;
            if(OptionalToken(Tokenizer, Token_Equals))
            {
                Value = ParseExpression(Parser, Tokenizer);
            }
            else
            {
                Value = GetOrCreateInteger(Parser, 0);
            }

            if(Value)
            {
                variable_definition *Variable = AddVariable(Parser, NameToken.Text, Value);

                for(variable_definition *TestVariable = Variable->Prev;
                    TestVariable && (TestVariable != SavedVariable);
                    TestVariable = TestVariable->Prev)
                {
                    if((Variable->NameHash == TestVariable->NameHash) &&
                       StringsAreEqual(Variable->Name, TestVariable->Name))
                    {
                        Error(Tokenizer, NameToken, "Redeclaration of variable");
//                            Error(Tokenizer, NameToken, "Original variable was declared here");
                    }
                }

#if 0
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
#endif

                RequireToken(Tokenizer, Token_Semicolon);
            }
            else
            {
                OptionalToken(Tokenizer, Token_Semicolon);
            }
        }
        else
        {
            ParseStatement(Parser, Tokenizer);
        }
    }

#if 0
    for(variable_definition *Variable = Parser->MostRecentVariable;
        Variable != SavedVariable;
        Variable = Variable->Prev)
    {
        printf("%.*s = ", ExpandString(Variable->Name));
        DebugNode(Variable->Value);
        printf("\n");

        // RemoveReference(Parser, Variable->Value);
    }
#endif

#if 0
    for(u32 VariableIndex = SavedVariableCount;
        VariableIndex < Parser->VariableCount;
        ++VariableIndex)
    {
        variable_definition *Variable = Parser->Variables + VariableIndex;
        printf("%.*s = ", ExpandString(Variable->Name));
        DebugNode(Variable->Value);
        printf("\n");

        RemoveReference(Parser, Variable->Value);
    }
#endif

    variable_definition *Result = Parser->MostRecentVariable;
    // Parser->VariableCount = SavedVariableCount;
    Parser->MostRecentVariable = SavedVariable;

    return Result;
}

// NOTE(alex): Parse expression being used at statement level
internal node *ParseExpressionStatement(parser *Parser, tokenizer *Tokenizer)
{
    node *Result = 0;

    tokenizer Temp = *Tokenizer;

    token Token1 = GetToken(&Temp);
    token Token2 = GetToken(&Temp);
    if((Token1.Type == Token_Identifier) &&
       (Token2.Type == Token_Equals) &&
       (PeekTokenRaw(&Temp).Type != Token_Equals))
    {
        token NameToken = GetToken(Tokenizer);
        token EqualsToken = GetToken(Tokenizer);

        variable_definition *Variable = GetVariable(Parser, NameToken.Text);
        if(!Variable)
        {
            Error(Tokenizer, NameToken, "Undeclared variable");
        }

        node *RHS = ParseExpression(Parser, Tokenizer);
        AddVariable(Parser, NameToken.Text, RHS);
#if 0
        if(Variable)
        {
            RemoveReference(Parser, Variable->Value);
            Variable->Value = RHS;
            AddReference(Parser, Variable->Value);
        }
#endif
    }
    else
    {
        node *Value = ParseExpression(Parser, Tokenizer);
        node *Print = GetOrCreateNode(Parser, Node_Print, Parser->ControlNode, Value);
        Parser->ControlNode = Print;
        Result = Print;
    }

    // TODO(alex): I wanted to just call ParseExpression here and to implement
    // assignment as a binary operator to allow chaining assignments like in C.
    // However, it was not clear as of 8/21/25 how to do this and pass down the
    // proper information to the assignment operator. Let me explain in more detail:
    //
    // When we call ParseExpression and see an identifier, we immediately look up
    // the identifier in its scope and find what node it refers to. This is good
    // because it allows us to quickly see if we can inline a variable and get rid
    // of it, but it is bad because now later in the call stack we do not have any
    // information about the variable (i.e. in the expression `x=5;` by the time
    // we get to `=` we have already replaced `x` with whatever its original value
    // was).

//    ParseExpression(Parser, Tokenizer);

    RequireToken(Tokenizer, Token_Semicolon);

    return Result;
}

internal void ParseStatement(parser *Parser, tokenizer *Tokenizer)
{
    if(OptionalToken(Tokenizer, Token_OpenBrace))
    {
        ParseBlock(Parser, Tokenizer);
    }
    else if(OptionalToken(Tokenizer, Token_Semicolon))
    {
        // NOTE(alex): Empty statement
    }
    else if(OptionalToken(Tokenizer, "if"))
    {
        node *Predicate = ParseExpression(Parser, Tokenizer);
        if(Predicate)
        {
            node *IF = GetOrCreateNode(Parser, Node_If, Parser->ControlNode, Predicate);
            AddReference(Parser, IF);
            IF = Peephole(Parser, IF);

            node *TrueBranch = Peephole(Parser, GetOrCreateProj(Parser, IF, 0, BundleZ("true")));
            node *FalseBranch = Peephole(Parser, GetOrCreateProj(Parser, IF, 1, BundleZ("false")));

            Parser->ControlNode = TrueBranch;
            RequireToken(Tokenizer, Token_OpenBrace);
            variable_definition *TopOfTrueBranch = ParseBlock(Parser, Tokenizer);

            Parser->ControlNode = FalseBranch;
            variable_definition *TopOfFalseBranch = Parser->MostRecentVariable;
            if(OptionalToken(Tokenizer, "else"))
            {
                RequireToken(Tokenizer, Token_OpenBrace);
                TopOfFalseBranch = ParseBlock(Parser, Tokenizer);
            }

            node *Region = GetOrCreateRegion(Parser,
                                             Parser->ControlNode->Control.Prev,
                                             TrueBranch, FalseBranch);

            Parser->MostRecentVariable = MergeScopes(Parser, Region, TopOfTrueBranch, TopOfFalseBranch);
            Parser->ControlNode = Region;

            printf("--- Variable bindings ---\n");
            for(variable_definition *Variable = Parser->MostRecentVariable;
                Variable;
                Variable = Variable->Prev)
            {
                printf("%.*s = ", ExpandString(Variable->Name));
                DebugNode(Variable->Value);
                printf("\n");
            }
            printf("--- End ---\n");
        }
        else
        {
            // NOTE(alex): You may see this pattern in a few places in the code,
            // where if something previously failed, we just optionally consume
            // the expected token instead of erroring out. This is so we could
            // continue parsing, but not report useless errors like in C/C++.
            OptionalToken(Tokenizer, Token_Semicolon);
        }
    }
    else
    {
        ParseExpressionStatement(Parser, Tokenizer);
    }
}

internal void ParseFile(parser *Parser, tokenizer Tokenizer_)
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
                ParseBlock(Parser, Tokenizer);

                Parser->EndNode->Control.Prev = Parser->ControlNode;
                Parser->ControlNode = Parser->EndNode;

                for(node *Node = Parser->EndNode;
                    Node;
                    Node = Node->Control.Prev)
                {
                    // Assert(IsControl(Node));
                    DebugNode(Node);
                    printf("\n");
                }
            }
        }
    }
}
