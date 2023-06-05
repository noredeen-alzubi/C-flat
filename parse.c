#include <stdio.h>
#include "cflat.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define RDPARSE_WRAP(body) Token *tmp_tk_ptr = *curr_tk; \
                           body \
                           *curr_tk = tmp_tk_ptr


typedef struct Scope Scope;
struct Scope {
    struct { char *key; Obj *value; } *vars; // symbol table
    Scope *next;
};

void declarator(Token **curr_tk, Type* ty, VarAttrs *attrs, bool is_func_params);
void direct_declarator(
    Token **curr_tk,
    Type *ty,
    VarAttrs *attrs,
    bool is_func_params,
    bool is_func_def_params
);
Expr *exprs(Token **curr_tk);
Expr *assnt_expr(Token **curr_tk);

Scope *scope_stack_head;

// utils

static void *safe_calloc(size_t n_items, size_t n_bytes)
{
    void *ptr = calloc(n_items, n_bytes);
    if (!ptr) {
        fprintf(stderr, "internal err: out of memeory");
        exit(1);
    }
    return ptr;
}


inline Scope *new_scope()
{

    Scope *res = safe_calloc(1, sizeof(Scope));
    res->vars = NULL;
    return res;
}

inline Token *next_tk(Token **curr_tk) {
    if ((*curr_tk)->next == NULL) {
        fprintf(stderr, "internal err: called next_tk when next is NULL\n");
        exit(1);
    }

    Token *temp = *curr_tk;
    *curr_tk = (*curr_tk)->next;
    return temp;
}

inline TokenType tk_ty(Token **curr_tk)
{
    if (*curr_tk == NULL) {
        fprintf(stderr, "internal err: called tk_ty on NULL\n");
        exit(1);
    }

    return (*curr_tk)->ty;
}

// parsing

/* primary_expr = TK_ID
 *              | TK_NUM
 *              | TK_CHAR
 *              | TK_STR
 *              | "(" exprs ")"
 * LATER        | generic_selection
 *
 * NOTE: generic_selection for preprocesser I think
 * NOTE: we treat TK_STR like a VarRef (to a compiler-generated variable)
 */
Expr *primary_expr(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    Expr *res;
    switch (tk_ty(&tmp_tk_ptr)) {
        case TK_ID:
        {
            // TODO: this shouldn't be in parser
            // Scope *top_scope = scope_stack_head;
            // Obj *ref = NULL;
            // while(top_scope) {
            //     ref = hmget(top_scope->vars, tmp_tk_ptr->s_value.str);
            //     if (ref) break;
            //     top_scope = top_scope->next;
            // }
            // if (!ref) return NULL; // undefined var/func identifier
            res = safe_calloc(1, sizeof(Expr));
            res->var_ref = safe_calloc(1, sizeof(VarRef));
            // res->var_ref->var = ref;
            res->var_ref->id = tmp_tk_ptr->text;
            break;
        }
        case TK_STR:
            // TODO: create Obj and a VarRef to it
            break;
        case TK_NUM: case TK_CHAR:
            res = safe_calloc(1, sizeof(Expr));
            res->val = tmp_tk_ptr->i_value;
            break;
        case TK_LPAREN:
            next_tk(&tmp_tk_ptr);
            Expr *first_expr = exprs(&tmp_tk_ptr);
            if (!first_expr) { return NULL; } /* problem: expected expressions */
            next_tk(&tmp_tk_ptr);
            if (tk_ty(&tmp_tk_ptr) != TK_RPAREN) return NULL;
            res = first_expr;
            break;
        default:
            break;
    }

    next_tk(&tmp_tk_ptr);

    *curr_tk = tmp_tk_ptr;
    return res;
}

/* type_qualifier TODO
 */
