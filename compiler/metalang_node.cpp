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
        case Node_Return: {return BundleZ("return"); }
        case Node_Print: {return BundleZ("print"); }
        case Node_Constant: {return BundleZ("constant");}
        case Node_Add: {return BundleZ("add");}
        case Node_Sub: {return BundleZ("sub");}
        case Node_Mul: {return BundleZ("mul");}
        case Node_Div: {return BundleZ("div");}
        case Node_Neg: {return BundleZ("neg");}
    }

    return BundleZ("unknown");
}
