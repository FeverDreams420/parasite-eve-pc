#ifndef PE_PC_INPUT_H
#define PE_PC_INPUT_H
#include <stddef.h>
#include <stdint.h>
typedef struct PeInput { uint32_t held,pressed,released; } PeInput;
void pe_input_update(PeInput *input,uint32_t held);
typedef void (*PeInputCallback)(void *context,int *mode);
typedef struct PeInputRule { uint32_t flags,all,any;PeInputCallback callback; } PeInputRule;
typedef struct PeInputRules { const PeInputRule *records;size_t count; } PeInputRules;
/* Original ordered Scene_CheckFlagBits. A zero-flags record is the fallback;
 * a null callback stops traversal. Validate the bounded table before calls.
 * Callback mode changes do not redirect the current traversal. */
int pe_input_rules(const PeInput *input,void *context,const PeInputRules *table,unsigned count,int *mode);
#endif
