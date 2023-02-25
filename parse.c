#include "cflat.h"
#include <stdio.h>

typedef struct {
    bool is_typedef;
    bool is_static;
    bool is_extern;
    bool is_inline;
    int alignment;
} VarAttrs;

typedef struct Obj Obj;
struct Obj {
    Type *ty;
    VarAttrs *attrs;
    char *id;

    int param_count;
    Obj **params;
};

typedef struct {
    char *id;
    Obj *var;
} VarRef;

typedef struct {
    char *id;
    Obj *func;
} FuncInvok;

inline void next_tk(Token **curr_tk) {
    if ((*curr_tk)->next == NULL) {
        fprintf(stderr, "internal err: called next_tk when next is NULL\n");
        exit(1);
    }
}

inline TokenType tk_ty(Token **curr_tk)
{
    if (*curr_tk == NULL) {
        fprintf(stderr, "internal err: called ty on NULL\n");
        exit(1);
    }

    return (*curr_tk)->ty;
}

void declarator(Token **curr_tk, Type* ty, VarAttrs *attrs, bool is_func_params);
void direct_declarator(
    Token **curr_tk,
    Type *ty,
    VarAttrs *attrs,
    bool is_func_params,
    bool is_func_def_params
);

/* cast_expr = ("(" unary_expr ")")* unary_expr
 */
void cast_expr(Token **curr_tk)
{
}

/* mul_expr = cast_expr (("*" | "/" | "%") cast_expr)*
 */
void mul_expr()
{
}

/* add_expr = mul_expr (("+" | "-") mul_expr)*
 */
void add_expr()
{
}

/* shift_expr = add_expr (("<<" | ">>") add_expr)*
 */
void shift_expr()
{
}

/* unary_expr = postfix_expr
 *            | ("++" | "--") unary_expr
 *            | ("&" | "*" | "+" | "-" | "~" | "!") cast_expr
 *            | TK_SIZEOF unary_expr
 *            | (TK_SIZEOF | TK__ALIGNOF) "(" type_name ")"
 */
void unary_expr()
{
}

/* assnt_expr = cond_expr
 *            | unary_expr ("="|"*="|"/="|"%="|"+="|"-="|"<<="|">>="|"&="|"^="|"|=")
 *              assnt_expr
 */
Expr *assnt_expr(Token **curr_tk)
{
    return NULL;
}

/* exprs = assnt_expr ("," assnt_expr)*
 */
Expr *exprs(int *expr_cnt)
{
    return NULL;
}

/* primary_expr = TK_ID
 *              | TK_CONST
 *              | TK_STR
 *              | "(" exprs ")"
 * LATER        | generic_selection
 *
 * NOTE: generic_selection for preprocesser I think
 * NOTE: we treat TK_CONST like a VarRef (to a compiler-generated variable)
 */
void primary_expr(Token **curr_tk)
{
}

/* postfix_expr = primary_expr
 *              | postfix_expr "[" expr "]"
 *              | postfix_expr "(" ( assnt_expr ("," assnt_expr)* )? ")"
 *              | postfix_expr ("." || "->") TK_ID
 *              | postfix_expr ("++" | "--")
 *              | "(" type_name ")" "{" init_list ","? "}"
 */
void postfix_expr()
{
}

/* decl_specs = ( TK_TYPEDEF | TK_EXTERN | TK_STATIC | TK__THR_LOC
 *               | TK_AUTO | TK_REGISTER | TK_VOID | TK_CHAR
 *               | TK_SHORT | TK_INT | TK_LONG | TK_FLOAT | TK_DOUBLE
 *               | TK_SIGNED | TK_UNSIGNED | TK__BOOL | TK__COMPLEX
 *               | TK_CONST | TK_RESTRICT | TK_VOLATILE | TK__Atomic
 *               | TK_INLINE | TK__NORETURN
 *               | atomic_type_spec
 *               | struct_or_union_spec
 *               | enum_spec
 *               | align_spec
 *               | TK_ID )+
 *
 * NOTE: at most one storage_class_spec is allowed
 *     (EXCEPT: TK__THR_LOC with TK_STATIC OR TK_EXTERN)
 *
 * NOTE: TK__THR_LOC can't be used in func decls OR defs
 * NOTE: TK_STATIC, TK__THR_LOC, TK_AUTO, TK_REGISTER
 *     all carry recursively to members of struct or union
 */
