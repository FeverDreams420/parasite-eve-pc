#include "pe1/script_expression.h"
#include "pe1/pad_state.h"
#include <stdint.h>
extern u32 g_FieldPadHeldFrameCounts[32];
int Script_DispatchCmd(int **args);
#include <limits.h>
#include <stdio.h>
static int evaluate(int op,int lhs,int rhs)
{
    int result=123456;
    int *args[]={&op,&result,&lhs,&rhs};
    if (Script_EvalExpr(args)!=1) return 0;
    return result;
}
int main(void)
{
    if (evaluate(0,INT_MAX,1)!=INT_MIN || evaluate(1,INT_MIN,1)!=INT_MAX ||
        evaluate(15,INT_MAX,2)!=-2 || evaluate(23,INT_MIN,0)!=INT_MIN) return 1;
    if (evaluate(17,1,33)!=2 || evaluate(18,-3,1)!=-2 || evaluate(18,INT_MIN,32)!=INT_MIN) return 2;
    if (evaluate(16,INT_MIN,-1)!=INT_MIN || evaluate(22,INT_MIN,-1)!=0 ||
        evaluate(16,-17,4)!=-4 || evaluate(22,-17,4)!=-1) return 3;
    if (evaluate(5,0,-1)!=1 || evaluate(6,0,1)!=0 || evaluate(7,0,1)!=1 ||
        evaluate(9,-1,0)!=0 || evaluate(13,-1,0)!=1 || evaluate(19,-123,0)!=-123) return 4;
    if (evaluate(20,0x10000,0x20000)!=0x20000 || evaluate(21,0x200,0x100)!=0x20000) return 5;
    if (evaluate(99,1,2)!=123456) return 6;
    /* Stack operands must retain their full native addresses. */
    int mode=3, mask=0x20, result=-1;
    int *args[]={&mode,&mask,&result};
    g_FieldPadHeldFrameCounts[5]=73;
    g_FieldPadBits=0x20;
    Script_DispatchCmd(args);
    if (result!=73 || mask!=26) return 7;
    mask=0x20; g_FieldPadBits=0;
    Script_DispatchCmd(args);
    if (result!=0 || mask!=0x20) return 8;
    mask=INT_MIN; g_FieldPadBits=0x80000000u;
    g_FieldPadHeldFrameCounts[31]=91;
    Script_DispatchCmd(args);
    if (result!=91 || mask!=INT_MIN) return 9;
    puts("Original expression opcodes preserve 32-bit arithmetic and signed comparisons");
    return 0;
}
