/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

enum data_type_class
{
    Class_All,
    Class_Any,
    Class_Control,
    Class_Simple,
    Class_Integer,
    Class_Tuple,
};

enum data_type_flags
{
    DataType_Constant = 0x1,
};

struct data_type
{
    u16 Class;
    u16 Flags;
    s32 Value;
};

#define IsSimpleType(Type) ((Type).Class < Class_Simple)
#define IsIntegerType(Type) ((Type).Class == Class_Integer)
#define IsConstantType(Type) ((Type).Flags & DataType_Constant)

enum node_type
{
    Node_Invalid,

    //
    // NOTE(alex): Control nodes
    //
    Node_Start,
    Node_End,
    Node_Print,
    Node_If,
    Node_Region,

    //
    // NOTE(alex): Data nodes
    //
    Node_Constant,
    Node_Proj,
    Node_Phi,

    Node_Add,
    Node_Sub,
    Node_Mul,
    Node_Div,

    Node_EQ,
    Node_NE,
    Node_LE,
    Node_LT,

    Node_Neg,
    Node_Not,

    Node_Count,
};

struct node;

struct node_control
{
    node *Prev;
    node *Data;
};

struct node_region
{
    node *Prev;
    node *TrueBranch;
    node *FalseBranch;
};

// TODO(alex): Maybe make there be a union that is like
// Region for Phi nodes, Prev for control nodes, and NextFree.
// Then Operands is an array of actual data values only.
struct node
{
    node_type Type;
    u32 ID;
    u32 RefCount;
    u32 Index; // NOTE(alex): Only for Node_Proj!

    data_type DataType;

    string DebugLabel;

    union
    {
        node *Array;
        node *Operand;
        node *Operands[2];
        node_control Control;
        node_region Region;
        node *NextFree;
    };
};

#define IsConstant(Node) ((Node)->Type == Node_Constant)
#define IsControl(Node) ((Node)->Type < Node_Constant)
#define IsData(Node) ((Node)->Type >= Node_Constant)

#define MAX_NODE_OPERAND_COUNT ((sizeof(node) - OffsetOf(node, Array)) / sizeof(node *))
internal node *GetOperand(node *Node, u32 Index)
{
    Assert(Index < MAX_NODE_OPERAND_COUNT);
    node *Result = (&Node->Array)[Index];
    return Result;
}
