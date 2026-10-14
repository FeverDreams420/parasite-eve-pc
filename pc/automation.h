#ifndef PE_PC_AUTOMATION_H
#define PE_PC_AUTOMATION_H

#include <stdint.h>

/* Events and observer callbacks run on the application's main thread. Values
 * are semantic state (selection, displayed frame), never host pointers. */
typedef void (*PeRuntimeObserver)(void *user, const char *event, int64_t value);
void pe_runtime_set_observer(PeRuntimeObserver observer, void *user);
void pe_runtime_set_headless(int enabled);
int pe_runtime_headless(void);
int pe_runtime_prepare(void);
int pe_runtime_load_replay(const char *path);
void pe_runtime_set_replay_timeout(uint64_t milliseconds);
int pe_runtime_open_trace(const char *path);
void pe_runtime_emit(const char *event, int64_t value);
int pe_runtime_should_stop(void);
const char *pe_runtime_error(void);
int pe_runtime_finish(int exit_code);

#endif