Type *type_qualifier(Token **curr_tk, VarAttrs *attrs)
{
    Token *tmp_tk_ptr = *curr_tk;

    while (tk_ty(&tmp_tk_ptr) == TK_CONST
            || tk_ty(&tmp_tk_ptr) == TK_RESTRICT
            || tk_ty(&tmp_tk_ptr) == TK_VOLATILE
            || tk_ty(&tmp_tk_ptr) == TK__ATOMIC) {

        switch (tk_ty(&tmp_tk_ptr)) {
            case TK_STATIC:
                attrs->is_static = true;
                break;
            case TK_CONST:
                // TODO
                break;
            case TK_RESTRICT:
                // TODO
                break;
            case TK_VOLATILE:
                // TODO
                break;
            case TK__ATOMIC:
                // TODO
                break;
            default:
                /* problem */
                return NULL;
        }

        next_tk(&tmp_tk_ptr);
    }

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* type_spec = TK_VOID | TK_CHAR | TK_SHORT | TK_INT
 *           | TK_LONG | TK_FLOAT | TK_DOUBLE | TK_SIGNED
 *           | TK_UNSIGNED | TK__BOOL | TK__COMPLEX
 *           | _Atomic "(" type_name ")"
 */
Type *type_spec()
{
    return NULL;
}

Type *type_name(Token **curr_tk, VarAttrs *attrs)
{
    return NULL;
}

/* compound_literal = "(" type_name ")" "{" init_list ","? "}"
 * TODO: finish this
 */
Expr *compound_literal(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    Expr *result;

    if (tk_ty(curr_tk) != TK_LPAREN) {
        return NULL;
    }

    next_tk(&tmp_tk_ptr);

    VarAttrs *attrs = safe_calloc(1, sizeof(VarAttrs));
    Type *ty = type_name(&tmp_tk_ptr, attrs);

    if (!ty) return NULL;

    next_tk(&tmp_tk_ptr);
    if (tk_ty(&tmp_tk_ptr) != TK_RPAREN) return NULL;

    next_tk(&tmp_tk_ptr);
    if (tk_ty(&tmp_tk_ptr) != TK_LBRACE) return NULL;

    // TODO: continue

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* expr = assnt_expr ("," assnt_expr)*
 */
Expr *expr(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    Expr *first = assnt_expr(&tmp_tk_ptr);
    if (!first) return NULL;

    Expr *curr = first;
    while (tk_ty(&tmp_tk_ptr) == TK_COMMA) {
        next_tk(&tmp_tk_ptr);
        Expr *next = assnt_expr(&tmp_tk_ptr);
        if (!next) return NULL;
        curr->next = next;
        curr = next;
    }

    *curr_tk = tmp_tk_ptr;
    return first;
}

/* arg_expr_list = assnt_expr ("," assnt_expr)*
 */
Expr *arg_expr_list(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    Expr *first = assnt_expr(&tmp_tk_ptr);
    if (!first) return NULL;

    Expr *curr = first;
    while (tk_ty(&tmp_tk_ptr) == TK_COMMA) {
        next_tk(&tmp_tk_ptr);
        Expr *next = assnt_expr(&tmp_tk_ptr);
        if (!next) return NULL;

        curr->next = next;
        curr = next;
    }

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* postfix_expr = primary_expr
 *              | compound_literal
 *              | postfix_expr "[" expr "]"
 *              | postfix_expr "(" ( assnt_expr ("," assnt_expr)* )? ")"
 *              | postfix_expr ("." || "->") TK_ID
 *              | postfix_expr ("++" | "--")
 *
 * postfix_expr = (primary_expr | compound_literal) ()* ("++" | "--")?
 *
 */
Expr *postfix_expr(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    Expr *curr = compound_literal(&tmp_tk_ptr);
    if (!curr) {
        curr = primary_expr(&tmp_tk_ptr);
        if (!curr) return NULL;
    }

    while (tk_ty(&tmp_tk_ptr) == TK_LBRACE ||
           tk_ty(&tmp_tk_ptr) == TK_LPAREN ||
           tk_ty(&tmp_tk_ptr) == TK_DEREF ||
           tk_ty(&tmp_tk_ptr) == TK_DOT)
    {
        Expr *new = safe_calloc(1, sizeof(Expr));
        new->l_operand = curr;
        curr = new;

        switch (tk_ty(&tmp_tk_ptr)) {
            case TK_LBRACE:
            {
                next_tk(&tmp_tk_ptr);
                Expr *index_expr = expr(&tmp_tk_ptr);
                next_tk(&tmp_tk_ptr);
                if (tk_ty(&tmp_tk_ptr) != TK_RBRACE) {
                    return NULL;
                }

                // TODO: set operator
                curr->r_operand = index_expr;

                if (tk_ty(&tmp_tk_ptr) != TK_RBRACE) return NULL;
                break;
            }
            case TK_LPAREN:
            {
                next_tk(&tmp_tk_ptr);
                Expr *first_arg = arg_expr_list(&tmp_tk_ptr);

                // TODO: set expr type
                curr->func_invok = safe_calloc(1, sizeof(FuncInvok));
                curr->func_invok->first_arg = first_arg;

                if (tk_ty(&tmp_tk_ptr) != TK_RPAREN) return NULL;
            }
            case TK_DEREF:
            case TK_DOT:
            {
                next_tk(&tmp_tk_ptr);
                if (tk_ty(&tmp_tk_ptr) != TK_ID) return NULL;

                // TODO: set operator
                Member *struct_members = curr->l_operand->ty->members;
                curr->r_operand = safe_calloc(1, sizeof(Expr));
                curr->r_operand->var_ref = safe_calloc(1, sizeof(VarRef));
                curr->r_operand->var_ref->id = tmp_tk_ptr->text;

                break;
            }
            default:
                return NULL;
        }
    }

    next_tk(&tmp_tk_ptr);

    *curr_tk = tmp_tk_ptr;
    return curr;
}

/* cast_expr = ("(" type_name ")")* unary_expr
 */
Expr *cast_expr(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* unary_expr = postfix_expr
 *            | ("++" | "--") unary_expr
 *            | ("&" | "*" | "+" | "-" | "~" | "!") cast_expr
 *            | TK_SIZEOF unary_expr
 *            | (TK_SIZEOF | TK__ALIGNOF) "(" type_name ")"
 */
Expr *unary_expr(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* mul_expr = cast_expr (("*" | "/" | "%") cast_expr)*
 */
Expr *mul_expr(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* add_expr = mul_expr (("+" | "-") mul_expr)*
 */
Expr *add_expr(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* shift_expr = add_expr (("<<" | ">>") add_expr)*
 */
Expr *shift_expr(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* cond_expr = logical_or_expr ("?" exprs ":" cond_expr)?
 */
Expr *cond_expr(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* assnt_expr = cond_expr
 *            | unary_expr ("="|"*="|"/="|"%="|"+="|"-="|"<<="|">>="|"&="|"^="|"|=")
 *              assnt_expr
 */
Expr *assnt_expr(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* exprs = assnt_expr ("," assnt_expr)*
 */
Expr *exprs(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    Expr *first_expr = assnt_expr(&tmp_tk_ptr);
    if (!first_expr) {
        return NULL;
    }

    next_tk(&tmp_tk_ptr);
    Expr *curr_expr = first_expr;
    while (tk_ty(&tmp_tk_ptr) == TK_COMMA) {
        next_tk(&tmp_tk_ptr);
        Expr *expr = assnt_expr(&tmp_tk_ptr);
        if (!expr) {
            return NULL;
        }
        curr_expr->next = expr;
        curr_expr = expr;
    }

    *curr_tk = tmp_tk_ptr;
    return first_expr;
}

/* decl_specs = ( TK_TYPEDEF | TK_EXTERN | TK_STATIC |
 *               | TK_AUTO | TK_VOID | TK_CHAR | TK_SHORT
 *               | TK_INT | TK_LONG | TK_FLOAT | TK_DOUBLE
 *               | TK_SIGNED | TK_UNSIGNED | TK__BOOL
 *               |  TK_CONST | TK_RESTRICT | TK_VOLATILE
 *               | TK__Atomic | TK_INLINE | TK__NORETURN
 *               | atomic_type_spec
 *               | struct_or_union_spec
 *               | enum_spec
 *               | align_spec
 *               | TK_ID )+
 *
 * NOTE: at most one storage_class_spec is allowed
 *     (EXCEPT: TK__THR_LOC with TK_STATIC OR TK_EXTERN)
 *
 * IGNORE: NOTE: TK__THR_LOC can't be used in func decls OR defs
 * IGNORE: TK__THR_LOC, TK_AUTO, TK_REGISTER
 *     all carry recursively to members of struct or union
 */
Type *decl_specs(Token **curr_tk, VarAttrs *attrs, bool is_func_def, bool is_param_decl)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

// TODO: check this out looks complicated
/* pointers = '*' type_qualifier* pointers?
 */
int pointers(Token **curr_tk, Type *ty, VarAttrs *attrs)
{
    Token *tmp_tk_ptr = *curr_tk;

    int ptr_cnt;
    while (tk_ty(&tmp_tk_ptr) == TK_STAR) {
        next_tk(curr_tk);
        while (tk_ty(&tmp_tk_ptr) == TK_CONST
                || tk_ty(&tmp_tk_ptr) == TK_RESTRICT
                || tk_ty(&tmp_tk_ptr) == TK_VOLATILE
                || tk_ty(&tmp_tk_ptr) == TK__ATOMIC) {

            switch (tk_ty(&tmp_tk_ptr)) {
                case TK_STATIC:
                    attrs->is_static = true;
                    break;
                case TK_CONST:
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

            next_tk(&tmp_tk_ptr);
        }
    }

    *curr_tk = tmp_tk_ptr;
    return ptr_cnt;
}

/* param_decl = decl_specs (declarator | abstract_declarator)
 */
Obj *param_decl(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    VarAttrs *attrs;
    Type *ty = decl_specs(curr_tk, attrs, false, true);
    Obj *var = safe_calloc(1, sizeof(Obj));
    var->ty = ty;
    var->attrs = attrs;

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* param_t_list = param_decl ("," param_decl)* (, "...")?
 */
void param_t_list(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return;
}

/* func_id = TK_ID
 *         | "(" func_id ")"
 */
char* func_id(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;
    return NULL;
}

/* func_params = "(" param_t_list? ")"
 *             | "(" id_list? ")"
 */
Obj *func_params(Token **curr_tk, int *param_cnt)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* func_declarator = pointers? func_id func_params
 */
Obj *func_declarator(Token **curr_tk, Type *ty, VarAttrs *attrs)
{
    Token *tmp_tk_ptr = *curr_tk;

    pointers(&tmp_tk_ptr, ty, attrs);

    char* id = func_id(&tmp_tk_ptr);
    if (id == NULL) { /* problem */ }

    int param_cnt;
    func_params(&tmp_tk_ptr, &param_cnt);

    // TODO: make func and fill in things

    *curr_tk = tmp_tk_ptr;
    return NULL;
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
    Token *tmp_tk_ptr = *curr_tk;

    char* id;
    if (tk_ty(&tmp_tk_ptr) == TK_LPAREN) {
        next_tk(&tmp_tk_ptr);
        declarator(&tmp_tk_ptr, ty, attrs, is_func_params);
        if (tk_ty(&tmp_tk_ptr) != TK_RPAREN) { /* problem */ }
        next_tk(&tmp_tk_ptr);
    } else if (tk_ty(&tmp_tk_ptr) == TK_ID) {
        id = tmp_tk_ptr->text.str;
        next_tk(&tmp_tk_ptr);
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

    *curr_tk = tmp_tk_ptr;
    return;
}

/* declarator = pointers? direct_declarator
 */
void declarator(Token **curr_tk, Type* ty, VarAttrs *attrs, bool is_func_params)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return;
}

/* abstract_declarator = pointers? direct_abstract_declarator
 */
void abstract_declarator(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return;
}

/* direct_abstract_declarator = ( abstract_declarator )
 *                            | direct_abstract_declarator? "[" type_qualifier* assnt_expr? "]"
 *                            | direct_abstract_declarator? "[" "*" "]"
 *                            | direct_abstract_declarator? "[" static type-qualifier* assnt_expr "]"
 *                            | direct_abstract_declarator? "[" type_qualifier+ static assnt_expr "]"
 */
void direct_abstract_declarator(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return;
}

/* designation = ("[" const_expr "]" | "." TK_ID)+ "="
 */
void designation(Token **curr_tk)
{
    while (tk_ty(curr_tk) == TK_LBRACE || tk_ty(curr_tk) == TK_DOT) {
        if (tk_ty(curr_tk) == TK_LBRACE) {
            cond_expr(curr_tk);
        } else {
        }
    }
}

/* init_list = designation? init ("," designation? init)*
 * TODO: verify this grammar
 */
void init_list(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return;
}

/* init = assnt_expr
 *      | "{" init_list (","?) "}"
 */
void init(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return;
}

/* init_declarator = declarator (= init)?
 *
 */
Obj *init_declarator(Token **curr_tk, Type *ty, VarAttrs *attrs, bool is_func_param)
{
    Token *tmp_tk_ptr = *curr_tk;

    declarator(&tmp_tk_ptr, ty, attrs, false);

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* init_declarator_list = init_declarator ("," init_declarator)*
 */
Obj *init_declarator_list(Token **curr_tk, int *var_cnt, Type *ty, VarAttrs *attrs)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* var_decl = decl_specs init_declarator_list? ;
 *      | static_assert_decl
 */
Obj *var_decl(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    VarAttrs *attrs;
    Type *ty = decl_specs(&tmp_tk_ptr, attrs, false);
    if (!ty) { /* problem: expected declaration specifiers */ }

    int var_cnt;
    init_declarator_list(&tmp_tk_ptr, &var_cnt, ty, attrs);

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* stmt = labeled_stmt
 *      | compound_stmt
 *      | expr_stmt
 *      | select_stmt
 *      | iter_stmt
 *      | jmp_stmt
 */
BlockItem *stmt(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* compound_stmt = "{" (var_decl | stmt)* "}"
 */
BlockItem *compound_stmt(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    *curr_tk = tmp_tk_ptr;
    return NULL;
}

/* func_def = decl_specs? func_declarator var_decl* compound_stmt
 *
 * NOTE: the decl* is legacy--no idea what to do with it
 * NOTE: extra care and checks needed--grammar not sufficient to parse
 */
Obj *func_def(Token **curr_tk)
{
    Token *tmp_tk_ptr = *curr_tk;

    VarAttrs *attrs;
    Type *ret_ty = decl_specs(&tmp_tk_ptr, attrs, true);
    if (!ret_ty) {
        // TODO: don't make new Type objects for funcs with no explicit ret type
        ret_ty = safe_calloc(1, sizeof(Type));
        ret_ty->kind = TY_INT;
    }

    Type *func_ty = safe_calloc(1, sizeof(Type));
    func_ty->kind = TY_FUNC;
    func_ty->ret_ty = ret_ty;

    Obj *func = func_declarator(&tmp_tk_ptr, func_ty, attrs);

    // after this point it's a func definition and not a declaration

    while (var_decl(&tmp_tk_ptr)); // consume but ignore (unsupported)

    BlockItem *block_items = compound_stmt(&tmp_tk_ptr);

    *curr_tk = tmp_tk_ptr;
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
