/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

enum node_type
{
    Node_Invalid,

    //
    // NOTE(alex): Control nodes
    //
    Node_Start,
    Node_End,
    Node_Print,

    //
    // NOTE(alex): Data nodes
    //
    Node_Constant,

    Node_Add,
    Node_Sub,
    Node_Mul,
    Node_Div,

    Node_EQ,
    Node_NE,
    Node_LE,
    Node_LT,
    Node_GE,
    Node_GT,

    Node_Neg,

    Node_Count,
};

struct node;

struct node_control
{
    node *Prev;
    node *Data;
};

struct node
{
    node_type Type;
    u32 RefCount;
    s32 Value;
    union
    {
        node *Array;
        node *Operand;
        node *Operands[2];
        node_control Control;
        node *NextFree;
    };
};

#define IsConstant(Node) ((Node)->Type == Node_Constant)
#define IsControl(Node) ((Node)->Type < Node_Constant)
#define IsData(Node) ((Node)->Type >= Node_Constant)

#define MAX_NODE_EDGE_COUNT ((sizeof(node) - OffsetOf(node, Array)) / sizeof(node *))
inline node *GetEdge(node *Node, u32 EdgeIndex)
{
    Assert(EdgeIndex < MAX_NODE_EDGE_COUNT);
    node *Result = (&Node->Array)[EdgeIndex];
    return Result;
}