Type *decl_specs(Token **curr_tk, VarAttrs *attrs, bool is_func_decl)
{
    return NULL;
}

/* pointers = '*' type_qualifier* pointers?
 */
int pointers(Token **curr_tk, Type *ty, VarAttrs *attrs)
{
    int ptr_cnt;
    while (tk_ty(curr_tk) == TK_STAR) {
        next_tk(curr_tk);
        while (tk_ty(curr_tk) == TK_CONST
                || tk_ty(curr_tk) == TK_RESTRICT
                || tk_ty(curr_tk) == TK_VOLATILE
                || tk_ty(curr_tk) == TK__ATOMIC) {

            switch (tk_ty(curr_tk)) {
                case TK_STATIC:
                    attrs->is_static = true;
                    break;
                case TK_CONST:
                    ty->is_const = true;
                    break;
                case TK_RESTRICT:
                    break;
                case TK_VOLATILE:
                    break;
                case TK__ATOMIC:
                    break;
                default:
                    /* problem */
                    return 0;
            }

            next_tk(curr_tk);
        }
    }

    return ptr_cnt;
}

/* param_decl = decl_specs (declarator | abstract_declarator)
 */
Obj *param_decl(Token **curr_tk)
{
    VarAttrs *attrs;
    Type *ty = decl_specs(curr_tk, attrs, false);
    Obj *var = malloc(sizeof(Obj));
    var->ty = ty;
    var->attrs = attrs;

    return NULL;
}

/* param_t_list = param_decl ("," param_decl)* (, "...")?
 */
void param_t_list()
{
}

/* func_id = TK_ID
 *         | "(" func_id ")"
 */
char* func_id(Token **curr_tk)
{
    return NULL;
}

/* func_params = "(" param_t_list? ")"
 *             | "(" id_list? ")"
 */
Obj *func_params(Token **curr_tk, int *param_cnt)
{
    return NULL;
}

/* func_declarator = pointers? func_id func_params
 */
void func_declarator(Token **curr_tk, Obj *func)
{
    pointers(curr_tk, func->ty, func->attrs);

    char* id = func_id(curr_tk);
    if (id == NULL) { /* problem */ }
    func->id = id;

    int param_cnt;
    func_params(curr_tk, &param_cnt);
}

/* direct_declarator = TK_ID
 *                   | "(" declarator ")"
 *                   | direct_declarator "[" type_qualifier* assnt_expr? "]"
 *                   | direct_declarator "[" type_qualifier* "*" ]"
 *                   | direct_declarator "[" static type-qualifier* assnt_expr "]"
 *                   | direct_declarator "[" type_qualifier+ static assnt_expr "]"
 *
 * NOTE: this is not as straightforward as it looks
 * NOTE: type_qualifier OR static only in func params
 * NOTE: "[" type_qualifier* "* "]" ONLY IN THE PARAMS OF A FUNCTION DECLARATION (NOT DEFINITION)
 */

