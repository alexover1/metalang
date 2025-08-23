/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

internal string GetNodeTypeName(node_type Type)
{
    switch(Type)
    {
        case Node_Start: {return BundleZ("start"); }
        case Node_End: {return BundleZ("end"); }
        case Node_Print: {return BundleZ("print"); }
        case Node_Constant: {return BundleZ("constant");}
        case Node_Proj: {return BundleZ("proj");}
        case Node_Add: {return BundleZ("add");}
        case Node_Sub: {return BundleZ("sub");}
        case Node_Mul: {return BundleZ("mul");}
        case Node_Div: {return BundleZ("div");}
        case Node_EQ: {return BundleZ("eq");}
        case Node_NE: {return BundleZ("ne");}
        case Node_LE: {return BundleZ("le");}
        case Node_LT: {return BundleZ("lt");}
        case Node_Neg: {return BundleZ("neg");}
    }

    return BundleZ("unknown");
}

internal string GetDataTypeName(data_type Type)
{
    switch(Type.Class)
    {
        case Class_All: {return BundleZ("all");}
        case Class_Any: {return BundleZ("any");}
        case Class_Control: {return BundleZ("control");}
        case Class_Simple: {return BundleZ("simple");}
        case Class_Integer: {return BundleZ("integer");}
        case Class_Tuple: {return BundleZ("tuple");}
    }

    return BundleZ("unknown");
}

internal data_type GetTopType(void)
{
    data_type Result = {};
    Result.Class = Class_Any;
    Result.Flags = DataType_Constant;

    return Result;
}

internal data_type GetBottomType(void)
{
    data_type Result = {};
    Result.Class = Class_All;

    return Result;
}

internal data_type GetIntegerTopType(void)
{
    data_type Result = {};
    Result.Class = Class_Integer;
    Result.Value = 0;

    return Result;
}

internal data_type GetIntegerBottomType(void)
{
    data_type Result = {};
    Result.Class = Class_Integer;
    Result.Value = 1;

    return Result;
}

internal data_type GetIntegerType(s32 Value)
{
    data_type Result = {};
    Result.Class = Class_Integer;
    Result.Flags = DataType_Constant;
    Result.Value = Value;

    return Result;
}

internal b32 IsTopInteger(data_type Type)
{
    b32 Result = ((Type.Class == Class_Integer) &&
                  (!(Type.Flags & DataType_Constant)) &&
                  (Type.Value == 0));

    return Result;
}

internal b32 IsBottomInteger(data_type Type)
{
    b32 Result = ((Type.Class == Class_Integer) &&
                  (!(Type.Flags & DataType_Constant)) &&
                  (Type.Value == 1));

    return Result;
}

internal b32 IsConstantInteger(data_type Type)
{
    b32 Result = ((Type.Class == Class_Integer) &&
                  (Type.Flags & DataType_Constant));

    return Result;
}

internal b32 TypesAreEqual(data_type A, data_type B)
{
    b32 Result = ((A.Class == B.Class) &&
                  (A.Value == B.Value) &&
                  (IsConstantType(A) == IsConstantType(B)));

    return Result;
}

internal data_type Meet(data_type A, data_type B)
{
    data_type Result = {};
    Result.Class = Class_All;

    if(IsBottomInteger(A) || IsTopInteger(B))
    {
        Result = A;
    }
    else if(IsBottomInteger(B) || IsTopInteger(A))
    {
        Result = B;
    }
    else if(TypesAreEqual(A, B))
    {
        Result = A;
    }
    else if(IsConstantInteger(A) && IsConstantInteger(B))
    {
        Result = GetIntegerBottomType();
    }

    return Result;
}

internal data_type ComputeType(node *Node)
{
    node *LHS = Node->Operands[0];
    node *RHS = Node->Operands[1];

    data_type A = LHS ? LHS->DataType : GetBottomType();
    data_type B = RHS ? RHS->DataType : GetBottomType();

    data_type Result = Meet(A, B);

    switch(Node->Type)
    {
        case Node_Add:
        {
            if(IsConstantInteger(A) && IsConstantInteger(B))
            {
                Result = GetIntegerType(A.Value + B.Value);
            }
        } break;

        case Node_Sub:
        {
            if(LHS == RHS)
            {
                Result = GetIntegerType(0);
            }
            else if(IsConstantInteger(A) && IsConstantInteger(B))
            {
                Result = GetIntegerType(A.Value - B.Value);
            }
        } break;

        case Node_Mul:
        {
            if(IsConstantInteger(A) && IsConstantInteger(B))
            {
                Result = GetIntegerType(A.Value * B.Value);
            }
        } break;

        case Node_Div:
        {
            if(IsConstantInteger(A) && IsConstantInteger(B))
            {
                Result = GetIntegerType(A.Value / B.Value);
            }
        } break;

        case Node_EQ:
        {
            if(LHS == RHS)
            {
                Result = GetIntegerType(1);
            }
            else if(IsConstantInteger(A) && IsConstantInteger(B))
            {
                Result = GetIntegerType(A.Value == B.Value);
            }
        } break;

        case Node_NE:
        {
            if(LHS == RHS)
            {
                Result = GetIntegerType(0);
            }
            else if(IsConstantInteger(A) && IsConstantInteger(B))
            {
                Result = GetIntegerType(A.Value != B.Value);
            }
        } break;

        case Node_LT:
        {
            if(LHS == RHS)
            {
                Result = GetIntegerType(0);
            }
            else if(IsConstantInteger(A) && IsConstantInteger(B))
            {
                Result = GetIntegerType(A.Value < B.Value);
            }
        } break;

        case Node_LE:
        {
            if(LHS == RHS)
            {
                Result = GetIntegerType(1);
            }
            else if(IsConstantInteger(A) && IsConstantInteger(B))
            {
                Result = GetIntegerType(A.Value <= B.Value);
            }
        } break;
    }

    return Result;
}

internal node *SwapOperands(node *Node)
{
    node *Temp = Node->Operands[0];
    Node->Operands[0] = Node->Operands[1];
    Node->Operands[1] = Temp;

    return Node;
}

internal b32 SplineCompare(node *High, node *Low)
{
    b32 Result = Low->ID > High->ID;

    if(IsConstantType(Low->DataType))
    {
        Result = false;
    }
    else if(IsConstantType(High->DataType))
    {
        Result = true;
    }

    return Result;
}
