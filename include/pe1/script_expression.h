#ifndef PE1_SCRIPT_EXPRESSION_H
#define PE1_SCRIPT_EXPRESSION_H

typedef struct ExprOpArgs {
    int *op;
    int *dst;
    int *lhs;
    int *rhs;
} ExprOpArgs;

int Script_EvalExpr(ExprOpArgs *args);

#endif
