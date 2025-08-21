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
    Node_Return,
    Node_Print,

    //
    // NOTE(alex): Data nodes
    //
    Node_Constant,

    Node_Add,
    Node_Sub,
    Node_Mul,
    Node_Div,

    Node_Neg,

    Node_Count,
};

struct node;

struct node_return
{
    node *ControlPrev;
    node *Data;
};

struct node_print
{
    node *ControlPrev;
    node *Data;
};

struct node_constant
{
    node *Start;
};

struct node_binary_op
{
    node *Operands[2];
};

struct node_unary_op
{
    node *Operand;
};

struct node
{
    node_type Type;
    s32 Value;
    union
    {
        node *Inputs;
        node_return Return;
        node_print Print;
        node_constant Constant;
        node_binary_op BinaryOp;
    };
};

#define IsControl(Node) ((Node)->Type < Node_Constant)
#define IsData(Node) ((Node)->Type >= Node_Constant)

#define MAX_NODE_INPUT_COUNT ((sizeof(node) - OffsetOf(node, Inputs)) / sizeof(node *))
inline node *GetInput(node *Node, u32 InputIndex)
{
    Assert(InputIndex < MAX_NODE_INPUT_COUNT);
    node *Result = (&Node->Inputs)[InputIndex];
    return Result;
}