void direct_declarator(
    Token **curr_tk,
    Type *ty,
    VarAttrs *attrs,
    bool is_func_params,
    bool is_func_def_params
)
{
    char* id;
    if (tk_ty(curr_tk) == TK_LPAREN) {
        next_tk(curr_tk);
        declarator(curr_tk, ty, attrs, is_func_params);
        if (tk_ty(curr_tk) != TK_RPAREN) { /* problem */ }
        next_tk(curr_tk);
    } else if (tk_ty(curr_tk) == TK_ID) {
        id = (*curr_tk)->text.str;
        next_tk(curr_tk);
    }

    if (tk_ty(curr_tk) != TK_LBRACKET) {
        return;
    }

    // grab all type qualifiers or static

    while (tk_ty(curr_tk) == TK_STATIC
            || tk_ty(curr_tk) == TK_CONST
            || tk_ty(curr_tk) == TK_RESTRICT
            || tk_ty(curr_tk) == TK_VOLATILE
            || tk_ty(curr_tk) == TK__ATOMIC) {
        if (!is_func_params) {
            /* problem */
        }

        switch (tk_ty(curr_tk)) {
            case TK_STATIC:
                attrs->is_static = true;
                break;
            case TK_CONST:
                ty->is_const = true;
                break;
            case TK_RESTRICT:
                break;
            case TK_VOLATILE:
                break;
            case TK__ATOMIC:
                break;
            default:
                /* problem */
                return;
        }

        next_tk(curr_tk);
    }

    if (tk_ty(curr_tk) == TK_STAR) {
        // "*"
        if (!is_func_params || (is_func_params && is_func_def_params)) {
            /* problem */
        }

        // stuff -- I don't know what a "*" here does to type info

        // -----

        next_tk(curr_tk);
    } else if (tk_ty(curr_tk) == TK_RBRACKET) {
        // TODO: return
    } else {
        // assnt_expr
        // TODO
        assnt_expr(curr_tk);
        // ...

        if (tk_ty(curr_tk) != TK_RBRACKET) { /* problem */ }
        next_tk(curr_tk);
    }
}

/* declarator = pointers? direct_declarator
 */
void declarator(Token **curr_tk, Type* ty, VarAttrs *attrs, bool is_func_params)
{
}

/* abstract_declarator = pointers? direct_abstract_declarator
 */
void abstract_declarator()
{
}

/* direct_abstract_declarator = ( abstract_declarator )
 *                            | direct_abstract_declarator? "[" type_qualifier* assnt_expr? "]"
 *                            | direct_abstract_declarator? "[" "*" "]"
 *                            | direct_abstract_declarator? "[" static type-qualifier* assnt_expr "]"
 *                            | direct_abstract_declarator? "[" type_qualifier+ static assnt_expr "]"
 */
void direct_abstract_declarator()
{
}

/* func_def = decl_specs? func_declarator var_decl* compound_stmt
 *
 * NOTE: the decl* is legacy--no idea what to do with it
 * NOTE: extra care and checks needed--grammar not sufficient to parse
 */
Obj *func_def(Token **curr_tk)
{
    VarAttrs *attrs;
    Type *ty = decl_specs(curr_tk, attrs, true);

    Obj *func = malloc(sizeof(Obj));
    func->attrs = attrs;
    func->ty = ty;

    func_declarator(curr_tk, func);

    return NULL;
}

/* designation = ("[" const_expr "]" | "." TK_ID)+ "="
 */
void designation()
{
}

/* init_list = designation? init ("," designation? init)*
 */
void init_list(Token **curr_tk)
{
}

/* init = assnt_expr
 *      | "{" init_list (","?) "}"
 */
void init(Token **curr_tk)
{
}

/* init_declarator = declarator (= init)?
 *
 */
Obj *init_declarator(Token **curr_tk, Type *ty, VarAttrs *attrs, bool is_func_param)
{
    declarator(curr_tk, ty, attrs, false);
    return NULL;
}

/* init_declarator_list = init_declarator ("," init_declarator)*
 */
Obj *init_declarator_list(Token **curr_tk, int *var_cnt, Type *ty, VarAttrs *attrs)
{
    return NULL;
}

/* var_decl = decl_specs init_declarator_list? ;
 *      | static_assert_decl
 */
Obj *var_decl(Token **curr_tk)
{
    VarAttrs *attrs;
    Type *ty = decl_specs(curr_tk, attrs, false);
    if (!ty) { /* expected declaration specifiers */ }

    int var_cnt;
    init_declarator_list(curr_tk, &var_cnt, ty, attrs);
    return NULL;
}

/* program = (func_def | func_decl | var_decl)*
 */
void parse(Token **curr_tk)
{
    while ((*curr_tk)->ty != TK_EOF) {
        Obj *func = func_def(curr_tk);
        if (func) continue;

        Obj *var = var_decl(curr_tk);
        if (!var) { /* err */ }
    }
}
