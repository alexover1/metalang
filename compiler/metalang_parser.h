/* ========================================================================

   (C) Copyright 2025 by Alexander Overstreet, All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://overgroup.org for more information

   ======================================================================== */

struct type_id
{
    // TODO(alex): Maybe we should switch to just storing a hash value,
    // and then we have a hash table (which we might need for getting
    // source code location already) where we can look up the name if
    // we need it. This would require us to have the type table present
    // if we need to compare types and their hashes are not equal.
    u32 HashValue;
};

struct type_definition
{
    token NameToken;

    string ID;
    u32 HashValue;

    type_definition *NextInHash;
};

struct parameter_definition
{
    type_id Type;
    token NameToken;
};

struct routine_definition
{
    token TypeToken;
    token NameToken;
    u32 NameHash;

    u32 ParameterCount;
    parameter_definition *Parameters;

    // TODO(alex): If we never end up needing to remove routines,
    // we can just make this a singly linked list.
    routine_definition *Prev;
    routine_definition *Next;

    routine_definition *NextInHash;
};

struct variable_binding
{
    string Name;
    u32 NameHash;
    node *Value;
    variable_binding *Original;
    union
    {
        variable_binding *Prev;
        variable_binding *NextFree;
    };
};

struct variable_scope
{
    variable_binding *End;
};

struct parser
{
    memory_arena Arena;
    FILE *Stream;

    node *StartNode;
    node *EndNode;
    node *ControlNode;

    node *FirstFreeNode;
    u32 NextNodeID;

    variable_binding *MostRecentVariable;
    variable_binding *FirstFreeVariable;

    routine_definition RoutineSentinel;
    routine_definition *RoutineHash[4096];

    type_definition *TypeHash[4096];
};

internal node *ParseExpression(parser *Parser, tokenizer *Tokenizer);
internal void ParseStatement(parser *Parser, tokenizer *Tokenizer, variable_scope Scope);

internal void AddReference(parser *Parser, node *Node);
internal void RemoveReference(parser *Parser, node *Node);
internal node *Peephole(parser *Parser, node *Node);
